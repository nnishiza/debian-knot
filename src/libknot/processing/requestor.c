/*  Copyright (C) 2014 CZ.NIC, z.s.p.o. <knot-dns@labs.nic.cz>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <assert.h>
#include <sys/fcntl.h>

#include "libknot/processing/requestor.h"
#include "libknot/errcode.h"
#include "libknot/internal/macros.h"
#include "libknot/internal/net.h"

static bool use_tcp(struct knot_request *request)
{
	return !(request->flags & KNOT_RQ_UDP);
}

static struct knot_request *request_make(mm_ctx_t *mm)
{
	struct knot_request *request =
	                mm_alloc(mm, sizeof(struct knot_request));
	if (request == NULL) {
		return NULL;
	}

	memset(request, 0, sizeof(struct knot_request));

	return request;
}

/*! \brief Wait for socket readiness. */
static int request_wait(int fd, int state, struct timeval *timeout)
{
	fd_set set;
	FD_ZERO(&set);
	FD_SET(fd, &set);

	switch(state) {
	case KNOT_NS_PROC_FULL: /* Wait for writeability. */
		return select(fd + 1, NULL, &set, NULL, timeout);
	case KNOT_NS_PROC_MORE: /* Wait for data. */
		return select(fd + 1, &set, NULL, NULL, timeout);
	default:
		return -1;
	}
}

static int request_send(struct knot_request *request,
                        const struct timeval *timeout)
{
	/* Each request has unique timeout. */
	struct timeval tv = { timeout->tv_sec, timeout->tv_usec };

	/* Wait for writeability or error. */
	int ret = request_wait(request->fd, KNOT_NS_PROC_FULL, &tv);
	if (ret == 0) {
		return KNOT_ETIMEOUT;
	}

	/* Check socket error. */
	int err = 0;
	socklen_t len = sizeof(int);
	getsockopt(request->fd, SOL_SOCKET, SO_ERROR, &err, &len);
	if (err != 0) {
		return KNOT_ECONNREFUSED;
	}

	/* Send query, construct if not exists. */
	knot_pkt_t *query = request->query;
	uint8_t *wire = query->wire;
	uint16_t wire_len = query->size;

	/* Send query. */
	if (use_tcp(request)) {
		ret = tcp_send_msg(request->fd, wire, wire_len);
	} else {
		ret = udp_send_msg(request->fd, wire, wire_len,
		                   (const struct sockaddr *)&request->remote);
	}
	if (ret != wire_len) {
		return KNOT_ECONN;
	}

	return KNOT_EOK;
}

static int request_recv(struct knot_request *request,
                        const struct timeval *timeout)
{
	int ret = 0;
	knot_pkt_t *resp = request->resp;
	knot_pkt_clear(resp);

	/* Each request has unique timeout. */
	struct timeval tv = { timeout->tv_sec, timeout->tv_usec };

	/* Receive it */
	if (use_tcp(request)) {
		ret = tcp_recv_msg(request->fd, resp->wire, resp->max_size, &tv);
	} else {
		ret = udp_recv_msg(request->fd, resp->wire, resp->max_size,
		                   (struct sockaddr *)&request->remote);
	}
	if (ret < 0) {
		resp->size = 0;
		return ret;
	}

	resp->size = ret;
	return ret;
}

_public_
struct knot_request *knot_request_make(mm_ctx_t *mm,
                                       const struct sockaddr *dst,
                                       const struct sockaddr *src,
                                       knot_pkt_t *query,
                                       unsigned flags)
{
	if (dst == NULL || query == NULL) {
		return NULL;
	}

	/* Form a pending request. */
	struct knot_request *request = request_make(mm);
	if (request == NULL) {
		return NULL;
	}

	memcpy(&request->remote, dst, sockaddr_len(dst));
	if (src) {
		memcpy(&request->origin, src, sockaddr_len(src));
	}

	request->fd = -1;
	request->query = query;
	request->resp  = NULL;
	request->flags = flags;
	return request;
}

_public_
int knot_request_free(mm_ctx_t *mm, struct knot_request *request)
{
	close(request->fd);
	knot_pkt_free(&request->query);
	knot_pkt_free(&request->resp);

	rem_node(&request->node);
	mm_free(mm, request);

	return KNOT_EOK;
}

_public_
void knot_requestor_init(struct knot_requestor *requestor, mm_ctx_t *mm)
{
	memset(requestor, 0, sizeof(struct knot_requestor));
	requestor->mm = mm;
	init_list(&requestor->pending);
	knot_overlay_init(&requestor->overlay, mm);
}

_public_
void knot_requestor_clear(struct knot_requestor *requestor)
{
	while (knot_requestor_dequeue(requestor) == KNOT_EOK)
		;

	knot_overlay_finish(&requestor->overlay);
	knot_overlay_deinit(&requestor->overlay);
}

_public_
bool knot_requestor_finished(struct knot_requestor *requestor)
{
	return requestor == NULL || EMPTY_LIST(requestor->pending);
}

_public_
int knot_requestor_overlay(struct knot_requestor *requestor,
                           const knot_layer_api_t *proc, void *param)
{
	return knot_overlay_add(&requestor->overlay, proc, param);
}

_public_
int knot_requestor_enqueue(struct knot_requestor *requestor,
                           struct knot_request *request)
{
	if (requestor == NULL || request == NULL) {
		return KNOT_EINVAL;
	}

	/* Determine comm protocol. */
	int sock_type = SOCK_DGRAM;
	if (use_tcp(request)) {
		sock_type = SOCK_STREAM;
	}

	/* Fetch a bound socket. */
	request->fd = net_connected_socket(sock_type, &request->remote,
	                                   &request->origin, O_NONBLOCK);
	if (request->fd < 0) {
		return KNOT_ECONN;
	}

	/* Prepare response buffers. */
	request->resp  = knot_pkt_new(NULL, KNOT_WIRE_MAX_PKTSIZE, requestor->mm);
	if (request->resp == NULL) {
		mm_free(requestor->mm, request);
		return KNOT_ENOMEM;
	}

	add_tail(&requestor->pending, &request->node);

	return KNOT_EOK;
}

_public_
int knot_requestor_dequeue(struct knot_requestor *requestor)
{
	if (knot_requestor_finished(requestor)) {
		return KNOT_ENOENT;
	}

	struct knot_request *last = HEAD(requestor->pending);
	return knot_request_free(requestor->mm, last);
}

static int request_io(struct knot_requestor *req, struct knot_request *last,
                      struct timeval *timeout)
{
	int ret = KNOT_EOK;
	knot_pkt_t *query = last->query;
	knot_pkt_t *resp = last->resp;

	/* Data to be sent. */
	if (req->overlay.state == KNOT_NS_PROC_FULL) {

		/* Process query and send it out. */
		knot_overlay_out(&req->overlay, query);

		ret = request_send(last, timeout);
		if (ret != KNOT_EOK) {
			return ret;
		}
	}

	/* Data to be read. */
	if (req->overlay.state == KNOT_NS_PROC_MORE) {
		/* Read answer and process it. */
		ret = request_recv(last, timeout);
		if (ret < 0) {
			return ret;
		}

		knot_overlay_in(&req->overlay, resp);
	}

	return KNOT_EOK;
}

static int exec_request(struct knot_requestor *req, struct knot_request *last,
                        struct timeval *timeout)
{
	int ret = KNOT_EOK;

	/* Do I/O until the processing is satisifed or fails. */
	while (req->overlay.state & (KNOT_NS_PROC_FULL|KNOT_NS_PROC_MORE)) {
		ret = request_io(req, last, timeout);
		if (ret != KNOT_EOK) {
			knot_overlay_reset(&req->overlay);
			return ret;
		}
	}

	/* Expect complete request. */
	if (req->overlay.state == KNOT_NS_PROC_FAIL) {
		ret = KNOT_ERROR;
	}

	/* Finish current query processing. */
	knot_overlay_reset(&req->overlay);

	return ret;
}

_public_
int knot_requestor_exec(struct knot_requestor *requestor,
                        struct timeval *timeout)
{
	if (knot_requestor_finished(requestor)) {
		return KNOT_ENOENT;
	}

	/* Execute next request. */
	int ret = exec_request(requestor, HEAD(requestor->pending), timeout);

	/* Remove it from processing. */
	knot_requestor_dequeue(requestor);

	return ret;
}

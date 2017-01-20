/*  Copyright (C) 2017 CZ.NIC, z.s.p.o. <knot-dns@labs.nic.cz>

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

#pragma once

#include "knot/query/layer.h"
#include "knot/server/server.h"
#include "knot/updates/acl.h"
#include "contrib/sockaddr.h"

/*!
 * \brief DNS query types.
 *
 * This type encompasses the different query types distinguished by both the
 * OPCODE and the QTYPE.
 */
typedef enum {
	KNOT_QUERY_INVALID   =      0, /*!< Invalid query. */
	KNOT_QUERY_NORMAL    = 1 << 1, /*!< Normal query. */
	KNOT_QUERY_AXFR      = 1 << 2, /*!< Request for AXFR transfer. */
	KNOT_QUERY_IXFR      = 1 << 3, /*!< Request for IXFR transfer. */
	KNOT_QUERY_NOTIFY    = 1 << 4, /*!< NOTIFY query. */
	KNOT_QUERY_UPDATE    = 1 << 5, /*!< Dynamic update. */
	KNOT_RESPONSE        = 1 << 0, /*!< Is response. */
	KNOT_RESPONSE_NORMAL = KNOT_RESPONSE|KNOT_QUERY_NORMAL,/*!< Normal response. */
	KNOT_RESPONSE_AXFR   = KNOT_RESPONSE|KNOT_QUERY_AXFR,  /*!< AXFR transfer response. */
	KNOT_RESPONSE_IXFR   = KNOT_RESPONSE|KNOT_QUERY_IXFR,  /*!< IXFR transfer response. */
	KNOT_RESPONSE_NOTIFY = KNOT_RESPONSE|KNOT_QUERY_NOTIFY,/*!< NOTIFY response. */
	KNOT_RESPONSE_UPDATE = KNOT_RESPONSE|KNOT_QUERY_UPDATE /*!< Dynamic update response. */
} pkt_type_t;

/* Query processing module implementation. */
const knot_layer_api_t *process_query_layer(void);

/* Query processing specific flags. */
enum process_query_flag {
	NS_QUERY_NO_AXFR    = 1 << 0, /* Don't process AXFR */
	NS_QUERY_NO_IXFR    = 1 << 1, /* Don't process IXFR */
	NS_QUERY_LIMIT_ANY  = 1 << 2, /* Limit ANY QTYPE (respond with TC=1) */
	NS_QUERY_LIMIT_SIZE = 1 << 3  /* Apply UDP size limit. */
};

/* Module load parameters. */
struct process_query_param {
	uint16_t   proc_flags;
	server_t   *server;
	int        socket;
	const struct sockaddr_storage *remote;
	unsigned   thread_id;
};

/*! \brief Query processing intermediate data. */
struct query_data {
	uint16_t rcode;       /*!< Resulting RCODE (Whole extended RCODE). */
	uint16_t rcode_tsig;  /*!< Resulting TSIG RCODE. */
	uint16_t packet_type; /*!< Resolved packet type. */
	knot_pkt_t *query;    /*!< Query to be solved. */
	const zone_t *zone;   /*!< Zone from which is answered. */
	list_t wildcards;     /*!< Visited wildcards. */
	list_t rrsigs;        /*!< Section RRSIGs. */
	bool err_truncated;   /*!< Set TC bit if error reply. */

	/* Current processed name and nodes. */
	const zone_node_t *node, *encloser, *previous;
	const knot_dname_t *name;

	/* Original QNAME case. */
	uint8_t orig_qname[KNOT_DNAME_MAXLEN];

	/* EDNS */
	knot_rrset_t opt_rr;
	uint8_t *opt_rr_pos;  /*!< Place of the OPT RR in wire. */

	/* Extensions. */
	void *ext;
	void (*ext_cleanup)(struct query_data*); /*!< Extensions cleanup callback. */
	knot_sign_context_t sign;            /*!< Signing context. */

	/* Everything below should be kept on reset. */
	struct process_query_param *param; /*!< Module parameters. */
	knot_mm_t *mm;                     /*!< Memory context. */
};

/*! \brief Visited wildcard node list. */
struct wildcard_hit {
	node_t n;
	const zone_node_t *node;   /* Visited node. */
	const zone_node_t *prev;   /* Previous node from the SNAME. */
	const knot_dname_t *sname; /* Name leading to this node. */
};

/*! \brief RRSIG info node list. */
struct rrsig_info {
	node_t n;
	knot_rrset_t synth_rrsig;  /* Synthesized RRSIG. */
	knot_rrinfo_t *rrinfo;      /* RR info. */
};

/*!
 * \brief Check current query against ACL.
 *
 * \param conf       Configuration.
 * \param zone_name  Current zone name.
 * \param action     ACL action.
 * \param qdata      Query data.
 * \return true if accepted, false if denied.
 */
bool process_query_acl_check(conf_t *conf, const knot_dname_t *zone_name,
                             acl_action_t action, struct query_data *qdata);

/*!
 * \brief Verify current query transaction security and update query data.
 *
 * \param qdata
 * \retval KNOT_EOK
 * \retval KNOT_TSIG_EBADKEY
 * \retval KNOT_TSIG_EBADSIG
 * \retval KNOT_TSIG_EBADTIME
 * \retval (other generic errors)
 */
int process_query_verify(struct query_data *qdata);

/*!
 * \brief Sign current query using configured TSIG keys.
 *
 * \param pkt    Outgoing message.
 * \param qdata  Query data.
 *
 * \retval KNOT_E*
 */
int process_query_sign_response(knot_pkt_t *pkt, struct query_data *qdata);

/*!
 * \brief Restore QNAME letter case.
 *
 * \param pkt    Incoming message.
 * \param qdata  Query data.
 */
void process_query_qname_case_restore(knot_pkt_t *pkt, struct query_data *qdata);

/*!
 * \brief Convert QNAME to lowercase format for processing.
 *
 * \param pkt    Incoming message.
 */
int process_query_qname_case_lower(knot_pkt_t *pkt);

/*!
 * \brief Puts RRSet to packet, will store its RRSIG for later use.
 *
 * \param pkt         Packet to store RRSet into.
 * \param qdata       Query data structure.
 * \param rr          RRSet to be stored.
 * \param rrsigs      RRSIGs to be stored.
 * \param compr_hint  Compression hint.
 * \param flags       Flags.
 *
 * \return KNOT_E*
 */
int process_query_put_rr(knot_pkt_t *pkt, struct query_data *qdata,
                         const knot_rrset_t *rr, const knot_rrset_t *rrsigs,
                         uint16_t compr_hint, uint32_t flags);

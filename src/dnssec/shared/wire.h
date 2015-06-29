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

#pragma once

#include <arpa/inet.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>

#include "bignum.h"
#include "binary.h"
#include "shared.h"

typedef struct wire_ctx {
	uint8_t *wire;
	size_t size;
	uint8_t *position;
} wire_ctx_t;

static inline wire_ctx_t wire_init(uint8_t *data, size_t size)
{
	assert(data);

	wire_ctx_t result = {
		.wire = data,
		.size = size,
		.position = data
	};
	return result;
}

static inline wire_ctx_t wire_init_binary(const dnssec_binary_t *binary)
{
	assert(binary);

	return wire_init(binary->data, binary->size);
}

static inline void wire_seek(wire_ctx_t *ctx, size_t offset)
{
	assert(ctx);

	ctx->position = ctx->wire + offset;
}

static inline size_t wire_tell(wire_ctx_t *ctx)
{
	assert(ctx);

	return ctx->position - ctx->wire;
}

static inline size_t wire_available(wire_ctx_t *ctx)
{
	assert(ctx);

	size_t position = wire_tell(ctx);

	return ctx->size > position ? (ctx->size - position) : 0;
}

static inline uint8_t wire_read_u8(wire_ctx_t *ctx)
{
	assert(ctx);

	uint8_t result = *ctx->position;
	ctx->position += sizeof(uint8_t);

	return result;
}

static inline uint16_t wire_read_u16(wire_ctx_t *ctx)
{
	assert(ctx);

	uint16_t result = *((uint16_t *)ctx->position);
	ctx->position += sizeof(uint16_t);

	return ntohs(result);
}

static inline void wire_read(wire_ctx_t *ctx, uint8_t *data, size_t size)
{
	assert(ctx);
	assert(data);

	memmove(data, ctx->position, size);
	ctx->position += size;
}

static inline void wire_read_binary(wire_ctx_t *ctx, dnssec_binary_t *data)
{
	assert(data);

	wire_read(ctx, data->data, data->size);
}

static inline void wire_available_binary(wire_ctx_t *ctx, dnssec_binary_t *data)
{
	assert(ctx);
	assert(data);

	data->data = ctx->position;
	data->size = wire_available(ctx);
}

static inline void wire_write_u8(wire_ctx_t *ctx, uint8_t value)
{
	assert(ctx);

	*ctx->position = value;
	ctx->position += sizeof(uint8_t);
}

static inline void wire_write_u16(wire_ctx_t *ctx, uint16_t value)
{
	assert(ctx);

	*((uint16_t *)ctx->position) = htons(value);
	ctx->position += sizeof(uint16_t);
}

static inline void wire_write(wire_ctx_t *ctx, const uint8_t *data, size_t size)
{
	assert(ctx);
	assert(data);

	memmove(ctx->position, data, size);
	ctx->position += size;
}

static inline void wire_write_binary(wire_ctx_t *ctx, const dnssec_binary_t *data)
{
	assert(ctx);
	assert(data);

	wire_write(ctx, data->data, data->size);
}

static inline void wire_write_bignum(wire_ctx_t *ctx, size_t width,
				     const dnssec_binary_t *bignum)
{
	assert(ctx);
	assert(bignum);

	dnssec_binary_t dest = { .data = ctx->position, .size = width };
	bignum_write(&dest, bignum);
	ctx->position += width;
}

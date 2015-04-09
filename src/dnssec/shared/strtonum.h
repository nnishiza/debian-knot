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

#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>

#include "error.h"

inline static int str_to_intmax(const char *src, intmax_t *dest)
{
	if (!isdigit(*src) && *src != '-' && *src != '+') {
		return DNSSEC_MALFORMED_DATA;
	}

	errno = 0;
	char *end = NULL;
	intmax_t result = strtoimax(src, &end, 10);

	if (errno == ERANGE) {
		return DNSSEC_OUT_OF_RANGE;
	}

	if (src == end || *end != '\0') {
		return DNSSEC_MALFORMED_DATA;
	}

	*dest = result;
	return DNSSEC_EOK;
}

inline static int str_to_uintmax(const char *src, uintmax_t *dest)
{
	if (!isdigit(*src) && *src != '-' && *src != '+') {
		return DNSSEC_MALFORMED_DATA;
	}

	errno = 0;
	char *end = NULL;
	uintmax_t result = strtoumax(src, &end, 10);

	if (errno == ERANGE) {
		return DNSSEC_OUT_OF_RANGE;
	}

	if (src == end || *end != '\0') {
		return DNSSEC_MALFORMED_DATA;
	}

	*dest = result;
	return DNSSEC_EOK;
}

#define CONVERT(function, maxtype, type, min, max, src, dest) \
{                                                             \
	maxtype value;                                        \
	int result = function(src, &value);                   \
	if (result != DNSSEC_EOK) {                           \
		return result;                                \
	}                                                     \
	if (value < (min) || value > (max)) {                 \
		return DNSSEC_OUT_OF_RANGE;                   \
	}                                                     \
	*dest = (type)value;                                  \
	return DNSSEC_EOK;                                    \
}

inline static int str_to_int(const char *src, int *dest)
{
	CONVERT(str_to_intmax, intmax_t, int, INT_MIN, INT_MAX, src, dest);
}

inline static int str_to_u8(const char *src, uint8_t *dest)
{
	CONVERT(str_to_uintmax, uintmax_t, uint8_t, 0, UINT8_MAX, src, dest);
}

inline static int str_to_u16(const char *src, uint16_t *dest)
{
	CONVERT(str_to_uintmax, uintmax_t, uint16_t, 0, UINT16_MAX, src, dest);
}

#undef CONVERT

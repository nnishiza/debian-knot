/*  Copyright (C) 2015 CZ.NIC, z.s.p.o. <knot-dns@labs.nic.cz>

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

#include "libknot/rrtype/ds.h"
#include "contrib/wire.h"

_public_
uint16_t knot_ds_key_tag(const knot_rdataset_t *rrs, size_t pos)
{
	KNOT_RDATASET_CHECK(rrs, pos, return 0);
	return wire_read_u16(knot_rdata_offset(rrs, pos, 0));
}

_public_
uint8_t knot_ds_alg(const knot_rdataset_t *rrs, size_t pos)
{
	KNOT_RDATASET_CHECK(rrs, pos, return 0);
	return *knot_rdata_offset(rrs, pos, 2);
}

_public_
uint8_t knot_ds_digest_type(const knot_rdataset_t *rrs, size_t pos)
{
	KNOT_RDATASET_CHECK(rrs, pos, return 0);
	return *knot_rdata_offset(rrs, pos, 3);
}

_public_
void knot_ds_digest(const knot_rdataset_t *rrs, size_t pos,
                    uint8_t **digest, uint16_t *digest_size)
{
	KNOT_RDATASET_CHECK(rrs, pos, return);
	*digest = knot_rdata_offset(rrs, pos, 4);
	const knot_rdata_t *rr = knot_rdataset_at(rrs, pos);
	*digest_size = knot_rdata_rdlen(rr) - 4;
}

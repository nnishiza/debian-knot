/*!
 * \file rdataset.h
 *
 * \author Jan Kadlec <jan.kadlec@nic.cz>
 *
 * \brief API for manipulating RR arrays.
 *
 * \addtogroup libknot
 * @{
 */
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

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "libknot/rdata.h"
#include "libknot/internal/mempattern.h"

/*!< \brief Set of RRs. */
typedef struct knot_rdataset {
	uint16_t rr_count;  /*!< \brief Count of RRs stored in the structure. */
	knot_rdata_t *data; /*!< \brief Actual data, canonically sorted. */
} knot_rdataset_t;

/* -------------------------- RRs init/clear ---------------------------------*/

/*!
 * \brief Initializes RRS structure.
 * \param rrs  Structure to be initialized.
 */
void knot_rdataset_init(knot_rdataset_t *rrs);

/*!
 * \brief Frees data initialized by RRS structure, but not the structure itself.
 * \param rrs  Structure to be cleared.
 * \param mm   Memory context used to create allocations.
 */
void knot_rdataset_clear(knot_rdataset_t *rrs, mm_ctx_t *mm);

/*!
 * \brief Deep copies RRS structure. All data are duplicated.
 * \param dst  Copy destination.
 * \param src  Copy source.
 * \param mm   Memory context.
 * \return KNOT_E*
 */
int knot_rdataset_copy(knot_rdataset_t *dst, const knot_rdataset_t *src, mm_ctx_t *mm);

/* ----------------------- RRs getters/setters ------------------------------ */

/*!
 * \brief Gets RR from RRS structure, using given position.
 * \param rrs  RRS structure to get RR from.
 * \param pos  Position to use.
 * \return Pointer to RR at \a pos position.
 */
knot_rdata_t *knot_rdataset_at(const knot_rdataset_t *rrs, size_t pos);

/*!
 * \brief Returns size of the structures holding the RR set.
 * \param rrs  RR array.
 * \return Array size.
 */
size_t knot_rdataset_size(const knot_rdataset_t *rrs);

/* ----------------------- RRs RR manipulation ------------------------------ */

/*!
 * \brief Adds single RR into RRS structure. All data are copied.
 * \param rrs  RRS structure to add RR into.
 * \param rr   RR to add.
 * \param mm   Memory context.
 * \return KNOT_E*
 */
int knot_rdataset_add(knot_rdataset_t *rrs, const knot_rdata_t *rr, mm_ctx_t *mm);

/*!
 * \brief Reserves space at the end of the RRS structure.
 * \param rrs   RRS structure to reserve space at.
 * \param size  How much space to reserve.
 * \param mm    Memory context.
 * \return KNOT_E*
 */
int knot_rdataset_reserve(knot_rdataset_t *rrs, size_t size, mm_ctx_t *mm);

/*!
 * \brief Removes the last RR from RRS structure, i.e. does the opposite of _reserve.
 * \param rrs  RRS structure to remove RR from.
 * \param mm   Memory context.
 * \return KNOT_E*
 */
int knot_rdataset_unreserve(knot_rdataset_t *rrs, mm_ctx_t *mm);

/* ---------------------- RRs set-like operations --------------------------- */

/*!
 * \brief RRS equality check.
 * \param rrs1  First RRS to be compared.
 * \param rrs2  Second RRS to be compared.
 * \retval true if rrs1 == rrs2.
 * \retval false if rrs1 != rrs2.
 */
bool knot_rdataset_eq(const knot_rdataset_t *rrs1, const knot_rdataset_t *rrs2);

/*!
 * \brief Returns true if \a rr is present in \a rrs, false otherwise.
 * \param rrs      RRS to search in.
 * \param rr       RR to compare with.
 * \param cmp_ttl  If set to true, TTLs will be compared as well.
 * \retval true if \a rr is present in \a rrs.
 * \retval false if \a rr is not present in \a rrs.
 */
bool knot_rdataset_member(const knot_rdataset_t *rrs, const knot_rdata_t *rr,
			  bool cmp_ttl);

/*!
 * \brief Merges two RRS into the first one. Second RRS is left intact.
 *        Canonical order is preserved.
 * \param rrs1  Destination RRS (merge here).
 * \param rrs2  RRS to be merged (merge from).
 * \param mm    Memory context.
 * \return KNOT_E*
 */
int knot_rdataset_merge(knot_rdataset_t *rrs1, const knot_rdataset_t *rrs2,
			mm_ctx_t *mm);

/*!
 * \brief RRS set-like intersection. Full compare is done.
 * \param a    First RRS to intersect.
 * \param b    Second RRS to intersect.
 * \param out  Output RRS with intersection, RDATA are created anew.
 * \param mm   Memory context. Will be used to create new RDATA.
 * \return KNOT_E*
 */
int knot_rdataset_intersect(const knot_rdataset_t *a, const knot_rdataset_t *b,
                            knot_rdataset_t *out, mm_ctx_t *mm);

/*!
 * \brief Does set-like RRS subtraction. \a from RRS is changed. Both sets must
          be unique, i.e. data point to different locations.
 * \param from  RRS to subtract from.
 * \param what  RRS to subtract.
 * \param mm    Memory context use to reallocated \a from data.
 * \return KNOT_E*
 */
int knot_rdataset_subtract(knot_rdataset_t *from, const knot_rdataset_t *what,
                           mm_ctx_t *mm);

/*!
 * \brief Sorts the dataset. Removes the element if found to be duplicate.
 * \param rss   RRS to sort.
 * \param pos   Position of the element to sort.
 * \param mm    Memory context used to remove the element if duplicate.
 * \return KNOT_E*
 */
int knot_rdataset_sort_at(knot_rdataset_t *rrs, size_t pos, mm_ctx_t *mm);

/*! \brief Accession helpers. */
#define KNOT_RDATASET_CHECK(rrs, pos, code) \
	if (rrs == NULL || rrs->data == NULL || rrs->rr_count == 0 || \
	    pos >= rrs->rr_count) { \
		code; \
	}

/*! \todo Documentation. */
static inline
uint8_t *knot_rdata_offset(const knot_rdataset_t *rrs, size_t pos, size_t offset) {
	knot_rdata_t *rr = knot_rdataset_at(rrs, pos);
	return knot_rdata_data(rr) + offset;
}

/*! @} */

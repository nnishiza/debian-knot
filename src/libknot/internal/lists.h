/*  Copyright (C) 2011 CZ.NIC, z.s.p.o. <knot-dns@labs.nic.cz>

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
/*
 *	BIRD Library -- Linked Lists
 *
 *	(c) 1998 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#pragma once

/*
 * I admit the list structure is very tricky and also somewhat awkward,
 * but it's both efficient and easy to manipulate once one understands the
 * basic trick: The list head always contains two synthetic nodes which are
 * always present in the list: the head and the tail. But as the `next'
 * entry of the tail and the `prev' entry of the head are both NULL, the
 * nodes can overlap each other:
 *
 *     head    head_node.next
 *     null    head_node.prev  tail_node.next
 *     tail                    tail_node.prev
 */

#include <string.h>  // size_t

typedef struct node {
  struct node *next, *prev;
} node_t;

typedef struct list {			/* In fact two overlayed nodes */
  struct node *head, *null, *tail;
} list_t;

#define NODE (node_t *)
#define HEAD(list) ((void *)((list).head))
#define TAIL(list) ((void *)((list).tail))
#define WALK_LIST(n,list) for(n=HEAD(list);(NODE (n))->next; \
				n=(void *)((NODE (n))->next))
#define WALK_LIST_DELSAFE(n,nxt,list) \
     for(n=HEAD(list); (nxt=(void *)((NODE (n))->next)); n=(void *) nxt)
/* WALK_LIST_FIRST supposes that called code removes each processed node */
#define WALK_LIST_FIRST(n,list) \
     while(n=HEAD(list), (NODE (n))->next)
#define WALK_LIST_BACKWARDS(n,list) for(n=TAIL(list);(NODE (n))->prev; \
				n=(void *)((NODE (n))->prev))
#define WALK_LIST_BACKWARDS_DELSAFE(n,prv,list) \
     for(n=TAIL(list); prv=(void *)((NODE (n))->prev); n=(void *) prv)

#define EMPTY_LIST(list) (!(list).head->next)

/*! \brief Free every node in the list. */
#define WALK_LIST_FREE(list) \
	do { \
	node_t *n=0,*nxt=0;  \
	WALK_LIST_DELSAFE(n,nxt,list) { \
	    free(n); \
	} \
	init_list(&list); \
	} while(0)

void add_tail(list_t *, node_t *);
void add_head(list_t *, node_t *);
void rem_node(node_t *);
void add_tail_list(list_t *, list_t *);
void init_list(list_t *);
void insert_node(node_t *, node_t *);
void list_dup(list_t *dst, list_t *src, size_t itemsz);
size_t list_size(const list_t *);

#include <stdbool.h>
#include "libknot/internal/mempattern.h"
/*!
 * \brief Generic pointer list implementation.
 */
typedef struct ptrnode {
	node_t n;
	const void *d;
} ptrnode_t;

ptrnode_t *ptrlist_add(list_t *, const void *, mm_ctx_t *);
void ptrlist_free(list_t *, mm_ctx_t *);
bool ptrlist_contains(list_t *, const void *);

/*! \todo This is broken atm.
#ifndef _BIRD_LISTS_C_
#define LIST_INLINE extern inline
#include "knot/lib/lists.c"
#undef LIST_INLINE
#else
#define LIST_INLINE
#endif
*/
#define LIST_INLINE

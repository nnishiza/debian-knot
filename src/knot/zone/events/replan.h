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

#include "knot/zone/events/events.h"
#include "knot/zone/zone.h"

/*! \brief Replans zone's events using old zone. */
void replan_events(zone_t *zone, zone_t *old_zone);

/*! \brief Replans zone's DDNS events using old zone's DDNS queue. */
void replan_update(zone_t *zone, zone_t *old_zone);

/*
OpenIO SDS meta0v2
Copyright (C) 2014 Worldline, as part of Redcurrant
Copyright (C) 2015-2017 OpenIO SAS, as part of OpenIO SDS

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License as
published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef OIO_SDS__meta0v2__meta0_gridd_dispatcher_h
# define OIO_SDS__meta0v2__meta0_gridd_dispatcher_h 1

#include <server/transport_gridd.h>

struct meta0_disp_s;

const struct gridd_request_descr_s* meta0_gridd_get_requests(void);

void meta0_gridd_free_dispatcher(struct meta0_disp_s *m0disp);

struct meta0_disp_s* meta0_gridd_get_dispatcher(struct meta0_backend_s *m0, const char *ns_name);

void meta0_gridd_requested_reload(struct meta0_disp_s *m0disp);

#endif /*OIO_SDS__meta0v2__meta0_gridd_dispatcher_h*/

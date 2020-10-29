/*
OpenIO SDS meta2v2
Copyright (C) 2021 OpenIO SAS, as part of OpenIO SDS

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

#ifndef OIO_SDS__meta2v2__meta2_utils_container_sharding_h
# define OIO_SDS__meta2v2__meta2_utils_container_sharding_h 1

#include <glib.h>

#include <metautils/lib/metautils.h>

struct shard_container_s {
	guint index;
	gchar *lower;
	gchar *upper;
	gchar *cid;
};

typedef GTree* shards_container_t;

GError* shards_container_decode(const gchar *str, shards_container_t *pshards);

gchar* shards_container_encode(shards_container_t shards);

struct shard_container_s *shards_container_get_shard(
		shards_container_t shards, const gchar *path);

void shards_container_free(shards_container_t shards);

#endif /*OIO_SDS__meta2v2__meta2_utils_container_sharding_h*/

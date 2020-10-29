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

#include <glib.h>

#include <metautils/lib/metautils.h>

#include <meta2v2/meta2_utils_container_sharding.h>

static GError*
_shard_container_decode(struct json_object *jshard,
		struct shard_container_s **pshard)
{
	GError *err = NULL;

	struct json_object *jindex = NULL, *jlower = NULL, *jupper = NULL,
			*jcid = NULL;
	struct oio_ext_json_mapping_s mapping[] = {
		{"index", &jindex, json_type_int,    1},
		{"lower", &jlower, json_type_string, 1},
		{"upper", &jupper, json_type_string, 1},
		{"cid",   &jcid,   json_type_string, 1},
		{NULL,NULL,0,0}
	};
	err = oio_ext_extract_json(jshard, mapping);
	if (err) {
		goto end;
	}

	struct shard_container_s *shard = g_malloc0(
			sizeof(struct shard_container_s));
	shard->index = json_object_get_int(jindex);
	shard->lower = g_strdup(json_object_get_string(jlower));
	shard->upper = g_strdup(json_object_get_string(jupper));
	shard->cid = g_strdup(json_object_get_string(jcid));
	*pshard = shard;

end:
	if (err)
		g_prefix_error(&err, "Failed to decode shard container info: ");
	return err;
}

static void
_shard_container_encode(struct shard_container_s *shard, GString *shard_json)
{
	g_string_append_c(shard_json, '{');
	oio_str_gstring_append_json_pair_int(shard_json, "index", shard->index);
	g_string_append_c(shard_json, ',');
	oio_str_gstring_append_json_pair(shard_json, "lower", shard->lower);
	g_string_append_c(shard_json, ',');
	oio_str_gstring_append_json_pair(shard_json, "upper", shard->upper);
	g_string_append_c(shard_json, ',');
	oio_str_gstring_append_json_pair(shard_json, "cid", shard->cid);
	g_string_append_c(shard_json, '}');
}

static void
_shard_container_free(struct shard_container_s *shard)
{
	if (!shard)
		return;

	g_free(shard->lower);
	g_free(shard->upper);
	g_free(shard->cid);
	g_free(shard);
}

static gint
_shards_container_cmp(gconstpointer a, gconstpointer b, gpointer u UNUSED)
{
	const struct shard_container_s *shard1 = a;
	const struct shard_container_s *shard2 = b;

	if (shard1->index < shard2->index) {
		return -1;
	} else if (shard1->index == shard2->index) {
		return 0;
	} else {
		return 1;
	}
}

GError*
shards_container_decode(const gchar *str, shards_container_t *pshards)
{
	GError *err = NULL;
	GTree *shards = NULL;

	struct json_tokener *tok = json_tokener_new();
	struct json_object *jshards = json_tokener_parse_ex(tok,
			str, strlen(str));
	json_tokener_free(tok);
	if (!jshards) {
		err = BADREQ("Parse error");
		goto end;
	}

	shards = g_tree_new_full(_shards_container_cmp, NULL, NULL,
			(GDestroyNotify)_shard_container_free);
	int nb_shards = json_object_array_length(jshards);
	for (int i = 0; i < nb_shards; i++) {
		struct json_object *jshard = json_object_array_get_idx(jshards, i);
		struct shard_container_s *shard = NULL;
		err = _shard_container_decode(jshard, &shard);
		if (err)
			goto end;
		g_tree_insert(shards, shard, shard);
	}

	*pshards = shards;

end:
	json_object_put(jshards);
	if (err) {
		if (shards)
			shards_container_free(shards);
		g_prefix_error(&err, "Failed to decode shards container info: ");
	}
	return err;
}

static gboolean
_shards_container_encode(gpointer key UNUSED, gpointer value, gpointer data) {
	GString *shards_json = data;
	if (shards_json->str[shards_json->len - 1] != '[')
		g_string_append_c(shards_json, ',');
	_shard_container_encode(value, shards_json);
	return FALSE;
}

gchar*
shards_container_encode(shards_container_t shards)
{
	GString *shards_json = g_string_new("[");
	g_tree_foreach(shards, _shards_container_encode, shards_json);
	g_string_append_c(shards_json, ']');
	return g_string_free(shards_json, FALSE);
}

static gint
_shard_check_range(const gchar *lower, const gchar *upper, const gchar *path)
{
	EXTRA_ASSERT(lower != NULL);
	EXTRA_ASSERT(upper != NULL);
	EXTRA_ASSERT(path != NULL);

	if (*lower && strncmp(path, lower, LIMIT_LENGTH_CONTENTPATH) <= 0) {
		return -1;
	}
	if (*upper && strncmp(path, upper, LIMIT_LENGTH_CONTENTPATH) > 0) {
		return 1;
	}
	// lower < path <= upper
	return 0;
}

static gint
_shard_container_cmp_with_path(gconstpointer a, gconstpointer b)
{
	const struct shard_container_s *shard = a;
	const gchar *path = b;

	return _shard_check_range(shard->lower, shard->upper, path);
}

struct shard_container_s *
shards_container_get_shard(shards_container_t shards, const gchar *path)
{
	if (!shards)
		return NULL;

	return g_tree_search(shards, _shard_container_cmp_with_path, path);
}

void
shards_container_free(shards_container_t shards)
{
	if (shards)
		g_tree_destroy(shards);
}

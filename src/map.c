/*
 * Copyright (c) 2018 Milan Suk
 *
 * Use of this software is governed by the Business Source License included
 * in the LICENSE file and at www.mariadb.com/bsl11.
 *
 * Change Date: 2025-03-01
 *
 * On the date above, in accordance with the Business Source License, use
 * of this software will be governed by version 2 or later of the General
 * Public License.
 */

 //collaborate
#include "os.h"
#include "std.h"
#include "log.h"
#include "file.h"
#include "media.h"

//header
#include "map.h"

#include "map/map_ini.h"
#include "map/map_tiles.h"
#include "map/map_poly.h"

typedef struct Map_s
{
	MapTiles* tilesBase;
	MapTiles* tilesUser;

	MapPoly* polyBase;

	MapIni* ini;
} Map;

Map* g_Map = 0;

void Map_delete(void)
{
	MapIni_delete(g_Map->ini);

	if (g_Map->tilesBase)	MapTiles_delete(g_Map->tilesBase);
	if (g_Map->tilesUser)	MapTiles_delete(g_Map->tilesUser);

	if (g_Map->polyBase)	MapPoly_delete(g_Map->polyBase);

	Os_free(g_Map, sizeof(Map));
	g_Map = 0;
}
BOOL Map_new(void)
{
	if (g_Map)
		Map_delete();

	g_Map = Os_malloc(sizeof(Map));

	g_Map->tilesBase = MapTiles_new("map_std.tiles", TRUE);
	g_Map->tilesUser = 0;

	g_Map->polyBase = MapPoly_newRead("map.poly", TRUE);

	g_Map->ini = MapIni_new();

	return TRUE;
}

UBIG Map_bytes(void)
{
	return MapTiles_bytes(g_Map->tilesBase) + MapTiles_bytes(g_Map->tilesUser) + MapPoly_bytes(g_Map->polyBase);
}
void Map_updateNet(void)
{
	MapIni_updateNet(g_Map->ini);
}

const BIG Map_findTile(int x, int y, int z, UCHAR** outBuff)
{
	BIG i = MapTiles_find(g_Map->tilesBase, x, y, z, outBuff);
	if (i < 0 && g_Map->tilesUser)
		i = MapTiles_find(g_Map->tilesUser, x, y, z, outBuff);
	return i;
}

const MapPolyIndex* Map_findPoly(const UNI* name)
{
	return MapPoly_find(g_Map->polyBase, name);
}

BOOL Map_downloadPos(const UNI* name, Vec2f* out_pos)
{
	*out_pos = Vec2f_init();
	BOOL find = FALSE;
	if (MapIni_tryDownload(g_Map->ini))
	{
		char* txt = Std_newCHAR_urlEncode(name);
		char* url = Std_addAfterCHAR(Std_addCHAR(MapIni_get_geolocation_url(g_Map->ini), txt), "&format=xml&limit=1");
		Std_deleteCHAR(txt);

		UCHAR* result;
		BIG bytes = MediaNetwork_download(url, 0.01, &result);
		find = (bytes > 0);
		if (find)
		{
			find = MapIni_getPosFrom(g_Map->ini, (char*)result, out_pos);

			Os_free(result, bytes);
		}

		Std_deleteCHAR(url);
	}
	return find;
}

BOOL Map_getTileUrl(int x, int y, int zoom, char url[64])
{
	BOOL ok = MapIni_tryDownload(g_Map->ini);
	snprintf(url, 64, (ok ? MapIni_get_tile_url(g_Map->ini) : "tile_offline/%d/%d/%d.png"), zoom, x, y);
	return ok;
}

void Map_addImage(int x, int y, int z, const UCHAR* buff, UBIG bytes)
{
	if (g_Map->tilesUser)
		MapTiles_addImage(g_Map->tilesUser, x, y, z, buff, bytes);
}

void Map_loadUserTiles(const char* path)
{
	if (g_Map->tilesUser)
		MapTiles_delete(g_Map->tilesUser);

	g_Map->tilesUser = MapTiles_new(path, FALSE);
}

void Map_deleteTilesFile(void)
{
	if (g_Map->tilesUser)
	{
		char* path = Std_newCHAR(g_Map->tilesUser->path);

		OsFileDir_removeFile(path);
		Map_loadUserTiles(path);

		Std_deleteCHAR(path);
	}
}
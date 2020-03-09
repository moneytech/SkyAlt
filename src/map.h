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

typedef struct MapPolyPos_s
{
	float x, y;
}MapPolyPos;

typedef struct MapPolyPolygon_s
{
	MapPolyPos* poses;
	UBIG num_poses;
	Quad2f bbox;
} MapPolyPolygon;

typedef struct MapPolyIndex_s
{
	UBIG file_pos;
	char* name;
	double load_time;

	Vec2f pos;  //lat, lon
	MapPolyPolygon* polygons;
	UBIG num_polygons;
} MapPolyIndex;

BOOL Map_new(void);
void Map_delete(void);
void Map_updateNet(void);
UBIG Map_bytes(void);

const BIG Map_findTile(int x, int y, int z, UCHAR** outBuff);
const MapPolyIndex* Map_findPoly(const UNI* name);
BOOL Map_downloadPos(const UNI* name, Vec2f* out_pos);

BOOL Map_getTileUrl(int x, int y, int zoom, char url[64]);

void Map_addImage(int x, int y, int z, const UCHAR* buff, UBIG bytes);

void Map_loadUserTiles(const char* path);
void Map_deleteTilesFile(void);

void MapTiles_createCache(void);
void MapPoly_createCache(void);
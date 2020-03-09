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

#define MapTiles_MAX_IN_FILE (5*1024)

 //[INDEXES-16 x MapTiles_MAX_IN_FILE][DATA]
 //New file data are writed imidietly, but indexes are writed at the end to avoid too many SSD re-writes

typedef struct MapTilesIndex_s	//must be 16 Bytes
{
	UBIG size;
	BIG x : 28,
		y : 28,
		z : 8;
} MapTilesIndex;

typedef struct MapTiles_s
{
	char* path;
	MapTilesIndex* items;
	UBIG num_items;

	UBIG num_items_open;
} MapTiles;

static void _MapTiles_inicializeFile(const MapTiles* self)
{
	if (OsFile_bytes(self->path) <= 0)	//not exist
	{
		OsFile file;
		if (OsFile_init(&file, self->path, OsFile_W))
		{
			MapTilesIndex org, enc;
			Os_memset(&org, sizeof(org));

			BIG i;
			for (i = 0; i < MapTiles_MAX_IN_FILE; i++)
			{
				FileProject_encryptDirect(i * sizeof(MapTilesIndex), (UCHAR*)&org, (UCHAR*)&enc, 16);
				OsFile_write(&file, &enc, sizeof(MapTilesIndex));
			}
			OsFile_free(&file);
		}
	}
}

static void _MapTiles_resize(MapTiles* self, UBIG n)
{
	self->num_items = n;
	self->items = Os_realloc(self->items, self->num_items * sizeof(MapTilesIndex));
}

MapTiles* MapTiles_new(const char* path, BOOL printWarning)
{
	MapTiles* self = Os_calloc(1, sizeof(MapTiles));
	self->path = Std_newCHAR(path);
	//alloc up
	_MapTiles_resize(self, MapTiles_MAX_IN_FILE);

	_MapTiles_inicializeFile(self);

	//read indexes from file
	int t = 0;
	UBIG p = 0;
	OsFile file;
	if (OsFile_init(&file, path, OsFile_R))
	{
		UBIG i;
		for (i = 0; i < MapTiles_MAX_IN_FILE; i++)
		{
			MapTilesIndex in, out;
			if (OsFile_read(&file, &in, sizeof(MapTilesIndex)) == sizeof(MapTilesIndex))
			{
				FileProject_decryptDirect(i * sizeof(MapTilesIndex), (UCHAR*)&in, (UCHAR*)&out, 16);
				if (out.size > 0)
					self->items[p++] = out;

				if (self->items[p - 1].z == 6)
					t++;
			}
			else
				break;
		}

		OsFile_free(&file);
	}
	else
		if (printWarning)
			printf("Warning: MapTiles cache file(%s) is missing\n", path);

	//alloc down
	_MapTiles_resize(self, p);

	self->num_items_open = self->num_items;

	return self;
}

void MapTiles_delete(MapTiles* self)
{
	if (self->num_items >= MapTiles_MAX_IN_FILE)
		self->num_items = 0;	//reset

	if (self->num_items_open != self->num_items)
	{
		//write indexes
		OsFile file;
		if (OsFile_init(&file, self->path, OsFile_RW))
		{
			OsFile_seekAbs(&file, 0);

			//write
			MapTilesIndex enc;
			BIG i;
			for (i = 0; i < self->num_items; i++)
			{
				MapTilesIndex* it = &self->items[i];
				FileProject_encryptDirect(i * sizeof(MapTilesIndex), (UCHAR*)it, (UCHAR*)&enc, 16);
				OsFile_write(&file, &enc, sizeof(MapTilesIndex));
			}

			OsFile_free(&file);
		}
	}

	Std_deleteCHAR(self->path);
	Os_free(self->items, self->num_items * sizeof(MapTilesIndex));
	Os_free(self, sizeof(MapTiles));
}

UCHAR* _MapTiles_readData(const MapTiles* self, UBIG pos, BIG bytes)
{
	UCHAR* buff = 0;

	OsFile file;
	if (OsFile_init(&file, self->path, OsFile_R))
	{
		OsFile_seekAbs(&file, pos);

		buff = Os_malloc(Std_round16(bytes));

		UBIG i = 0;
		UCHAR t[16];
		while (i < bytes)
		{
			if (OsFile_read(&file, t, 16) == 16)
				FileProject_decryptDirect(pos + i, t, &buff[i], 16);
			i += 16;
		}

		OsFile_free(&file);
	}
	return buff;
}

const void _MapTiles_writeData(const MapTiles* self, UBIG pos, const UCHAR* buff, BIG bytes)
{
	OsFile file;
	if (OsFile_init(&file, self->path, OsFile_RW))
	{
		OsFile_seekAbs(&file, pos);

		//write
		UBIG i = 0;
		UCHAR b[16];
		UCHAR t[16];
		while (i < bytes)
		{
			Os_memset(b, 16);
			Os_memcpy(b, &buff[i], Std_min(16, bytes - i));

			FileProject_encryptDirect(pos + i, b, t, 16);
			OsFile_write(&file, t, 16);
			i += 16;
		}

		OsFile_free(&file);
	}
}

const BIG MapTiles_find(const MapTiles* self, int x, int y, int z, UCHAR** outBuff)
{
	BIG pos = MapTiles_MAX_IN_FILE * sizeof(MapTilesIndex);

	BIG i;
	for (i = 0; i < self->num_items; i++)
	{
		MapTilesIndex* it = &self->items[i];
		if (it->x == x && it->y == y && it->z == z)
		{
			*outBuff = _MapTiles_readData(self, pos, it->size);
			return *outBuff ? it->size : -1;
		}

		pos += Std_round16(it->size);
	}
	return -1;
}

UBIG MapTiles_bytes(const MapTiles* self)
{
	BIG pos = MapTiles_MAX_IN_FILE * sizeof(MapTilesIndex);	//start
	BIG i;
	for (i = 0; i < self->num_items; i++)
		pos += Std_round16(self->items[i].size);
	return pos;
}

void MapTiles_addImage(MapTiles* self, int x, int y, int z, const UCHAR* buff, UBIG bytes)
{
	//find end pos
	UBIG pos = MapTiles_bytes(self);

	//add index(will be written later in MapTiles_delete);
	MapTilesIndex ind;
	ind.x = x;
	ind.y = y;
	ind.z = z;
	ind.size = bytes;
	_MapTiles_resize(self, self->num_items + 1);
	self->items[self->num_items - 1] = ind;

	//write data
	_MapTiles_writeData(self, pos, buff, bytes);
}

void MapTiles_createCache(void)
{
	const int startZoom = 0;
	const int endZoom = 6;

	MapTiles* Map = MapTiles_new("map_std.tiles", FALSE);

	int x, y, z;

	int num_cells = 0;
	for (z = startZoom; z < endZoom; z++)
		num_cells += Os_pow(2, z) * Os_pow(2, z);

	double time = Os_time();
	int cell = 0;
	for (z = startZoom; z < endZoom; z++)
	{
		const int max_res = Os_pow(2, z);

		for (y = 0; y < max_res; y++)
		{
			for (x = 0; x < max_res; x++)
			{
				char url[64];
				snprintf(url, 64, "http://c.tile.openstreetmap.org/%d/%d/%d.png", z, x, y);

				BOOL running = TRUE;
				float done;
				char* data = 0;
				BIG bytes = OsHTTPS_downloadWithStatus(url, &done, &running, &data);
				if (bytes >= 0)
				{
					MapTiles_addImage(Map, x, y, z, (UCHAR*)data, bytes);
					Os_free(data, bytes);
				}

				double cell_time = Os_time() - time;
				printf("%d from %d (%.1f%% - %.1fmin)\n", cell, num_cells, cell / (float)num_cells * 100, (cell > 0) ? cell_time / cell * (num_cells - cell) / 60.0 : 0);

				cell++;
				OsThread_sleep(1000);
			}
		}
	}

	MapTiles_delete(Map);
	printf("done\n");
}

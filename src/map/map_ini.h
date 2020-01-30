/*
 * Copyright (c) 2018 Milan Suk
 *
 * Use of this software is governed by the Business Source License included
 * in the LICENSE file and at www.mariadb.com/bsl11.
 *
 * Change Date: 2025-02-01
 *
 * On the date above, in accordance with the Business Source License, use
 * of this software will be governed by version 2 or later of the General
 * Public License.
 */

typedef struct MapIni_s
{
	float geolocation_delay;
	float tile_delay;

	char* geolocation_url;
	char* tile_url;

	char* copyright_text;
	char* copyright_url;
	char* fixmap_url;

	char* geolocation_item;
	char* geolocation_lon;
	char* geolocation_lat;
} MapIni;

float MapIni_get_geolocation_delay(const MapIni* self)
{
	return self->geolocation_delay;
}
float MapIni_get_tile_delay(const MapIni* self)
{
	return self->tile_delay;
}
char* MapIni_get_geolocation_url(const MapIni* self)
{
	return self->geolocation_url;
}
char* MapIni_get_tile_url(const MapIni* self)
{
	return self->tile_url;
}

char* MapIni_get_copyright_text(const MapIni* self)
{
	return self->copyright_text;
}
char* MapIni_get_copyright_url(const MapIni* self)
{
	return self->copyright_url;
}
char* MapIni_get_fixmap_url(const MapIni* self)
{
	return self->fixmap_url;
}

char* MapIni_get_geolocation_item(const MapIni* self)
{
	return self->geolocation_item;
}
char* MapIni_get_geolocation_lon(const MapIni* self)
{
	return self->geolocation_lon;
}
char* MapIni_get_geolocation_lat(const MapIni* self)
{
	return self->geolocation_lat;
}

void MapIni_set_geolocation_url(MapIni* self, const char* str)
{
	Std_deleteCHAR(self->geolocation_url);
	self->geolocation_url = Std_newCHAR(str);
}
void MapIni_set_tile_url(MapIni* self, const char* str)
{
	Std_deleteCHAR(self->tile_url);
	self->tile_url = Std_newCHAR(str);
}

void MapIni_set_copyright_text(MapIni* self, const char* str)
{
	Std_deleteCHAR(self->copyright_text);
	self->copyright_text = Std_newCHAR(str);
}
void MapIni_set_copyright_url(MapIni* self, const char* str)
{
	Std_deleteCHAR(self->copyright_url);
	self->copyright_url = Std_newCHAR(str);
}
void MapIni_set_fixmap_url(MapIni* self, const char* str)
{
	Std_deleteCHAR(self->fixmap_url);
	self->fixmap_url = Std_newCHAR(str);
}

void MapIni_set_geolocation_item(MapIni* self, const char* str)
{
	Std_deleteCHAR(self->geolocation_item);
	self->geolocation_item = Std_newCHAR(str);
}
void MapIni_set_geolocation_lon(MapIni* self, const char* str)
{
	Std_deleteCHAR(self->geolocation_lon);
	self->geolocation_lon = Std_newCHAR(str);
}
void MapIni_set_geolocation_lat(MapIni* self, const char* str)
{
	Std_deleteCHAR(self->geolocation_lat);
	self->geolocation_lat = Std_newCHAR(str);
}

void _MapIni_addLine(MapIni* self, char* line)
{
	float value1;
	char str[64];

	if (sscanf(line, "geolocation_delay = %f", &value1) == 1)
		self->geolocation_delay = value1;
	if (sscanf(line, "tile_delay = %f", &value1) == 1)
		self->tile_delay = value1;

	if (sscanf(line, "geolocation_url = %63s", str) == 1)
		MapIni_set_geolocation_url(self, str);
	if (sscanf(line, "tile_url = %63s", str) == 1)
		MapIni_set_tile_url(self, str);

	if (sscanf(line, "copyright_text = %63s", str) == 1)
		MapIni_set_copyright_text(self, str);
	if (sscanf(line, "copyright_url = %63s", str) == 1)
		MapIni_set_copyright_url(self, str);
	if (sscanf(line, "fixmap_url = %63s", str) == 1)
		MapIni_set_fixmap_url(self, str);

	if (sscanf(line, "geolocation_item = %63s", str) == 1)
		MapIni_set_geolocation_item(self, str);
	if (sscanf(line, "geolocation_lat = %63s", str) == 1)
		MapIni_set_geolocation_lat(self, str);
	if (sscanf(line, "geolocation_lon = %63s", str) == 1)
		MapIni_set_geolocation_lon(self, str);
}

void MapIni_addFile(MapIni* self, const char* path)
{
	OsFile file;
	if (OsFile_init(&file, path, OsFile_R))
	{
		char* line;
		while ((line = OsFile_readLine(&file)))
		{
			_MapIni_addLine(self, line);
			Std_deleteCHAR(line);
		}

		OsFile_free(&file);
	}
}

void MapIni_addBuffer(MapIni* self, const char* buff, const BIG N)
{
	BIG i = 0;
	while (i < N)
	{
		//skip empty
		while (i < N && (buff[i] == '\r' || buff[i] == '\n'))
			i++;

		//size
		const char* stBuff = &buff[i];
		int n = 0;
		while (i < N && buff[i] != '\r' && buff[i] != '\n')
		{
			i++;
			n++;
		}

		//parse line
		if (n)
		{
			char* line = Std_newCHAR_n(stBuff, n);
			_MapIni_addLine(self, line);
			Std_deleteCHAR(line);
		}
	}
}

MapIni* MapIni_new(void)
{
	MapIni* self = Os_malloc(sizeof(MapIni));

	self->geolocation_delay = 1.2f;
	self->tile_delay = 0.01f;

	self->geolocation_url = 0;
	self->tile_url = 0;

	self->copyright_text = 0;
	self->copyright_url = 0;
	self->fixmap_url = 0;

	self->geolocation_item = 0;
	self->geolocation_lon = 0;
	self->geolocation_lat = 0;

	MapIni_addFile(self, STD_INI_MAP_PATH);

	return self;
}

void MapIni_delete(MapIni* self)
{
	Std_deleteCHAR(self->geolocation_url);
	Std_deleteCHAR(self->tile_url);

	Std_deleteCHAR(self->copyright_text);
	Std_deleteCHAR(self->copyright_url);
	Std_deleteCHAR(self->fixmap_url);

	Std_deleteCHAR(self->geolocation_item);
	Std_deleteCHAR(self->geolocation_lon);
	Std_deleteCHAR(self->geolocation_lat);

	Os_free(self, sizeof(MapIni));
}

BOOL MapIni_is(const MapIni* self)
{
	return Std_isCHAR(self->geolocation_url) && Std_isCHAR(self->tile_url);
}

void MapIni_updateNet(MapIni* self)
{
	if (MapIni_is(self))
	{
		MediaNetwork_addDelay(self->geolocation_url, self->geolocation_delay);
		MediaNetwork_addDelay(self->tile_url, self->tile_delay);
	}
}

BOOL MapIni_tryDownload(MapIni* self)
{
	if (!MapIni_is(self))
	{
		UCHAR* buff;
		BIG bytes = MediaNetwork_download(STD_INI_MAP_URL, 0.1, &buff);
		if (bytes > 0)
		{
			MapIni_addBuffer(self, (char*)buff, bytes);
			Os_free(buff, bytes);
		}

		MapIni_updateNet(self);
	}
	return MapIni_is(self);
}

BOOL MapIni_getPosFrom(const MapIni* self, const char* text, Vec2f* out_pos)
{
	*out_pos = Vec2f_init();

	const char* itemStr = MapIni_get_geolocation_item(self);
	const char* xStr = MapIni_get_geolocation_lon(self);
	const char* yStr = MapIni_get_geolocation_lat(self);

	const char* st = Std_findSubCHAR(text, itemStr);
	if (st)
	{
		const char* stX = Std_findSubCHAR(st, xStr);
		if (stX)
		{
			stX += Std_sizeCHAR(xStr);
			out_pos->x = Os_atof(stX);
		}

		const char* stY = Std_findSubCHAR(st, yStr);
		if (stY)
		{
			stY += Std_sizeCHAR(yStr);
			out_pos->y = Os_atof(stY);
		}

		return stX != 0 && stY != 0;
	}

	return FALSE;
}
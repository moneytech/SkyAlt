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

 //add translation: same state, but with multiple names ...

static void _MapPolyPolygon_resize(MapPolyPolygon* self, UBIG n)
{
	self->num_poses = n;
	self->poses = Os_realloc(self->poses, self->num_poses * sizeof(MapPolyPos));
}
static MapPolyPos* _MapPolyPolygon_add(MapPolyPolygon* self)
{
	UBIG p = self->num_poses;
	_MapPolyPolygon_resize(self, self->num_poses + 1);
	return &self->poses[p];
}
static void _MapPolyPolygon_computeBBox(MapPolyPolygon* self)
{
	Vec2f mn = Vec2f_init2(Std_maxDouble(), Std_maxDouble());
	Vec2f mx = Vec2f_init2(Std_minDouble(), Std_minDouble());
	BIG i;
	for (i = 0; i < self->num_poses; i++)
	{
		Vec2f v = Vec2f_init2(self->poses[i].x, self->poses[i].y);
		mn = Vec2f_min(mn, v);
		mx = Vec2f_max(mx, v);
	}

	self->bbox = self->num_poses ? Quad2f_init4(mn.x, mn.y, mx.x - mn.x, mx.y - mn.y) : Quad2f_init();
}
MapPolyPolygon MapPolyPolygon_init(UBIG num_poses)
{
	MapPolyPolygon self;
	self.poses = Os_malloc(num_poses * sizeof(MapPolyPos));
	self.num_poses = num_poses;
	return self;
}
MapPolyPolygon MapPolyPolygon_initRead(OsFile* file)
{
	UBIG num_poses;
	OsFile_read(file, &num_poses, sizeof(UBIG));

	MapPolyPolygon self = MapPolyPolygon_init(num_poses);

	OsFile_read(file, self.poses, num_poses * sizeof(MapPolyPos));

	_MapPolyPolygon_computeBBox(&self);

	return self;
}
void MapPolyPolygon_free(MapPolyPolygon* self)
{
	Os_free(self->poses, self->num_poses * sizeof(MapPolyPos));
}
void MapPolyPolygon_write(const MapPolyPolygon* self, OsFile* file)
{
	OsFile_write(file, &self->num_poses, sizeof(UBIG));
	OsFile_write(file, self->poses, self->num_poses * sizeof(MapPolyPos));
}
UBIG MapPolyPolygon_bytes(const MapPolyPolygon* self)
{
	return sizeof(UBIG) + self->num_poses * sizeof(MapPolyPos);
}

static void _MapPolyIndex_resize(MapPolyIndex* self, UBIG n)
{
	self->polygons = Os_realloc(self->polygons, n * sizeof(MapPolyPolygon));

	if (n > self->num_polygons)
		Os_memset(&self->polygons[self->num_polygons], (n - self->num_polygons) * sizeof(MapPolyPolygon));

	self->num_polygons = n;
}
static MapPolyPolygon* _MapPolyIndex_add(MapPolyIndex* self)
{
	UBIG p = self->num_polygons;
	_MapPolyIndex_resize(self, self->num_polygons + 1);
	return &self->polygons[p];
}

void MapPolyIndex_clearPolygon(MapPolyIndex* self)
{
	BIG i;
	for (i = 0; i < self->num_polygons; i++)
		MapPolyPolygon_free(&self->polygons[i]);
	Os_free(self->polygons, self->num_polygons * sizeof(MapPolyPolygon));

	self->num_polygons = 0;
	self->polygons = 0;
}

void MapPolyIndex_free(MapPolyIndex* self)
{
	MapPolyIndex_clearPolygon(self);
	Std_deleteCHAR(self->name);
}

void MapPolyIndex_read(MapPolyIndex* self, const char* path)
{
	if (self->load_time > 0)
		return;

	OsFile file;
	if (OsFile_init(&file, path, OsFile_R))
	{
		OsFile_seekAbs(&file, self->file_pos);

		float x, y;
		OsFile_read(&file, &x, sizeof(x));
		OsFile_read(&file, &y, sizeof(y));
		self->pos.x = x;
		self->pos.y = y;

		OsFile_read(&file, &self->num_polygons, sizeof(UBIG));
		self->polygons = Os_malloc(self->num_polygons * sizeof(MapPolyPolygon));
		BIG i;
		for (i = 0; i < self->num_polygons; i++)
			self->polygons[i] = MapPolyPolygon_initRead(&file);

		OsFile_free(&file);
		self->load_time = Os_time();
	}
}

void MapPolyIndex_write(const MapPolyIndex* self, OsFile* file)
{
	//UBIG file_pos = OsFile_getSeekPos(file);
	//if (file_pos != self->file_pos)
	//	printf("Error: MapPolyIndex_write()\n");

	float x = self->pos.x;
	float y = self->pos.y;
	OsFile_write(file, &x, sizeof(x));
	OsFile_write(file, &y, sizeof(y));

	OsFile_write(file, &self->num_polygons, sizeof(UBIG));
	BIG i;
	for (i = 0; i < self->num_polygons; i++)
		MapPolyPolygon_write(&self->polygons[i], file);
}

void MapPolyIndex_maintenance(MapPolyIndex* self, double dt)
{
	if (Os_time() - self->load_time > dt)
	{
		MapPolyIndex_clearPolygon(self);
		self->load_time = 0;
	}
}

UBIG MapPolyIndex_bytes(const MapPolyIndex* self)
{
	BIG i;
	UBIG sum = 0;

	//pos
	sum += sizeof(float);
	sum += sizeof(float);

	//polygons
	sum += sizeof(UBIG);
	for (i = 0; i < self->num_polygons; i++)
		sum += MapPolyPolygon_bytes(&self->polygons[i]);

	return sum;
}

typedef struct MapPoly_s
{
	char* path;
	MapPolyIndex* items;
	UBIG num_items;
} MapPoly;

static void _MapPoly_resize(MapPoly* self, UBIG n)
{
	self->items = Os_realloc(self->items, n * sizeof(MapPolyIndex));

	if (n > self->num_items)
		Os_memset(&self->items[self->num_items], (n - self->num_items) * sizeof(MapPolyIndex));

	self->num_items = n;
}

MapPoly* MapPoly_new(const char* path)
{
	MapPoly* self = Os_calloc(1, sizeof(MapPoly));
	self->path = Std_newCHAR(path);
	return self;
}

MapPoly* MapPoly_newRead(const char* path, BOOL printWarning)
{
	MapPoly* self = MapPoly_new(path);

	OsFile file;
	if (OsFile_init(&file, path, OsFile_R))
	{
		UBIG num_items;
		OsFile_read(&file, &num_items, sizeof(UBIG));
		_MapPoly_resize(self, num_items);

		//read indexes
		BIG i;
		for (i = 0; i < num_items; i++)
		{
			//file_pos
			OsFile_read(&file, &self->items[i].file_pos, sizeof(UBIG));

			//name
			UBIG nameSize;
			OsFile_read(&file, &nameSize, sizeof(UBIG));
			self->items[i].name = Std_newCHAR_N(nameSize);
			OsFile_read(&file, self->items[i].name, nameSize);
		}

		OsFile_free(&file);
	}
	else
		if (printWarning)
			printf("Warning: MapTiles cache file(%s) is missing\n", path);

	return self;
}

void MapPoly_delete(MapPoly* self)
{
	BIG i;
	for (i = 0; i < self->num_items; i++)
		MapPolyIndex_free(&self->items[i]);
	Os_free(self->items, self->num_items * sizeof(MapPolyIndex));

	Std_deleteCHAR(self->path);

	Os_free(self, sizeof(MapPoly));
}

void MapPoly_maintenance(MapPoly* self, double dt)
{
	BIG i;
	for (i = 0; i < self->num_items; i++)
		MapPolyIndex_maintenance(&self->items[i], dt);
}

UBIG MapPoly_bytesIndexes(const MapPoly* self)
{
	UBIG sum = 0;

	sum += sizeof(UBIG);	//num_indexes

	BIG i;
	for (i = 0; i < self->num_items; i++)
	{
		//file_pos
		sum += sizeof(UBIG);

		//name
		UBIG nameSize = Std_sizeCHAR(self->items[i].name);
		sum += sizeof(UBIG);
		sum += nameSize;
	}
	return sum;
}
UBIG MapPoly_bytes(const MapPoly* self)
{
	UBIG sum = 0;
	sum += MapPoly_bytesIndexes(self);

	BIG i;
	for (i = 0; i < self->num_items; i++)
		sum += MapPolyIndex_bytes(&self->items[i]);
	return sum;
}

UBIG _MapPoly_updateFilePoses(const MapPoly* self)
{
	UBIG pos = 0;

	//start indexes
	pos += MapPoly_bytesIndexes(self);

	//items
	BIG i;
	for (i = 0; i < self->num_items; i++)
	{
		self->items[i].file_pos = pos;
		pos += MapPolyIndex_bytes(&self->items[i]);
	}

	return pos;
}

void MapPoly_write(const MapPoly* self)
{
	_MapPoly_updateFilePoses(self);

	OsFile file;
	if (OsFile_init(&file, self->path, OsFile_W))
	{
		OsFile_write(&file, &self->num_items, sizeof(UBIG));

		//write indexes
		BIG i;
		for (i = 0; i < self->num_items; i++)
		{
			//file_pos
			OsFile_write(&file, &self->items[i].file_pos, sizeof(UBIG));

			//name
			UBIG nameSize = Std_sizeCHAR(self->items[i].name);
			OsFile_write(&file, &nameSize, sizeof(UBIG));
			OsFile_write(&file, self->items[i].name, nameSize);
		}

		//write data
		for (i = 0; i < self->num_items; i++)
			MapPolyIndex_write(&self->items[i], &file);

		OsFile_free(&file);
	}
}

const MapPolyIndex* MapPoly_find(const MapPoly* self, const UNI* name)
{
	BIG i;
	for (i = 0; i < self->num_items; i++)
	{
		MapPolyIndex* it = &self->items[i];

		if (Std_cmpUNI_CHAR_small(name, it->name))
		{
			MapPolyIndex_read(it, self->path);
			return it;
		}
	}
	return 0;
}

UBIG MapPoly_addPoly(MapPoly* self, const char* name, Vec2f posXY, const char* str)
{
	UBIG num_poly = 0;

	BIG pos = self->num_items;
	_MapPoly_resize(self, self->num_items + 1);

	MapPolyIndex* index = &self->items[pos];
	index->name = Std_newCHAR(name);
	index->pos = posXY;

	//poly
	MapPolyPolygon* polygon = 0;
	if (str[0] == '[')
	{
		str++;

		while (*str && *str != '}')
		{
			if (str[0] == '[' && str[1] == '[')
			{
				polygon = _MapPolyIndex_add(index);	//create new Polygon
				num_poly++;
				str++;
			}
			else
				if (str[0] == '[')
				{
					str++;

					double x = Os_atof(str);
					str = Std_findSubCHAR(str, ",") + 1;
					double y = Os_atof(str);
					str = Std_findSubCHAR(str, "]");

					MapPolyPos* pos = _MapPolyPolygon_add(polygon);	//add vertice to Polygon
					pos->x = x;
					pos->y = y;
				}
				else
					if (str[0] == ',')
						str++;
					else
						if (str[0] == ']')
							str++;
						else
						{
							printf("Error: [] - %s\n", str);
							break;
						}
		}
	}

	if (polygon)
		_MapPolyPolygon_computeBBox(polygon);

	return num_poly;
}

const char* g_countries[] =
{
"Afghanistan",
"Albania",
"Algeria",
"Andorra",
"Angola",
"Antiguaand Barbuda",
"Argentina",
"Armenia",
"Australia",
"Austria",
"Azerbaijan",
"Bahamas",
"Bahrain",
"Bangladesh",
"Barbados",
"Belarus",
"Belgium",
"Belize",
"Benin",
"Bhutan",
"Bolivia",
"Bosnia and Herzegovina",
"Botswana",
"Brazil",
"Brunei",
"Bulgaria",
"Burkina Faso",
"Burundi",
"Côte d'Ivoire",
"Cabo Verde",
"Cambodia",
"Cameroon",
"Canada",
"CAR",
"Chad",
"Chile",
"China",
"Colombia",
"Comoros",
"Congo",
"Costa Rica",
"Croatia",
"Cuba",
"Cyprus",
"Czechia",
"Czech Republic",
"Denmark",
"Djibouti",
"Dominica",
"Dominican Republic",
"DPRK",
"DRC",
"Ecuador",
"Egypt",
"El Salvador",
"Equatorial Guinea",
"Eritrea",
"Estonia",
"Eswatini",
"Ethiopia",
"Fiji",
"Finland",
"France",
"Gabon",
"Gambia",
"Georgia",
"Germany",
"Ghana",
"Greece",
"Grenada",
"Guatemala",
"Guinea",
"Guinea - Bissau",
"Guyana",
"Haiti",
"Holy See",
"Honduras",
"Hungary",
"Iceland",
"India",
"Indonesia",
"Iran",
"Iraq",
"Ireland",
"Israel",
"Italy",
"Jamaica",
"Japan",
"Jordan",
"Kazakhstan",
"Kenya",
"Kiribati",
"Kuwait",
"Kyrgyzstan",
"Laos",
"Latvia",
"Lebanon",
"Lesotho",
"Liberia",
"Libya",
"Liechtenstein",
"Lithuania",
"Luxembourg",
"Madagascar",
"Malawi",
"Malaysia",
"Maldives",
"Mali",
"Malta",
"Marshall Islands",
"Mauritania",
"Mauritius",
"Mexico",
"Micronesia",
"Moldova",
"Monaco",
"Mongolia",
"Montenegro",
"Morocco",
"Mozambique",
"Myanmar",
"Namibia",
"Nauru",
"Nepal",
"Netherlands",
"New Zealand",
"Nicaragua",
"Niger",
"Nigeria",
"North Macedonia",
"Norway",
"Oman",
"Pakistan",
"Palau",
"Panama",
"Papua New Guinea",
"Paraguay",
"Peru",
"Philippines",
"Poland",
"Portugal",
"Qatar",
"Romania",
"Russia",
"Rwanda",
"Saint Kittsand Nevis",
"Saint Lucia",
"Samoa",
"San Marino",
"Sao Tomeand Principe",
"Saudi Arabia",
"Senegal",
"Serbia",
"Seychelles",
"Sierra Leone",
"Singapore",
"Slovakia",
"Slovenia",
"Solomon Islands",
"Somalia",
"South Africa",
"South Korea",
"South Sudan",
"Spain",
"Sri Lanka",
"St.Vincent Grenadines",
"State of Palestine",
"Sudan",
"Suriname",
"Sweden",
"Switzerland",
"Syria",
"Tajikistan",
"Tanzania",
"Thailand",
"Timor - Leste",
"Togo",
"Tonga",
"Trinidadand Tobago",
"Tunisia",
"Turkey",
"Turkmenistan",
"Tuvalu",
"U.A.E.",
"U.K.",
"U.S.",
"Uganda",
"Ukraine",
"Uruguay",
"Uzbekistan",
"Vanuatu",
"Venezuela",
"Vietnam",
"Yemen",
"Zambia",
"Zimbabwe",

"Alabama",
"Alaska",
"Arizona",
"Arkansas",
"California",
"Colorado",
"Connecticut",
"Delaware",
"Florida",
"Georgia",
"Hawaii",
"Idaho",
"Illinois",
"Indiana",
"Iowa",
"Kansas",
"Kentucky",
"Louisiana",
"Maine",
"Maryland",
"Massachusetts",
"Michigan",
"Minnesota",
"Mississippi",
"Missouri",
"Montana",
"Nebraska",
"Nevada",
"New Hampshire",
"New Jersey",
"New Mexico",
"New York",
"North Carolina",
"North Dakota",
"Ohio",
"Oklahoma",
"Oregon",
"Pennsylvania",
"Rhode Island",
"South Carolina",
"South Dakota",
"Tennessee",
"Texas",
"Utah",
"Vermont",
"Virginia",
"Washington",
"West Virginia",
"Wisconsin",
"Wyoming",
};

void MapPoly_createCache(void)
{
	MapPoly* map = MapPoly_new("map.poly");

	double time = Os_time();
	const int N = sizeof(g_countries) / sizeof(char*);
	int i;
	for (i = 0; i < N; i++)
	{
		UNI* nameUni = Std_newUNI_char(g_countries[i]);
		char* name = Std_newCHAR_urlEncode(nameUni);
		char url[256];
		snprintf(url, 256, "https://nominatim.openstreetmap.org/search?q=%s&polygon_geojson=1&polygon_threshold=0.01&format=xml&limit=1", name);
		Std_deleteUNI(nameUni);
		Std_deleteCHAR(name);

		UBIG num_poly = 0;
		BOOL running = TRUE;
		float done;
		char* data = 0;
		BIG bytes = OsHTTPS_downloadWithStatus(url, &done, &running, &data);
		if (bytes >= 0)
		{
			Std_removeLetterCHAR(data, ' ');	//remove all spaces

			//pos
			Vec2f pos;
			{
				const char* posX = Std_findSubCHAR(data, "lon='");
				if (posX)
					posX += Std_sizeCHAR("lon='");
				pos.x = posX ? Os_atof(posX) : 0;

				const char* posY = Std_findSubCHAR(data, "lat='");
				if (posY)
					posY += Std_sizeCHAR("lat='");
				pos.y = posY ? Os_atof(posY) : 0;
			}

			//poly
			const char* findStart = "\"coordinates\":";
			const char* start = Std_findSubCHAR(data, findStart);
			if (start)
			{
				start += Std_sizeCHAR(findStart);
				num_poly = MapPoly_addPoly(map, g_countries[i], pos, start);
			}

			Os_free(data, bytes);
		}

		double cell_time = Os_time() - time;
		printf("%d from %d (%.1f%% - %.1fmin) - %s - %s - %lldpoly - %.3fKB\n", i, N, i / (float)N * 100, (i > 0) ? cell_time / i * (N - i) / 60.0 : 0, bytes ? "ok" : "error", g_countries[i], num_poly, (num_poly) ? MapPolyIndex_bytes(&map->items[map->num_items - 1]) / 1024.0f : 0);

		OsThread_sleep(1500);
	}

	//MapPoly_simplify(Map, 0.1f);
	MapPoly_write(map);
	MapPoly_delete(map);

	//test - remove ...
	map = MapPoly_newRead("map.poly", FALSE);
	MapPoly_delete(map);

	printf("done\n");
}

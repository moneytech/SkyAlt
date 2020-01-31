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

#define GuiItemMap_MAX_ZOOM 18	//<0, 19>
#define GuiItemMap_RES 256

typedef struct GuiItemMapItem_s
{
	BIG row;
	Vec2f pos;
	int rad;

	StdArr labels;	//<UNI*>
	Rgba cd;

	const MapPolyIndex* polyInd;

	BOOL check;

	struct GuiItemMapItem_s* next;
} GuiItemMapItem;

void GuiItemMapItem_setLabel(GuiItemMapItem* self, BIG i, const UNI* str)
{
	Std_replaceUNI((UNI**)&self->labels.ptrs[i], str);
}
void GuiItemMapItem_setNumLabels(GuiItemMapItem* self, BIG N)
{
	StdArr_resize(&self->labels, N);
}

GuiItemMapItem* GuiItemMapItem_new(BIG row, Vec2f pos, const MapPolyIndex* polyInd, int rad, BIG num_labels, Rgba cd)
{
	GuiItemMapItem* self = Os_malloc(sizeof(GuiItemMapItem));
	self->row = row;
	self->pos = pos;
	self->rad = rad;
	self->cd = cd;
	self->check = TRUE;
	self->polyInd = polyInd;
	self->labels = StdArr_init();
	GuiItemMapItem_setNumLabels(self, num_labels);

	self->next = 0;
	return self;
}
GuiItemMapItem* GuiItemMapItem_delete(GuiItemMapItem* self)
{
	GuiItemMapItem* next = self->next;

	StdArr_freeFn(&self->labels, (StdArrFREE)&Std_deleteUNI);

	Os_free(self, sizeof(GuiItemMapItem));

	return next;
}

BOOL GuiItemMapItem_updateLabels(GuiItemMapItem* self, BIG row, Vec2f pos, const MapPolyIndex* polyInd, int rad, BIG num_labels, Rgba cd)
{
	BOOL changed = (self->row != row || !Vec2f_cmp(self->pos, pos) || self->rad != rad || !Rgba_cmp(self->cd, cd) || self->labels.num != num_labels || self->polyInd != polyInd);

	self->row = row;
	self->pos = pos;
	self->rad = rad;
	self->cd = cd;
	GuiItemMapItem_setNumLabels(self, num_labels);
	self->check = TRUE;
	self->polyInd = polyInd;

	return changed;
}

BOOL GuiItemMapItem_cmp(GuiItemMapItem* self, Vec2f pos, int rad)
{
	return Vec2f_cmp(self->pos, pos) && self->rad == rad;
}

Vec2i GuiItemMapItem_getMapPosEx(Vec2f pos, Vec2i start, Quad2f bbox, int zoom)
{
	Vec2f tilePos = StdMap_getTile(pos.x, pos.y, zoom);
	Vec2i mid = Vec2i_init2((int)((tilePos.x - bbox.start.x) * GuiItemMap_RES), (int)((tilePos.y - bbox.start.y) * GuiItemMap_RES));
	mid = Vec2i_add(start, mid);
	return mid;
}
Vec2i GuiItemMapItem_getMapPos(GuiItemMapItem* self, Vec2i start, Quad2f bbox, int zoom)
{
	return GuiItemMapItem_getMapPosEx(self->pos, start, bbox, zoom);
}

typedef enum
{
	GuiItemMap_ICON = 0,
	GuiItemMap_CIRCLE,
	GuiItemMap_AREA,
}GuiItemMapTYPE;

typedef struct GuiItemMap_s
{
	GuiItem base;
	BIG viewRow;
	DbRows filter;

	DbValue cam_lat;
	DbValue cam_long;
	DbValue cam_zoom;

	DbRows location;
	DbValue map_render;

	DbRows item_radius;
	DbValue item_radius_multiplier;
	DbRows item_color;
	DbValue item_color_cd1;
	DbValue item_color_cd2;

	//DbValue item_label_location;
	DbValue item_label_center;

	DbValue search;

	BOOL focusItems;
	BOOL focusSearch;

	Image1 locationMark;

	Vec2i startTouch;
	Vec2f startTile;

	BIG clickMarkRow;
	BIG clickCopyright;
	BIG clickFixthemap;

	GuiItemMapItem* items;

	GuiItemMapItem* searchItem;

	BOOL allGeolocated;
} GuiItemMap;

static void _GuiItemMap_updateIcon(GuiItemMap* self)
{
	const int rad = (int)Std_max(1, DbValue_getNumber(&self->item_radius_multiplier)) * OsWinIO_cellSize();
	if (self->locationMark.size.x != rad)
	{
		Image1_resize(&self->locationMark, Vec2i_init2(rad, rad));
		Image1 srcIcon = UiIcons_init_locator();
		Image1_scale(&self->locationMark, &srcIcon);
		Image1_free(&srcIcon);
	}
}

static BOOL _GuiItemMap_getSearchPos(GuiItemMap* self, Vec2f* out_pos, const MapPolyIndex** polyInd)
{
	*out_pos = Vec2f_init();

	const UNI* addr = DbValue_result(&self->search);
	if (Std_sizeUNI(addr))
		return DbRoot_searchMapLocation(addr, out_pos, polyInd);

	return TRUE;
}

GuiItem* GuiItemMap_new(Quad2i grid, BIG viewRow, DbRows filter, DbValue cam_lat, DbValue cam_long, DbValue cam_zoom, DbValue search)
{
	GuiItemMap* self = Os_malloc(sizeof(GuiItemMap));
	self->base = GuiItem_init(GuiItem_MAP, grid);

	self->viewRow = viewRow;
	self->filter = filter;

	self->cam_lat = cam_lat;
	self->cam_long = cam_long;
	self->cam_zoom = cam_zoom;

	self->search = search;

	self->location = (viewRow >= 0) ? DbRows_initRefLink(viewRow, "location") : DbRows_initEmpty();
	self->map_render = (viewRow >= 0) ? DbValue_initOption(viewRow, "map_render", 0) : DbValue_initEmpty();

	self->item_radius = (viewRow >= 0) ? DbRows_initRefLink(viewRow, "radius") : DbRows_initEmpty();
	self->item_radius_multiplier = (viewRow >= 0) ? DbRows_getSubOption(viewRow, "radius", "multiplier", 0) : DbValue_initEmpty();

	self->item_color = (viewRow >= 0) ? DbRows_initRefLink(viewRow, "color") : DbRows_initEmpty();
	self->item_color_cd1 = (viewRow >= 0) ? DbRows_getSubOption(viewRow, "color", "cd1", _UNI32("4280690392")) : DbValue_initEmpty();	//red
	self->item_color_cd2 = (viewRow >= 0) ? DbRows_getSubOption(viewRow, "color", "cd2", _UNI32("4280735835")) : DbValue_initEmpty();	//green

	//self->item_label_location = (viewRow >= 0) ? DbValue_initOption(viewRow, "label_location", _UNI32("1")) : DbValue_initEmpty();
	self->item_label_center = (viewRow >= 0) ? DbValue_initOption(viewRow, "label_center", _UNI32("1")) : DbValue_initEmpty();

	self->focusItems = FALSE;
	self->focusSearch = FALSE;
	self->allGeolocated = FALSE;

	self->locationMark = Image1_init();
	_GuiItemMap_updateIcon(self);

	self->clickMarkRow = -1;
	self->clickCopyright = FALSE;
	self->clickFixthemap = FALSE;

	self->items = 0;
	self->searchItem = GuiItemMapItem_new(-1, Vec2f_init(), 0, 1, 0, g_theme.main);

	return (GuiItem*)self;
}

GuiItem* GuiItemMap_newCopy(GuiItemMap* src, BOOL copySub)
{
	GuiItemMap* self = Os_malloc(sizeof(GuiItemMap));
	*self = *src;
	GuiItem_initCopy(&self->base, &src->base, copySub);

	self->filter = DbRows_initCopy(&src->filter);
	self->cam_lat = DbValue_initCopy(&src->cam_lat);
	self->cam_long = DbValue_initCopy(&src->cam_long);
	self->cam_zoom = DbValue_initCopy(&src->cam_zoom);
	self->search = DbValue_initCopy(&src->search);

	self->location = DbRows_initCopy(&src->location);
	self->map_render = DbValue_initCopy(&src->map_render);

	self->item_radius = DbRows_initCopy(&src->item_radius);
	self->item_radius_multiplier = DbValue_initCopy(&src->item_radius_multiplier);
	self->item_color = DbRows_initCopy(&src->item_color);
	self->item_color_cd1 = DbValue_initCopy(&src->item_color_cd1);
	self->item_color_cd2 = DbValue_initCopy(&src->item_color_cd2);
	//self->item_label_location = DbValue_initCopy(&src->item_label_location);
	self->item_label_center = DbValue_initCopy(&src->item_label_center);

	self->items = 0;
	self->searchItem = GuiItemMapItem_new(-1, Vec2f_init(), 0, 1, 0, g_theme.main);

	self->locationMark = Image1_initCopy(&src->locationMark);

	return (GuiItem*)self;
}

static void _GuiItemMap_undownload(GuiItemMap* self, Quad2i coord);

void GuiItemMap_clearItems(GuiItemMap* self)
{
	_GuiItemMap_undownload(self, self->base.coordScreen);

	GuiItemMapItem* it = self->items;
	while (it)
		it = GuiItemMapItem_delete(it);
}

void GuiItemMap_delete(GuiItemMap* self)
{
	GuiItemMap_clearItems(self);

	Image1_free(&self->locationMark);

	GuiItemMapItem_delete(self->searchItem);

	DbRows_free(&self->filter);
	DbValue_free(&self->cam_lat);
	DbValue_free(&self->cam_long);
	DbValue_free(&self->cam_zoom);
	DbValue_free(&self->search);

	DbRows_free(&self->location);
	DbValue_free(&self->map_render);

	DbRows_free(&self->item_radius);
	DbValue_free(&self->item_radius_multiplier);
	DbRows_free(&self->item_color);
	DbValue_free(&self->item_color_cd1);
	DbValue_free(&self->item_color_cd2);
	//DbValue_free(&self->item_label_location);
	DbValue_free(&self->item_label_center);

	GuiItem_free(&self->base);
	Os_free(self, sizeof(GuiItemMap));
}

BIG GuiItemMap_getBaseRow(GuiItemMap* self)
{
	return DbRows_getBaseRow(&self->filter);
}

void GuiItemMap_setBaseRow(GuiItemMap* self, BIG row)
{
	DbRows_setBaseRow(&self->filter, row);
}

void GuiItemMap_focusItems(GuiItemMap* self)
{
	self->focusItems = TRUE;
}
void GuiItemMap_focusSearch(GuiItemMap* self)
{
	self->focusSearch = TRUE;
}

UBIG GuiItemMap_numLabels(const GuiItemMap* self)
{
	return (self->viewRow >= 0) ? DbRows_getSubsNum(self->viewRow, "label_columns", TRUE) : 0;
}
DbColumn* GuiItemMap_getLabel(const GuiItemMap* self, UBIG i)
{
	return (self->viewRow >= 0) ? DbRows_getSubsColumn(self->viewRow, "label_columns", TRUE, i) : 0;
}
StdArr GuiItemMap_getLabels(const GuiItemMap* self)
{
	return (self->viewRow >= 0) ? DbRows_getSubsColumns(self->viewRow, "label_columns", TRUE) : StdArr_init();
}
BIG GuiItemMap_getLabelRow(const GuiItemMap* self, UBIG i)
{
	return (self->viewRow >= 0) ? DbRows_getSubsRow(self->viewRow, "label_columns", TRUE, i) : 0;
}

void GuiItemMap_clickShowPage(GuiItemMap* self, BIG row)
{
	GuiItemLayout* card = GuiItemTable_buildPage(self->viewRow, row, TRUE, TRUE);// , FALSE);
	GuiItemRoot_addDialogRel(&card->base, &self->base, Quad2i_init2(OsWinIO_getTouchPos(), Vec2i_init2(1, 1)), TRUE);
}

static Vec3f _GuiItemMap_getCam(const GuiItemMap* self)
{
	return Vec3f_init3(DbValue_getNumber(&self->cam_long), DbValue_getNumber(&self->cam_lat), DbValue_getNumber(&self->cam_zoom));
}

static Quad2f _GuiItemMap_getBBox(Vec3f cam, Quad2i coord)
{
	return StdMap_lonLatToTileBbox(coord.size, GuiItemMap_RES, cam.x, cam.y, (int)cam.z);
}

GuiItemMapTYPE _GuiItemMap_getMapType(const GuiItemMap* self)
{
	return DbValue_getNumber(&self->map_render);
}

int _GuiItemMap_getMarkMoveY(const GuiItemMap* self)
{
	if (_GuiItemMap_getMapType(self) == GuiItemMap_CIRCLE)
		return 0;
	else
		return self->locationMark.size.y;
}

Quad2i _GuiItemMap_getTileCoord(int x, int y, Quad2i coord, Quad2f bbox)
{
	return Quad2i_init4((int)(coord.start.x + (x - bbox.start.x) * GuiItemMap_RES), (int)(coord.start.y + (y - bbox.start.y) * GuiItemMap_RES), GuiItemMap_RES, GuiItemMap_RES);
}

static void _GuiItemMap_drawIcon(const GuiItemMap* self, Image4* img, Vec2i mid, Rgba cd)
{
	Vec2i t = mid;
	t.y -= (int)(self->locationMark.size.y / 1.5f);
	Image4_drawCircle(img, t, (int)(self->locationMark.size.y * 0.2f), g_theme.black);

	mid.x -= self->locationMark.size.x / 2;
	mid.y -= self->locationMark.size.y;
	Image4_copyImage1(img, mid, cd, &self->locationMark);
}

void GuiItemMap_drawPolyLines(Image4* img, Quad2i coord, const Quad2f bbox, const int zoom, MapPolyPolygon* polygon, int width, Rgba cd)
{
	if (polygon->num_poses < 2)
		return;

	//is on screen?
	{
		Vec2i st = GuiItemMapItem_getMapPosEx(polygon->bbox.start, coord.start, bbox, zoom);
		Vec2i en = GuiItemMapItem_getMapPosEx(Quad2f_end(polygon->bbox), coord.start, bbox, zoom);
		Quad2i bb = Quad2i_repair(Quad2i_initEnd(st, en), 0);
		Vec2i rr = Quad2i_getIntersect(coord, bb).size;
		if (!Vec2i_is(rr))
			return;
	}

	//printf("draw()\n");

	const UBIG N = polygon->num_poses;
	float* xy = Os_malloc(N * sizeof(float) * 2);
	BIG i;
	for (i = 0; i < N; i++)
	{
		Vec2i v = GuiItemMapItem_getMapPosEx(Vec2f_init2(polygon->poses[i].x, polygon->poses[i].y), coord.start, bbox, zoom);	//transform lat/lon to map
		xy[i * 2 + 0] = v.x;
		xy[i * 2 + 1] = v.y;
	}
	Image4_drawPolyFill(img, xy, N, cd, 0.7f);

	Os_free(xy, N * sizeof(float) * 2);

	/*
	const UBIG N = polygon->num_poses;
	Vec2i first = GuiItemMapItem_getMapPosEx(Vec2f_init2(polygon->poses[0].x, polygon->poses[0].y), coord.start, bbox, zoom);
	Vec2i last = first;
	BIG i;
	for (i = 1; i < N; i++)
	{
		Vec2i next = GuiItemMapItem_getMapPosEx(Vec2f_init2(polygon->poses[i].x, polygon->poses[i].y), coord.start, bbox, zoom);
		Image4_drawLine(img, last, next, width, cd);
		last = next;
	}
	Image4_drawLine(img, last, first, width, cd);*/
}

void GuiItemMap_draw(GuiItemMap* self, Image4* img, Quad2i coord, Win* win)
{
	int textH = _GuiItem_textSize(0, OsWinIO_cellSize());
	OsFont* font = OsWinIO_getFontDefault();

	Vec3f cam = _GuiItemMap_getCam(self);
	const Quad2f bbox = _GuiItemMap_getBBox(cam, coord);
	const int zoom = (int)cam.z;

	//map background
	const Vec2i mid = Vec2i_init2(bbox.start.x + bbox.size.x / 2, bbox.start.y + bbox.size.y / 2);
	const double maxDistance = Vec2f_len(bbox.size);
	int x, y;
	for (y = (int)bbox.start.y; y < Quad2f_end(bbox).y; y++)
	{
		for (x = (int)bbox.start.x; x < Quad2f_end(bbox).x; x++)
		{
			if (x < 0 || y < 0)
				continue;

			BOOL draw = FALSE;
			BOOL inLibrary = FALSE;
			Quad2i q = _GuiItemMap_getTileCoord(x, y, coord, bbox);

			char url[64];
			Map_getTileUrl(x, y, zoom, url);
			inLibrary = draw = MediaLibrary_hasImageBuffer(url, "png", q.size);
			if (!draw)
			{
				UCHAR* buff;
				BIG bytes = Map_findTile(x, y, zoom, &buff);
				if (bytes < 0)
				{
					bytes = MediaNetwork_download(url, 1 - (Vec2i_len(Vec2i_sub(mid, Vec2i_init2(x, y))) / maxDistance), &buff);
					if (bytes > 0)
						Map_addImage(x, y, zoom, buff, bytes);
				}

				draw = (bytes > 0);
				if (draw)
					inLibrary = MediaLibrary_addImageBuffer(url, "png", buff, bytes, q.size);
			}



			if (draw)
				draw = MediaLibrary_imageBufferDraw(url, "png", img, q);
			else
			{
				Rgba cdGrey = Rgba_aprox(Rgba_initWhite(), Rgba_initBlack(), 0.2f);
				Image4_drawBorder(img, q, 1, cdGrey);
				Image4_drawText(img, Quad2i_getMiddle(q), TRUE, font, Lang_find(MediaNetwork_is() ? "NET_LOADING" : "NET_OFFLINE"), textH, 0, cdGrey);
			}

			if (!draw && (inLibrary || MediaNetwork_is()))
				GuiItem_setRedraw(&self->base, TRUE);	//try again
		}
	}

	//location points
	GuiItemMapTYPE mapType = _GuiItemMap_getMapType(self);
	const BOOL lable_center = DbValue_getNumber(&self->item_label_center);

	const double radMult = DbValue_getNumber(&self->item_radius_multiplier);

	GuiItemMapItem* it = self->items;
	while (it)
	{
		Vec2i mid = GuiItemMapItem_getMapPos(it, coord.start, bbox, zoom);
		Rgba cd = it->cd;
		int rad = it->rad;

		const int labelH = textH + (OsWinIO_cellSize() / 8) * 2;

		//item
		if (mapType == GuiItemMap_ICON)
		{
			_GuiItemMap_drawIcon(self, img, mid, cd);

			if (!lable_center)
				mid.y -= self->locationMark.size.y;
		}
		else
			if (mapType == GuiItemMap_CIRCLE)
			{
				Image4_drawCircle(img, mid, rad, cd);

				if (!lable_center)
					mid.y -= rad;
			}
			else
				if (mapType == GuiItemMap_AREA)
				{
					if (it->polyInd)
					{
						BIG i;
						for (i = 0; i < it->polyInd->num_polygons; i++)
							GuiItemMap_drawPolyLines(img, coord, bbox, zoom, &it->polyInd->polygons[i], radMult, cd);
					}
				}

		const BIG numLabels = it->labels.num;

		if (lable_center)
			mid.y -= (numLabels * labelH) / 2 - labelH / 2;
		else
			mid.y -= (numLabels * labelH) - labelH / 2;

		//titles
		BIG ii;
		for (ii = 0; ii < numLabels; ii++)
		{
			Image4_drawTextBackground(img, Vec2i_init2(mid.x, mid.y), TRUE, font, it->labels.ptrs[ii], textH, FALSE, Rgba_initBlack(), Rgba_initWhite(), OsWinIO_cellSize() / 8);
			mid.y += labelH;
		}

		it = it->next;
	}

	//search
	if (Vec2f_is(self->searchItem->pos))
	{
		Vec2i mid = GuiItemMapItem_getMapPos(self->searchItem, coord.start, bbox, zoom);
		//_GuiItemMap_drawIcon(self, img, mid, self->searchItem.cd);
		Image4_drawCircleLine(img, mid, 20, 1, Rgba_initRed());
	}

	//scale
	double metersPerPixels = StdMap_getMetersPerPixel(DbValue_getNumber(&self->cam_lat), DbValue_getNumber(&self->cam_zoom));
	double metersPerCell = metersPerPixels * OsWinIO_cellSize();
	double meters = Std_roundUp(metersPerCell);
	double pixels = meters / metersPerCell * OsWinIO_cellSize();

	const UNI* unit = _UNI32("m");
	if (meters > 1000)
	{
		meters /= 1000;
		unit = _UNI32("km");
	}

	UNI txt[32];
	char nmbr[32];
	textH = _GuiItem_textSize(0, OsWinIO_cellSize() / 1.5);
	const int h = Std_max(5, OsWinIO_cellSize() / 8);
	Vec2i start = coord.start;
	start.x += OsWinIO_cellSize() / 2;
	start.y += coord.size.y - OsWinIO_cellSize() / 4 * 3;

	//black
	Image4_drawBoxQuad(img, Quad2i_init4(start.x, start.y, pixels, h), Rgba_initBlack());
	Image4_drawText(img, Vec2i_init2(start.x, start.y - h - textH / 2), TRUE, font, _UNI32("0"), textH, FALSE, Rgba_initBlack());

	//white
	start.x += pixels;
	Image4_drawBoxQuad(img, Quad2i_init4(start.x, start.y, pixels, h), Rgba_initWhite());
	if (meters < 0)	snprintf(nmbr, 30, "%.1f", meters);
	else			snprintf(nmbr, 30, "%d", (int)meters);
	Std_setUNI_char(txt, nmbr, 30);
	Image4_drawText(img, Vec2i_init2(start.x, start.y - h - textH / 2), TRUE, font, txt, textH, FALSE, Rgba_initBlack());

	//black
	start.x += pixels;
	Image4_drawBoxQuad(img, Quad2i_init4(start.x, start.y, pixels, h), Rgba_initBlack());
	if (meters < 0)	snprintf(nmbr, 30, "%.1f", meters * 2);
	else			snprintf(nmbr, 30, "%d", (int)meters * 2);
	Std_setUNI_char(txt, nmbr, 30);
	Image4_drawText(img, Vec2i_init2(start.x, start.y - h - textH / 2), TRUE, font, txt, textH, FALSE, Rgba_initBlack());

	//last text
	start.x += pixels;
	if (meters < 0)	snprintf(nmbr, 30, "%.1f", meters * 3);
	else			snprintf(nmbr, 30, "%d", (int)meters * 3);
	Std_setUNI_char(txt, nmbr, 30);
	Image4_drawText(img, Vec2i_init2(start.x, start.y - h - textH / 2), TRUE, font, txt, textH, FALSE, Rgba_initBlack());

	//unit
	Image4_drawText(img, Vec2i_init2(start.x + 5, start.y), FALSE, font, unit, textH, FALSE, Rgba_initBlack());

	//copyright
	start.x -= 3 * pixels;
	Image4_drawText(img, Vec2i_init2(start.x, start.y + OsWinIO_cellSize() / 2), FALSE, font, Lang_find("MAPS_CONTRIBUTORS"), textH, FALSE, Rgba_initBlack());

	//fixthemap
	Image4_drawText(img, Vec2i_init2(coord.start.x + coord.size.x - OsWinIO_cellSize() * 2, start.y + OsWinIO_cellSize() / 2), FALSE, font, Lang_find("MAPS_FIX_THE_MAP"), textH, FALSE, Rgba_initBlack());
}

static void _GuiItemMap_undownload(GuiItemMap* self, Quad2i coord)
{
	Vec3f cam = _GuiItemMap_getCam(self);
	const Quad2f bbox = _GuiItemMap_getBBox(cam, coord);

	int x, y;
	for (y = (int)bbox.start.y; y < Quad2f_end(bbox).y; y++)
	{
		for (x = (int)bbox.start.x; x < Quad2f_end(bbox).x; x++)
		{
			if (x < 0 || y < 0)
				continue;

			char url[64];
			Map_getTileUrl(x, y, (int)cam.z, url);
			MediaNetwork_undownload(url);
		}
	}
}

static void _GuiItemMap_resetFocus(GuiItemMap* self, Quad2i coord)
{
	Vec2f mn = Vec2f_init2(180, 90);
	Vec2f mx = Vec2f_init2(-180, -90);

	GuiItemMapItem* it = self->items;
	while (it)
	{
		mn = Vec2f_min(mn, it->pos);
		mx = Vec2f_max(mx, it->pos);
		it = it->next;
	}

	//mid
	Vec2f mid = Vec2f_aprox(mn, mx, 0.5f);

	//zoom
	int zoom = GuiItemMap_MAX_ZOOM;
	while (zoom > 0)
	{
		Vec2f s = StdMap_getTile(mn.x, mn.y, zoom);
		Vec2f e = StdMap_getTile(mx.x, mx.y, zoom);
		Vec2f sub = Vec2f_sub(e, s);
		sub = Vec2f_mulV(sub, GuiItemMap_RES);

		if (Std_fabs(sub.x) < coord.size.x && Std_fabs(sub.y) < coord.size.y)
			break;

		zoom--;
	}

	//save
	BOOL reset = !GuiItemRoot_hasChanges();

	DbValue_setNumber(&self->cam_long, mid.x);
	DbValue_setNumber(&self->cam_lat, mid.y);
	DbValue_setNumber(&self->cam_zoom, Std_max(1, zoom - 1));

	if (reset)
		GuiItemRoot_resetNumChanges();
}

static void _GuiItemMap_setMapItemLabels(GuiItemMapItem* it, BIG row, StdArr* labelColumns, StdString* labelTemp)
{
	BIG i;
	for (i = 0; i < labelColumns->num; i++)
	{
		StdString_empty(labelTemp);
		DbColumn_getStringCopyLong(labelColumns->ptrs[i], row, labelTemp);
		GuiItemMapItem_setLabel(it, i, labelTemp->str);
	}
}

static BOOL _GuiItemMap_findOrAdd(GuiItemMap* self, BIG row, Vec2f pos, const MapPolyIndex* polyInd, int rad, /*const UNI* name, BOOL label_location,*/ StdArr* labelColumns, StdString* labelTemp, Rgba cd)
{
	GuiItemMapItem** lastPtr = &self->items;
	GuiItemMapItem* it = self->items;
	while (it)
	{
		if (GuiItemMapItem_cmp(it, pos, rad))
		{
			BOOL changed = GuiItemMapItem_updateLabels(it, row, pos, polyInd, rad, labelColumns->num, cd);
			_GuiItemMap_setMapItemLabels(it, row, labelColumns, labelTemp);
			return changed;
		}
		lastPtr = &it->next;
		it = it->next;
	}

	*lastPtr = GuiItemMapItem_new(row, pos, polyInd, rad, labelColumns->num, cd);
	_GuiItemMap_setMapItemLabels(*lastPtr, row, labelColumns, labelTemp);
	return TRUE;
}

static void _GuiItemMap_removeUnused(GuiItemMap* self)
{
	GuiItemMapItem* it = self->items;
	while (it)
	{
		if (!it->check)
			it = GuiItemMapItem_delete(it);
		else
			it = it->next;
	}
}

static BOOL _GuiItemMap_updateItems(GuiItemMap* self)
{
	BOOL changed = FALSE;

	StdString name = StdString_init();
	StdString labelTemp = StdString_init();

	StdArr labelColumns = GuiItemMap_getLabels(self);

	const double radMult = DbValue_getNumber(&self->item_radius_multiplier);
	Rgba cd1 = DbValue_getCd(&self->item_color_cd1);
	Rgba cd2 = DbValue_getCd(&self->item_color_cd2);

	DbColumn* columnLocation = DbRoot_findColumn(DbRows_getRow(&self->location, 0));
	DbColumn* columnRad = DbRoot_findColumn(DbRows_getRow(&self->item_radius, 0));
	DbColumn* columnCd = DbRoot_findColumn(DbRows_getRow(&self->item_color, 0));

	//const BOOL label_location = DbValue_getNumber(&self->item_label_location);

	double cdMinV = 0;
	double cdMaxV = 0;
	if (columnCd)
		DbRows_getColumnMinMax(&self->filter, columnCd, &cdMinV, &cdMaxV);

	self->allGeolocated = TRUE;
	const UBIG N = DbRows_getSize(&self->filter);
	UBIG i;
	for (i = 0; i < N; i++)
	{
		BIG r = DbRows_getRow(&self->filter, i);
		if (columnLocation)
			DbColumn_getStringCopyLong(columnLocation, r, &name);

		if (Std_sizeUNI(name.str))
		{
			const MapPolyIndex* polyInd;
			Vec2f pos;
			if (DbRoot_searchMapLocation(name.str, &pos, &polyInd))
			{
				int mm = OsWinIO_cellSize() / 8;
				int rad = mm + (columnRad ? DbColumn_getFlt(columnRad, r, 0) * radMult * mm : mm * radMult);

				double t = columnCd ? (DbColumn_getFlt(columnCd, r, 0) - cdMinV) / (cdMaxV - cdMinV) : 0;
				Rgba cd = columnCd ? Rgba_aprox(cd1, cd2, t) : cd1;

				changed |= _GuiItemMap_findOrAdd(self, r, pos, polyInd, rad, &labelColumns, &labelTemp, cd);
			}
			else
				self->allGeolocated = FALSE;
		}
	}

	StdString_free(&name);
	StdString_free(&labelTemp);

	StdArr_freeBase(&labelColumns);

	_GuiItemMap_removeUnused(self);

	return changed;
}

void GuiItemMap_update(GuiItemMap* self, Quad2i coord, Win* win)
{
	_GuiItemMap_updateIcon(self);

	Vec3f cam = _GuiItemMap_getCam(self);
	const Quad2f bbox = _GuiItemMap_getBBox(cam, coord);
	const int zoom = (int)cam.z;

	//keep tiles in media library
	int x, y;
	for (y = (int)bbox.start.y; y < Quad2f_end(bbox).y; y++)
	{
		for (x = (int)bbox.start.x; x < Quad2f_end(bbox).x; x++)
		{
			char url[64];
			Map_getTileUrl(x, y, zoom, url);

			Quad2i q = _GuiItemMap_getTileCoord(x, y, coord, bbox);
			MediaLibrary_imageBufferUpdate(url, "png", q.size);
		}
	}

	//geolocation for addresses
	const BOOL filterChange = DbRows_hasChanged(&self->filter);
	if (!self->allGeolocated || filterChange)
	{
		if (_GuiItemMap_updateItems(self))
			GuiItem_setRedraw(&self->base, TRUE);
	}

	if (self->focusItems || DbValue_getNumber(&self->cam_zoom) == 0)
	{
		_GuiItemMap_resetFocus(self, coord);
		GuiItem_setRedraw(&self->base, TRUE);
		self->focusItems = FALSE;
	}

	if (DbValue_hasChanged(&self->search))
	{
		const MapPolyIndex* polyInd;
		_GuiItemMap_getSearchPos(self, &self->searchItem->pos, &polyInd);
		GuiItem_setRedraw(&self->base, TRUE);
	}

	if (self->focusSearch)
	{
		const MapPolyIndex* polyInd;
		if (_GuiItemMap_getSearchPos(self, &self->searchItem->pos, &polyInd))
		{
			BOOL reset = !GuiItemRoot_hasChanges();

			//change camera
			DbValue_setNumber(&self->cam_long, self->searchItem->pos.x);
			DbValue_setNumber(&self->cam_lat, self->searchItem->pos.y);
			DbValue_setNumber(&self->cam_zoom, 11);

			if (reset)
				GuiItemRoot_resetNumChanges();

			GuiItem_setRedraw(&self->base, TRUE);
			self->focusSearch = FALSE;
		}
	}

	//GuiItem_setRedraw(&self->base, (filterChange || DbRows_hasChanged(&self->location) || DbValue_hasChanged(&self->item_color_cd1) || DbValue_hasChanged(&self->item_color_cd2) || DbValue_hasChanged(&self->cam_lat) || DbValue_hasChanged(&self->cam_long) || DbValue_hasChanged(&self->cam_zoom) || DbValue_hasChanged(&self->map_render) || DbValue_hasChanged(&self->item_radius_multiplier)));
}

void GuiItemMap_touch(GuiItemMap* self, Quad2i coord, Win* win)
{
	Rgba back_cd = g_theme.black;
	Rgba front_cd = g_theme.warning;

	Vec3f cam = _GuiItemMap_getCam(self);
	const Quad2f bbox = _GuiItemMap_getBBox(cam, coord);
	const int zoom = (int)cam.z;

	if (self->base.touch && GuiItem_isEnable(&self->base) && OsWinIO_canActiveRenderItem(self))
	{
		BOOL inside = Quad2i_inside(coord, OsWinIO_getTouchPos());
		if (OsWinIO_getTouch_action() == Win_TOUCH_WHEEL && inside)
		{
			BOOL reset = !GuiItemRoot_hasChanges();

			_GuiItemMap_undownload(self, coord);	//first

			if (OsWinIO_getTouch_wheel() < 0 && zoom < GuiItemMap_MAX_ZOOM)	//zoom in
			{
				Vec2f rel;
				rel.x = (OsWinIO_getTouchPos().x - coord.start.x) / (float)coord.size.x;
				rel.y = (OsWinIO_getTouchPos().y - coord.start.y) / (float)coord.size.y;

				Vec2f touch_longLat = Vec2f_add(bbox.start, Vec2f_mul(bbox.size, rel));
				Vec2f longLat = Vec2f_aprox(StdMap_getTile(cam.x, cam.y, zoom), touch_longLat, 0.25f);	//go only 1/4 closer

				longLat = StdMap_getLonLat(longLat.x, longLat.y, zoom);
				DbValue_setNumber(&self->cam_long, longLat.x);
				DbValue_setNumber(&self->cam_lat, longLat.y);
			}

			DbValue_setNumber(&self->cam_zoom, Std_clamp(zoom - OsWinIO_getTouch_wheel(), 1, GuiItemMap_MAX_ZOOM));
			OsWinIO_resetTouch();

			GuiItem_setRedraw(&self->base, TRUE);

			if (reset)
				GuiItemRoot_resetNumChanges();
		}

		BOOL startTouch = (OsWinIO_getTouch_action() == Win_TOUCH_DOWN_S) || (OsWinIO_getTouch_action() == Win_TOUCH_FORCE_DOWN_S);
		BOOL endTouch = (OsWinIO_getTouch_action() == Win_TOUCH_DOWN_E) || (OsWinIO_getTouch_action() == Win_TOUCH_FORCE_DOWN_E);

		BOOL active = OsWinIO_isActiveRenderItem(self);
		BOOL touch = startTouch || active;

		Quad2i copyrightRect = Quad2i_init4(coord.start.x, coord.start.y + coord.size.y - OsWinIO_cellSize() / 2, OsWinIO_cellSize() * 7, OsWinIO_cellSize() / 2);
		BOOL insideCopyrightRect = Quad2i_inside(copyrightRect, OsWinIO_getTouchPos());

		Quad2i fixthemapRect = Quad2i_init4(coord.start.x + coord.size.x - 2 * OsWinIO_cellSize(), coord.start.y + coord.size.y - OsWinIO_cellSize() / 2, OsWinIO_cellSize() * 2, OsWinIO_cellSize() / 2);
		BOOL insideFixthemapRect = Quad2i_inside(fixthemapRect, OsWinIO_getTouchPos());

		BIG clickMarkRow = -1;

		const int moveY = _GuiItemMap_getMarkMoveY(self) / 2;
		GuiItemMapItem* it = self->items;
		while (it)
		{
			Vec2i mid = GuiItemMapItem_getMapPos(it, coord.start, bbox, zoom);
			int rad = it->rad;

			mid.y -= moveY;
			double len = Vec2i_len(Vec2i_sub(OsWinIO_getTouchPos(), mid));
			if (len < rad)
			{
				clickMarkRow = it->row;
				rad = len;
			}
			it = it->next;
		}

		if (startTouch)
		{
			self->clickMarkRow = clickMarkRow;
			self->clickCopyright = insideCopyrightRect;
			self->clickFixthemap = insideFixthemapRect;
		}

		if (inside && touch) //full touch
		{
			OsWinIO_setActiveRenderItem(self);
		}

		if (self->clickMarkRow == -1)		//move
		{
			if (inside && touch) //full touch
			{
				if (startTouch)
				{
					self->startTouch = OsWinIO_getTouchPos();
					self->startTile = StdMap_getTile(cam.x, cam.y, zoom);
				}
			}

			if (active)
			{
				BOOL reset = !GuiItemRoot_hasChanges();

				_GuiItemMap_undownload(self, coord);	//first
				Vec2i move = Vec2i_sub(self->startTouch, OsWinIO_getTouchPos());

				double rx = (move.x / (float)coord.size.x) * bbox.size.x;
				double ry = (move.y / (float)coord.size.y) * bbox.size.y;

				double tileX = self->startTile.x + rx;
				double tileY = self->startTile.y + ry;

				Vec2f longLat = StdMap_getLonLat(tileX, tileY, zoom);
				DbValue_setNumber(&self->cam_long, longLat.x);
				DbValue_setNumber(&self->cam_lat, longLat.y);

				GuiItem_setRedraw(&self->base, TRUE);

				if (reset)
					GuiItemRoot_resetNumChanges();
			}
		}

		if (self->clickCopyright || self->clickFixthemap || (!active && (insideCopyrightRect || insideFixthemapRect)))
			Win_updateCursor(win, Win_CURSOR_HAND);
		else
			if (inside || active)
				Win_updateCursor(win, ((!active && clickMarkRow >= 0) || (active && clickMarkRow >= 0 && clickMarkRow == self->clickMarkRow)) ? Win_CURSOR_HAND : Win_CURSOR_MOVE);

		if (active && endTouch) //end
		{
			GuiItemEdit_saveCache();
			OsWinIO_resetActiveRenderItem();

			if (self->clickCopyright)
				OsWeb_openWebBrowser("https://www.openstreetmap.org/copyright");

			if (self->clickFixthemap)
				OsWeb_openWebBrowser("https://www.openstreetmap.org/fixthemap");

			if (clickMarkRow >= 0 && clickMarkRow == self->clickMarkRow)
				GuiItemMap_clickShowPage(self, self->clickMarkRow);
			self->clickMarkRow = -1;
			self->clickCopyright = FALSE;
			self->clickFixthemap = FALSE;
		}
	}

	_GuiItem_updateFinalCd(&self->base, back_cd, front_cd, coord, win);
}

void GuiItemMap_key(GuiItemMap* self, Quad2i coord, Win* win)
{
	if (!self->base.touch || !GuiItem_isEnable(&self->base))
		return;

	Vec3f cam = _GuiItemMap_getCam(self);
	Vec2f tile = StdMap_getTile(cam.x, cam.y, cam.z);

	UNI key = OsWinIO_getKeyID();
	UBIG keyExtra = OsWinIO_getKeyExtra();

	const float JUMP = 0.25f;

	if (keyExtra & Win_EXTRAKEY_UP)
	{
		Vec2f longLat = StdMap_getLonLat(tile.x, tile.y - JUMP, cam.z);
		DbValue_setNumber(&self->cam_lat, longLat.y);
	}
	else
		if (keyExtra & Win_EXTRAKEY_DOWN)
		{
			Vec2f longLat = StdMap_getLonLat(tile.x, tile.y + JUMP, cam.z);
			DbValue_setNumber(&self->cam_lat, longLat.y);
		}
		else
			if (keyExtra & Win_EXTRAKEY_LEFT)
			{
				Vec2f longLat = StdMap_getLonLat(tile.x - JUMP, tile.y, cam.z);
				DbValue_setNumber(&self->cam_long, longLat.x);
			}
			else
				if (keyExtra & Win_EXTRAKEY_RIGHT)
				{
					Vec2f longLat = StdMap_getLonLat(tile.x + JUMP, tile.y, cam.z);
					DbValue_setNumber(&self->cam_long, longLat.x);
				}

	if (key == '+')
	{
		DbValue_setNumber(&self->cam_zoom, Std_clamp(cam.z + 1, 1, GuiItemMap_MAX_ZOOM));
	}
	else
		if (key == '-')
		{
			DbValue_setNumber(&self->cam_zoom, Std_clamp(cam.z - 1, 1, GuiItemMap_MAX_ZOOM));
		}

	if (keyExtra & Win_EXTRAKEY_HOME)
	{
		DbValue_setNumber(&self->cam_long, 0);
		DbValue_setNumber(&self->cam_lat, 0);
		DbValue_setNumber(&self->cam_zoom, 2);
	}

	if (key == 's')
	{
		GuiItemMap_focusSearch(self);
	}

	if (key == 'f')
	{
		GuiItemMap_focusItems(self);
	}
}

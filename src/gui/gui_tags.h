/*
 * Copyright (c) 2018 Milan Suk
 *
 * Use of this software is governed by the Business Source License included
 * in the LICENSE file and at www.mariadb.com/bsl11.
 *
 * Change Date: 2024-11-01
 *
 * On the date above, in accordance with the Business Source License, use
 * of this software will be governed by version 2 or later of the General
 * Public License.
 */

typedef struct GuiItemTagsItem_s
{
	Quad2i coord;
	BIG row;

	DbValues columns;
	UNI* text;	//all columns together

	BOOL touchInside;
	BOOL touchPress;

	BOOL touchInsideClose;
	BOOL touchPressClose;

	Rgba cdBack;
	Rgba cdClose;

	FileRow file;
	BOOL isImage;
	BOOL isAudio;
	BOOL isText;
}GuiItemTagsItem;

void GuiItemTagsItem_resetTouch(GuiItemTagsItem* self)
{
	self->touchInside = FALSE;
	self->touchPress = FALSE;

	self->touchInsideClose = FALSE;
	self->touchPressClose = FALSE;
}

GuiItemTagsItem* GuiItemTagsItem_new(DbValues columns)
{
	GuiItemTagsItem* self = Os_malloc(sizeof(GuiItemTagsItem));

	self->columns = columns;
	self->coord = Quad2i_init();
	self->text = 0;

	self->cdBack = g_theme.main;
	self->cdClose = g_theme.black;

	self->row = -1;
	self->file = FileRow_initEmpty();

	self->isImage = FALSE;
	self->isAudio = FALSE;
	self->isText = FALSE;

	GuiItemTagsItem_resetTouch(self);
	return self;
}

GuiItemTagsItem* GuiItemTagsItem_newCopy(const GuiItemTagsItem* src)
{
	GuiItemTagsItem* self = GuiItemTagsItem_new(DbValues_initCopy(&src->columns));
	self->coord = src->coord;
	self->text = Std_newUNI(src->text);
	return self;
}

void GuiItemTagsItem_delete(GuiItemTagsItem* self)
{
	DbValues_free(&self->columns);
	Std_deleteUNI(self->text);
	Os_free(self, sizeof(GuiItemTagsItem));
}

static BOOL _GuiItemTagsItem_hasChanged(GuiItemTagsItem* self, BIG row, UBIG index)
{
	BOOL changed = FALSE;

	self->row = row;

	UBIG i;
	for (i = 0; i < self->columns.num; i++)
	{
		DbValue* column = &self->columns.values[i];
		DbValue_setRow(column, row, index);
		changed |= DbValue_hasChanged(column);
	}
	return changed;
}

BOOL GuiItemTagsItem_updateText(GuiItemTagsItem* self, BIG row, UBIG index)
{
	BOOL changed = _GuiItemTagsItem_hasChanged(self, row, index);
	if (changed)
	{
		Std_deleteUNI(self->text);
		self->text = 0;

		BIG i;
		for (i = 0; i < self->columns.num; i++)
		{
			DbValue* column = &self->columns.values[i];
			DbValue_setRow(column, row, index);

			changed |= DbValue_hasChanged(column);
			self->text = Std_addAfterUNI(self->text, DbValue_result(column));

			if (i + 1 < self->columns.num)
				self->text = Std_addAfterUNI(self->text, _UNI32(" "));
		}
	}
	return changed;
}

BOOL GuiItemTagsItem_updateCoord(GuiItemTagsItem* self, Quad2i coord)
{
	Quad2i oldCoord = self->coord;
	self->coord = coord;
	return !Quad2i_cmp(oldCoord, coord);
}

static Quad2i _GuiItemTagsItem_getImageCoord(Quad2i coord)
{
	return Quad2i_addSpace(coord, 2);
}
BOOL GuiItemTagsItem_updateCoordFile(GuiItemTagsItem* self, Quad2i coord, FileRow file)
{
	BOOL changed = GuiItemTagsItem_updateCoord(self, coord);

	changed |= !FileRow_cmp(self->file, file);
	self->file = file;

	if (changed && FileRow_is(self->file))
	{
		MediaLibrary_add(self->file, _GuiItemTagsItem_getImageCoord(coord).size, &self->isImage, &self->isAudio, &self->isText);
	}
	else
	{
		changed |= MediaLibrary_imageUpdate(self->file, _GuiItemTagsItem_getImageCoord(coord).size);	//update timer and returns true if just loaded
	}

	return changed;
}

int GuiItemTagsItem_closeX(void)
{
	return OsWinIO_cellSize() / 2;
}

Quad2i GuiItemTagsItem_rectText(const GuiItemTagsItem* self)
{
	const int sx = GuiItemTagsItem_closeX();
	return Quad2i_init4(self->coord.start.x, self->coord.start.y, self->coord.size.x - sx, self->coord.size.y);
}
Quad2i GuiItemTagsItem_rectClose(const GuiItemTagsItem* self)
{
	const int sx = GuiItemTagsItem_closeX();
	return Quad2i_init4(self->coord.start.x + self->coord.size.x - sx, self->coord.start.y, sx, self->coord.size.y);
}

BOOL GuiItemTagsItem_isActive(const GuiItemTagsItem* self)
{
	return self->touchPress || self->touchPressClose;
}

BOOL GuiItemTagsItem_updateColors(GuiItemTagsItem* self)
{
	Rgba cdBack = g_theme.main;
	if (self->touchPress != self->touchInside)	cdBack = Rgba_aprox(g_theme.white, g_theme.main, 0.5);
	if (self->touchPress && self->touchInside)	cdBack = g_theme.white;

	Rgba cdClose = g_theme.black;
	if (self->touchPressClose != self->touchInsideClose)	cdClose = Rgba_aprox(g_theme.white, g_theme.black, 0.5);
	if (self->touchPressClose && self->touchInsideClose)	cdClose = g_theme.white;

	BOOL redraw = (!Rgba_cmp(cdBack, self->cdBack) || !Rgba_cmp(cdClose, self->cdClose));

	self->cdBack = cdBack;
	self->cdClose = cdClose;
	return redraw;
}

int GuiItemTagsItem_touch(GuiItemTagsItem* self, BOOL startTouch, BOOL moveTouch, BOOL endTouch, Vec2i pos)
{
	int click = 0;

	if (startTouch)
	{
		self->touchInside = Quad2i_inside(GuiItemTagsItem_rectText(self), pos);
		self->touchInsideClose = Quad2i_inside(GuiItemTagsItem_rectClose(self), pos);

		self->touchPress = self->touchInside;
		self->touchPressClose = self->touchInsideClose;
	}
	else
		if (endTouch)
		{
			if (self->touchPress && self->touchInside)
				click = 1;

			if (self->touchPressClose && self->touchInsideClose)
				click = 2;

			GuiItemTagsItem_resetTouch(self);
		}
		else
			if (moveTouch)
			{
				if (self->touchPress)
					self->touchInside = Quad2i_inside(GuiItemTagsItem_rectText(self), pos);
				if (self->touchPressClose)
					self->touchInsideClose = Quad2i_inside(GuiItemTagsItem_rectClose(self), pos);
			}
			else
			{
				self->touchInside = Quad2i_inside(GuiItemTagsItem_rectText(self), pos);
				self->touchInsideClose = Quad2i_inside(GuiItemTagsItem_rectClose(self), pos);
			}

	//colors
	if (GuiItemTagsItem_updateColors(self) && click == 0)
		click = 3;	//redraw

	return click;
}

void GuiItemTagsItem_draw(GuiItemTagsItem* self, Image4* img, Rgba front_cd, BOOL showClose, BOOL imagePreview)
{
	int textH = _GuiItem_textSize(1, self->coord.size.y);
	OsFont* font = OsWinIO_getFontDefault();

	Quad2i q = GuiItemTagsItem_rectText(self);
	Image4_drawRBox(img, self->coord, textH / 2, self->cdBack);

	if (Std_sizeUNI(self->text))
		Image4_drawText(img, Quad2i_getMiddle(q), TRUE, font, self->text, textH, 0, front_cd);
	else
		if (self->isImage && imagePreview)
		{
			MediaLibrary_imageDraw(self->file, img, _GuiItemTagsItem_getImageCoord(self->coord));
		}
		else
			if (FileRow_is(self->file))
				MediaLibrary_imageDrawExt(self->file, img, _GuiItemTagsItem_getImageCoord(self->coord), textH, front_cd);

	if (showClose)
	{
		Quad2i qq = GuiItemTagsItem_rectClose(self);
		//Image4_drawRBox(img, self->coord, textH / 2, g_theme.main);
		Image4_drawText(img, Quad2i_getMiddle(qq), TRUE, font, _UNI32("X"), textH * 1, 0, self->cdClose);
	}
}

typedef struct GuiItemTags_s
{
	GuiItem base;

	DbRows source;

	DbValues columns;	//<DbConnectValue*>

	GuiItemTagsCallback* callbackAdd;
	GuiItemTagsCallback* callbackItem;

	BOOL relativeAdd;
	BOOL relativeItem;

	BOOL setRowAdd;
	BOOL setRowItem;

	StdArr items;

	BOOL showClose;

	Quad2i clickCoord;
	DbValue imagePreview;
}GuiItemTags;

GuiItem* GuiItemTags_new(Quad2i grid, DbRows source, DbValues columns, GuiItemTagsCallback* callbackAdd, GuiItemTagsCallback* callbackItem, BOOL relativeAdd, BOOL relativeItem, BOOL setRowAdd, BOOL setRowItem, DbValue imagePreview)
{
	GuiItemTags* self = Os_malloc(sizeof(GuiItemTags));
	self->base = GuiItem_init(GuiItem_TAGS, grid);

	self->source = source;
	self->columns = columns;
	self->callbackAdd = callbackAdd;
	self->callbackItem = callbackItem;

	self->relativeAdd = relativeAdd;
	self->relativeItem = relativeItem;

	self->setRowAdd = setRowAdd;
	self->setRowItem = setRowItem;

	self->items = StdArr_init();

	self->showClose = TRUE;
	self->imagePreview = imagePreview;
	return (GuiItem*)self;
}

GuiItem* GuiItemTags_newCopy(GuiItemTags* src, BOOL copySub)
{
	GuiItemTags* self = Os_malloc(sizeof(GuiItemTags));
	*self = *src;
	GuiItem_initCopy(&self->base, &src->base, copySub);

	self->source = DbRows_initCopy(&src->source);
	self->columns = DbValues_initCopy(&src->columns);
	self->imagePreview = DbValue_initCopy(&src->imagePreview);

	self->items = StdArr_initCopyFn(&src->items, (StdArrCOPY)&GuiItemTagsItem_newCopy);

	return (GuiItem*)self;
}

void GuiItemTags_clearItems(GuiItemTags* self)
{
	StdArr_freeFn(&self->items, (StdArrFREE)&GuiItemTagsItem_delete);
	self->items = StdArr_init();
}

void GuiItemTags_delete(GuiItemTags* self)
{
	GuiItemTags_clearItems(self);

	DbRows_free(&self->source);
	DbValues_free(&self->columns);
	DbValue_free(&self->imagePreview);

	GuiItem_free(&self->base);
	Os_free(self, sizeof(GuiItemTags));
}

BOOL GuiItemTags_isTypeFile(GuiItemTags* self)
{
	return self->source.column && DbColumnFormat_findColumn(self->source.column) == DbFormat_FILE_N;
}
BOOL GuiItemTags_isTypeTag(GuiItemTags* self)
{
	return self->source.column && DbColumnFormat_findColumn(self->source.column) == DbFormat_TAGS;
}

void GuiItemTags_setRow(GuiItemTags* self, BIG row)
{
	DbRows_setBaseRow(&self->source, row);
}
BIG GuiItemTags_getRow(const GuiItemTags* self)
{
	return DbRows_getBaseRow(&self->source);
}

UBIG GuiItemTags_numItems(const GuiItemTags* self)
{
	return DbRows_getSize(&self->source);
}

DbColumn* GuiItemTags_getColumn(const GuiItemTags* self)
{
	return self->source.column;
}

void GuiItemTags_draw(GuiItemTags* self, Image4* img, Quad2i coord, Win* win)
{
	if (!self->base.drawTable)
		coord = Quad2i_addSpace(coord, 3);

	//background
	Image4_drawBoxQuad(img, coord, g_theme.white);

	const BOOL preview = DbValue_getNumber(&self->imagePreview);

	UBIG i;
	for (i = 0; i < self->items.num; i++)
	{
		GuiItemTagsItem* item = self->items.ptrs[i];
		GuiItemTagsItem_draw(item, img, self->base.front_cd, self->showClose, preview);
	}

	if (self->base.drawTable)
	{
		Quad2i q = coord;
		q.size.x += 1;
		q.size.y += 1;
		//img->rect = q;
		Image4_drawBorder(img, q, 1, Rgba_aprox(g_theme.background, g_theme.black, 0.5f));
	}
	else
		Image4_drawBorder(img, coord, 1, self->base.front_cd);
}

void GuiItemTags_update(GuiItemTags* self, Quad2i coord, Win* win)
{
	BOOL changed = FALSE;

	BOOL isFile = GuiItemTags_isTypeFile(self);
	const BOOL preview = DbValue_getNumber(&self->imagePreview);

	const int cell = OsWinIO_cellSize();

	Vec2i start = coord.start;
	Vec2i end = Quad2i_end(coord);

	int lineY = (isFile && preview && coord.size.y >= cell * 2) ? cell * 2 : cell;

	const int textH = _GuiItem_textSize(1, coord.size.y);
	OsFont* font = OsWinIO_getFontDefault();

	start.x += cell + textH / 2;

	const UBIG N = DbRows_getSize(&self->source);

	if (self->items.num > N)
	{
		GuiItemTags_clearItems(self);
		changed = TRUE;
	}

	UBIG i;
	for (i = 0; i < N; i++)
	{
		GuiItemTagsItem* item;
		if (i < self->items.num)
			item = self->items.ptrs[i];
		else
		{
			//add new
			item = GuiItemTagsItem_new(DbValues_initCopy(&self->columns));
			StdArr_add(&self->items, item);
		}

		changed |= GuiItemTagsItem_updateText(item, DbRows_getRow(&self->source, i), i);

		Quad2i coordItem = Quad2i_init();

		//text
		int textSize = 0;
		if (isFile)
		{
			textSize = cell / 9 * 16;
		}
		else
		{
			int extra_down;
			textSize = OsFont_getTextSize(font, item->text, textH, 0, &extra_down).x + textH / 2;
		}

		if (self->showClose)
			textSize += textH / 2 + GuiItemTagsItem_closeX();

		int add_start = textSize + textH / 2;
		if (start.x + add_start > end.x)	//new line
		{
			start.y += lineY;
			start.x = coord.start.x + textH / 2;
		}

		coordItem = Quad2i_addSpaceY(Quad2i_init4(start.x, start.y, textSize, lineY), 4);

		start.x += add_start;

		if (start.y > end.y)	//outside
			i = N;				//break loop

		changed |= isFile ? GuiItemTagsItem_updateCoordFile(item, coordItem, DbRows_getFileId(&self->source, i)) : GuiItemTagsItem_updateCoord(item, coordItem);
	}

	GuiItem_setRedraw(&self->base, changed);
}

void GuiItemTags_clickAddItem(GuiItem* self)
{
	GuiItemTags* tags = GuiItem_findParentType(self, GuiItem_TAGS);
	if (tags && self->type == GuiItem_BUTTON)
	{
		BIG row = ((GuiItemButton*)self)->text.row;
		DbRows_addLinkRow(&tags->source, row);
	}
}

void GuiItemTags_clickShowAdd(GuiItem* self)
{
	GuiItemTags* tags = GuiItem_findParentType(self, GuiItem_TAGS);
	if (tags)
	{
		GuiItemLayout* layoutAdd = tags->callbackAdd(tags->source.column);

		if (tags->setRowAdd)
			GuiItem_setRow(&layoutAdd->base, DbRows_getBaseRow(&tags->source), 0);

		if (tags->relativeAdd)	GuiItemRoot_addDialogRel((GuiItem*)layoutAdd, self, self->coordMove, TRUE);
		else					GuiItemRoot_addDialogLayout(layoutAdd);
	}
}

void GuiItemTags_clickShowItem(GuiItemTags* self, UBIG index)
{
	GuiItemLayout* layoutDetails = self->callbackItem(self->source.column);
	if (self->setRowItem)
	{
		if (GuiItemTags_isTypeFile(self) || GuiItemTags_isTypeTag(self))
			GuiItem_setRow(&layoutDetails->base, DbRows_getBaseRow(&self->source), index);
		else
			GuiItem_setRow(&layoutDetails->base, DbRows_getRow(&self->source, index), 0);
	}

	if (self->relativeItem)	GuiItemRoot_addDialogRel((GuiItem*)layoutDetails, (GuiItem*)self, self->clickCoord, TRUE);
	else					GuiItemRoot_addDialogLayout(layoutDetails);
}

void GuiItemTags_touch(GuiItemTags* self, Quad2i coord, Win* win)
{
	Rgba back_cd = g_theme.white;
	Rgba front_cd = g_theme.black;

	if (self->base.touch && GuiItem_isEnable(&self->base) && OsWinIO_canActiveRenderItem(self))
	{
		BOOL startTouch = (OsWinIO_getTouch_action() == Win_TOUCH_DOWN_S) || (OsWinIO_getTouch_action() == Win_TOUCH_FORCE_DOWN_S);
		BOOL endTouch = (OsWinIO_getTouch_action() == Win_TOUCH_DOWN_E) || (OsWinIO_getTouch_action() == Win_TOUCH_FORCE_DOWN_E);

		BOOL active = OsWinIO_isActiveRenderItem(self);
		BOOL inside = Quad2i_inside(coord, OsWinIO_getTouchPos());
		BOOL touch = startTouch || active;

		UBIG i;
		BOOL itemClick = FALSE;
		for (i = 0; i < self->items.num; i++)
		{
			GuiItemTagsItem* item = self->items.ptrs[i];
			int click = GuiItemTagsItem_touch(item, startTouch, active, endTouch, OsWinIO_getTouchPos());
			if (click == 1)
			{
				self->clickCoord = item->coord;
				GuiItemTags_clickShowItem(self, i);
			}
			if (click == 2)
			{
				if (FileRow_is(item->file))
					DbRows_removeFile(&self->source, item->file);
				else
					DbRows_removeRow(&self->source, item->row);
			}

			itemClick |= click > 0;
			GuiItem_setRedraw(&self->base, click > 0);

			if (item->touchInside || item->touchInsideClose)
				Win_updateCursor(win, Win_CURSOR_HAND);	//cursor
		}

		if (inside && touch) //full touch
		{
			back_cd = g_theme.main;
			front_cd = g_theme.black;
			OsWinIO_setActiveRenderItem(self);
		}

		if (active && endTouch)
			OsWinIO_resetActiveRenderItem();

		//cursor
		//if(inside)
		//	Win_udpateCursor(win, Win_CURSOR_HAND);
	}

	_GuiItem_updateFinalCd(&self->base, back_cd, front_cd, coord, win);
}

GuiItemLayout* GuiItemTags_resize(GuiItemTags* self, GuiItemLayout* layout, Win* win)
{
	if (!self->base.resize)
		return (GuiItemLayout*)GuiItem_getSub(&self->base, 0);

	GuiItem_freeSubs(&self->base);

	//layout
	layout = GuiItemLayout_newCoord(&self->base, FALSE, FALSE, win);
	layout->drawBackground = FALSE;

	GuiItem_addSubName(&self->base, "layout_main", &layout->base);

	if (self->callbackAdd)
		GuiItem_addSubName((GuiItem*)layout, "+", GuiItemButton_newBlackEx(Quad2i_init4(0, 0, 1, 1), DbValue_initStaticCopyCHAR("+"), &GuiItemTags_clickShowAdd));

	return layout;
}

GuiItemLayout* GuiItemTags_dialogTagsAdd(DbColumn* column)
{
	BIG crowT = DbColumn_getRow(column);
	BIG optionsRow = DbRows_findSubType(crowT, "options");

	GuiItemLayout* layAdd = GuiItemLayout_new(Quad2i_init());

	GuiItemLayout_addColumn(layAdd, 0, 10);
	GuiItemLayout_addRow(layAdd, 0, 10);
	GuiItem* skin = GuiItemButton_newClassicEx(Quad2i_init4(0, 0, 1, 1), DbValue_initOption(-1, "name", 0), &GuiItemTags_clickAddItem);
	GuiItem_addSubName((GuiItem*)layAdd, "list", GuiItemList_new(Quad2i_init4(0, 0, 1, 1), DbRows_initLink(DbRoot_getColumnSubs(), optionsRow), skin, DbValue_initEmpty()));

	return layAdd;
}

GuiItemLayout* GuiItemTags_dialogTagsDetails(DbColumn* column)
{
	GuiItemLayout* layDetails = GuiItemLayout_new(Quad2i_init());

	GuiItemLayout_addColumn(layDetails, 0, 10);
	GuiItemLayout_addRow(layDetails, 0, 10);

	GuiItemLayout* skin = GuiItemLayout_new(Quad2i_init4(0, 0, 1, 1));
	GuiItemLayout_addColumn(skin, 1, 100);

	GuiItem* drag = GuiItem_addSubName((GuiItem*)skin, "drag", GuiItemBox_newEmpty(Quad2i_init4(0, 0, 1, 1)));
	GuiItem_setIcon(drag, GuiImage_new1(UiIcons_init_reoder()));
	GuiItem_setDrop(drag, "option", "option", FALSE, DbRows_initLink((DbColumnN*)column, -1), -1);

	GuiItem_addSubName((GuiItem*)skin, "name", GuiItemText_new(Quad2i_init4(1, 0, 1, 1), FALSE, DbValue_initOption(-1, "name", 0), DbValue_initEmpty()));

	//list
	GuiItemList* list = (GuiItemList*)GuiItem_addSubName((GuiItem*)layDetails, "list", GuiItemList_new(Quad2i_init4(0, 0, 1, 1), DbRows_initLink((DbColumnN*)column, -1), (GuiItem*)skin, DbValue_initEmpty()));
	GuiItemList_setShowRemove(list, TRUE);

	return layDetails;
}

GuiItemLayout* GuiItemTags_dialogLinksAdd(DbColumn* column)
{
	GuiItemLayout* layAdd = GuiItemTable_buildDialogLinks(column);
	return layAdd;
}

GuiItemLayout* GuiItemTags_dialogLinksDetails(DbColumn* column)
{
	GuiItemLayout* layDetails = 0;

	DbTable* btable = DbColumn_getBTable(column);
	if (btable)
		layDetails = GuiItemTable_buildPage(DbTable_getRow(btable), FALSE, FALSE);
	return layDetails;
}

GuiItemLayout* GuiItemTags_dialogFileAdd(DbColumn* column)
{
	GuiItemLayout* layAdd = GuiItemLayout_new(Quad2i_init4(0, 0, 1, 1));

	GuiItemLayout_addColumn(layAdd, 0, 10);

	//path
	GuiItemEdit* pathFile = (GuiItemEdit*)GuiItem_addSubName((GuiItem*)layAdd, "pathFile", GuiItemEdit_new(Quad2i_init4(0, 0, 1, 2), DbValue_initEmpty(), DbValue_initLang("PATH")));
	GuiItemEdit_setShowPicker(pathFile, TRUE, TRUE, FALSE, TRUE, Lang_find("IMPORT"), 0);
	//Button
	GuiItem_addSubName((GuiItem*)layAdd, "import_file", GuiItemButton_newClassicEx(Quad2i_init4(0, 2, 1, 2), DbValue_initLang("IMPORT"), &GuiItemTable_clickImportFile));

	GuiItem_addSubName((GuiItem*)layAdd, "pathWeb", GuiItemEdit_new(Quad2i_init4(0, 6, 1, 2), DbValue_initEmpty(), DbValue_initLang("URL")));
	//Button
	GuiItem_addSubName((GuiItem*)layAdd, "import_web", GuiItemButton_newClassicEx(Quad2i_init4(0, 8, 1, 2), DbValue_initLang("IMPORT"), &GuiItemTable_clickImportWeb));

	return layAdd;
}

GuiItemLayout* GuiItemTags_dialogFileDetails(DbColumn* column)
{
	GuiItemLayout* layDetails = GuiItemLayout_new(Quad2i_init());

	GuiItemLayout_addColumn(layDetails, 0, 20);
	GuiItemLayout_addRow(layDetails, 0, 20);

	GuiItem_addSubName((GuiItem*)layDetails, "file", GuiItemFile_new(Quad2i_init4(0, 0, 1, 1), DbValue_initGET(column, -1), DbValue_initNumber(1), FALSE));

	return layDetails;
}
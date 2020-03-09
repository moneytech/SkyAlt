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

typedef struct GuiItemList_s
{
	GuiItem base;

	DbRows filter;
	GuiItem* skin;

	DbValue description;

	BIG oldNumRows;

	GuiScroll scroll;
	BOOL showScroll;
	BOOL showRemove;
	BOOL showBorder;
	BOOL warningIfEmpty;

	BOOL whiteScroll;

	BOOL drawBackground;

	UINT asGrid;	//value = #cells
	GuiItemCallback* clickRemove;
}GuiItemList;

GuiItem* GuiItemList_new(Quad2i grid, DbRows filter, GuiItem* skin, DbValue description)
{
	GuiItemList* self = Os_malloc(sizeof(GuiItemList));
	self->base = GuiItem_init(GuiItem_LIST, grid);

	self->description = description;

	self->filter = filter;
	self->skin = skin;

	self->scroll = GuiScroll_init(DbValue_initOption(filter.row, "scroll", 0));
	self->showScroll = TRUE;
	self->showRemove = FALSE;
	self->showBorder = TRUE;
	self->warningIfEmpty = FALSE;

	self->oldNumRows = -1;

	self->asGrid = 0;

	self->clickRemove = 0;

	self->drawBackground = FALSE;

	self->whiteScroll = FALSE;

	return (GuiItem*)self;
}

GuiItem* GuiItemList_newCopy(GuiItemList* src, BOOL copySub)
{
	GuiItemList* self = Os_malloc(sizeof(GuiItemList));
	*self = *src;
	GuiItem_initCopy(&self->base, &src->base, copySub);

	self->description = DbValue_initCopy(&src->description);

	self->filter = DbRows_initCopy(&src->filter);
	self->skin = GuiItem_newCopy(src->skin, TRUE);

	self->oldNumRows = -1;
	self->scroll = GuiScroll_initCopy(&src->scroll);

	return (GuiItem*)self;
}

GuiItem* GuiItemList_newButtons(Quad2i grid, DbRows source, DbValue button, DbValue description, GuiItemCallback* clickButton)
{
	GuiItemButton* skinBase = (GuiItemButton*)GuiItemButton_newAlphaEx(Quad2i_init4(0, 0, 1, 1), button, clickButton);
	skinBase->type = GuiItemButton_ALPHA;
	return GuiItemList_new(grid, source, &skinBase->base, description);
}

GuiItem* GuiItemList_newButtonsMenu(Quad2i grid, DbRows source, DbValue description, DbValue buttonText, GuiItemCallback* buttonCall, GuiItemMenu** out_menu)
{
	GuiItemLayout* layout = GuiItemLayout_new(Quad2i_init4(0, 0, 1, 1));
	GuiItemLayout_addColumn(layout, 0, 99);
	GuiItemLayout_addColumn(layout, 1, 1);

	//Button
	GuiItemButton* b = (GuiItemButton*)GuiItemButton_newAlphaEx(Quad2i_init4(0, 0, 1, 1), buttonText, buttonCall);
	b->textCenter = FALSE;
	GuiItem_addSubName(&layout->base, "button", &b->base);

	//Menu
	*out_menu = (GuiItemMenu*)GuiItem_addSubName(&layout->base, "...", GuiItemMenu_new(Quad2i_init4(1, 0, 1, 1), DbValue_initStaticCopy(_UNI32("...")), TRUE));

	return GuiItemList_new(grid, source, &layout->base, description);
}

/*void GuiItemList_disableScrollSend(GuiItemList* self)
{
	self->scroll.sendScrollDown = FALSE;
}*/

void GuiItemList_setShowScroll(GuiItemList* self, BOOL showScroll)
{
	self->showScroll = showScroll;
}
void GuiItemList_setShowRemove(GuiItemList* self, BOOL showRemove)
{
	self->showRemove = showRemove;
}
void GuiItemList_setShowBorder(GuiItemList* self, BOOL showBorder)
{
	self->showBorder = showBorder;
}
void GuiItemList_setShowWarningIfEmpty(GuiItemList* self, BOOL warningIfEmpty)
{
	self->warningIfEmpty = warningIfEmpty;
}

void GuiItemList_setAsGrid(GuiItemList* self, UINT numCells)
{
	self->asGrid = numCells;
}

void GuiItemList_setSource(GuiItemList* self, DbRows filter)
{
	DbRows_free(&self->filter);
	self->filter = filter;
}

void GuiItemList_setScroll(GuiItemList* self, DbValue value)
{
	GuiScroll_free(&self->scroll);
	self->scroll = GuiScroll_init(value);
}

void GuiItemList_setDrawBackground(GuiItemList* self, BOOL drawBackground)
{
	self->drawBackground = drawBackground;
}

void GuiItemList_delete(GuiItemList* self)
{
	GuiItem_delete(self->skin);
	DbRows_free(&self->filter);

	GuiScroll_free(&self->scroll);
	DbValue_free(&self->description);

	GuiItem_free(&self->base);
	Os_free(self, sizeof(GuiItemList));
}

BOOL _GuiItemList_hasDescription(const GuiItemList* self)
{
	return Std_sizeUNI(DbValue_result(&self->description));
}

static UBIG _GuiItemList_numRows(GuiItemList* self)
{
	return DbRows_getSize(&self->filter);
}

static int _GuiItemList_itemGridY(GuiItemList* self, int cell)
{
	Quad2i rr = GuiItem_getSubMaxCoord(self->skin);
	return rr.size.y / cell;
}

UBIG GuiItemList_numColumns(const GuiItemList* self)
{
	const int cell = OsWinIO_cellSize();
	int sizeX = self->base.coordScreen.size.x;

	return self->asGrid ? Std_max(1, sizeX / cell / self->asGrid) : 1;
}

static BOOL _GuiItemList_isBackgroundWarning(const GuiItemList* self)
{
	return (self->warningIfEmpty && self->oldNumRows == 0);
}

void GuiItemList_draw(GuiItemList* self, Image4* img, Quad2i coord, Win* win)
{
	const int scroll_width = GuiScroll_widthWin(win);
	const int cell = OsWinIO_cellSize();

	if (_GuiItemList_isBackgroundWarning(self))
	{
		Quad2i q = coord;
		q.size.x -= scroll_width;
		q = Quad2i_addSpace(q, 2);
		Image4_drawBorder(img, q, 1, self->base.back_cd);
	}

	//scroll
	int titleMove = _GuiItemList_hasDescription(self) ? cell : 0;
	if (titleMove)
	{
		coord.start.y += titleMove;
		coord.size.y -= titleMove;
	}

	if (self->showScroll)
	{
		const int itemGridY = Std_max(1, _GuiItemList_itemGridY(self, cell) + (self->asGrid > 0));	//+spaceBetween
		const int itemY = itemGridY * cell;

		const int sreenY = coord.size.y;

		const int N_COLUMNS = GuiItemList_numColumns(self);
		UBIG num_rows = (self->oldNumRows / N_COLUMNS) + (self->oldNumRows % N_COLUMNS > 0);	//floor up(columns)
		if (sreenY % itemY)	//floor up(screenY vs itemY)
			num_rows++;

		const UBIG maxY = num_rows * itemY;

		if (self->whiteScroll)
			self->scroll.cd = g_theme.white;

		GuiScroll_set(&self->scroll, maxY, sreenY, itemY);
		GuiScroll_drawV(&self->scroll, Vec2i_init2(coord.start.x + coord.size.x - scroll_width, coord.start.y), img, win);
	}

	if (titleMove)
	{
		if (DbRows_getSize(&self->filter) == 0 && coord.size.y > 0)
		{
			//background
			coord = Quad2i_addSpace(coord, 3);
			Image4_drawBoxQuad(img, coord, self->base.back_cd);

			//text
			int textH = _GuiItem_textSize(2, coord.size.y);
			OsFont* font = OsWinIO_getFontDefault();
			Image4_drawText(img, Quad2i_getMiddle(coord), TRUE, font, _UNI32("+"), textH, 0, self->base.front_cd);
		}
	}
}

BIG GuiItemList_getRow(GuiItemList* self)
{
	return DbRows_getBaseRow(&self->filter);
}
void GuiItemList_setRow(GuiItemList* self, BIG row)
{
	DbRows_setBaseRow(&self->filter, row);
}

BIG GuiItemList_getClickPos(GuiItemList* self, BIG row)
{
	return Std_max(0, DbRows_findLinkPos(&self->filter, row));
}

void GuiItemList_setClickRemove(GuiItemList* self, GuiItemCallback* clickRemove)
{
	self->clickRemove = clickRemove;
}

void GuiItemList_clickRemove(GuiItem* self)
{
	GuiItemList* list = GuiItem_findParentType(self->parent, GuiItem_LIST);
	if (list && self->type == GuiItem_BUTTON)
	{
		DbRows_removeRow(&list->filter, DbValue_getRow(&((GuiItemButton*)self)->text));

		if (list->clickRemove)
			list->clickRemove(self);

		//GuiItem_setResize(self, TRUE);
	}
}

void GuiItemList_update(GuiItemList* self, Quad2i coord, Win* win)
{
	BIG oldNumRows = self->oldNumRows;
	self->oldNumRows = _GuiItemList_numRows(self);

	//update cells
	GuiItem* layout = self->base.subs.num ? GuiItem_getSub(&self->base, 0) : 0;
	layout = layout ? GuiItem_lastSub(layout) : 0;
	if (layout)
	{
		const int N_COLUMNS = GuiItemList_numColumns(self);
		const UBIG WHEEL = GuiScroll_getWheelRow(&self->scroll) * N_COLUMNS;

		BIG extra = self->showRemove + 1;

		BIG r, rr;
		for (r = 0; r < layout->subs.num / extra; r++)
		{
			for (rr = 0; rr < extra; rr++)
			{
				GuiItem* it = layout->subs.ptrs[r * extra + rr];

				BIG row = DbRows_getRow(&self->filter, WHEEL + r);

				GuiItem_setRow(it, row, 0);
				GuiItem_setShow(it, (row >= 0));	//visibility

				GuiItem_setDropRow(it, DbRows_getBaseRow(&self->filter));
			}
		}
	}

	GuiItem_setRedraw(&self->base, (DbRows_hasChanged(&self->filter) || DbValue_hasChanged(&self->description) || oldNumRows != self->oldNumRows || GuiScroll_getRedrawAndReset(&self->scroll)));
}

void GuiItemList_touch(GuiItemList* self, Quad2i coord, Win* win)
{
	const int scroll_width = GuiScroll_widthWin(win);
	const int cell = OsWinIO_cellSize();

	Rgba back_cd = _GuiItemList_isBackgroundWarning(self) ? g_theme.warning : g_theme.background;
	Rgba front_cd = g_theme.main;

	if (self->base.touch && GuiItem_isEnable(&self->base) && OsWinIO_canActiveRenderItem(self))
	{
		int titleMove = _GuiItemList_hasDescription(self) ? cell : 0;
		coord.start.y += titleMove;
		coord.size.y -= titleMove;

		if (self->showScroll)
			GuiScroll_touchV(&self->scroll, self, coord, Vec2i_init2(coord.start.x + coord.size.x - scroll_width, coord.start.y), win);
	}

	_GuiItem_updateFinalCd(&self->base, back_cd, front_cd, coord, win);
}

GuiItemLayout* GuiItemList_resize(GuiItemList* self, GuiItemLayout* layout, Win* win)
{
	if (!self->base.resize)
		return (GuiItemLayout*)GuiItem_getSub(&self->base, 0);

	//List
	//Layout
	//Description <optional>
	//Layout
	//Skin
	//Skin

	GuiItem_freeSubs(&self->base);
	GuiItemList* tree = (GuiItemList*)self;

	const int cell = OsWinIO_cellSize();

	//Root layout
	layout = GuiItemLayout_newCoord(&self->base, tree->showScroll, FALSE, win);
	GuiItemLayout_setDrawBackground(layout, self->drawBackground);
	layout->drawBorder = !tree->showScroll && tree->showBorder;
	layout->extraSpace = 3;
	GuiItemLayout_addColumn(layout, 0, 100);
	GuiItem_addSubName(&self->base, "layout_main", &layout->base);

	//Cells will not be under scroll
	if (tree->showScroll)
		layout->base.coordScreen.size.x -= GuiScroll_widthWin(win);

	int sizeY = self->base.coordScreen.size.y;

	//title
	BOOL description = _GuiItemList_hasDescription(tree);
	if (description)
	{
		sizeY -= cell;
		GuiItem_addSubName(&layout->base, "description", GuiItemText_new(Quad2i_init4(0, 0, 1, 1), TRUE, DbValue_initCopy(&tree->description), DbValue_initEmpty()));
	}

	//Stretch layout with skins
	GuiItemLayout_addRow(layout, description ? 1 : 0, 100);

	//Skin layout
	GuiItemLayout* layout2 = GuiItemLayout_new(Quad2i_init4(0, description ? 1 : 0, 1, 1));
	layout2->drawBackground = FALSE;
	layout2->drawBorder = tree->showScroll && tree->showBorder;
	layout2->showScrollV = FALSE;
	//if (tree->showRemove)
	//	GuiItemLayout_addColumn(layout2, 1, 1);
	GuiItem_addSubName(&layout->base, "layout2", &layout2->base);

	//Skins
	GuiItem* skin = tree->skin;
	int skinGridH = skin->grid.start.y + skin->grid.size.y;

	if (skin->type == GuiItem_LAYOUT)
	{
		GuiItem_resize(&layout->base, layout, win); //also resize layout2

		//resize Skin
		skin->coordScreen = layout2->base.coordScreen;
		skin->coordMove = layout2->base.coordMove;
		GuiItem_resize(skin, (GuiItemLayout*)skin, win);

		Quad2i rr = GuiItem_getSubMaxCoord(skin);
		skinGridH = rr.size.y / cell;
	}

	const BOOL spaceBetween = (self->asGrid > 0);
	int N_ROWS_VISIBLE = 0;
	if (skinGridH)
	{
		const int itemH = skinGridH * cell;
		N_ROWS_VISIBLE = sizeY / itemH;
		if (sizeY % itemH)
			N_ROWS_VISIBLE++;
	}

	//Vec2i screenSize;
	//OsScreen_getMonitorResolution(&screenSize);
	//const int N_ROWS_VISIBLE = Std_max(1, screenSize.y / cell / skinGridH);

	const int N_COLUMNS = GuiItemList_numColumns(self);

	Vec2i itemPos = Vec2i_init();
	BIG y, x;
	for (y = 0; y < N_ROWS_VISIBLE; y++)
	{
		for (x = 0; x < N_COLUMNS; x++)
		{
			char nameId[64];
			snprintf(nameId, 64, "%lld_%lld_skin", y, x);

			GuiItemLayout_addColumn(layout2, itemPos.x, 100);

			GuiItem* cit = GuiItem_newCopy(skin, TRUE);
			cit->grid = Quad2i_init4(itemPos.x, itemPos.y, 1, skinGridH);
			((GuiItem*)GuiItem_addSubName(&layout2->base, nameId, cit))->show = FALSE;

			if (tree->showRemove)
			{
				snprintf(nameId, 64, "%lld_%lld_remove", y, x);
				itemPos.x++;

				GuiItemLayout* layout3 = GuiItemLayout_new(Quad2i_init4(itemPos.x, itemPos.y, 1, skinGridH));
				layout3->drawBackground = FALSE;
				GuiItem_addSubName(&layout2->base, nameId, &layout3->base);
				BOOL center = (skinGridH >= 3);
				if (center)
				{
					GuiItemLayout_addRow(layout3, 0, 99);
					GuiItemLayout_addRow(layout3, 1, 2);
					GuiItemLayout_addRow(layout3, 2, 99);
				}

				cit = GuiItemButton_newClassicEx(Quad2i_init4(0, center, 1, 1), DbValue_initStaticCopy(_UNI32("X")), &GuiItemList_clickRemove);
				cit->show = FALSE;

				if (skin->type == GuiItem_LAYOUT && ((GuiItemLayout*)skin)->back_cd_value.column)
					GuiItemButton_setBackgroundCdValue((GuiItemButton*)cit, TRUE, DbValue_initCopy(&((GuiItemLayout*)skin)->back_cd_value));


				GuiItem_addSubName(&layout3->base, "x", cit);
			}

			itemPos.x += spaceBetween;
			itemPos.x++;
		}

		itemPos.x = 0;
		itemPos.y += spaceBetween;
		itemPos.y += skinGridH;
	}

	return layout;
}

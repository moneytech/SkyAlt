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

typedef struct GuiItemLayout_s
{
	GuiItem base;

	GuiItemLayoutArray cols;
	GuiItemLayoutArray rows;

	BOOL drawBackground;
	BOOL drawBackgroundWhite;
	BOOL drawBackgroundBlack;

	BOOL drawBackgroundMain;
	BOOL drawBorder;
	BOOL drawBackgroundError; //for gui
	BOOL drawBorderError; //for step

	BOOL whiteScroll;

	//BOOL drawShadows;
	int extraSpace;

	GuiScroll scrollH;
	GuiScroll scrollV;

	BOOL enableClickLayout;

	DbValue title;
	DbValue value;	//row + index

	BIG highlightRow;
	//BIG extra_big;

	BOOL showScrollV;
	BOOL showScrollH;

	Rgba cdSelect;
	Rgba cdWarning;

	GuiItemCallback* drop;
	GuiItemCallback* resize;

	Image4 imgBackground;
} GuiItemLayout;

void GuiItemLayout_initScroll(GuiItemLayout* self, Quad2i grid, GuiItemCallback* click, GuiScroll scrollV)
{
	self->base = GuiItem_init(GuiItem_LAYOUT, grid);

	self->cols = GuiItemLayoutArray_init();
	self->rows = GuiItemLayoutArray_init();

	self->scrollH = GuiScroll_initEmpty();
	self->scrollV = scrollV;

	self->drawBackground = TRUE;
	self->drawBackgroundWhite = FALSE;
	self->drawBackgroundMain = FALSE;
	self->drawBackgroundError = FALSE;
	self->drawBackgroundBlack = FALSE;

	self->drawBorder = FALSE;
	self->drawBorderError = FALSE;

	//self->drawShadows = FALSE;
	self->extraSpace = 0;

	self->enableClickLayout = FALSE;
	self->highlightRow = -1;
	//self->extra_big = 0;

	self->showScrollV = TRUE;
	self->showScrollH = FALSE;

	self->drop = 0;
	self->resize = 0;

	self->title = DbValue_initEmpty();
	self->value = DbValue_initEmpty();

	self->imgBackground = Image4_init();

	self->whiteScroll = FALSE;

	GuiItem_setCallClick(&self->base, click);
}
void GuiItemLayout_init(GuiItemLayout* self, Quad2i grid, GuiItemCallback* click)
{
	GuiItemLayout_initScroll(self, grid, click, GuiScroll_initEmpty());
}

GuiItemLayout* GuiItemLayout_newExScroll(Quad2i grid, GuiItemCallback* click, GuiScroll scrollV)
{
	GuiItemLayout* self = Os_malloc(sizeof(GuiItemLayout));
	GuiItemLayout_initScroll(self, grid, click, scrollV);
	return self;
}
GuiItemLayout* GuiItemLayout_newScroll(Quad2i grid, GuiScroll scrollV)
{
	return GuiItemLayout_newExScroll(grid, 0, scrollV);
}
GuiItemLayout* GuiItemLayout_new(Quad2i grid)
{
	return GuiItemLayout_newScroll(grid, GuiScroll_initEmpty());
}
GuiItemLayout* GuiItemLayout_newEx(Quad2i grid, GuiItemCallback* click)
{
	return GuiItemLayout_newExScroll(grid, click, GuiScroll_initEmpty());
}

GuiItemLayout* GuiItemLayout_newCoord(GuiItem* src, BOOL sliderV, BOOL sliderH, Win* win)
{
	GuiItemLayout* self = GuiItemLayout_new(Quad2i_init());
	self->base.coordScreen = src->coordScreen;
	self->base.coordMove = src->coordMove;

	self->showScrollV = sliderV;
	self->showScrollH = sliderH;

	return self;
}
GuiItemLayout* GuiItemLayout_newTitle(Quad2i grid, DbValue title)
{
	GuiItemLayout* self = GuiItemLayout_new(grid);
	self->title = title;
	return self;
}

void GuiItemLayout_initCopy(GuiItemLayout* self, GuiItemLayout* src, BOOL copySub)
{
	*self = *src;
	GuiItem_initCopy(&self->base, &src->base, copySub);

	self->cols = GuiItemLayoutArray_initCopy(&src->cols);
	self->rows = GuiItemLayoutArray_initCopy(&src->rows);

	self->scrollH = GuiScroll_initCopy(&src->scrollH);
	self->scrollV = GuiScroll_initCopy(&src->scrollV);

	self->imgBackground = Image4_initCopy(&src->imgBackground);
}

GuiItem* GuiItemLayout_newCopy(GuiItemLayout* src, BOOL copySub)
{
	GuiItemLayout* self = Os_malloc(sizeof(GuiItemLayout));
	GuiItemLayout_initCopy(self, src, copySub);

	return(GuiItem*)self;
}

void GuiItemLayout_free(GuiItemLayout* self)
{
	GuiItemLayoutArray_free(&self->cols);
	GuiItemLayoutArray_free(&self->rows);

	GuiScroll_free(&self->scrollV);
	GuiScroll_free(&self->scrollH);

	Image4_free(&self->imgBackground);

	DbValue_free(&self->title);
	DbValue_free(&self->value);

	GuiItem_free(&self->base);
	Os_memset(self, sizeof(GuiItemLayout));
}
void GuiItemLayout_delete(GuiItemLayout* self)
{
	GuiItemLayout_free(self);
	Os_free(self, sizeof(GuiItemLayout));
}

void GuiItemLayout_clear(GuiItemLayout* self)
{
	GuiItemLayoutArray_clear(&self->cols);
	GuiItemLayoutArray_clear(&self->rows);
}

BIG GuiItemLayout_getRow(GuiItemLayout* self)
{
	return DbValue_getRow(&self->value);
}

void GuiItemLayout_setRow(GuiItemLayout* self, BIG row, UBIG index)
{
	DbValue_setRow(&self->value, row, index);
}

void GuiItemLayout_setDrop(GuiItemLayout* self, GuiItemCallback* drop)
{
	self->drop = drop;
}

void GuiItemLayout_setResize(GuiItemLayout* self, GuiItemCallback* resize)
{
	self->resize = resize;
}

void GuiItemLayout_setBackgroundWhite(GuiItemLayout* self, BOOL drawBackgroundWhite)
{
	self->drawBackgroundWhite = drawBackgroundWhite;
}
void GuiItemLayout_setBackgroundBlack(GuiItemLayout* self, BOOL drawBackgroundBlack)
{
	self->drawBackgroundBlack = drawBackgroundBlack;
}
void GuiItemLayout_setBackgroundMain(GuiItemLayout* self, BOOL drawBackgroundMain)
{
	self->drawBackgroundMain = drawBackgroundMain;
}

void GuiItemLayout_setBackgroundError(GuiItemLayout* self, BOOL drawBackgroundError)
{
	self->drawBackgroundError = drawBackgroundError;
}

void GuiItemLayout_setDrawBackground(GuiItemLayout* self, BOOL drawBackground)
{
	self->drawBackground = drawBackground;
}

void GuiItemLayout_setScrollV(GuiItemLayout* self, DbValue scrollV)
{
	GuiScroll_free(&self->scrollV);
	self->scrollV = GuiScroll_init(scrollV);
}

void GuiItemLayout_setScrollH(GuiItemLayout* self, DbValue value)
{
	GuiScroll_setValue(&self->scrollH, value);
}

UBIG GuiItemLayout_getWheelV(GuiItemLayout* self)
{
	return GuiScroll_getWheel(&self->scrollV);
}
UBIG GuiItemLayout_getWheelH(GuiItemLayout* self)
{
	return GuiScroll_getWheel(&self->scrollH);
}
void GuiItemLayout_setWheelV(GuiItemLayout* self, UBIG wheel)
{
	GuiScroll_setWheelDirect(&self->scrollV, wheel);
}

void GuiItemLayout_setDrawBorder(GuiItemLayout* self, BOOL drawBorder)
{
	self->drawBorder = drawBorder;
}

void GuiItemLayout_setScrollVPos(GuiItemLayout* self, BIG pos)
{
	GuiScroll_setWheelDirect(&self->scrollV, pos);
}

void GuiItemLayout_resizeArrayColumn(GuiItemLayout* self, UINT n)
{
	GuiItemLayoutArray_resize(&self->cols, n);
}
void GuiItemLayout_resizeArrayRow(GuiItemLayout* self, UINT n)
{
	GuiItemLayoutArray_resize(&self->rows, n);
}
void GuiItemLayout_clearArrays(GuiItemLayout* self)
{
	GuiItemLayout_resizeArrayColumn(self, 0);
	GuiItemLayout_resizeArrayRow(self, 0);
}

void GuiItemLayout_addColumn(GuiItemLayout* self, UINT pos, int value)
{
	GuiItemLayoutArray_add(&self->cols, pos, value);
}

void GuiItemLayout_addRow(GuiItemLayout* self, UINT pos, int value)
{
	GuiItemLayoutArray_add(&self->rows, pos, value);
}

Quad2i GuiItemLayout_convert(const GuiItemLayout* self, const int cell, const Quad2i in)
{
	Vec2i c = GuiItemLayoutArray_convert(&self->cols, cell, in.start.x, in.start.x + in.size.x);
	Vec2i r = GuiItemLayoutArray_convert(&self->rows, cell, in.start.y, in.start.y + in.size.y);

	if (self->base.icon)
		c.x += GuiItem_getIconSizeX();

	return Quad2i_init4(c.x, r.x, c.y, r.y);
}

Vec2i GuiItemLayout_getSubMaxGrid(GuiItemLayout* self)
{
	Quad2i mx = GuiItem_getSubMaxGrid(&self->base);
	return Quad2i_end(mx);
}

Vec2i GuiItemLayout_forceAutoResizeArray(GuiItemLayout* self)
{
	Vec2i end = GuiItemLayout_getSubMaxGrid(self);

	GuiItemLayoutArray_resize(&self->cols, end.x);
	GuiItemLayoutArray_resize(&self->rows, end.y);

	return end;
}

static Vec2i _GuiItemLayout_autoResizeArray(GuiItemLayout* self)
{
	Vec2i end = GuiItemLayout_getSubMaxGrid(self);

	if (end.x > self->cols.num) GuiItemLayoutArray_resize(&self->cols, end.x);
	if (end.y > self->rows.num) GuiItemLayoutArray_resize(&self->rows, end.y);

	//empty cols will have PRIOR (for rows there is scrolling so no PRIOR is not needed)
	int i;
	for (i = 0; i < self->cols.num; i++)
	{
		int v = Std_abs(self->cols.inputs[i]);
		self->cols.inputs[i] = GuiItem_isColEmpty(&self->base, i) && v > 1 ? -v : v;

		if (self->cols.inputs[i] < 0)
			self->cols.inputs[i] = self->cols.inputs[i] * 1;
	}

	return end;
}

void GuiItemLayout_updateArray(GuiItemLayout* self, int cell, Vec2i window)
{
	_GuiItemLayout_autoResizeArray(self);

	if (self->base.icon)
		window.x -= GuiItem_getIconSizeX();

	GuiItemLayoutArray_update(&self->cols, cell, window.x, TRUE);
	GuiItemLayoutArray_update(&self->rows, cell, window.y, TRUE);

	GuiItemLayoutArray_updateLittle(&self->cols, cell, window.x);

	//note: for rows there is scrolling so no PRIOR is not needed
}

BOOL GuiItemLayout_hasScrollV(const GuiItemLayout* self)
{
	return(GuiScroll_is(&self->scrollV) && self->showScrollV);
}

BOOL GuiItemLayout_hasScrollH(const GuiItemLayout* self)
{
	if (self->showScrollH)
		return GuiScroll_is(&self->scrollH);

	return(GuiScroll_is(&self->scrollH) && self->showScrollH);
}

void GuiItemLayout_showScroll(GuiItemLayout* self, BOOL v, BOOL h)
{
	self->showScrollV = v;
	self->showScrollH = h;
}

BOOL GuiItemLayout_isTitle(const GuiItemLayout* self)
{
	const UNI* title = DbValue_result(&self->title);
	return Std_sizeUNI(title) > 0;
}

Quad2i GuiItemLayout_getCoordSpace(GuiItemLayout* self, Quad2i coord)
{
	return Quad2i_addSpace(coord, self->extraSpace);
}

void GuiItemLayout_draw(GuiItemLayout* self, Image4* img, Quad2i coord, Win* win)
{
	coord = GuiItemLayout_getCoordSpace(self, coord);

	//if (self->drawShadows)
	//	coord = Quad2i_addSpace(coord, OsWinIO_shadows());

	if (self->drawBackground || self->drawBackgroundMain || self->drawBackgroundError || (self->highlightRow >= 0 && self->value.row == self->highlightRow))
	{
		Quad2i q = coord;
		if (GuiItemLayout_isTitle(self))
		{
			q.start.y += OsWinIO_cellSize() / 2;
			q.size.y -= OsWinIO_cellSize() / 2;
		}

		//background
		Image4_drawBoxQuad(img, q, self->base.back_cd);

		//background image
		Image4_copyDirect(img, q, &self->imgBackground);
	}
}

void GuiItemLayout_drawPost(GuiItemLayout* self, Image4* img, Quad2i coord, Win* win)
{
	coord = GuiItemLayout_getCoordSpace(self, coord);

	if (GuiItemLayout_hasScrollV(self))
	{
		if (self->whiteScroll)
			self->scrollV.cd = g_theme.white;
		GuiScroll_drawV(&self->scrollV, Vec2i_init2(coord.start.x + coord.size.x - GuiScroll_widthWin(win), coord.start.y), img, win);
	}

	if (GuiItemLayout_hasScrollH(self))
	{
		if (self->whiteScroll)
			self->scrollH.cd = g_theme.white;
		GuiScroll_drawH(&self->scrollH, Vec2i_init2(coord.start.x, coord.start.y + coord.size.y - GuiScroll_widthWin(win)), img, win);
	}
	if (GuiItemLayout_isTitle(self))
	{
		const int cell = OsWinIO_cellSize();
		int textH = _GuiItem_textSize(1, cell);
		OsFont* font = OsWinIO_getFontDefault();

		//border
		{
			Quad2i q = coord;
			q.start.y += cell / 2;
			q.size.y -= cell / 2;
			Image4_drawBorder(img, q, 1, self->base.front_cd);
		}

		//text
		int space = cell / 4;
		Vec2i mid = Vec2i_init2(coord.start.x + coord.size.x / 2, coord.start.y + cell / 2);
		Image4_drawTextBackground(img, mid, TRUE, font, DbValue_result(&self->title), textH, 0, self->base.front_cd, self->base.back_cd, space);

		//border around text
		{
			int ex;
			Vec2i ts = OsFont_getTextSize(font, DbValue_result(&self->title), textH, 0, &ex);
			Quad2i q = Quad2i_initMid(mid, ts);
			q = Quad2i_addSpace(q, -space);

			q.start.y++;
			Vec2i qe = Quad2i_end(q);
			Image4_drawLine(img, q.start, Vec2i_init2(qe.x, q.start.y), 1, self->base.front_cd);	//top H
			Image4_drawLine(img, Vec2i_init2(q.start.x, q.start.y - 1), Vec2i_init2(q.start.x, q.start.y + q.size.y / 2), 1, self->base.front_cd);	//left V
			Image4_drawLine(img, Vec2i_init2(qe.x, q.start.y), Vec2i_init2(qe.x, q.start.y + q.size.y / 2), 1, self->base.front_cd);	//right V
		}
	}
	else
		if (self->drawBorder)
			Image4_drawBorder(img, coord, self->drawBorderError ? 2 : 1, self->base.front_cd);
}

void GuiItemLayout_update(GuiItemLayout* self, Quad2i coord, Win* win)
{
	if (GuiItemLayout_hasScrollV(self))
		GuiItem_setRedraw(&self->base, GuiScroll_getRedrawAndReset(&self->scrollV));
	if (GuiItemLayout_hasScrollH(self))
		GuiItem_setRedraw(&self->base, GuiScroll_getRedrawAndReset(&self->scrollH));

	GuiItem_setRedraw(&self->base, DbValue_hasChanged(&self->title));
}

void GuiItemLayout_touch(GuiItemLayout* self, Quad2i coord, Win* win)
{
	Rgba back_cd = g_theme.background;
	Rgba front_cd = g_theme.black;

	if (self->drawBorderError)
		front_cd = g_theme.warning;

	if (self->drawBackgroundMain)
		back_cd = g_theme.main;

	if (self->drawBackgroundWhite)
		back_cd = GuiItemTheme_getWhite_Background();

	if (self->drawBackgroundBlack)
		back_cd = g_theme.black;

	if (self->drawBackgroundError)
		back_cd = g_theme.warning;

	if (self->value.row >= 0 && self->value.row == self->highlightRow)
		back_cd = Rgba_aprox(g_theme.background, g_theme.main, 0.5f);

	self->cdSelect = g_theme.main;
	self->cdWarning = g_theme.warning;

	BOOL endTouch = (OsWinIO_getTouch_action() == Win_TOUCH_DOWN_E) || (OsWinIO_getTouch_action() == Win_TOUCH_FORCE_DOWN_E);
	BOOL inside = Quad2i_inside(coord, OsWinIO_getTouchPos());

	if (endTouch && self->enableClickLayout) //must be 'endTouch', because startTouch calls click() which reset layout and button/text weren't never touched
	{
		if (inside)
		{
			GuiItem_setRedraw(&self->base, TRUE);
			GuiItem_callClick(&self->base);
		}
	}

	if (OsWinIO_isDrop(&coord) && self->drop)
	{
		if (self->drop)
			self->drop(&self->base);
	}

	if (GuiItem_isEnable(&self->base) && OsWinIO_canActiveRenderItem(self))
	{
		const int scroll_width = GuiScroll_widthWin(win);

		if (GuiItemLayout_hasScrollV(self))
			GuiScroll_touchV(&self->scrollV, self, coord, Vec2i_init2(coord.start.x + coord.size.x - scroll_width, coord.start.y), win);
		if (GuiItemLayout_hasScrollH(self))
			GuiScroll_touchH(&self->scrollH, self, coord, Vec2i_init2(coord.start.x, coord.start.y + coord.size.y - scroll_width), win, TRUE);
	}

	_GuiItem_updateFinalCd(&self->base, back_cd, front_cd, coord, win);
}

GuiItemLayout* GuiItemLayout_resize(GuiItemLayout* self, GuiItemLayout* layout, Win* win)
{
	if (!self->base.resize)
		return self;

	if (self->resize)
		self->resize(&self->base);

	layout = (GuiItemLayout*)self;

	const int cell = OsWinIO_cellSize();

	//update scroll
	GuiScroll_set(&layout->scrollV, GuiItem_maxSubY(&layout->base, win), self->base.coordScreen.size.y, cell);
	GuiScroll_set(&layout->scrollH, GuiItem_maxSubX(&layout->base, win), self->base.coordScreen.size.x, cell);

	BOOL ver = GuiItem_hasScrollLayoutV(&self->base);
	BOOL hor = GuiItem_hasScrollLayoutH(&self->base);

	if (ver)	self->base.coordScreen.size.x -= GuiScroll_widthWin(win);	//makes space for slider
	if (hor)	self->base.coordScreen.size.y -= GuiScroll_widthWin(win);	//makes space for slider

	GuiItemLayout_updateArray(layout, cell, self->base.coordScreen.size);

	if (ver)	self->base.coordScreen.size.x += GuiScroll_widthWin(win);
	if (hor)	self->base.coordScreen.size.y += GuiScroll_widthWin(win);

	return layout;
}

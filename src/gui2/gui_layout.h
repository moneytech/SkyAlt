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

typedef struct GuiLayout_s
{
	GuiBase base;

	GuiLayoutArray cols;
	GuiLayoutArray rows;

	GuiScroller scrollH;
	GuiScroller scrollV;

	BOOL showScrollV;
	BOOL showScrollH;
} GuiLayout;

GuiLayout* GuiLayout_newScroll(Quad2i grid)
{
	GuiLayout* self = Os_malloc(sizeof(GuiLayout));

	self->base = GuiBase_init(grid);

	self->cols = GuiLayoutArray_init();
	self->rows = GuiLayoutArray_init();

	self->scrollH = GuiScroller_init();
	self->scrollV = GuiScroller_init();

	self->showScrollV = TRUE;
	self->showScrollH = FALSE;

	return self;
}

void GuiLayout_initCopy(GuiLayout* self, GuiLayout* src, BOOL copySub)
{



}

GuiLayout* GuiLayout_newCopy(GuiLayout* src)
{
	GuiLayout* self = Os_malloc(sizeof(GuiLayout));

	*self = *src;
	GuiBase_initCopy(&self->base, &src->base);

	self->cols = GuiLayoutArray_initCopy(&src->cols);
	self->rows = GuiLayoutArray_initCopy(&src->rows);

	self->scrollH = GuiScroller_initCopy(&src->scrollH);
	self->scrollV = GuiScroller_initCopy(&src->scrollV);

	return self;
}


void GuiLayout_delete(GuiLayout* self)
{
	GuiLayoutArray_free(&self->cols);
	GuiLayoutArray_free(&self->rows);

	GuiScroller_free(&self->scrollV);
	GuiScroller_free(&self->scrollH);

	Os_free(self, sizeof(GuiLayout));
}

void GuiLayout_clear(GuiLayout* self)
{
	GuiLayoutArray_clear(&self->cols);
	GuiLayoutArray_clear(&self->rows);
}

/*BIG GuiLayout_getRow(GuiLayout* self)
{
	return DbValue_getRow(&self->value);
}

void GuiLayout_setRow(GuiLayout* self, BIG row, UBIG index)
{
	DbValue_setRow(&self->value, row, index);
}

void GuiLayout_setDrop(GuiLayout* self, GuiCallback* drop)
{
	self->drop = drop;
}

void GuiLayout_setResize(GuiLayout* self, GuiCallback* resize)
{
	self->resize = resize;
}


void GuiLayout_setBackgroundCdValue(GuiLayout* self, DbValue back_cd_value)
{
	self->back_cd_value = back_cd_value;
	self->drawBackground = (back_cd_value.column != 0);
}

void GuiLayout_setBackgroundWhite(GuiLayout* self, BOOL drawBackgroundWhite)
{
	self->drawBackgroundWhite = drawBackgroundWhite;
}
void GuiLayout_setBackgroundBlack(GuiLayout* self, BOOL drawBackgroundBlack)
{
	self->drawBackgroundBlack = drawBackgroundBlack;
}
void GuiLayout_setBackgroundGrey(GuiLayout* self, BOOL drawBackgroundGrey)
{
	self->drawBackgroundGrey = drawBackgroundGrey;
}

void GuiLayout_setBackgroundMain(GuiLayout* self, BOOL drawBackgroundMain)
{
	self->drawBackgroundMain = drawBackgroundMain;
}

void GuiLayout_setBackgroundError(GuiLayout* self, BOOL drawBackgroundError)
{
	self->drawBackgroundError = drawBackgroundError;
}

void GuiLayout_setDrawBackground(GuiLayout* self, BOOL drawBackground)
{
	self->drawBackground = drawBackground;
}*/


void GuiLayout_setScroll(GuiLayout* self, InterObject* v, InterObject* h)
{
	GuiScroller_setValue(&self->scrollV, v);
	GuiScroller_setValue(&self->scrollH, h);
}

/*void GuiLayout_setScrollV(GuiLayout* self, DbValue scrollV)
{
	GuiScroller_free(&self->scrollV);
	self->scrollV = GuiScroller_init(scrollV);
}

void GuiLayout_setScrollH(GuiLayout* self, DbValue value)
{
	GuiScroller_setValue(&self->scrollH, value);
}

UBIG GuiLayout_getWheelV(GuiLayout* self)
{
	return GuiScroller_getWheel(&self->scrollV);
}
UBIG GuiLayout_getWheelH(GuiLayout* self)
{
	return GuiScroller_getWheel(&self->scrollH);
}
void GuiLayout_setWheelV(GuiLayout* self, UBIG wheel)
{
	GuiScroller_setWheelDirect(&self->scrollV, wheel);
}
void GuiLayout_setWheelH(GuiLayout* self, UBIG wheel)
{
	GuiScroller_setWheelDirect(&self->scrollH, wheel);
}

void GuiLayout_setDrawBorder(GuiLayout* self, BOOL drawBorder)
{
	self->drawBorder = drawBorder;
}*/

void GuiLayout_resizeArrayColumn(GuiLayout* self, UINT n)
{
	GuiLayoutArray_resize(&self->cols, n);
}
void GuiLayout_resizeArrayRow(GuiLayout* self, UINT n)
{
	GuiLayoutArray_resize(&self->rows, n);
}
void GuiLayout_clearArrays(GuiLayout* self)
{
	GuiLayout_resizeArrayColumn(self, 0);
	GuiLayout_resizeArrayRow(self, 0);
}

void GuiLayout_addColumn(GuiLayout* self, UINT pos, int value)
{
	GuiLayoutArray_add(&self->cols, pos, value);
}

void GuiLayout_addRow(GuiLayout* self, UINT pos, int value)
{
	GuiLayoutArray_add(&self->rows, pos, value);
}

Quad2i GuiLayout_convert(const GuiLayout* self, const int cell, const Quad2i in)
{
	Vec2i c = GuiLayoutArray_convert(&self->cols, cell, in.start.x, in.start.x + in.size.x);
	Vec2i r = GuiLayoutArray_convert(&self->rows, cell, in.start.y, in.start.y + in.size.y);

	//if (self->base.icon)
	//	c.x += Gui_getIconSizeX();

	return Quad2i_init4(c.x, r.x, c.y, r.y);
}

Vec2i GuiLayout_getSubMaxGrid(GuiLayout* self)
{
	Quad2i mx = Gui_getSubMaxGrid(&self->base);
	return Quad2i_end(mx);
}


static Vec2i _GuiLayout_autoResizeArray(GuiLayout* self)
{
	Vec2i end = GuiLayout_getSubMaxGrid(self);

	if (end.x > self->cols.num) GuiLayoutArray_resize(&self->cols, end.x);
	if (end.y > self->rows.num) GuiLayoutArray_resize(&self->rows, end.y);

	//empty cols will have PRIOR (for rows there is scrolling so no PRIOR is not needed)
	int i;
	for (i = 0; i < self->cols.num; i++)
	{
		int v = Std_abs(self->cols.inputs[i]);
		self->cols.inputs[i] = Gui_isColEmpty(&self->base, i) && v > 1 ? -v : v;

		if (self->cols.inputs[i] < 0)
			self->cols.inputs[i] = self->cols.inputs[i] * 1;
	}

	return end;
}

void GuiLayout_updateArray(GuiLayout* self, int cell, Vec2i window)
{
	_GuiLayout_autoResizeArray(self);

	if (self->base.icon)
		window.x -= Gui_getIconSizeX();

	GuiLayoutArray_update(&self->cols, cell, window.x, TRUE);
	GuiLayoutArray_update(&self->rows, cell, window.y, TRUE);

	GuiLayoutArray_updateLittle(&self->cols, cell, window.x);

	//note: for rows there is scrolling so no PRIOR is not needed
}

BOOL GuiLayout_hasScrollV(const GuiLayout* self)
{
	return(GuiScroller_is(&self->scrollV) && self->showScrollV);
}

BOOL GuiLayout_hasScrollH(const GuiLayout* self)
{
	//if (self->showScrollH)
	//	return GuiScroller_is(&self->scrollH);

	return(GuiScroller_is(&self->scrollH) && self->showScrollH);
}

void GuiLayout_showScroll(GuiLayout* self, BOOL v, BOOL h)
{
	self->showScrollV = v;
	self->showScrollH = h;
}

/*BOOL GuiLayout_isTitle(const GuiLayout* self)
{
	const UNI* title = DbValue_result(&self->title);
	return Std_sizeUNI(title) > 0;
}*/

Quad2i GuiLayout_getCoordSpace(GuiLayout* self, Quad2i coord)
{
	return Quad2i_addSpace(coord, self->extraSpace);
}

void GuiLayout_draw(GuiLayout* self, Image4* img, Quad2i coord, Win* win)
{
	coord = GuiLayout_getCoordSpace(self, coord);

	//if (self->drawShadows)
	//	coord = Quad2i_addSpace(coord, OsWinIO_shadows());

	if (self->drawBackground || self->drawBackgroundMain || self->drawBackgroundError || (self->highlightRow >= 0 && self->value.row == self->highlightRow))
	{
		Quad2i q = coord;
		if (GuiLayout_isTitle(self))
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

void GuiLayout_drawPost(GuiLayout* self, Image4* img, Quad2i coord, Win* win)
{
	coord = GuiLayout_getCoordSpace(self, coord);

	//if (coord.start.x == 0 && self->scrollV.value.column)
	//	DbValue_setNumber(&self->scrollV.value, 0);
		//self->scrollV.wheel = 0;
		//printf("dd");

	if (GuiLayout_hasScrollV(self))
	{
		//if (self->whiteScroll)
		//	self->scrollV.cd = g_theme.white;
		GuiScroller_drawV(&self->scrollV, Vec2i_init2(coord.start.x + coord.size.x - GuiScroller_widthWin(win), coord.start.y), img, win);
	}

	if (GuiLayout_hasScrollH(self))
	{
		//if (self->whiteScroll)
		//	self->scrollH.cd = g_theme.white;
		GuiScroller_drawH(&self->scrollH, Vec2i_init2(coord.start.x, coord.start.y + coord.size.y - GuiScroller_widthWin(win)), img, win);
	}
	if (GuiLayout_isTitle(self))
	{
		const int cell = OsWinIO_cellSize();
		int textH = _Gui_textSize(1, cell);
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

void GuiLayout_update(GuiLayout* self, Quad2i coord, Win* win)
{
	DbValue_setRow(&self->back_cd_value, Gui_getRow(&self->base), 0);
	if (DbValue_is(&self->back_cd_value))
		self->back_cd = DbValue_getCd(&self->back_cd_value);

	//if (GuiLayout_hasScrollV(self))
		Gui_setRedraw(&self->base, GuiScroller_getRedrawAndReset(&self->scrollV));
	//if (GuiLayout_hasScrollH(self))
		Gui_setRedraw(&self->base, GuiScroller_getRedrawAndReset(&self->scrollH));

	Gui_setRedraw(&self->base, DbValue_hasChanged(&self->back_cd_value) || DbValue_hasChanged(&self->title));
}

void GuiLayout_touch(GuiLayout* self, Quad2i coord, Win* win)
{
	Rgba back_cd = g_theme.background;
	Rgba front_cd = g_theme.black;

	/*if (self->drawBorderError)
		front_cd = g_theme.warning;

	if (self->drawBackgroundMain)
		back_cd = g_theme.main;

	if (self->drawBackgroundWhite)
		back_cd = GuiTheme_getWhite_Background();

	if (self->drawBackgroundBlack)
		back_cd = g_theme.black;

	if (self->drawBackgroundGrey)
		back_cd = Rgba_aprox(g_theme.white, g_theme.black, 0.3f);
	

	if (self->drawBackgroundError)
		back_cd = g_theme.warning;

	if (self->value.row >= 0 && self->value.row == self->highlightRow)
		back_cd = Rgba_aprox(g_theme.background, g_theme.main, 0.5f);

	if (DbValue_is(&self->back_cd_value))
		back_cd = self->back_cd;

	self->cdSelect = g_theme.main;
	self->cdWarning = g_theme.warning;*/

	BOOL endTouch = (OsWinIO_getTouch_action() == Win_TOUCH_DOWN_E) || (OsWinIO_getTouch_action() == Win_TOUCH_FORCE_DOWN_E);
	BOOL inside = Quad2i_inside(coord, OsWinIO_getTouchPos());

	if (endTouch && self->enableClickLayout) //must be 'endTouch', because startTouch calls click() which reset layout and button/text weren't never touched
	{
		if (inside)
		{
			Gui_setRedraw(&self->base, TRUE);
			Gui_callClick(&self->base);
		}
	}

	/*if (OsWinIO_isDrop(&coord) && self->drop)
	{
		if (self->drop)
			self->drop(&self->base);
	}*/

	if (Gui_isEnable(&self->base) && OsWinIO_canActiveRenderItem(self))
	{
		const int scroll_width = GuiScroller_widthWin(win);

		if (GuiLayout_hasScrollV(self))
			GuiScroller_touchV(&self->scrollV, self, coord, Vec2i_init2(coord.start.x + coord.size.x - scroll_width, coord.start.y), win);
		if (GuiLayout_hasScrollH(self))
			GuiScroller_touchH(&self->scrollH, self, coord, Vec2i_init2(coord.start.x, coord.start.y + coord.size.y - scroll_width), win, TRUE);
	}

	_Gui_updateFinalCd(&self->base, back_cd, front_cd, coord, win);
}

GuiLayout* GuiLayout_resize(GuiLayout* self, GuiLayout* layout, Win* win)
{
	if (!self->base.resize)
		return self;

	//if (self->resize)
	//	self->resize(&self->base);

	layout = (GuiLayout*)self;

	const int cell = OsWinIO_cellSize();

	//update scroll
	GuiScroller_set(&layout->scrollV, Gui_maxSubY(&layout->base, win), self->base.coordScreen.size.y, cell);
	GuiScroller_set(&layout->scrollH, Gui_maxSubX(&layout->base, win), self->base.coordScreen.size.x, cell);

	BOOL ver = Gui_hasScrollLayoutV(&self->base);
	BOOL hor = Gui_hasScrollLayoutH(&self->base);

	if (ver)	self->base.coordScreen.size.x -= GuiScroller_widthWin(win);	//makes space for slider
	if (hor)	self->base.coordScreen.size.y -= GuiScroller_widthWin(win);	//makes space for slider

	GuiLayout_updateArray(layout, cell, self->base.coordScreen.size);

	if (ver)	self->base.coordScreen.size.x += GuiScroller_widthWin(win);
	if (hor)	self->base.coordScreen.size.y += GuiScroller_widthWin(win);

	return layout;
}

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

typedef struct GuiItemTextMulti_s
{
	GuiItem base;

	DbValue text;
	GuiScroll scroll;
}GuiItemTextMulti;
GuiItem* GuiItemTextMulti_new(Quad2i grid, DbValue text)
{
	GuiItemTextMulti* self = Os_malloc(sizeof(GuiItemTextMulti));

	self->base = GuiItem_init(GuiItem_TEXT_MULTI, grid);
	self->text = text;
	self->scroll = GuiScroll_initEmpty();

	return (GuiItem*)self;
}
GuiItem* GuiItemTextMulti_newCopy(GuiItemTextMulti* src, BOOL copySub)
{
	GuiItemTextMulti* self = Os_malloc(sizeof(GuiItemTextMulti));
	*self = *src;
	GuiItem_initCopy(&self->base, &src->base, copySub);

	self->text = DbValue_initCopy(&src->text);
	self->scroll = GuiScroll_initCopy(&src->scroll);

	return (GuiItem*)self;
}

void GuiItemTextMulti_delete(GuiItemTextMulti* self)
{
	GuiScroll_free(&self->scroll);
	DbValue_free(&self->text);

	GuiItem_free(&self->base);
	Os_free(self, sizeof(GuiItemTextMulti));
}

void GuiItemTextMulti_setText(GuiItemTextMulti* self, DbValue value)
{
	DbValue_free(&self->text);
	self->text = value;
}

void GuiItemTextMulti_drawIt(Image4* img, Quad2i coord, Win* win, const UNI* text, Rgba back_cd, Rgba front_cd, GuiScroll* scroll)
{
	//background
	Image4_drawBoxQuad(img, coord, back_cd);

	coord.size.x -= GuiScroll_widthWin(win);

	//border
	Image4_drawBorder(img, coord, 1, front_cd);

	Vec2i size = coord.size;
	int textH = _GuiItem_textSize(1, size.y);
	OsFont* font = OsWinIO_getFontDefault();
	Quad2i coordIn = Quad2i_addSpace(coord, 5);

	//scroll
	UBIG numLines = Image4_numTextLines(coordIn, font, text, textH) + 1;
	int itemH = textH + OsWinIO_lineSpace();
	GuiScroll_set(scroll, numLines * itemH, coord.size.y, itemH);
	GuiScroll_drawV(scroll, Vec2i_init2(coord.start.x + coord.size.x, coord.start.y), img, win);

	//text
	UBIG skipLines = GuiScroll_getWheelRow(scroll);
	const UNI* skipText = Image4_skipTextLines(coordIn, font, text, textH, skipLines);

	Image4_drawTextMulti(img, coordIn, FALSE, font, skipText, textH, OsWinIO_lineSpace(), front_cd);	//text
}

void GuiItemTextMulti_draw(GuiItemTextMulti* self, Image4* img, Quad2i coord, Win* win)
{
	GuiItemTextMulti_drawIt(img, coord, win, DbValue_result(&self->text), self->base.back_cd, self->base.front_cd, &self->scroll);
}

void GuiItemTextMulti_update(GuiItemTextMulti* self, Quad2i coord, Win* win)
{
	GuiItem_setRedraw(&self->base, (DbValue_hasChanged(&self->text) || GuiScroll_getRedrawAndReset(&self->scroll)));
}

void GuiItemTextMulti_touch(GuiItemTextMulti* self, Quad2i coord, Win* win)
{
	Rgba back_cd = g_theme.background;
	Rgba front_cd = g_theme.black;

	if (self->base.touch && GuiItem_isEnable(&self->base) && OsWinIO_canActiveRenderItem(self))
	{
		const int scroll_width = GuiScroll_widthWin(win);
		GuiScroll_touchV(&self->scroll, self, coord, Vec2i_init2(coord.start.x + coord.size.x - scroll_width, coord.start.y), win);
	}

	_GuiItem_updateFinalCd(&self->base, back_cd, front_cd, coord, win);
}

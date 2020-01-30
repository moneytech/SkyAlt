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

typedef struct GuiItemCheck_s
{
	GuiItem base;
	DbValue value;
	DbValue description;

	UINT numClickIn;
}GuiItemCheck;

GuiItem* GuiItemCheck_newEx(Quad2i grid, DbValue value, DbValue description, GuiItemCallback* click)
{
	GuiItemCheck* self = Os_malloc(sizeof(GuiItemCheck));
	self->base = GuiItem_init(GuiItem_CHECK, grid);

	self->value = value;
	self->description = description;

	GuiItem_setCallClick(&self->base, click);

	self->numClickIn = 1;

	return (GuiItem*)self;
}

GuiItem* GuiItemCheck_new(Quad2i grid, DbValue value, DbValue description)
{
	return GuiItemCheck_newEx(grid, value, description, 0);
}

GuiItem* GuiItemCheck_newCopy(GuiItemCheck* src, BOOL copySub)
{
	GuiItemCheck* self = Os_malloc(sizeof(GuiItemCheck));
	*self = *src;
	GuiItem_initCopy(&self->base, &src->base, copySub);

	self->value = DbValue_initCopy(&src->value);
	self->description = DbValue_initCopy(&src->description);

	return (GuiItem*)self;
}

void GuiItemCheck_delete(GuiItemCheck* self)
{
	DbValue_free(&self->value);
	DbValue_free(&self->description);

	GuiItem_free(&self->base);
	Os_free(self, sizeof(GuiItemCheck));
}

BOOL GuiItemCheck_isActive(GuiItemCheck* self)
{
	return (DbValue_getNumber(&self->value) != 0);
}

void GuiItemCheck_draw(GuiItemCheck* self, Image4* img, Quad2i coord, Win* win)
{
	const int cell = OsWinIO_cellSize();
	//Quad2i box = Quad2i_addSpace(coord, cell / 4);

	const int s = cell / 2;
	//if (self->base.drawTable)
	Quad2i box = Quad2i_initMid(Quad2i_getMiddle(coord), Vec2i_init2(s, s));

	if (!self->base.drawTable)
		box.start.x = coord.start.x + cell / 4;

	box.size.x = box.size.y;

	//background
	if (self->base.drawTable)
		Image4_drawBoxQuad(img, coord, self->base.back_cd);

	if (self->base.drawTable)
	{
		Quad2i q = coord;
		q.size.x += 1;
		q.size.y += 1;
		//img->rect = q;
		Image4_drawBorder(img, q, 1, Rgba_aprox(self->base.back_cd, self->base.front_cd, 0.5f));
	}

	//border
	Image4_drawBorder(img, box, 1, self->base.front_cd);

	//inside(checked)
	if (GuiItemCheck_isActive(self))
	{
		box = Quad2i_addSpace(box, 3);

		const int FAT = Std_max(1, cell / 10);
		const int sm = box.size.x / 3 * 1;
		const int bg = box.size.x / 3 * 2;

		Vec2i mid = Vec2i_init2(box.start.x + box.size.x / 3,
			box.start.y + box.size.y - FAT);

		Image4_drawLine(img, mid, Vec2i_init2(mid.x - sm, mid.y - sm), FAT, self->base.front_cd);	//to left
		Image4_drawLine(img, mid, Vec2i_init2(mid.x + bg, mid.y - bg), FAT, self->base.front_cd);	//to right
	}

	//text
	Vec2i pos;
	pos.x = coord.start.x + cell;
	pos.y = coord.start.y + coord.size.y / 2;
	int textH = _GuiItem_textSize(1, coord.size.y);
	OsFont* font = OsWinIO_getFontDefault();
	Image4_drawText(img, pos, FALSE, font, DbValue_result(&self->description), textH, 0, self->base.front_cd);
}

void GuiItemCheck_update(GuiItemCheck* self, Quad2i coord, Win* win)
{
	GuiItem_setRedraw(&self->base, (DbValue_hasChanged(&self->value) || DbValue_hasChanged(&self->description)));
}

void GuiItemCheck_touch(GuiItemCheck* self, Quad2i coord, Win* win)
{
	Rgba back_cd = g_theme.white;
	Rgba front_cd = g_theme.black;

	if (self->base.touch && GuiItem_isEnable(&self->base) && OsWinIO_canActiveRenderItem(self))
	{
		BOOL startTouch = (OsWinIO_getTouch_action() == Win_TOUCH_DOWN_S) || (OsWinIO_getTouch_action() == Win_TOUCH_FORCE_DOWN_S);
		BOOL endTouch = (OsWinIO_getTouch_action() == Win_TOUCH_DOWN_E) || (OsWinIO_getTouch_action() == Win_TOUCH_FORCE_DOWN_E);
		UINT numClickIn = self->numClickIn;

		BOOL active = OsWinIO_isActiveRenderItem(self);
		BOOL inside = Quad2i_inside(coord, OsWinIO_getTouchPos());
		BOOL touch = (startTouch && OsWinIO_getTouchNum() >= numClickIn) || active;

		if (inside && touch)	//full touch
		{
			back_cd = g_theme.white;
			front_cd = g_theme.black;
			OsWinIO_setActiveRenderItem(self);
		}
		else
			if ((inside && !touch) || (active && !inside))		//mid color
			{
				back_cd = Rgba_aprox(back_cd, front_cd, 0.2f);
				front_cd = Rgba_aprox(front_cd, back_cd, 0.2f);
			}

		if (inside && active && endTouch)	//end
		{
			GuiItemEdit_saveCache();
			DbValue_setNumber(&self->value, !GuiItemCheck_isActive(self));
			GuiItem_callClick(&self->base);
		}

		if (endTouch)
			OsWinIO_resetActiveRenderItem();

		//cursor ...
		//if(inside)
		//	Win_udpateCursor(win, Win_CURSOR_HAND);
	}

	_GuiItem_updateFinalCd(&self->base, back_cd, front_cd, coord, win);
}

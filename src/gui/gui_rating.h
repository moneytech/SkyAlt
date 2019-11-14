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

typedef struct GuiItemRating_s
{
	GuiItem base;

	GuiImage* starBlack;
	GuiImage* starWhite;

	DbValue value;
	DbValue maxValue;
	DbValue description;
}GuiItemRating;

GuiItem* GuiItemRating_new(Quad2i grid, DbValue value, DbValue maxValue, DbValue description, GuiItemCallback* call)
{
	GuiItemRating* self = Os_malloc(sizeof(GuiItemRating));
	self->base = GuiItem_init(GuiItem_RATING, grid);

	self->value = value;
	self->maxValue = maxValue;
	self->description = description;

	self->starBlack = GuiImage_new1(UiIcons_init_star_black());
	self->starWhite = GuiImage_new1(UiIcons_init_star_white());

	GuiItem_setCallClick((GuiItem*)self, call);

	return (GuiItem*)self;
}

GuiItem* GuiItemRating_newCopy(GuiItemRating* src, BOOL copySub)
{
	GuiItemRating* self = Os_malloc(sizeof(GuiItemRating));
	*self = *src;
	GuiItem_initCopy(&self->base, &src->base, copySub);

	self->value = DbValue_initCopy(&src->value);
	self->maxValue = DbValue_initCopy(&src->maxValue);
	self->description = DbValue_initCopy(&src->description);

	self->starBlack = GuiImage_newCopy(src->starBlack);
	self->starWhite = GuiImage_newCopy(src->starWhite);

	return (GuiItem*)self;
}

void GuiItemRating_delete(GuiItemRating* self)
{
	DbValue_free(&self->maxValue);
	DbValue_free(&self->value);
	DbValue_free(&self->description);

	GuiImage_delete(self->starBlack);
	GuiImage_delete(self->starWhite);

	GuiItem_free(&self->base);
	Os_free(self, sizeof(GuiItemRating));
}

double GuiItemRating_getNumber(GuiItemRating* self)
{
	return Std_bclamp(DbValue_getNumber(&self->value), 0, DbValue_getNumber(&self->maxValue));
}
void GuiItemRating_setNumber(GuiItemRating* self, double value)
{
	DbValue_setNumber(&self->value, Std_bclamp(value, 0, DbValue_getNumber(&self->maxValue)));
}

static double _GuiItemRating_getValueT(GuiItemRating* self)
{
	return GuiItemRating_getNumber(self) / DbValue_getNumber(&self->maxValue);
}

static void _GuiItemRating_setValueT(GuiItemRating* self, double t)
{
	double v = DbValue_getNumber(&self->maxValue) * t;
	DbValue_setNumber(&self->value, v);
}

static int _GuiItemRating_starSize()
{
	return OsWinIO_cellSize() / 2;
}

static Vec2i _GuiItemRating_sizeSize(GuiItemRating* self)
{
	const double max = DbValue_getNumber(&self->maxValue);
	int sz = _GuiItemRating_starSize();

	return Vec2i_init2(sz * max, sz);
}
static Vec2i _GuiItemRating_starStart(GuiItemRating* self, Quad2i coord)
{
	const double max = DbValue_getNumber(&self->maxValue);
	int sz = _GuiItemRating_starSize();

	int x = (max * sz > coord.size.x) ? coord.start.x : (coord.start.x + coord.size.x / 2 - sz * max / 2);
	int y = coord.start.y + coord.size.y / 2 - sz / 2;

	return Vec2i_init2(x, y);
}

void GuiItemRating_draw(GuiItemRating* self, Image4* img, Quad2i coord, Win* win)
{
	const int cell = OsWinIO_cellSize();

	Quad2i origCoord = coord;

	if (self->base.drawTable)
		Image4_drawBoxQuad(img, coord, self->base.back_cd);

	//description
	const UNI* description = DbValue_result(&self->description);
	if (Std_sizeUNI(description) && coord.size.y * 1.5f > 2 * cell)
	{
		int textH = _GuiItem_textSize(1, coord.size.y);
		OsFont* font = OsWinIO_getFontDefault();
		Image4_drawText(img, Vec2i_add(Vec2i_init2(coord.size.x / 2, cell / 2), coord.start), TRUE, font, description, textH, 0, self->base.front_cd);

		//move
		coord.start.y += cell;
		coord.size.y -= cell;
	}

	int sz = _GuiItemRating_starSize();

	const double n = GuiItemRating_getNumber(self);
	const double max = DbValue_getNumber(&self->maxValue);

	Vec2i st = _GuiItemRating_starStart(self, coord);

	int i;
	for (i = 0; i < n; i++)
		GuiImage_draw(self->starBlack, img, Quad2i_init4(st.x + i * sz, st.y, sz, sz), Rgba_initBlack());

	for (i = n; i < max; i++)
		GuiImage_draw(self->starWhite, img, Quad2i_init4(st.x + i * sz, st.y, sz, sz), Rgba_initBlack());

	if (self->base.drawTable)
	{
		Quad2i q = origCoord;
		q.size.x += 1;
		q.size.y += 1;
		Image4_drawBorder(img, q, 1, Rgba_aprox(self->base.back_cd, self->base.front_cd, 0.5f));
	}
}

void GuiItemRating_update(GuiItemRating* self, Quad2i coord, Win* win)
{
	Quad2i q = coord;
	q.size.x = q.size.y = _GuiItemRating_starSize();
	GuiImage_update(self->starBlack, q);
	GuiImage_update(self->starWhite, q);

	GuiItem_setRedraw(&self->base, (DbValue_hasChanged(&self->maxValue) || DbValue_hasChanged(&self->value) || DbValue_hasChanged(&self->description)));
}

void GuiItemRating_touch(GuiItemRating* self, Quad2i coord, Win* win)
{
	Rgba back_cd = self->base.drawTable ? g_theme.white : g_theme.black;
	Rgba front_cd = g_theme.black;

	if (self->base.touch && GuiItem_isEnable(&self->base) && OsWinIO_canActiveRenderItem(self))
	{
		double origT = _GuiItemRating_getValueT(self);
		double t = origT;

		BOOL startTouch = (OsWinIO_getTouch_action() == Win_TOUCH_DOWN_S) || (OsWinIO_getTouch_action() == Win_TOUCH_FORCE_DOWN_S);
		BOOL endTouch = (OsWinIO_getTouch_action() == Win_TOUCH_DOWN_E) || (OsWinIO_getTouch_action() == Win_TOUCH_FORCE_DOWN_E);

		BOOL active = OsWinIO_isActiveRenderItem(self);
		BOOL inside = Quad2i_inside(coord, OsWinIO_getTouchPos());
		BOOL touch = (startTouch && OsWinIO_getTouchNum() >= 1) || active;

		if (inside && touch)	//full touch
		{
			back_cd = Rgba_aprox(back_cd, g_theme.white, 0.3f);
			OsWinIO_setActiveRenderItem(self);

			//set
			int stX = _GuiItemRating_starStart(self, coord).x - _GuiItemRating_starSize();
			int szX = _GuiItemRating_sizeSize(self).x;

			t = (OsWinIO_getTouchPos().x - stX) / (double)szX;
			_GuiItemRating_setValueT(self, t);
		}
		else
			if ((inside && !touch) || (active && !inside))		//mid color
			{
				back_cd = Rgba_aprox(back_cd, g_theme.white, 0.2f);
			}

		if (inside && active && endTouch)	//end
		{
			GuiItemEdit_saveCache();
			GuiItem_callClick(&self->base);
		}

		if (endTouch)
			OsWinIO_resetActiveRenderItem();

		//cursor
		if (inside)
			Win_updateCursor(win, Win_CURSOR_HAND);

		if (origT != t)
			GuiItem_setRedraw(&self->base, TRUE);
	}

	_GuiItem_updateFinalCd(&self->base, back_cd, front_cd, coord, win);
}

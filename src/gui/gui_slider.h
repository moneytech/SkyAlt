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

typedef struct GuiItemSlider_s
{
	GuiItem base;
	DbValue minValue;
	DbValue maxValue;
	DbValue jumpValue;
	int text_level;

	DbValue value;
	DbValue description;
	DbValue left;
	DbValue right;

	UINT numClickIn;
}GuiItemSlider;

GuiItem* GuiItemSlider_newEx(Quad2i grid, DbValue minValue, DbValue maxValue, DbValue jumpValue, DbValue value, DbValue description, DbValue left, DbValue right, GuiItemCallback* call)
{
	GuiItemSlider* self = Os_malloc(sizeof(GuiItemSlider));
	self->base = GuiItem_init(GuiItem_SLIDER, grid);
	self->minValue = minValue;
	self->maxValue = maxValue;
	self->jumpValue = jumpValue;
	self->text_level = 1;

	self->value = value;
	self->description = description;

	self->left = left;
	self->right = right;

	self->numClickIn = 1;

	GuiItem_setCallClick((GuiItem*)self, call);

	return (GuiItem*)self;
}

GuiItem* GuiItemSlider_new(Quad2i grid, DbValue minValue, DbValue maxValue, DbValue jumpValue, DbValue value, DbValue description)
{
	return GuiItemSlider_newEx(grid, minValue, maxValue, jumpValue, value, description, DbValue_initEmpty(), DbValue_initEmpty(), 0);
}

GuiItem* GuiItemSlider_newCopy(GuiItemSlider* src, BOOL copySub)
{
	GuiItemSlider* self = Os_malloc(sizeof(GuiItemSlider));
	*self = *src;
	GuiItem_initCopy(&self->base, &src->base, copySub);

	self->minValue = DbValue_initCopy(&src->minValue);
	self->maxValue = DbValue_initCopy(&src->maxValue);
	self->jumpValue = DbValue_initCopy(&src->jumpValue);

	self->value = DbValue_initCopy(&src->value);
	self->description = DbValue_initCopy(&src->description);
	self->left = DbValue_initCopy(&src->left);
	self->right = DbValue_initCopy(&src->right);

	return (GuiItem*)self;
}

void GuiItemSlider_delete(GuiItemSlider* self)
{
	DbValue_free(&self->minValue);
	DbValue_free(&self->maxValue);
	DbValue_free(&self->jumpValue);

	DbValue_free(&self->value);
	DbValue_free(&self->description);

	DbValue_free(&self->left);
	DbValue_free(&self->right);

	GuiItem_free(&self->base);
	Os_free(self, sizeof(GuiItemSlider));
}

double GuiItemSlider_getNumber(GuiItemSlider* self)
{
	return DbValue_getNumber(&self->value);
}
void GuiItemSlider_setNumber(GuiItemSlider* self, double value)
{
	DbValue_setNumber(&self->value, value);
}

static double _GuiItemSlider_getValue(double v, double minValue, double maxValue, double jumpValue)
{
	v = Std_roundBy(v, minValue, jumpValue);
	return Std_fclamp(v, minValue, maxValue);
}

double GuiItemSlider_getValueT(GuiItemSlider* self)
{
	double minValue = DbValue_getNumber(&self->minValue);
	double maxValue = DbValue_getNumber(&self->maxValue);
	double jumpValue = DbValue_getNumber(&self->jumpValue);

	double v = _GuiItemSlider_getValue(DbValue_getNumber(&self->value), minValue, maxValue, jumpValue);

	return Std_fclamp((v - minValue) / (maxValue - minValue), 0, 1);
}

static void _GuiItemSlider_setValue(GuiItemSlider* self, double t)
{
	double minValue = DbValue_getNumber(&self->minValue);
	double maxValue = DbValue_getNumber(&self->maxValue);
	double jumpValue = DbValue_getNumber(&self->jumpValue);

	double v = minValue + (maxValue - minValue) * t;

	v = _GuiItemSlider_getValue(v, minValue, maxValue, jumpValue);

	DbValue_setNumber(&self->value, v);
}

void GuiItemSlider_draw(GuiItemSlider* self, Image4* img, Quad2i coord, Win* win)
{
	if (self->base.drawTable)
		Image4_drawBoxQuad(img, coord, self->base.back_cd);

	int cell = OsWinIO_cellSize();

	Vec2i size = coord.size;
	int textH = _GuiItem_textSize(self->text_level, size.y);
	OsFont* font = OsWinIO_getFontDefault();

	double t = GuiItemSlider_getValueT(self);

	//if (size.x >= size.y)	//horizontal
	{
		const int lineH2 = cell * 0.2f;
		const int lineH3 = cell * 0.3f;
		Quad2i slider = Quad2i_initSE(Vec2i_add(Vec2i_init2(size.x * t - 3, size.y / 2 - lineH3), coord.start), Vec2i_add(Vec2i_init2(size.x * t + 3, size.y / 2 + lineH3), coord.start));

		//description
		Image4_drawText(img, Vec2i_add(Vec2i_init2(size.x / 2, size.y / 4), coord.start), TRUE, font, DbValue_result(&self->description), textH, 0, self->base.front_cd);

		//title
		Image4_drawText(img, Vec2i_add(Vec2i_init2(0, size.y / 4 * 3), coord.start), FALSE, font, DbValue_result(&self->left), textH, 0, self->base.front_cd);
		int extra_down;
		int r = OsFont_getTextSize(font, DbValue_result(&self->right), textH, 0, &extra_down).x;
		Image4_drawText(img, Vec2i_add(Vec2i_init2(size.x - r, size.y / 4 * 3), coord.start), FALSE, font, DbValue_result(&self->right), textH, 0, self->base.front_cd);

		const int FAT = 1;
		//mid-line
		Image4_drawBoxStartEnd(img, Vec2i_add(Vec2i_init2(FAT, size.y / 2 - FAT / 2), coord.start), Vec2i_add(Vec2i_init2(size.x - FAT, size.y / 2 + FAT), coord.start), self->base.front_cd);
		Image4_drawBoxStartEnd(img, Vec2i_add(Vec2i_init2(FAT, size.y / 2 - lineH2), coord.start), Vec2i_add(Vec2i_init2(FAT * 2, size.y / 2 + lineH2), coord.start), self->base.front_cd);
		Image4_drawBoxStartEnd(img, Vec2i_add(Vec2i_init2(size.x - 2 * FAT, size.y / 2 - lineH2), coord.start), Vec2i_add(Vec2i_init2(size.x - FAT, size.y / 2 + lineH2), coord.start), self->base.front_cd);

		//slider
		//Image4_drawBoxQuad(img, slider, self->base.front_cd);
		Image4_drawCircle(img, Quad2i_getMiddle(slider), cell * 0.2f, self->base.front_cd);
	}
	//else	//vertical
	{
		//...
	}

	if (self->base.drawTable)
	{
		Quad2i q = coord;
		q.size.x += 1;
		q.size.y += 1;
		//img->rect = q;
		Image4_drawBorder(img, q, 1, Rgba_aprox(self->base.back_cd, self->base.front_cd, 0.5f));
	}
}

void GuiItemSlider_update(GuiItemSlider* self, Quad2i coord, Win* win)
{
	GuiItem_setRedraw(&self->base, (DbValue_hasChanged(&self->minValue) || DbValue_hasChanged(&self->maxValue) || DbValue_hasChanged(&self->jumpValue) || DbValue_hasChanged(&self->value) || DbValue_hasChanged(&self->description) || DbValue_hasChanged(&self->left) || DbValue_hasChanged(&self->right)));
}

void GuiItemSlider_touch(GuiItemSlider* self, Quad2i coord, Win* win)
{
	Rgba back_cd = self->base.drawTable ? g_theme.white : g_theme.black;
	Rgba front_cd = g_theme.black;

	if (self->base.touch && GuiItem_isEnable(&self->base) && OsWinIO_canActiveRenderItem(self))
	{
		double origT = GuiItemSlider_getValueT(self);
		double t = origT;

		BOOL startTouch = (OsWinIO_getTouch_action() == Win_TOUCH_DOWN_S) || (OsWinIO_getTouch_action() == Win_TOUCH_FORCE_DOWN_S);
		BOOL endTouch = (OsWinIO_getTouch_action() == Win_TOUCH_DOWN_E) || (OsWinIO_getTouch_action() == Win_TOUCH_FORCE_DOWN_E);
		UINT numClickIn = self->numClickIn;

		BOOL active = OsWinIO_isActiveRenderItem(self);
		BOOL inside = Quad2i_inside(coord, OsWinIO_getTouchPos());
		BOOL touch = (startTouch && OsWinIO_getTouchNum() >= numClickIn) || active;

		BOOL call = FALSE;

		if (OsWinIO_getTouch_action() == Win_TOUCH_WHEEL && inside && numClickIn == 1)
		{
			t -= 0.05f * OsWinIO_getTouch_wheel();	//+- 5%
			_GuiItemSlider_setValue(self, Std_fclamp(t, 0, 1));

			call = TRUE;
			OsWinIO_resetTouch();
		}

		if (inside && touch)	//full touch
		{
			back_cd = Rgba_aprox(back_cd, g_theme.white, 0.3f);
			OsWinIO_setActiveRenderItem(self);

			//set
			t = (OsWinIO_getTouchPos().x - coord.start.x) / (double)coord.size.x;
			_GuiItemSlider_setValue(self, t);
			call = TRUE;
		}
		else
			if ((inside && !touch) || (active && !inside))		//mid color
			{
				back_cd = Rgba_aprox(back_cd, g_theme.white, 0.2f);
			}

		if (inside && active && endTouch)	//end
		{
			GuiItemEdit_saveCache();
			call = TRUE;
		}

		if (call)
			GuiItem_callClick(&self->base);

		if (endTouch)
			OsWinIO_resetActiveRenderItem();

		//cursor
		//if(inside)
		//	Win_updateCursor(win, Win_CURSOR_HAND);

		if (origT != t)
			GuiItem_setRedraw(&self->base, TRUE);
	}

	_GuiItem_updateFinalCd(&self->base, back_cd, front_cd, coord, win);
}

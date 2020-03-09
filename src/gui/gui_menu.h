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

typedef struct GuiItemMenuItem_s
{
	GuiImage* icon;
	DbValue text;
	GuiItemCallback* click;
	BOOL confirm;
	BOOL separ_line;
	BIG attr_type;
}GuiItemMenuItem;

GuiItemMenuItem* GuiItemMenuItem_new(GuiImage* icon, DbValue text, GuiItemCallback* click, BOOL confirm, BOOL separ_line, BIG attr_type)
{
	GuiItemMenuItem* self = Os_malloc(sizeof(GuiItemMenuItem));
	self->icon = icon;
	self->text = text;
	self->click = click;
	self->confirm = confirm;
	self->attr_type = attr_type;
	self->separ_line = separ_line;
	return self;
}
GuiItemMenuItem* GuiItemMenuItem_newCopy(GuiItemMenuItem* src)
{
	return GuiItemMenuItem_new(GuiImage_newCopy(src->icon), DbValue_initCopy(&src->text), src->click, src->confirm, src->separ_line, src->attr_type);
}

void GuiItemMenuItem_delete(GuiItemMenuItem* self)
{
	GuiImage_delete(self->icon);
	DbValue_free(&self->text);
	Os_free(self, sizeof(GuiItemMenuItem));
}

typedef struct GuiItemMenu_s
{
	GuiItem base;

	DbValue value;
	//DbValue hasColors;

	int text_level;

	StdArr items;	//context(button list)

	BOOL circle;
	BOOL underline;
	BOOL mouseOver;
	BOOL textCenter;
	BOOL transparent;

	BOOL closeAuto;

	GuiImage* image;
	BOOL imageIcon;

	BOOL highligthBackground;
	float highligthAlpha;

	BOOL err;

	GuiItemLayout* context;
}GuiItemMenu;

GuiItemMenuItem* GuiItemMenu_getItem(GuiItemMenu* self, int i)
{
	return self->items.ptrs[i];
}

void GuiItemMenu_addItemIcon(GuiItemMenu* self, GuiImage* icon, DbValue text, GuiItemCallback* click, BOOL confirm, BOOL separ_line, BIG attr_type)
{
	GuiItemMenuItem* it = GuiItemMenuItem_new(icon, text, click, confirm, separ_line, attr_type);
	StdArr_add(&self->items, it);
}

void GuiItemMenu_addItemEx(GuiItemMenu* self, DbValue text, GuiItemCallback* click, BOOL confirm, BOOL separ_line, BIG attr_type)
{
	GuiItemMenu_addItemIcon(self, 0, text, click, confirm, separ_line, attr_type);
}

void GuiItemMenu_addItem(GuiItemMenu* self, DbValue text, GuiItemCallback* click)
{
	GuiItemMenu_addItemEx(self, text, click, FALSE, FALSE, -1);
}

void GuiItemMenu_addItemEmpty(GuiItemMenu* self)
{
	GuiItemMenu_addItem(self, DbValue_initEmpty(), 0);
}

GuiItem* GuiItemMenu_new(Quad2i grid, DbValue value, BOOL circle)
{
	GuiItemMenu* self = Os_malloc(sizeof(GuiItemMenu));
	self->base = GuiItem_init(GuiItem_MENU, grid);

	self->value = value;
	//self->hasColors = DbValue_initEmpty();
	self->text_level = 1;

	self->items = StdArr_init();

	self->circle = circle;
	self->underline = TRUE;
	self->mouseOver = TRUE;
	self->image = 0;
	self->imageIcon = FALSE;

	self->context = 0;
	self->textCenter = TRUE;
	self->closeAuto = TRUE;
	self->highligthBackground = FALSE;
	self->highligthAlpha = 1.0f;
	self->transparent = TRUE;

	self->base.icon_draw_back = FALSE;

	self->err = FALSE;

	return (GuiItem*)self;
}

GuiItem* GuiItemMenu_newImage(Quad2i grid, GuiImage* image, BOOL imageIcon)
{
	GuiItemMenu* self = (GuiItemMenu*)GuiItemMenu_new(grid, DbValue_initEmpty(), FALSE);
	self->image = image;
	self->imageIcon = imageIcon;
	return (GuiItem*)self;
}

GuiItem* GuiItemMenu_newCopy(GuiItemMenu* src, BOOL copySub)
{
	GuiItemMenu* self = Os_malloc(sizeof(GuiItemMenu));
	*self = *src;
	GuiItem_initCopy(&self->base, &src->base, copySub);

	if (self->image)	self->image = GuiImage_newCopy(src->image);

	self->value = DbValue_initCopy(&src->value);
	//self->hasColors = DbValue_initCopy(&src->hasColors);

	self->items = StdArr_initCopyFn(&src->items, (StdArrCOPY)&GuiItemMenuItem_newCopy);

	if (src->context)
		self->context = (GuiItemLayout*)GuiItem_newCopy((GuiItem*)src->context, TRUE);

	return (GuiItem*)self;
}

void GuiItemMenu_clearItems(GuiItemMenu* self)
{
	StdArr_freeFn(&self->items, (StdArrFREE)&GuiItemMenuItem_delete);
}

void GuiItemMenu_delete(GuiItemMenu* self)
{
	if (self->image)
		GuiImage_delete(self->image);

	DbValue_free(&self->value);
	//DbValue_free(&self->hasColors);

	GuiItemMenu_clearItems(self);

	if (self->context)
		GuiItem_delete((GuiItem*)self->context);

	GuiItem_free(&self->base);
	Os_free(self, sizeof(GuiItemMenu));
}

DbValue* _GuiItemMenu_findItem(GuiItemMenu* self, const UNI* name)
{
	int i;
	for (i = 0; i < self->items.num; i++)
	{
		GuiItemMenuItem* it = GuiItemMenu_getItem(self, i);
		DbValue_hasChanged(&it->text);
		if (Std_cmpUNI(DbValue_result(&it->text), name))
			return &it->text;
	}
	return 0;
}


/*void GuiItemMenu_setHasColors(GuiItemMenu* self, DbValue hasColors)
{
	DbValue_free(&self->hasColors);
	self->hasColors = hasColors;
}
BOOL GuiItemMenu_hasColors(const GuiItemMenu* self)
{
	return DbValue_getNumber(&self->hasColors);
}*/

void GuiItemMenu_setUnderline(GuiItemMenu* self, BOOL underline)
{
	self->underline = underline;
}

void GuiItemMenu_setCenter(GuiItemMenu* self, BOOL textCenter)
{
	self->textCenter = textCenter;
}

static void _GuiItemMenu_updateIconDrawBack(GuiItemMenu* self)
{
	self->base.icon_draw_back = !self->transparent || !self->highligthBackground;
}

void GuiItemMenu_setTransparent(GuiItemMenu* self, BOOL transparent)
{
	self->transparent = transparent;
	_GuiItemMenu_updateIconDrawBack(self);
}

void GuiItemMenu_setHighligthBackground(GuiItemMenu* self, BOOL highligthBackground, float highligthAlpha)
{
	self->highligthBackground = highligthBackground;
	self->highligthAlpha = highligthAlpha;
	_GuiItemMenu_updateIconDrawBack(self);
}

void GuiItemMenu_setCloseAuto(GuiItemMenu* self, BOOL closeAuto)
{
	self->closeAuto = closeAuto;
}

void GuiItemMenu_setContext(GuiItemMenu* self, GuiItemLayout* context)
{
	if (self->context)
		GuiItem_delete(&self->context->base);
	self->context = context;
	GuiItemMenu_setCloseAuto(self, FALSE);
}

BIG GuiItemMenu_getRow(GuiItemMenu* self)
{
	return DbValue_getRow(&self->value);
}
void GuiItemMenu_setRow(GuiItemMenu* self, BIG row, UBIG index)
{
	int i;
	for (i = 0; i < self->items.num; i++)
	{
		GuiItemMenuItem* it = GuiItemMenu_getItem(self, i);
		DbValue_setRow(&it->text, row, index);
	}

	DbValue_setRow(&self->value, row, index);
}

static GuiItem* _GuiItemMenu_createContext(GuiItemMenu* self)
{
	GuiItemLayout* layout = GuiItemLayout_new(Quad2i_init4(0, 0, 1, 1));
	layout->drawBorder = TRUE;

	GuiItemLayout_addColumn(layout, 0, 7);

	int i;
	for (i = 0; i < self->items.num; i++)
	{
		GuiItemMenuItem* it = GuiItemMenu_getItem(self, i);

		DbValue_hasChanged(&it->text);
		if (Std_sizeUNI(DbValue_result(&it->text)))
		{
			char nameId[64];
			Std_buildNumber(i, 0, nameId);

			GuiItem* item;
			if (it->confirm)
			{
				item = GuiItem_addSubName(&layout->base, nameId, GuiItemMenu_new(Quad2i_init4(0, i, 1, 1), DbValue_initCopy(&it->text), FALSE));
				//GuiItemMenu_setHasColors((GuiItemMenu*)item, DbValue_initCopy(&self->hasColors));
				GuiItemMenu_addItem((GuiItemMenu*)item, DbValue_initLang("YES_IAM_SURE"), it->click);

				GuiItemMenu_setUnderline((GuiItemMenu*)item, TRUE);
				((GuiItemMenu*)item)->textCenter = FALSE;
			}
			else
			{
				item = GuiItem_addSubName(&layout->base, nameId, GuiItemButton_newAlphaEx(Quad2i_init4(0, i, 1, 1), DbValue_initCopy(&it->text), it->click));
				((GuiItemButton*)item)->textCenter = FALSE;
				((GuiItemButton*)item)->drawBottomSepar = it->separ_line;
			}

			GuiItem_setIcon(item, GuiImage_newCopy(it->icon));
			GuiItem_setEnable(item, it->click != 0);

			GuiItem_setAttribute(item, "type", it->attr_type);
		}
	}

	return (GuiItem*)layout;
}

static Quad2i _GuiItemMenu_getImageCoord(const GuiItemMenu* self, Quad2i coord)
{
	return self->imageIcon ? GuiItem_getIconCoord(&coord) : coord;
}

Quad2i GuiItemMenu_getCoordSpace(GuiItemMenu* self, Quad2i coord)
{
	if (self->base.drawTable)
		return Quad2i_addSpace(coord, 1);
	else
		return Quad2i_addSpace(coord, 3);
}

void GuiItemMenu_draw(GuiItemMenu* self, Image4* img, Quad2i coord, Win* win)
{
	Rgba back_cd = self->base.back_cd;
	Rgba front_cd = self->base.front_cd;

	coord = GuiItemMenu_getCoordSpace(self, coord);

	if (self->image)
	{
		GuiImage_draw(self->image, img, _GuiItemMenu_getImageCoord(self, coord), front_cd);
	}
	else
	{
		Vec2i size = coord.size;
		int textH = _GuiItem_textSize(self->text_level, size.y);
		OsFont* font = OsWinIO_getFontDefault();
		int extra_down;
		int text_x = OsFont_getTextSize(font, DbValue_result(&self->value), textH, 0, &extra_down).x;
		BOOL centerText = text_x < (size.x - textH);	//too length => no center //-height is 2*text_offset
		if (centerText)
			centerText = self->textCenter;

		//background
		if (self->circle)
		{
			float rad = Std_bmin(coord.size.x, coord.size.y) / 2.5f;
			Image4_drawCircle(img, Quad2i_getMiddle(coord), rad, back_cd);
		}
		else
		{
			if (!self->transparent || self->mouseOver || self->base.drawTable || self->highligthBackground)
				Image4_drawBoxQuad(img, coord, back_cd);
		}

		//text
		size = coord.size;
		Vec2i pos;
		pos.y = size.y / 2;
		pos.x = centerText ? size.x / 2 : textH / 2 + 3;
		pos = Vec2i_add(pos, coord.start);
		Image4_drawText(img, pos, centerText, font, DbValue_result(&self->value), textH, 0, front_cd);

		if (!self->circle)
		{
			if (self->underline)
				Image4_drawBoxQuad(img, Image4_getUnderline(pos, centerText, Vec2i_init2(text_x, textH)), front_cd);
		}
	}
}

void GuiItemMenu_update(GuiItemMenu* self, Quad2i coord, Win* win)
{
	if (self->image)
		GuiImage_update(self->image, _GuiItemMenu_getImageCoord(self, coord).size);

	GuiItem_setRedraw(&self->base, (DbValue_hasChanged(&self->value)));
}

void GuiItemMenu_showContext(GuiItemMenu* self)
{
	GuiItem* lay = self->context ? GuiItem_newCopy(&self->context->base, TRUE) : _GuiItemMenu_createContext(self);
	GuiItemRoot_addDialogRel(lay, &self->base, self->base.coordMove, self->closeAuto);
}

void GuiItemMenu_touch(GuiItemMenu* self, Quad2i coord, Win* win)
{
	Rgba back_cd = self->transparent ? g_theme.white : g_theme.background;
	Rgba front_cd = g_theme.black;

	if (self->base.drawTable)
		back_cd = g_theme.white;

	if (self->circle)
		back_cd = Rgba_aprox(back_cd, front_cd, 0.2f);

	if (self->highligthBackground)
		back_cd = Rgba_aprox(g_theme.background, g_theme.main, self->highligthAlpha);

	if (self->err)
		back_cd = g_theme.warning;

	self->mouseOver = FALSE;

	if (self->base.touch && GuiItem_isEnable(&self->base) && OsWinIO_canActiveRenderItem(self))
	{
		BOOL startTouch = (OsWinIO_getTouch_action() == Win_TOUCH_DOWN_S) || (OsWinIO_getTouch_action() == Win_TOUCH_FORCE_DOWN_S);
		BOOL endTouch = (OsWinIO_getTouch_action() == Win_TOUCH_DOWN_E) || (OsWinIO_getTouch_action() == Win_TOUCH_FORCE_DOWN_E);

		BOOL active = OsWinIO_isActiveRenderItem(self);
		BOOL inside = Quad2i_inside(coord, OsWinIO_getTouchPos());
		BOOL touch = startTouch || active;

		if (inside)
			self->mouseOver = TRUE;

		if (inside && touch)	//full touch
		{
			front_cd = g_theme.main;
			OsWinIO_setActiveRenderItem(self);
		}
		else
			if ((inside && !touch) || (active && !inside))		//mid color
			{
				front_cd = Rgba_aprox(g_theme.main, front_cd, 0.5f);
				if (self->highligthBackground)
					back_cd = Rgba_aprox(g_theme.main, back_cd, 0.8f);
			}

		if (inside && active && endTouch)	//end
		{
			GuiItemEdit_saveCache();
			GuiItemMenu_showContext(self);
		}

		if (endTouch)
			OsWinIO_resetActiveRenderItem();

		//cursor
		//if(inside)
		//	Win_udpateCursor(win, Win_CURSOR_HAND);
	}

	_GuiItem_updateFinalCd(&self->base, back_cd, front_cd, coord, win);
}

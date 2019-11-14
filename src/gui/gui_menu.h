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

typedef struct GuiItemMenuItem_s
{
	GuiImage* icon;
	DbValue text;
	GuiItemCallback* click;
	BOOL confirm;
	BIG attr_type;
}GuiItemMenuItem;

GuiItemMenuItem* GuiItemMenuItem_new(GuiImage* icon, DbValue text, GuiItemCallback* click, BOOL confirm, BIG attr_type)
{
	GuiItemMenuItem* self = Os_malloc(sizeof(GuiItemMenuItem));
	self->icon = icon;
	self->text = text;
	self->click = click;
	self->confirm = confirm;
	self->attr_type = attr_type;
	return self;
}
GuiItemMenuItem* GuiItemMenuItem_newCopy(GuiItemMenuItem* src)
{
	return GuiItemMenuItem_new(GuiImage_newCopy(src->icon), DbValue_initCopy(&src->text), src->click, src->confirm, src->attr_type);
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
	int text_level;

	StdArr items;	//context(button list)

	BOOL circle;
	BOOL underline;
	BOOL mouseOver;
	BOOL textCenter;

	BOOL closeAuto;

	GuiImage* image;

	BOOL highligthBackground;

	GuiItemLayout* context;
}GuiItemMenu;

GuiItemMenuItem* GuiItemMenu_getItem(GuiItemMenu* self, int i)
{
	return self->items.ptrs[i];
}

void GuiItemMenu_addItemIcon(GuiItemMenu* self, GuiImage* icon, DbValue text, GuiItemCallback* click, BOOL confirm, BIG attr_type)
{
	GuiItemMenuItem* it = GuiItemMenuItem_new(icon, text, click, confirm, attr_type);
	StdArr_add(&self->items, it);
}

void GuiItemMenu_addItemEx(GuiItemMenu* self, DbValue text, GuiItemCallback* click, BOOL confirm, BIG attr_type)
{
	GuiItemMenu_addItemIcon(self, 0, text, click, confirm, attr_type);
}

void GuiItemMenu_addItem(GuiItemMenu* self, DbValue text, GuiItemCallback* click)
{
	GuiItemMenu_addItemEx(self, text, click, FALSE, -1);
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
	self->text_level = 1;

	self->items = StdArr_init();

	self->circle = circle;
	self->underline = TRUE;
	self->mouseOver = TRUE;
	self->image = 0;

	self->context = 0;
	self->textCenter = TRUE;
	self->closeAuto = TRUE;
	self->highligthBackground = FALSE;

	return (GuiItem*)self;
}

GuiItem* GuiItemMenu_newImage(Quad2i grid, GuiImage* image)
{
	GuiItemMenu* self = (GuiItemMenu*)GuiItemMenu_new(grid, DbValue_initEmpty(), FALSE);
	self->image = image;
	return (GuiItem*)self;
}

GuiItem* GuiItemMenu_newCopy(GuiItemMenu* src, BOOL copySub)
{
	GuiItemMenu* self = Os_malloc(sizeof(GuiItemMenu));
	*self = *src;
	GuiItem_initCopy(&self->base, &src->base, copySub);

	if (self->image)	self->image = GuiImage_newCopy(src->image);
	self->value = DbValue_initCopy(&src->value);

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

void GuiItemMenu_setUnderline(GuiItemMenu* self, BOOL underline)
{
	self->underline = underline;
}

void GuiItemMenu_setCenter(GuiItemMenu* self, BOOL textCenter)
{
	self->textCenter = textCenter;
}

void GuiItemMenu_setHighligthBackground(GuiItemMenu* self, BOOL highligthBackground)
{
	self->highligthBackground = highligthBackground;
	self->base.icon_transparent_back = !highligthBackground;
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
				GuiItemMenu_addItem((GuiItemMenu*)item, DbValue_initLang("YES_IAM_SURE"), it->click);

				GuiItemMenu_setUnderline((GuiItemMenu*)item, FALSE);
				((GuiItemMenu*)item)->textCenter = FALSE;
			}
			else
			{
				item = GuiItem_addSubName(&layout->base, nameId, GuiItemButton_newAlphaEx(Quad2i_init4(0, i, 1, 1), DbValue_initCopy(&it->text), it->click));
				((GuiItemButton*)item)->textCenter = FALSE;
			}

			GuiItem_setIcon(item, GuiImage_newCopy(it->icon));
			GuiItem_setEnable(item, it->click != 0);

			GuiItem_setAttribute(item, "type", it->attr_type);
		}
	}

	return (GuiItem*)layout;
}

void GuiItemMenu_draw(GuiItemMenu* self, Image4* img, Quad2i coord, Win* win)
{
	Rgba back_cd = self->base.back_cd;
	Rgba front_cd = self->base.front_cd;

	if (self->base.drawTable)
		coord = Quad2i_addSpace(coord, 1);

	if (self->image)
	{
		GuiImage_draw(self->image, img, coord, front_cd);
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
			if (self->mouseOver || self->base.drawTable || self->highligthBackground)
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
		GuiImage_update(self->image, coord);

	GuiItem_setRedraw(&self->base, (DbValue_hasChanged(&self->value)));
}

void GuiItemMenu_touch(GuiItemMenu* self, Quad2i coord, Win* win)
{
	Rgba back_cd = g_theme.white;
	Rgba front_cd = g_theme.black;

	//u Table header použít 'highligthBackground'
	if (self->base.drawTable)
	{
		back_cd = g_theme.white;
	}

	if (self->circle)
	{
		back_cd = Rgba_aprox(back_cd, front_cd, 0.2f);
	}

	if (self->highligthBackground)
	{
		back_cd = Rgba_aprox(g_theme.background, g_theme.main, 0.5f);
	}

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

			GuiItem* lay = self->context ? GuiItem_newCopy(&self->context->base, TRUE) : _GuiItemMenu_createContext(self);
			GuiItemRoot_addDialogRel(lay, &self->base, self->base.coordMove, self->closeAuto);
		}

		if (endTouch)
			OsWinIO_resetActiveRenderItem();

		//cursor
		//if(inside)
		//	Win_udpateCursor(win, Win_CURSOR_HAND);
	}

	_GuiItem_updateFinalCd(&self->base, back_cd, front_cd, coord, win);
}

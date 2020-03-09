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

typedef struct GuiItemComboStaticItem_s
{
	GuiImage* icon;
	DbValue text;
}GuiItemComboStaticItem;
GuiItemComboStaticItem* GuiItemComboStaticItem_new(GuiImage* icon, DbValue text)
{
	GuiItemComboStaticItem* self = Os_malloc(sizeof(GuiItemComboStaticItem));
	self->icon = icon;
	self->text = text;
	return self;
}
GuiItemComboStaticItem* GuiItemComboStaticItem_newCopy(GuiItemComboStaticItem* src)
{
	return GuiItemComboStaticItem_new(GuiImage_newCopy(src->icon), DbValue_initCopy(&src->text));
}
void GuiItemComboStaticItem_delete(GuiItemComboStaticItem* self)
{
	GuiImage_delete(self->icon);
	DbValue_free(&self->text);
	Os_free(self, sizeof(GuiItemComboStaticItem));
}
void GuiItemComboStaticItem_update(GuiItemComboStaticItem* self, GuiItem* orig)
{
	GuiItem_setRedraw(orig, DbValue_hasChanged(&self->text));

	if (self->icon)
		GuiItem_setRedraw(orig, GuiImage_update(self->icon, Vec2i_init2(OsWinIO_cellSize() / 2, OsWinIO_cellSize() / 2)));
}

typedef struct GuiItemComboStatic_s
{
	GuiItem base;
	StdArr items;

	DbValue value;
	DbValue description;
	int text_level;

	UINT oldValue;

	Rgba backCd;
}GuiItemComboStatic;

GuiItemComboStaticItem* GuiItemComboStatic_getItem(GuiItemComboStatic* self, int i)
{
	return self->items.ptrs ? self->items.ptrs[i] : 0;
}

void GuiItemComboStatic_addItemIcon(GuiItemComboStatic* self, GuiImage* icon, DbValue text)
{
	GuiItemComboStaticItem* it = GuiItemComboStaticItem_new(icon, text);
	StdArr_add(&self->items, it);
}

void GuiItemComboStatic_addItem(GuiItemComboStatic* self, DbValue text)
{
	GuiItemComboStatic_addItemIcon(self, 0, text);
}

void GuiItemComboStatic_addItemSepar(GuiItemComboStatic* self, const UNI* options)
{
	if (options)
	{
		BIG i;
		const BIG N = Std_separNumItemsUNI(options, '/');
		for (i = 0; i < N; i++)
			GuiItemComboStatic_addItem(self, DbValue_initStatic(Std_separGetItemUNI(options, i, '/')));
	}
}

GuiItem* GuiItemComboStatic_new(Quad2i grid, DbValue value, const UNI* options, DbValue description)
{
	GuiItemComboStatic* self = Os_malloc(sizeof(GuiItemComboStatic));
	self->base = GuiItem_init(GuiItem_COMBO_STATIC, grid);

	self->value = value;

	self->description = description;
	self->text_level = 1;

	self->items = StdArr_init();
	GuiItemComboStatic_addItemSepar(self, options);

	self->backCd = g_theme.white;

	self->base.icon_draw_back = FALSE;

	return (GuiItem*)self;
}

GuiItem* GuiItemComboStatic_newEx(Quad2i grid, DbValue value, const UNI* options, DbValue description, GuiItemCallback* call)
{
	GuiItem* item = GuiItemComboStatic_new(grid, value, options, description);
	GuiItem_setCallClick(item, call);
	return item;
}

GuiItem* GuiItemComboStatic_newCopy(GuiItemComboStatic* src, BOOL copySub)
{
	GuiItemComboStatic* self = Os_malloc(sizeof(GuiItemComboStatic));
	*self = *src;
	GuiItem_initCopy(&self->base, &src->base, copySub);

	self->value = DbValue_initCopy(&src->value);
	self->description = DbValue_initCopy(&src->description);
	self->items = StdArr_initCopyFn(&src->items, (StdArrCOPY)&GuiItemComboStaticItem_newCopy);

	return (GuiItem*)self;
}

void GuiItemComboStatic_clearItems(GuiItemComboStatic* self)
{
	StdArr_freeFn(&self->items, (StdArrFREE)&GuiItemComboStaticItem_delete);
}

void GuiItemComboStatic_delete(GuiItemComboStatic* self)
{
	GuiItemComboStatic_clearItems(self);

	DbValue_free(&self->description);
	DbValue_free(&self->value);
	GuiItem_free(&self->base);
	Os_free(self, sizeof(GuiItemComboStatic));
}

void GuiItemComboStatic_setBackgroundColor(GuiItemComboStatic* self, Rgba cd)
{
	self->backCd = cd;
}

UBIG GuiItemComboStatic_getValue(const GuiItemComboStatic* self)
{
	return self->items.num ? Std_clamp(DbValue_getNumber(&self->value), 0, self->items.num - 1) : 0;
}

void GuiItemComboStatic_setValue(GuiItemComboStatic* self, double pos)
{
	self->oldValue = DbValue_getNumber(&self->value);
	DbValue_setNumber(&self->value, pos);
	GuiItem_callClick(&self->base);
}

const UNI* GuiItemComboStatic_getValueName(GuiItemComboStatic* self, int i)
{
	return DbValue_result(&GuiItemComboStatic_getItem(self, i)->text);
}

void GuiItemComboStatic_clickSelect(GuiItem* self)
{
	int pos = GuiItem_getParentPos(self->parent, self);
	GuiItemComboStatic* combo = GuiItem_findParentType(self->parent, GuiItem_COMBO_STATIC);

	if (combo && pos >= 0)
		GuiItemComboStatic_setValue(combo, pos);
}

static GuiItemLayout* _GuiItemComboStatic_createDialog(GuiItemComboStatic* self)
{
	GuiItemLayout* layout = GuiItemLayout_new(Quad2i_init4(0, 0, 1, 1));
	layout->drawBorder = TRUE;

	UBIG value = GuiItemComboStatic_getValue(self);

	GuiItemLayout_addColumn(layout, 0, 7);

	UBIG i;
	for (i = 0; i < self->items.num; i++)
	{
		GuiItemComboStaticItem* it = GuiItemComboStatic_getItem(self, i);

		char nameId[64];
		Std_buildNumber(i, 0, nameId);

		GuiItemButton* button = (GuiItemButton*)GuiItem_addSubName(&layout->base, nameId, GuiItemButton_newAlphaEx(Quad2i_init4(0, i, 1, 1), DbValue_initStaticCopy(DbValue_result(&it->text)), &GuiItemComboStatic_clickSelect));
		button->textCenter = FALSE;
		button->stayPressed = (i == value);
		GuiItem_setIcon(&button->base, GuiImage_newCopy(it->icon));
	}

	return layout;
}

void GuiItemComboStatic_draw(GuiItemComboStatic* self, Image4* img, Quad2i coord, Win* win)
{
	Rgba front = self->base.front_cd;
	Rgba back = self->base.back_cd;

	Quad2i origCoord = coord;
	if (self->base.drawTable)
		Image4_drawBoxQuad(img, origCoord, g_theme.white);

	const int cell = OsWinIO_cellSize();
	int textH = _GuiItem_textSize(1, coord.size.y);
	OsFont* font = OsWinIO_getFontDefault();;

	const UNI* description = DbValue_result(&self->description);
	if (Std_sizeUNI(description))
	{
		if (coord.size.y * 1.5f > 2 * cell)
		{
			Image4_drawText(img, Vec2i_add(Vec2i_init2(coord.size.x / 2, cell / 2), coord.start), TRUE, font, description, textH, 0, self->base.front_cd);

			//move
			coord.start.y += cell;
			coord.size.y -= cell;
		}
		else
		{
			Image4_drawText(img, Vec2i_add(Vec2i_init2(0, cell / 2), coord.start), FALSE, font, description, textH, 0, front);

			int s = Std_min(OsFont_getTextSizeX(font, description, textH), coord.size.x * 0.4f);
			coord.start.x += s;
			coord.size.x -= s;
		}
	}

	//combo
	{
		coord.start.y = coord.start.y + coord.size.y / 2 - cell / 2;
		coord.size.y = cell;
		Quad2i iCoord = coord;
		coord = Quad2i_addSpace(coord, 3);

		//background and border around combo
		Image4_drawBoxQuad(img, coord, back);
		Image4_drawBorder(img, coord, 1, front);

		const GuiItemComboStaticItem* item = GuiItemComboStatic_getItem(self, GuiItemComboStatic_getValue(self));

		//icon
		if (item->icon)
		{
			GuiImage_draw(item->icon, img, GuiItem_getIconCoord(&iCoord), front);
			coord.start.x = iCoord.start.x;
			coord.size.x = iCoord.size.x;
			coord = Quad2i_addSpaceX(coord, 3);
		}

		//arrow
		int s = coord.size.y;
		Quad2i arrow = Quad2i_init4(coord.start.x + coord.size.x - s, coord.start.y, s, s);
		arrow = Quad2i_addSpace(arrow, 2);
		s /= 12;
		Vec2i mid = Quad2i_getMiddle(arrow);
		Image4_drawArrow(img, Vec2i_init2(mid.x, mid.y + s), Vec2i_init2(mid.x, mid.y - s), 4 * s, front);

		//value
		const UNI* text = self->items.num ? DbValue_result(&item->text) : 0;
		img->rect.size.x -= coord.size.y;	//don't draw over
		Image4_repairRect(img);
		Image4_drawText(img, Vec2i_add(Vec2i_init2(textH / 2, coord.size.y / 2), coord.start), FALSE, font, text, textH, 0, front);
	}

	if (self->base.drawTable)
	{
		Quad2i q = origCoord;
		q.size.x += 1;
		q.size.y += 1;
		Image4_drawBorder(img, q, 1, Rgba_aprox(self->base.back_cd, self->base.front_cd, 0.5f));
	}
}

void GuiItemComboStatic_update(GuiItemComboStatic* self, Quad2i coord, Win* win)
{
	UBIG i;
	for (i = 0; i < self->items.num; i++)
		GuiItemComboStaticItem_update(GuiItemComboStatic_getItem(self, i), &self->base);

	GuiItem_setRedraw(&self->base, (DbValue_hasChanged(&self->value) || DbValue_hasChanged(&self->description)));
}

void GuiItemComboStatic_touch(GuiItemComboStatic* self, Quad2i coord, Win* win)
{
	Rgba back_cd = self->backCd;
	Rgba front_cd = g_theme.black;

	if (self->base.touch && GuiItem_isEnable(&self->base) && OsWinIO_canActiveRenderItem(self))
	{
		BOOL startTouch = (OsWinIO_getTouch_action() == Win_TOUCH_DOWN_S) || (OsWinIO_getTouch_action() == Win_TOUCH_FORCE_DOWN_S);
		BOOL endTouch = (OsWinIO_getTouch_action() == Win_TOUCH_DOWN_E) || (OsWinIO_getTouch_action() == Win_TOUCH_FORCE_DOWN_E);

		BOOL active = OsWinIO_isActiveRenderItem(self);
		BOOL inside = Quad2i_inside(coord, OsWinIO_getTouchPos());
		BOOL touch = startTouch || active;

		if (inside && touch)	//full touch
		{
			back_cd = Rgba_aprox(back_cd, front_cd, 0.5f);
			OsWinIO_setActiveRenderItem(self);
		}
		else
			if ((inside && !touch) || (active && !inside))		//mid color
			{
				back_cd = Rgba_aprox(back_cd, front_cd, 0.2f);
			}

		if (inside && active && endTouch)	//end
		{
			GuiItemEdit_saveCache();

			if (OsWinIO_getKeyExtra() & Win_EXTRAKEY_CTRL)
			{
				if (self->items.num > 1)
				{
					UBIG pos = GuiItemComboStatic_getValue(self) + 1;
					if (pos >= self->items.num)
						pos = 0;
					GuiItemComboStatic_setValue(self, pos);
				}	
			}
			else
				GuiItemRoot_addDialogRelLayout(_GuiItemComboStatic_createDialog(self), &self->base, self->base.coordMove, TRUE);
		}

		if (endTouch)
			OsWinIO_resetActiveRenderItem();

		//cursor
		//if(inside)
		//	Win_udpateCursor(win, Win_CURSOR_HAND);
	}

	_GuiItem_updateFinalCd(&self->base, back_cd, front_cd, coord, win);
}

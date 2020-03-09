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

typedef struct GuiItemComboDynamic_s
{
	GuiItem base;

	DbValue optionsValues;
	DbRows optionsLinks;

	DbRows value;
	DbValue description;
	int text_level;

	DbValue hasColors;
	DbValue cd;

	//GuiItemCallback* openCall;
	BOOL warningBackground;
}GuiItemComboDynamic;

GuiItem* GuiItemComboDynamic_new(Quad2i grid, BOOL warningBackground, DbRows value, DbValue optionsValues, DbRows optionsLinks, DbValue description)
{
	GuiItemComboDynamic* self = Os_malloc(sizeof(GuiItemComboDynamic));
	self->base = GuiItem_init(GuiItem_COMBO_DYNAMIC, grid);

	self->value = value;
	self->optionsValues = optionsValues;
	self->optionsLinks = optionsLinks;
	self->description = description;
	self->text_level = 1;

	self->warningBackground = warningBackground;
	//self->openCall = 0;

	self->base.icon_draw_back = FALSE;

	self->hasColors = DbValue_initEmpty();
	self->cd = DbValue_initEmpty();

	return (GuiItem*)self;
}

GuiItem* GuiItemComboDynamic_newEx(Quad2i grid, BOOL warningBackground, DbRows value, DbValue optionsValues, DbRows optionsLinks, DbValue description, GuiItemCallback* call)
{
	GuiItem* item = GuiItemComboDynamic_new(grid, warningBackground, value, optionsValues, optionsLinks, description);
	GuiItem_setCallClick(item, call);
	return item;
}

GuiItem* GuiItemComboDynamic_newCopy(GuiItemComboDynamic* src, BOOL copySub)
{
	GuiItemComboDynamic* self = Os_malloc(sizeof(GuiItemComboDynamic));
	*self = *src;
	GuiItem_initCopy(&self->base, &src->base, copySub);

	self->optionsValues = DbValue_initCopy(&src->optionsValues);
	self->optionsLinks = DbRows_initCopy(&src->optionsLinks);
	self->value = DbRows_initCopy(&src->value);
	self->description = DbValue_initCopy(&src->description);
	//self->openCall = src->openCall;

	self->hasColors = DbValue_initCopy(&src->hasColors);
	self->cd = DbValue_initCopy(&src->cd);

	return (GuiItem*)self;
}

void GuiItemComboDynamic_delete(GuiItemComboDynamic* self)
{
	DbValue_free(&self->description);
	DbValue_free(&self->optionsValues);
	DbRows_free(&self->optionsLinks);
	DbRows_free(&self->value);
	DbValue_free(&self->hasColors);
	DbValue_free(&self->cd);

	GuiItem_free(&self->base);
	Os_free(self, sizeof(GuiItemComboDynamic));
}

/*void GuiItemComboDynamic_setOpenCall(GuiItemComboDynamic* self, GuiItemCallback* openCall)
{
	self->openCall = openCall;
}*/

void GuiItemComboDynamic_setColor(GuiItemComboDynamic* self, DbValue cd, DbValue hasColors)
{
	DbValue_free(&self->cd);
	DbValue_free(&self->hasColors);
	self->cd = cd;
	self->hasColors = hasColors;
}

BOOL GuiItemComboDynamic_hasColors(const GuiItemComboDynamic* self)
{
	return DbValue_getNumber(&self->hasColors);
}



void GuiItemComboDynamic_setOptionsLinks(GuiItemComboDynamic* self, DbRows optionsLinks)
{
	DbRows_free(&self->optionsLinks);
	self->optionsLinks = optionsLinks;
}

BIG GuiItemComboDynamic_getValueRow(const GuiItemComboDynamic* self)
{
	return DbRows_getRow(&self->value, 0);
}

UBIG GuiItemComboDynamic_numOptions(const GuiItemComboDynamic* self)
{
	return DbRows_getSize(&self->optionsLinks);
}

BIG GuiItemComboDynamic_getOptionRow(const GuiItemComboDynamic* self, UBIG i)
{
	return DbRows_getRow(&self->optionsLinks, i);
}
const UNI* GuiItemComboDynamic_getOption(GuiItemComboDynamic* self, BIG row)
{
	DbValue_setRow(&self->optionsValues, row, 0);
	DbValue_hasChanged(&self->optionsValues);
	return DbValue_result(&self->optionsValues);
}

void GuiItemComboDynamic_setFirstOptionToValue(GuiItemComboDynamic* self)
{
	BIG row = GuiItemComboDynamic_getOptionRow(self, 0);
	DbRows_setLinkRow(&self->value, row);
}

void GuiItemComboDynamic_resetValue(GuiItemComboDynamic* self)
{
	DbRows_setLinkRow(&self->value, -1);
}

BIG GuiItemComboDynamic_findRowPos(GuiItemComboDynamic* self, BIG row)
{
	const UBIG N = GuiItemComboDynamic_numOptions(self);
	UBIG i;
	for (i = 0; i < N; i++)
	{
		if (row == GuiItemComboDynamic_getOptionRow(self, i))
			return i;
	}
	return -1;
}


void GuiItemComboDynamic_clickSelect(GuiItem* self)
{
	GuiItemComboDynamic* combo = GuiItem_findParentType(self->parent, GuiItem_COMBO_DYNAMIC);
	if (combo && self->type == GuiItem_BUTTON)
	{
		DbRows_setLinkRow(&combo->value, GuiItem_getRow(self));

		GuiItem_callClick(&combo->base);	//call again? ...
	}
}

static GuiItemLayout* _GuiItemComboDynamic_createDialog(GuiItemComboDynamic* self)
{
	const UBIG N = GuiItemComboDynamic_numOptions(self);
	if (N == 0)
		return 0;

	GuiItemLayout* layout = GuiItemLayout_new(Quad2i_init4(0, 0, 1, 1));
	layout->drawBorder = TRUE;
	GuiItemLayout_addColumn(layout, 0, 7);

	BIG valueRow = GuiItemComboDynamic_getValueRow(self);

	{
	/*GuiItemButton* button = (GuiItemButton*)*/GuiItem_addSubName(&layout->base, "choose", GuiItemButton_newAlphaEx(Quad2i_init4(0, 0, 1, 1), DbValue_initLang("CHOOSE"), &GuiItemComboDynamic_clickSelect));
	//button->stayPressed = (-1 == valueRow);
	//if (GuiItemComboDynamic_hasColors(self))
	//	GuiItemButton_setBackgroundCdValue(button, TRUE, DbValue_initCopy(&self->cd));
	}

	UBIG i;
	for (i = 0; i < N; i++)
	{
		BIG it = GuiItemComboDynamic_getOptionRow(self, i);

		char nameId[64];
		Std_buildNumber(it, 0, nameId);

		DbValue nameValue = DbValue_initCopy(&self->optionsValues);
		DbValue_setRow(&nameValue, it, 0);

		GuiItemButton* button = (GuiItemButton*)GuiItem_addSubName(&layout->base, nameId, GuiItemButton_newAlphaEx(Quad2i_init4(0, 1 + i, 1, 1), nameValue, &GuiItemComboDynamic_clickSelect));
		button->stayPressed = (it == valueRow);
		button->textCenter = FALSE;
		GuiItemTable_callListIcon((GuiItem*)button);

		if (GuiItemComboDynamic_hasColors(self))
		{
			DbValue vv = DbValue_initCopy(&self->cd);
			DbValue_setRow(&vv, it, 0);
			GuiItemButton_setBackgroundCdValue(button, TRUE, vv);
		}
	}

	return layout;
}

void GuiItemComboDynamic_draw(GuiItemComboDynamic* self, Image4* img, Quad2i coord, Win* win)
{
	Rgba front = self->base.front_cd;
	Rgba back = self->base.back_cd;

	const int cell = OsWinIO_cellSize();
	int textH = _GuiItem_textSize(1, coord.size.y);
	OsFont* font = OsWinIO_getFontDefault();

	Quad2i origCoord = coord;
	if (self->base.drawTable)
		Image4_drawBoxQuad(img, origCoord, g_theme.white);

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
		//value
		BIG valueRow = GuiItemComboDynamic_getValueRow(self);
		const UNI* text = valueRow >= 0 ? GuiItemComboDynamic_getOption(self, valueRow) : 0;
		if (valueRow < 0)
		{
			text = Lang_find("CHOOSE");
			if (self->warningBackground && GuiItem_isEnable(&self->base))
				back = g_theme.warning;
		}

		coord.start.y = coord.start.y + coord.size.y / 2 - cell / 2;
		coord.size.y = cell;
		coord = Quad2i_addSpace(coord, 3);

		//background and border around combo
		Image4_drawBoxQuad(img, coord, back);
		Image4_drawBorder(img, coord, 1, front);

		//arrow
		int s = coord.size.y;
		Quad2i arrow = Quad2i_init4(coord.start.x + coord.size.x - s, coord.start.y, s, s);
		arrow = Quad2i_addSpace(arrow, 2);
		s /= 12;
		Vec2i mid = Quad2i_getMiddle(arrow);
		Image4_drawArrow(img, Vec2i_init2(mid.x, mid.y + s), Vec2i_init2(mid.x, mid.y - s), 4 * s, front);

		img->rect.size.x -= coord.size.y;	//don't draw over
		Image4_repairRect(img);
		Image4_drawText(img, Vec2i_add(Vec2i_init2(textH / 2, coord.size.y / 2), coord.start), FALSE, font, text, textH, 0, front);
		img->rect.size.x += coord.size.y;
		Image4_repairRect(img);
	}

	if (self->base.drawTable)
	{
		Quad2i q = origCoord;
		q.size.x += 1;
		q.size.y += 1;
		Image4_drawBorder(img, q, 1, Rgba_aprox(self->base.back_cd, self->base.front_cd, 0.5f));
	}
}

void GuiItemComboDynamic_update(GuiItemComboDynamic* self, Quad2i coord, Win* win)
{
	BIG row = GuiItemComboDynamic_getValueRow(self);

	DbValue_setRow(&self->cd, row, 0);

	GuiItem_setRedraw(&self->base, (DbValue_hasChanged(&self->hasColors) || DbValue_hasChanged(&self->cd) || DbValue_hasChanged(&self->optionsValues) || DbValue_hasChanged(&self->description)));
}

void GuiItemComboDynamic_touch(GuiItemComboDynamic* self, Quad2i coord, Win* win)
{
	Rgba back_cd = g_theme.white;
	Rgba front_cd = g_theme.black;

	if (GuiItemComboDynamic_hasColors(self) && GuiItemComboDynamic_getValueRow(self) >= 0)
		back_cd = DbValue_getCd(&self->cd);

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

			if (OsWinIO_getKeyExtra() & Win_EXTRAKEY_CTRL && OsWinIO_getKeyExtra() & Win_EXTRAKEY_SHIFT)
				GuiItemComboDynamic_resetValue(self);
			else
				if (OsWinIO_getKeyExtra() & Win_EXTRAKEY_CTRL)
				{
					const int N = GuiItemComboDynamic_numOptions(self);

						//UBIG GuiItemComboDynamic_numOptions(const GuiItemComboDynamic * self)
	
					//BIG GuiItemComboDynamic_getOptionRow(const GuiItemComboDynamic * self, UBIG i)

					if (N > 1)
					{
						BIG pos = GuiItemComboDynamic_findRowPos(self, GuiItemComboDynamic_getValueRow(self)) + 1;
						if (pos >= N)
							pos = 0;

						DbRows_setLinkRow(&self->value, GuiItemComboDynamic_getOptionRow(self, pos));
					}
				}
				else
				{
					GuiItemLayout* lay = _GuiItemComboDynamic_createDialog(self);
					if (lay)
						GuiItemRoot_addDialogRelLayout(lay, &self->base, self->base.coordMove, TRUE);
				}
		}

		if (endTouch)
			OsWinIO_resetActiveRenderItem();

		//cursor
		//if(inside)
		//	Win_udpateCursor(win, Win_CURSOR_HAND);
	}

	_GuiItem_updateFinalCd(&self->base, back_cd, front_cd, coord, win);
}

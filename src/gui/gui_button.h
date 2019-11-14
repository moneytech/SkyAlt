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

typedef enum
{
	GuiItemButton_CLASSIC,
	GuiItemButton_BLACK,
	GuiItemButton_WHITE,
	GuiItemButton_ALPHA,
	GuiItemButton_IMAGE,
}GuiItemButton_TYPE;

typedef struct GuiItemButton_s
{
	GuiItem base;

	int text_level;
	DbValue text;
	DbValue description;

	BOOL stayPressed;
	BOOL stayPressedLeft;
	BOOL stayPressedRight;
	BOOL selectMode;

	BOOL circle;
	GuiItemButton_TYPE type;

	BOOL textCenter;

	GuiImage* image;
	int imageSpace;
	Rgba back_cd;
} GuiItemButton;

GuiItem* GuiItemButton_new(Quad2i grid, DbValue text)
{
	GuiItemButton* self = Os_malloc(sizeof(GuiItemButton));
	self->base = GuiItem_init(GuiItem_BUTTON, grid);

	self->text_level = 1;
	self->text = text;
	self->description = DbValue_initEmpty();

	self->stayPressed = FALSE;
	self->stayPressedLeft = FALSE;
	self->stayPressedRight = FALSE;

	self->selectMode = FALSE;

	self->circle = FALSE;
	self->type = GuiItemButton_ALPHA;

	self->textCenter = TRUE;

	self->image = 0;
	self->imageSpace = 0;

	return(GuiItem*)self;
}

void GuiItemButton_setTextCenter(GuiItemButton* self, BOOL center)
{
	self->textCenter = center;
}

void GuiItemButton_setCircle(GuiItemButton* self, BOOL circle)
{
	self->circle = circle;
}

GuiItem* GuiItemButton_newAlphaEx(Quad2i grid, DbValue text, GuiItemCallback* call)
{
	GuiItem* item = GuiItemButton_new(grid, text);
	((GuiItemButton*)item)->type = GuiItemButton_ALPHA;
	GuiItem_setCallClick(item, call);
	return item;
}

GuiItem* GuiItemButton_newNoCenterEx(Quad2i grid, DbValue text, GuiItemCallback* call)
{
	GuiItem* item = GuiItemButton_newAlphaEx(grid, text, call);
	((GuiItemButton*)item)->type = GuiItemButton_ALPHA;
	GuiItemButton_setTextCenter((GuiItemButton*)item, FALSE);
	return item;
}

GuiItem* GuiItemButton_newClassicEx(Quad2i grid, DbValue text, GuiItemCallback* call)
{
	GuiItem* item = GuiItemButton_new(grid, text);
	((GuiItemButton*)item)->type = GuiItemButton_CLASSIC;
	GuiItem_setCallClick(item, call);
	return item;
}

GuiItem* GuiItemButton_newBlackEx(Quad2i grid, DbValue text, GuiItemCallback* call)
{
	GuiItem* item = GuiItemButton_new(grid, text);
	((GuiItemButton*)item)->type = GuiItemButton_BLACK;
	GuiItem_setCallClick(item, call);
	return item;
}
GuiItem* GuiItemButton_newWhiteEx(Quad2i grid, DbValue text, GuiItemCallback* call)
{
	GuiItem* item = GuiItemButton_new(grid, text);
	((GuiItemButton*)item)->type = GuiItemButton_WHITE;
	GuiItem_setCallClick(item, call);
	return item;
}

GuiItem* GuiItemButton_newImage(Quad2i grid, GuiImage* image, GuiItemCallback* call)
{
	GuiItem* item = GuiItemButton_new(grid, DbValue_initEmpty());
	((GuiItemButton*)item)->type = GuiItemButton_IMAGE;
	((GuiItemButton*)item)->image = image;
	GuiItem_setCallClick(item, call);
	return item;
}

GuiItem* GuiItemButton_newCopy(GuiItemButton* src, BOOL copySub)
{
	GuiItemButton* self = Os_malloc(sizeof(GuiItemButton));
	*self = *src;
	GuiItem_initCopy(&self->base, &src->base, copySub);

	self->text = DbValue_initCopy(&src->text);
	self->description = DbValue_initCopy(&src->description);

	if (self->image)
		self->image = GuiImage_newCopy(src->image);

	return(GuiItem*)self;
}

void GuiItemButton_delete(GuiItemButton* self)
{
	if (self->image)
		GuiImage_delete(self->image);

	DbValue_free(&self->text);
	DbValue_free(&self->description);

	GuiItem_free(&self->base);
	Os_free(self, sizeof(GuiItemButton));
}


void GuiItemButton_setDescription(GuiItemButton* self, DbValue description)
{
	DbValue_free(&self->description);
	self->description = description;
}

Rgba GuiItemButton_getBackCd(const GuiItemButton* self)
{
	return self->back_cd;
}

DbValue GuiItemButton_getTextMsg(const GuiItemButton* self)
{
	return self->text;
}

void GuiItemButton_setPressed(GuiItemButton* self, BOOL stayPressed)
{
	if (self->stayPressed != stayPressed)
		GuiItem_setRedraw(&self->base, TRUE);

	self->stayPressed = stayPressed;
}

void GuiItemButton_setPressedEx(GuiItemButton* self, BOOL stayPressed, BOOL stayPressedLeft, BOOL stayPressedRight)
{
	GuiItemButton_setPressed(self, stayPressed);

	if (self->stayPressedLeft != stayPressedLeft || self->stayPressedRight != stayPressedRight)
		GuiItem_setRedraw(&self->base, TRUE);

	self->stayPressedLeft = stayPressedLeft;
	self->stayPressedRight = stayPressedRight;
}

void GuiItemButton_draw(GuiItemButton* self, Image4* img, Quad2i coord, Win* win)
{
	Rgba back_cd = self->base.back_cd;
	Rgba front_cd = self->base.front_cd;

	Quad2i origCoord = coord;
	coord = Quad2i_addSpace(coord, 3);

	if (self->base.drawTable)
		Image4_drawBoxQuad(img, origCoord, g_theme.white);

	if (self->circle)
	{
		float rad = Std_bmin(coord.size.x, coord.size.y) / 2.5f;
		coord = Quad2i_addSpace(coord, 3);
		Image4_drawCircle(img, Quad2i_getMiddle(coord), rad, back_cd);
	}
	else
	{
		if (self->type == GuiItemButton_IMAGE)
		{
			if (self->image)
				GuiImage_draw(self->image, img, Quad2i_addSpace(coord, self->imageSpace), back_cd);
		}
		else
			Image4_drawBoxQuad(img, coord, back_cd);
	}


	



	//if (Std_sizeUNI(DbValue_result(&self->text)))
	{
		Vec2i size = coord.size;
		int textH = _GuiItem_textSize(self->text_level, size.y);
		OsFont* font = OsWinIO_getFontDefault();

		//text
		int extra_down;
		Vec2i textSize = OsFont_getTextSize(font, DbValue_result(&self->text), textH, 0, &extra_down);
		Vec2i descSize = OsFont_getTextSize(font, DbValue_result(&self->description), textH, 0, &extra_down);

		BOOL centerText = (textSize.x < (size.x - textH)); //too length => no center //-height is 2*text_offset
		BOOL centerDesc = (descSize.x < (size.x - textH)); //too length => no center //-height is 2*text_offset
		if (centerText)
			centerText = self->textCenter;

		const int num_lines = (Std_sizeUNI(DbValue_result(&self->description)) && (coord.size.y / (float)OsWinIO_cellSize()) > 1.5f) ? 2 : 1;

		Vec2i posDesc, posText;
		posDesc.x = centerDesc ? size.x / 2 : textH / 2;
		posText.x = centerText ? size.x / 2 : textH / 2;
		if (num_lines == 1)
		{
			posDesc.y = posText.y = size.y / 2;
		}
		else
		{
			posDesc.y = OsWinIO_cellSize() / 2;// size.y / 4;
			posText.y = OsWinIO_cellSize() + (size.y - OsWinIO_cellSize()) / 2;
		}

		//description
		if (num_lines == 2)
			Image4_drawText(img, Vec2i_add(posDesc, coord.start), centerDesc, font, DbValue_result(&self->description), textH, 0, self->base.front_cd);

		//text
		Image4_drawText(img, Vec2i_add(posText, coord.start), centerText, font, DbValue_result(&self->text), textH, 0, front_cd);

		//underline
		//if (Std_sizeUNI(DbValue_result(&self->text)) && self->type == GuiItemButton_TAG)
		//	Image4_drawBoxQuad(img, Image4_getUnderline(pos, centerText, textSize), self->base.front_cd);
	}

	if (self->stayPressed)
	{
		const int border_size = 4;
		if (self->stayPressedLeft)
		{
			Quad2i q = coord;
			q.size.x = border_size;
			Image4_drawBoxQuad(img, q, front_cd);
		}
		if (self->stayPressedRight)
		{
			Quad2i q = coord;
			q.start.x += q.size.x - border_size;
			q.size.x = border_size;
			Image4_drawBoxQuad(img, q, front_cd);
		}
	}

	if (self->base.drawTable)
	{
		Quad2i q = origCoord;
		q.size.x += 1;
		q.size.y += 1;
		//img->rect = q;
		Image4_drawBorder(img, q, 1, Rgba_aprox(g_theme.background, g_theme.black, 0.5f));
	}
	else
	if (self->type == GuiItemButton_WHITE)
		Image4_drawBorder(img, coord, 1, self->base.front_cd);
}

void GuiItemButton_update(GuiItemButton* self, Quad2i coord, Win* win)
{
	coord = Quad2i_addSpace(coord, 3);

	BOOL changed = FALSE;
	if (self->image)
		changed |= GuiImage_update(self->image, Quad2i_addSpace(coord, self->imageSpace));

	GuiItem_setRedraw(&self->base, (changed || DbValue_hasChanged(&self->text) || DbValue_hasChanged(&self->description)));// || old_pressed != self->stayPressed));
}

void GuiItemButton_touch(GuiItemButton* self, Quad2i coord, Win* win)
{
	Rgba back_cd = g_theme.main;
	Rgba front_cd = g_theme.black;

	if (self->type == GuiItemButton_CLASSIC)
	{
		back_cd = g_theme.main;
		front_cd = (self->base.shortKey_extra & Win_EXTRAKEY_ENTER) ? g_theme.black : g_theme.white;
	}
	else
		if (self->type == GuiItemButton_IMAGE)
		{
			back_cd = g_theme.black;
			front_cd = g_theme.white;
		}
		else
			if (self->type == GuiItemButton_BLACK)
			{
				back_cd = g_theme.black;
				front_cd = g_theme.white;
			}
			else
				if (self->type == GuiItemButton_WHITE)
				{
					back_cd = g_theme.white;
					front_cd = g_theme.black;
				}
			else
				if (self->type == GuiItemButton_ALPHA)
				{
					if (self->stayPressed)
						front_cd = g_theme.black;

					back_cd = self->stayPressed ? g_theme.main : g_theme.background;
					front_cd = g_theme.black;
				}

	if (self->base.drawTable)
		back_cd = g_theme.white;

	if (GuiItem_isEnable(&self->base) && self->selectMode)
	{
		back_cd = g_theme.selectFormula;
		front_cd = g_theme.black;
	}

	if (self->base.touch && GuiItem_isEnable(&self->base) && OsWinIO_canActiveRenderItem(self))
	{
		BOOL startTouch = (OsWinIO_getTouch_action() == Win_TOUCH_DOWN_S) || (OsWinIO_getTouch_action() == Win_TOUCH_FORCE_DOWN_S);
		BOOL endTouch = (OsWinIO_getTouch_action() == Win_TOUCH_DOWN_E) || (OsWinIO_getTouch_action() == Win_TOUCH_FORCE_DOWN_E);

		BOOL active = OsWinIO_isActiveRenderItem(self);
		BOOL inside = Quad2i_inside(coord, OsWinIO_getTouchPos());
		BOOL touch = startTouch || active;

		if (inside && touch) //full touch
		{
			back_cd = g_theme.main;
			front_cd = g_theme.black;
			OsWinIO_setActiveRenderItem(self);
		}
		else
			if ((inside && !touch) || (active && !inside)) //mid color
			{
				back_cd = Rgba_aprox(g_theme.main, g_theme.white, 0.5f);
			}

		if (inside && active && endTouch) //end
		{
			GuiItemEdit_saveCache();

			GuiItem_callClick(&self->base);

			GuiItemLevel* level = GuiItem_findParentType(&self->base, GuiItem_LEVEL);
			if (level)
				GuiItemLevel_tryCloseLater(level);
		}

		if (active && endTouch)
			OsWinIO_resetActiveRenderItem();

		//cursor
		if (inside)
			Win_updateCursor(win, Win_CURSOR_HAND);
	}

	_GuiItem_updateFinalCd(&self->base, back_cd, front_cd, coord, win);
}

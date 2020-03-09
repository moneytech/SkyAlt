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

typedef enum
{
	GuiItemButton_CLASSIC,
	GuiItemButton_BLACK,
	GuiItemButton_WHITE,
	GuiItemButton_ALPHA,
	GuiItemButton_IMAGE,
	GuiItemButton_COLOR,
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

	BOOL circle;
	GuiItemButton_TYPE type;

	BOOL textCenter;
	BOOL drawBottomSepar;

	GuiImage* image;
	BOOL imageIcon;

	BOOL useBackCd;
	Rgba back_cd;
	DbValue back_cd_value;

	BOOL drawBackground;
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

	self->circle = FALSE;
	self->type = GuiItemButton_ALPHA;

	self->textCenter = TRUE;

	self->image = 0;
	//self->imageSpace = 0;

	self->useBackCd = FALSE;
	self->back_cd = g_theme.main;
	self->back_cd_value = DbValue_initEmpty();

	self->drawBackground = TRUE;
	self->drawBottomSepar = FALSE;

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

GuiItem* GuiItemButton_newImage(Quad2i grid, GuiImage* image, BOOL imageIcon, GuiItemCallback* call)
{
	GuiItem* item = GuiItemButton_new(grid, DbValue_initEmpty());
	((GuiItemButton*)item)->type = GuiItemButton_IMAGE;
	((GuiItemButton*)item)->image = image;
	((GuiItemButton*)item)->imageIcon = imageIcon;

	GuiItem_setCallClick(item, call);
	return item;
}

GuiItem* GuiItemButton_newCd(Quad2i grid, Rgba cd, GuiItemCallback* call)
{
	GuiItem* item = GuiItemButton_new(grid, DbValue_initEmpty());
	((GuiItemButton*)item)->type = GuiItemButton_COLOR;
	((GuiItemButton*)item)->back_cd = cd;
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

	self->back_cd_value = DbValue_initCopy(&src->back_cd_value);

	return(GuiItem*)self;
}

void GuiItemButton_delete(GuiItemButton* self)
{
	if (self->image)
		GuiImage_delete(self->image);

	DbValue_free(&self->text);
	DbValue_free(&self->description);
	DbValue_free(&self->back_cd_value);

	GuiItem_free(&self->base);
	Os_free(self, sizeof(GuiItemButton));
}

void GuiItemButton_setBackgroundCd(GuiItemButton* self, BOOL active, Rgba back_cd)
{
	self->useBackCd = active;
	if (active)
		self->back_cd = back_cd;
}
void GuiItemButton_setWarningCd(GuiItemButton* self, BOOL active)
{
	GuiItemButton_setBackgroundCd(self, active, g_theme.warning);
}
void GuiItemButton_setBackgroundCdValue(GuiItemButton* self, BOOL active, DbValue back_cd_value)
{
	self->useBackCd = active;
	if (active)
		self->back_cd_value = back_cd_value;
}

void GuiItemButton_setDescription(GuiItemButton* self, DbValue description)
{
	DbValue_free(&self->description);
	self->description = description;
}

DbValue GuiItemButton_getTextMsg(const GuiItemButton* self)
{
	return self->text;
}

const UNI* GuiItemButton_getText(const GuiItemButton* self)
{
	return DbValue_result(&self->text);
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

static Quad2i _GuiItemButton_getImageCoord(const GuiItemButton* self, Quad2i coord)
{
	if (self->imageIcon)
	{
		Quad2i backup = coord;
		coord = GuiItem_getIconCoord(&coord);

		if (self->textCenter)
			coord = Quad2i_initMid(Quad2i_getMiddle(backup), coord.size);
	}
	return coord;
}

Quad2i GuiItemButton_getCoordSpace(GuiItemButton* self, Quad2i coord)
{
	return Quad2i_addSpace(coord, 3);
}

void GuiItemButton_drawPress(Image4* img, Rgba cd, Quad2i coord, BOOL left, BOOL right)
{
	const int border_size = 4;
	if (left)
	{
		Quad2i q = coord;
		q.size.x = border_size;
		Image4_drawBoxQuad(img, q, cd);
	}
	if (right)
	{
		Quad2i q = coord;
		q.start.x += q.size.x - border_size;
		q.size.x = border_size;
		Image4_drawBoxQuad(img, q, cd);
	}
}

void GuiItemButton_drawSeparLine(GuiItemButton* self, Image4* img, Quad2i coord)
{
	if (self->drawBottomSepar)
	{
		Image4_drawBoxQuad(img, Quad2i_init4(coord.start.x, coord.start.y + coord.size.y - 1, coord.size.x, 1), g_theme.white);
	}
}

void GuiItemButton_draw(GuiItemButton* self, Image4* img, Quad2i coord, Win* win)
{
	Rgba back_cd = self->base.back_cd;
	Rgba front_cd = self->base.front_cd;

	Quad2i origCoord = coord;
	coord = GuiItemButton_getCoordSpace(self, coord);

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
				GuiImage_draw(self->image, img, _GuiItemButton_getImageCoord(self, coord), back_cd);
		}
		else
			if (self->drawBackground)
			{
				Image4_drawBoxQuad(img, coord, back_cd);

				GuiItemButton_drawSeparLine(self, img, coord);
			}
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
		if (self->useBackCd)
			Image4_drawBorder(img, origCoord, 3, self->base.front_cd);
		else
			GuiItemButton_drawPress(img, self->base.front_cd, coord, (self->stayPressedLeft && !self->base.icon), self->stayPressedRight);
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
	coord = GuiItemButton_getCoordSpace(self, coord);

	BOOL changed = FALSE;
	if (self->image)
		changed |= GuiImage_update(self->image, _GuiItemButton_getImageCoord(self, coord).size);

	if (self->useBackCd)
	{
		DbValue_setRow(&self->back_cd_value, GuiItem_getRow(&self->base), 0);
		if (DbValue_is(&self->back_cd_value))
			self->back_cd = DbValue_getCd(&self->back_cd_value);
	}

	GuiItem_setRedraw(&self->base, (changed || DbValue_hasChanged(&self->back_cd_value) || DbValue_hasChanged(&self->text) || DbValue_hasChanged(&self->description)));
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
		if (self->type == GuiItemButton_COLOR)
		{
			back_cd = self->back_cd;
			front_cd = g_theme.white;
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

	if (self->useBackCd)
		back_cd = self->back_cd;

	if (self->base.touch && GuiItem_isEnable(&self->base) && OsWinIO_canActiveRenderItem(self))
	{
		BOOL startTouch = (OsWinIO_getTouch_action() == Win_TOUCH_DOWN_S) || (OsWinIO_getTouch_action() == Win_TOUCH_FORCE_DOWN_S);
		BOOL endTouch = (OsWinIO_getTouch_action() == Win_TOUCH_DOWN_E) || (OsWinIO_getTouch_action() == Win_TOUCH_FORCE_DOWN_E);

		BOOL active = OsWinIO_isActiveRenderItem(self);
		BOOL inside = Quad2i_inside(coord, OsWinIO_getTouchPos());
		BOOL touch = startTouch || active;

		if (inside && touch) //full touch
		{
			if (self->type != GuiItemButton_COLOR)
			{
				back_cd = g_theme.main;
				front_cd = g_theme.black;
			}
			OsWinIO_setActiveRenderItem(self);
		}
		else
			if ((inside && !touch) || (active && !inside)) //mid color
			{
				if (self->type != GuiItemButton_COLOR)
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

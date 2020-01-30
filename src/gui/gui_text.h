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

typedef struct GuiItemText_s
{
	GuiItem base;

	DbValue text;
	DbValue description;
	int text_level;

	BOOL enableCentring;

	BOOL doubleBorder;

	BOOL stayPressed;
	BOOL enableSelect;

	BOOL cursorHand;
	BOOL drawBorder;

	BOOL drawWhiteBack;
	BOOL drawBackground;
	BOOL drawBackground_procentage_visualize;
	BOOL drawBackground_procentage_visualize_mult100;

	BOOL cursorNone;

	Rgba colorBorder;

	BOOL formatURL;
	BOOL formatEmail;
}GuiItemText;

GuiItem* GuiItemText_new(Quad2i grid, BOOL enableCentring, DbValue text, DbValue description)
{
	GuiItemText* self = Os_malloc(sizeof(GuiItemText));
	self->base = GuiItem_init(GuiItem_TEXT, grid);
	self->enableCentring = enableCentring;
	self->text = text;
	self->description = description;

	self->text_level = 1;
	self->doubleBorder = FALSE;
	self->stayPressed = FALSE;
	self->cursorHand = FALSE;

	self->cursorNone = FALSE;

	self->colorBorder = Rgba_initBlack();

	self->drawBackground = TRUE;
	self->drawBorder = FALSE;
	self->drawWhiteBack = FALSE;
	self->drawBackground_procentage_visualize = FALSE;
	self->drawBackground_procentage_visualize_mult100 = FALSE;
	self->enableSelect = TRUE;

	self->formatURL = FALSE;
	self->formatEmail = FALSE;

	self->base.icon_draw_back = FALSE;
	return (GuiItem*)self;
}
GuiItem* GuiItemText_newUnderline(Quad2i grid, BOOL enableCentring, DbValue text, DbValue description, BOOL formatURL, BOOL formatEmail)
{
	GuiItemText* self = (GuiItemText*)GuiItemText_new(grid, enableCentring, text, description);
	self->formatURL = formatURL;
	self->formatEmail = formatEmail;
	return (GuiItem*)self;
}

GuiItem* GuiItemText_newCopy(GuiItemText* src, BOOL copySub)
{
	GuiItemText* self = Os_malloc(sizeof(GuiItemText));
	*self = *src;
	GuiItem_initCopy(&self->base, &src->base, copySub);

	self->text = DbValue_initCopy(&src->text);
	self->description = DbValue_initCopy(&src->description);

	return (GuiItem*)self;
}

void GuiItemText_delete(GuiItemText* self)
{
	DbValue_free(&self->text);
	DbValue_free(&self->description);

	GuiItem_free(&self->base);
	Os_free(self, sizeof(GuiItemText));
}

void GuiItemText_setCursorHand(GuiItemText* self, BOOL cursorHand)
{
	self->cursorHand = cursorHand;
}

const UNI* GuiItemText_getText(const GuiItemText* self)
{
	return self ? DbValue_result(&self->text) : 0;
}
void GuiItemText_setText(GuiItemText* self, const UNI* str)
{
	DbValue_setTextCopy(&self->text, str);
}

void GuiItemText_setColorBorder(GuiItemText* self, Rgba colorBorder)
{
	self->colorBorder = colorBorder;
}

double GuiItemText_getNumber(const GuiItemText* self)
{
	return self ? DbValue_getNumber(&self->text) : 0;
}
void GuiItemText_setNumber(GuiItemText* self, double value)
{
	DbValue_setNumber(&self->text, value);
}

void GuiItemText_setPressed(GuiItemText* self, BOOL stayPressed)
{
	if (self->stayPressed != stayPressed)
		GuiItem_setRedraw(&self->base, TRUE);

	self->stayPressed = stayPressed;
}

void GuiItemText_setWhiteBack(GuiItemText* self, BOOL on)
{
	self->drawWhiteBack = on;
}

void GuiItemText_setEnableSelect(GuiItemText* self, BOOL enableSelect)
{
	self->enableSelect = enableSelect;
}

BOOL GuiItemText_isUnderline(const GuiItemText* self)
{
	return (self->formatURL || self->formatEmail || DbValue_isFormatUnderline(&self->text));
}

void GuiItemText_draw(GuiItemText* self, Image4* img, Quad2i coord, Win* win)
{
	double v = DbValue_getNumber(&self->text);

	const Rgba textCd = (v < 0) ? Rgba_initRed() : self->base.front_cd;

	int num_lines = coord.size.y > OsWinIO_cellSize() * 1.5f ? 2 : 1;
	if (Std_sizeUNI(DbValue_result(&self->description)) == 0)
		num_lines = 1;

	BOOL isColored = !Rgba_isBlack(&self->colorBorder);
	BOOL drawBorder = (num_lines > 1 || isColored || self->drawBorder);

	if (!self->base.drawTable)
		coord = Quad2i_addSpace(coord, 3);

	if ((self->drawBackground || self->stayPressed) && (self->stayPressed || self->base.drawTable || self->drawWhiteBack))
		Image4_drawBoxQuad(img, self->drawWhiteBack ? Quad2i_addSpace(coord, 2) : coord, self->base.back_cd);

	if (self->drawBackground_procentage_visualize)
		GuiItemEdit_drawBackgroundVisualize(img, coord, v, self->drawBackground_procentage_visualize_mult100, Rgba_aprox(self->base.back_cd, g_theme.main, 0.5f));

	if (drawBorder)
	{
		Rgba bCd = isColored ? self->colorBorder : self->base.front_cd;

		const int FAT = Rgba_isBlack(&self->colorBorder) ? 1 : 2;

		//if (self->drawBorder)
		{
			if (self->doubleBorder)
			{
				Image4_drawBorder(img, Quad2i_addSpace(coord, -1), FAT, bCd);
				Image4_drawBorder(img, Quad2i_addSpace(coord, 1), FAT, bCd);
			}
			else
				Image4_drawBorder(img, coord, FAT, bCd);
		}
	}

	//text
	Vec2i size = coord.size;
	int textH = _GuiItem_textSize(self->text_level, size.y);	//pokud je dynamic, tak nesmí být také větší než 'x' ...

	if (DbValue_getFormat(&self->text) == DbFormat_RATING)
		textH *= 3;

	OsFont* font = OsWinIO_getFontDefault();

	int extra_down;
	Vec2i textSize = OsFont_getTextSize(font, DbValue_result(&self->text), textH, 0, &extra_down);
	BOOL centerText = self->enableCentring ? textSize.x < (size.x - textH) : FALSE;	//too length => no center //-height is 2*text_offset
	BOOL centerDesc = self->enableCentring ? OsFont_getTextSize(font, DbValue_result(&self->description), textH, 0, &extra_down).x < (size.x - textH) : FALSE;

	Vec2i posDesc, posText;
	posDesc.x = centerDesc ? size.x / 2 : textH / 2;
	posText.x = centerText ? size.x / 2 : textH / 2;
	if (num_lines == 1)
	{
		posDesc.y = posText.y = size.y / 2;
	}
	else
	{
		posDesc.y = size.y / 4;
		posText.y = size.y * 3 / 4;
	}

	Vec2i start = coord.start;//Vec2i_add(coord.start, Vec2i_init2(OsWinIO_shadows(), OsWinIO_shadows()));

	if (num_lines == 2)
		Image4_drawText(img, Vec2i_add(posDesc, start), centerDesc, font, DbValue_result(&self->description), textH, 0, self->base.front_cd);		//description

	posText = Vec2i_add(posText, start);
	Image4_drawText(img, posText, centerText, font, DbValue_result(&self->text), textH, 0, textCd);				//text

	//underline
	if (Std_sizeUNI(DbValue_result(&self->text)) && GuiItemText_isUnderline(self))
		Image4_drawBoxQuad(img, Image4_getUnderline(posText, centerText, textSize), textCd);

	if (self->base.drawTable)
	{
		Quad2i q = coord;
		q.size.x += 1;
		q.size.y += 1;
		//img->rect = q;
		Image4_drawBorder(img, q, 1, Rgba_aprox(self->base.back_cd, self->base.front_cd, 0.5f));
	}
}

void GuiItemText_update(GuiItemText* self, Quad2i coord, Win* win)
{
	GuiItem_setRedraw(&self->base, (DbValue_hasChanged(&self->text) || DbValue_hasChanged(&self->description)));
}

void GuiItemText_touch(GuiItemText* self, Quad2i coord, Win* win)
{
	Rgba back_cd = g_theme.background;
	Rgba front_cd = g_theme.black;

	if (self->drawWhiteBack)
		back_cd = g_theme.white;

	if (self->base.drawTable)
	{
		back_cd = g_theme.white;
	}

	if (GuiItemText_isUnderline(self))
		front_cd = g_theme.warning;	//blue ...

	if (self->stayPressed)
		back_cd = Rgba_aprox(g_theme.background, g_theme.main, 0.5f);

	if (self->enableSelect && self->base.touch && GuiItem_isEnable(&self->base) && OsWinIO_canActiveRenderItem(self))
	{
		BOOL inside = Quad2i_inside(coord, OsWinIO_getTouchPos());

		BOOL startTouch = (OsWinIO_getTouch_action() == Win_TOUCH_DOWN_S) || (OsWinIO_getTouch_action() == Win_TOUCH_FORCE_DOWN_S);
		BOOL endTouch = (OsWinIO_getTouch_action() == Win_TOUCH_DOWN_E) || (OsWinIO_getTouch_action() == Win_TOUCH_FORCE_DOWN_E);

		BOOL active = OsWinIO_isActiveRenderItem(self);
		BOOL touch = startTouch || active;

		if (inside && touch)	//full touch
		{
			OsWinIO_setActiveRenderItem(self);
		}
		else
			if ((inside && !touch) || (active && !inside))		//mid color
			{
				front_cd = Rgba_aprox(back_cd, front_cd, 0.9f);
			}

		if (inside && active && endTouch)	//end
		{
			GuiItemEdit_saveCache();

			GuiItem_callClick(&self->base);

			GuiItem_clickUnderline(&self->base, &self->text, self->formatURL, self->formatEmail);
		}

		if (endTouch)
			OsWinIO_resetActiveRenderItem();

		if (inside && !self->base.click)
			front_cd = Rgba_aprox(back_cd, front_cd, 0.7f);

		//cursor
		if (inside)
		{
			if (!self->cursorNone)
			{
				if (self->cursorHand || GuiItemText_isUnderline(self))
					Win_updateCursor(win, Win_CURSOR_HAND);
				else
					Win_updateCursor(win, Win_CURSOR_IBEAM);
			}
		}
	}

	_GuiItem_updateFinalCd(&self->base, back_cd, front_cd, coord, win);
}

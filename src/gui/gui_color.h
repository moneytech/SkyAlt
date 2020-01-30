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

typedef struct GuiItemColor_s
{
	GuiItem base;
	DbValue value;

	BOOL pickuper;
}GuiItemColor;

GuiItemColor* GuiItemColor_new(Quad2i grid, DbValue value, BOOL pickuper)
{
	GuiItemColor* self = Os_malloc(sizeof(GuiItemColor));
	self->base = GuiItem_init(GuiItem_COLOR, grid);

	self->value = value;
	self->pickuper = pickuper;

	return self;
}

GuiItem* GuiItemColor_newCopy(GuiItemColor* src, BOOL copySub)
{
	GuiItemColor* self = Os_malloc(sizeof(GuiItemColor));
	*self = *src;
	GuiItem_initCopy(&self->base, &src->base, copySub);

	self->value = DbValue_initCopy(&src->value);

	return (GuiItem*)self;
}

void GuiItemColor_delete(GuiItemColor* self)
{
	DbValue_free(&self->value);

	GuiItem_free(&self->base);
	Os_free(self, sizeof(GuiItemColor));
}

static Quad2i _GuiItemColor_getFinalCdCoord(Quad2i coord)
{
	const int cell = OsWinIO_cellSize();
	return Quad2i_init4(coord.start.x, coord.start.y, coord.size.x, cell * 2);
}

static Quad2i _GuiItemColor_getHueCoord(Quad2i coord)
{
	const int cell = OsWinIO_cellSize();
	return Quad2i_init4(coord.start.x, coord.start.y + cell * 10, coord.size.x, cell * 2);
}

static Rgba _GuiItemColor_getValue(GuiItemColor* self)
{
	return DbValue_getCd(&self->value);
}

static void _GuiItemColor_updateEdits(GuiItemColor* self)
{
	//get values
	Rgba rgba = _GuiItemColor_getValue(self);
	int hV;
	float sV, lV;
	Rgba_getHSL(&rgba, &hV, &sV, &lV);

	//get edit-boxes
	GuiItemEdit* r = GuiItem_findName(&self->base, "r");
	GuiItemEdit* g = GuiItem_findName(&self->base, "g");
	GuiItemEdit* b = GuiItem_findName(&self->base, "b");

	GuiItemEdit* h = GuiItem_findName(&self->base, "h");
	GuiItemEdit* s = GuiItem_findName(&self->base, "s");
	GuiItemEdit* l = GuiItem_findName(&self->base, "l");

	GuiItemSlider* r2 = GuiItem_findName(&self->base, "r2");
	GuiItemSlider* g2 = GuiItem_findName(&self->base, "g2");
	GuiItemSlider* b2 = GuiItem_findName(&self->base, "b2");

	GuiItemSlider* h2 = GuiItem_findName(&self->base, "h2");
	GuiItemSlider* s2 = GuiItem_findName(&self->base, "s2");
	GuiItemSlider* l2 = GuiItem_findName(&self->base, "l2");

	GuiItemEdit* hex = GuiItem_findName(&self->base, "hex");

	if (r && g && b && h && s && l && hex)
	{
		//update edit-boxes
		GuiItemEdit_setNumber(r, Rgba_r(rgba));
		GuiItemEdit_setNumber(g, Rgba_g(rgba));
		GuiItemEdit_setNumber(b, Rgba_b(rgba));

		GuiItemEdit_setNumber(h, hV);
		GuiItemEdit_setNumber(s, sV);
		GuiItemEdit_setNumber(l, lV);

		GuiItemSlider_setNumber(r2, Rgba_r(rgba));
		GuiItemSlider_setNumber(g2, Rgba_g(rgba));
		GuiItemSlider_setNumber(b2, Rgba_b(rgba));

		GuiItemSlider_setNumber(h2, hV);
		GuiItemSlider_setNumber(s2, sV);
		GuiItemSlider_setNumber(l2, lV);

		//hex code
		{
			char code[8];
			Rgba_getHex(&rgba, code);
			UNI* str = Std_newUNI_char(code);
			DbValue_setTextCopy(&hex->text, str);
			Std_deleteUNI(str);
		}
	}
}

static void _GuiItemColor_setValue(GuiItemColor* self, Rgba cd)
{
	cd.a = 255;	//always full
	DbValue_setCd(&self->value, cd);
	_GuiItemColor_updateEdits(self);
}

void GuiItemColor_clickEditedHsl(GuiItem* self)
{
	GuiItemColor* color = GuiItem_findParentType(self, GuiItem_COLOR);
	GuiItemEdit* h = GuiItem_findName(self, "h");
	GuiItemEdit* s = GuiItem_findName(self, "s");
	GuiItemEdit* l = GuiItem_findName(self, "l");
	if (color && h && s && l)
	{
		_GuiItemColor_setValue(color, Rgba_initHSL(GuiItemEdit_getNumber(h), GuiItemEdit_getNumber(s), GuiItemEdit_getNumber(l)));
		GuiItem_setRedraw(&color->base, TRUE);
	}
}
void GuiItemColor_clickEditedRgb(GuiItem* self)
{
	GuiItemColor* color = GuiItem_findParentType(self, GuiItem_COLOR);
	GuiItemEdit* r = GuiItem_findName(self, "r");
	GuiItemEdit* g = GuiItem_findName(self, "g");
	GuiItemEdit* b = GuiItem_findName(self, "b");
	if (color && r && g && b)
	{
		_GuiItemColor_setValue(color, Rgba_init4(GuiItemEdit_getNumber(r), GuiItemEdit_getNumber(g), GuiItemEdit_getNumber(b), 255));
		GuiItem_setRedraw(&color->base, TRUE);
	}
}

void GuiItemColor_clickEditedHsl2(GuiItem* self)
{
	GuiItemColor* color = GuiItem_findParentType(self, GuiItem_COLOR);
	GuiItemSlider* h = GuiItem_findName(self, "h2");
	GuiItemSlider* s = GuiItem_findName(self, "s2");
	GuiItemSlider* l = GuiItem_findName(self, "l2");
	if (color && h && s && l)
	{
		Rgba cd = Rgba_initHSL(GuiItemSlider_getNumber(h), GuiItemSlider_getNumber(s), GuiItemSlider_getNumber(l));

		_GuiItemColor_setValue(color, cd);
		GuiItem_setRedraw(&color->base, TRUE);
	}
}
void GuiItemColor_clickEditedRgb2(GuiItem* self)
{
	GuiItemColor* color = GuiItem_findParentType(self, GuiItem_COLOR);
	GuiItemSlider* r = GuiItem_findName(self, "r2");
	GuiItemSlider* g = GuiItem_findName(self, "g2");
	GuiItemSlider* b = GuiItem_findName(self, "b2");
	if (color && r && g && b)
	{
		_GuiItemColor_setValue(color, Rgba_init4(GuiItemSlider_getNumber(r), GuiItemSlider_getNumber(g), GuiItemSlider_getNumber(b), 255));
		GuiItem_setRedraw(&color->base, TRUE);
	}
}

void GuiItemColor_clickEditedHex(GuiItem* self)
{
	GuiItemColor* color = GuiItem_findParentType(self, GuiItem_COLOR);
	GuiItemEdit* hex = GuiItem_findName(self, "hex");
	if (color && hex)
	{
		const UNI* code = GuiItemEdit_getText(hex);
		char* str = Std_newCHAR_uni(code);
		Rgba cd = Rgba_initFromHex(str);
		Std_deleteCHAR(str);

		_GuiItemColor_setValue(color, cd);
		GuiItem_setRedraw(&color->base, TRUE);
	}
}

void GuiItemColor_draw(GuiItemColor* self, Image4* img, Quad2i coord, Win* win)
{
	Rgba cd = _GuiItemColor_getValue(self);

	if (self->pickuper)
	{
		//final color
		Image4_drawBoxQuad(img, _GuiItemColor_getFinalCdCoord(coord), cd);

		//hue "rainbow"
		Quad2i hueCoord = _GuiItemColor_getHueCoord(coord);
		int i;
		for (i = 0; i < coord.size.x; i++)
		{
			Rgba cdl = Rgba_initHSL(i * 360 / coord.size.x, 1, 0.5);
			Image4_drawBoxQuad(img, Quad2i_init4(hueCoord.start.x + i, hueCoord.start.y, 1, hueCoord.size.y), cdl);
		}

		//hue "rainbow" actual color position
		int hV;
		float sV, lV;
		Rgba_getHSL(&cd, &hV, &sV, &lV);

		i = hV * coord.size.x / 360;
		Image4_drawBoxQuad(img, Quad2i_init4(hueCoord.start.x + i - 1, hueCoord.start.y, 2, hueCoord.size.y), Rgba_initBlack());
	}
	else
	{
		BOOL disable = !GuiItem_isEnable(&self->base);

		coord = Quad2i_addSpace(coord, 2);
		//if(disable)
		//	cd = Rgba_multV(cd, 0.5f);

		Image4_drawBoxQuad(img, coord, cd);

		if (disable)
		{
			const int cell = OsWinIO_cellSize();
			Rgba cd = Rgba_initGrey();

			Quad2i q = Quad2i_initMid(Quad2i_getMiddle(coord), Vec2i_init2(cell / 2, cell / 2));
			//Quad2i q = Quad2i_addSpace(coord, 6);
			Vec2i qend = Quad2i_end(q);

			Image4_drawLine(img, q.start, qend, 2, cd);
			Image4_drawLine(img, Vec2i_init2(q.start.x, qend.y), Vec2i_init2(qend.x, q.start.y), 2, cd);

			Image4_drawBorder(img, coord, 1, cd);
		}
	}
}

void GuiItemColor_update(GuiItemColor* self, Quad2i coord, Win* win)
{
	BOOL changed = DbValue_hasChanged(&self->value);
	GuiItem_setRedraw(&self->base, changed);
}

void GuiItemColor_touch(GuiItemColor* self, Quad2i coord, Win* win)
{
	Rgba back_cd = g_theme.white;
	Rgba front_cd = g_theme.black;

	if (self->base.touch && GuiItem_isEnable(&self->base) && OsWinIO_canActiveRenderItem(self))
	{
		Quad2i coo = self->pickuper ? _GuiItemColor_getHueCoord(coord) : coord;

		BOOL startTouch = (OsWinIO_getTouch_action() == Win_TOUCH_DOWN_S) || (OsWinIO_getTouch_action() == Win_TOUCH_FORCE_DOWN_S);
		BOOL endTouch = (OsWinIO_getTouch_action() == Win_TOUCH_DOWN_E) || (OsWinIO_getTouch_action() == Win_TOUCH_FORCE_DOWN_E);

		BOOL active = OsWinIO_isActiveRenderItem(self);
		BOOL inside = Quad2i_inside(coo, OsWinIO_getTouchPos());
		BOOL touch = startTouch || active;

		if (inside && touch) //full touch
		{
			back_cd = g_theme.white;
			front_cd = g_theme.black;
			OsWinIO_setActiveRenderItem(self);

			if (self->pickuper)
			{
				int hue = (OsWinIO_getTouchPos().x - coord.start.x) * 360 / coord.size.x;
				_GuiItemColor_setValue(self, Rgba_initHSL(hue, 0.8f, 0.6f));
			}
		}
		else
			if ((inside && !touch) || (active && !inside)) //mid color
			{
				back_cd = Rgba_aprox(back_cd, front_cd, 0.2f);
			}

		if (inside && active && endTouch) //end
		{
			GuiItemEdit_saveCache();

			if (!self->pickuper)
			{
				//layout
				GuiItemLayout* layout = GuiItemLayout_new(Quad2i_init4(0, 0, 1, 1));
				GuiItemLayout_addColumn(layout, 0, 20);
				GuiItemLayout_addRow(layout, 0, 13);
				GuiItem_addSubName((GuiItem*)layout, "color", (GuiItem*)GuiItemColor_new(Quad2i_init4(0, 0, 1, 1), DbValue_initCopy(&self->value), TRUE));

				GuiItemRoot_addDialogRelLayout(layout, &self->base, self->base.coordMove, FALSE);
				//GuiItem_addSub(&self->base, GuiItemLevel_new(TRUE, FALSE, (GuiItem*)layout));
			}
		}

		if (endTouch)
			OsWinIO_resetActiveRenderItem();

		//cursor
		if (inside)
			Win_updateCursor(win, self->pickuper ? Win_CURSOR_COL_RESIZE : Win_CURSOR_HAND);
	}

	_GuiItem_updateFinalCd(&self->base, back_cd, front_cd, coord, win);
}

BOOL GuiItemColor_enableFromLightness(GuiItem* item)
{
	GuiItemSlider* l2 = GuiItem_findName(item, "l2");
	return GuiItemSlider_getValueT(l2) > 0;
}
void GuiItemColor_clickPrebuildColor(GuiItem* item)
{
	GuiItemColor* color = GuiItem_findParentType(item, GuiItem_COLOR);
	_GuiItemColor_setValue(color, ((GuiItemButton*)item)->back_cd);

	GuiItem_setRedraw(&color->base, TRUE);
}

GuiItemLayout* GuiItemColor_resize(GuiItemColor* self, GuiItemLayout* layout, Win* win)
{
	if (!self->base.resize)
		return GuiItem_numSub(&self->base) ? (GuiItemLayout*)GuiItem_getSub(&self->base, 0) : layout;

	if (self->pickuper)
	{
		GuiItem_freeSubs(&self->base);

		//layout
		layout = GuiItemLayout_newCoord(&self->base, FALSE, FALSE, win);

		layout->drawBackground = FALSE;
		layout->drawBorder = TRUE;
		GuiItemLayout_addColumn(layout, 0, 3);
		GuiItemLayout_addColumn(layout, 1, 99);
		GuiItemLayout_addColumn(layout, 2, 3);
		GuiItemLayout_addColumn(layout, 3, 99);

		GuiItem_addSubName(&self->base, "layout_main", &layout->base);

		GuiItem_addSubName((GuiItem*)layout, "r", GuiItemEdit_newEx(Quad2i_init4(0, 2, 1, 2), DbValue_initEmpty(), DbValue_initLang("RED"), &GuiItemColor_clickEditedRgb));
		GuiItem_addSubName((GuiItem*)layout, "g", GuiItemEdit_newEx(Quad2i_init4(0, 4, 1, 2), DbValue_initEmpty(), DbValue_initLang("GREEN"), &GuiItemColor_clickEditedRgb));
		GuiItem_addSubName((GuiItem*)layout, "b", GuiItemEdit_newEx(Quad2i_init4(0, 6, 1, 2), DbValue_initEmpty(), DbValue_initLang("BLUE"), &GuiItemColor_clickEditedRgb));

		GuiItem_addSubName((GuiItem*)layout, "h", GuiItemEdit_newEx(Quad2i_init4(2, 2, 1, 2), DbValue_initEmpty(), DbValue_initLang("HUE"), &GuiItemColor_clickEditedHsl));
		GuiItem_addSubName((GuiItem*)layout, "s", GuiItemEdit_newEx(Quad2i_init4(2, 4, 1, 2), DbValue_initEmpty(), DbValue_initLang("SATURATION"), &GuiItemColor_clickEditedHsl));
		GuiItem_addSubName((GuiItem*)layout, "l", GuiItemEdit_newEx(Quad2i_init4(2, 6, 1, 2), DbValue_initEmpty(), DbValue_initLang("LIGHTNESS"), &GuiItemColor_clickEditedHsl));

		GuiItem_addSubName((GuiItem*)layout, "r2", GuiItemSlider_newEx(Quad2i_init4(1, 2, 1, 2), DbValue_initNumber(0), DbValue_initNumber(255), DbValue_initNumber(1), DbValue_initEmpty(), DbValue_initEmpty(), DbValue_initEmpty(), DbValue_initEmpty(), &GuiItemColor_clickEditedRgb2));
		GuiItem_addSubName((GuiItem*)layout, "g2", GuiItemSlider_newEx(Quad2i_init4(1, 4, 1, 2), DbValue_initNumber(0), DbValue_initNumber(255), DbValue_initNumber(1), DbValue_initEmpty(), DbValue_initEmpty(), DbValue_initEmpty(), DbValue_initEmpty(), &GuiItemColor_clickEditedRgb2));
		GuiItem_addSubName((GuiItem*)layout, "b2", GuiItemSlider_newEx(Quad2i_init4(1, 6, 1, 2), DbValue_initNumber(0), DbValue_initNumber(255), DbValue_initNumber(1), DbValue_initEmpty(), DbValue_initEmpty(), DbValue_initEmpty(), DbValue_initEmpty(), &GuiItemColor_clickEditedRgb2));

		GuiItem* h2 = GuiItem_addSubName((GuiItem*)layout, "h2", GuiItemSlider_newEx(Quad2i_init4(3, 2, 1, 2), DbValue_initNumber(0), DbValue_initNumber(360), DbValue_initNumber(1), DbValue_initEmpty(), DbValue_initEmpty(), DbValue_initEmpty(), DbValue_initEmpty(), &GuiItemColor_clickEditedHsl2));
		GuiItem* s2 = GuiItem_addSubName((GuiItem*)layout, "s2", GuiItemSlider_newEx(Quad2i_init4(3, 4, 1, 2), DbValue_initNumber(0), DbValue_initNumber(1), DbValue_initNumber(0.01), DbValue_initEmpty(), DbValue_initEmpty(), DbValue_initEmpty(), DbValue_initEmpty(), &GuiItemColor_clickEditedHsl2));
		GuiItem_addSubName((GuiItem*)layout, "l2", GuiItemSlider_newEx(Quad2i_init4(3, 6, 1, 2), DbValue_initNumber(0), DbValue_initNumber(1), DbValue_initNumber(0.01), DbValue_initEmpty(), DbValue_initEmpty(), DbValue_initEmpty(), DbValue_initEmpty(), &GuiItemColor_clickEditedHsl2));
		GuiItem_setEnableCallback(h2, &GuiItemColor_enableFromLightness);
		GuiItem_setEnableCallback(s2, &GuiItemColor_enableFromLightness);

		//hex
		GuiItem_addSubName((GuiItem*)layout, "hex", GuiItemEdit_newEx(Quad2i_init4(0, 8, 4, 2), DbValue_initEmpty(), DbValue_initLang("HEX_COLOR"), &GuiItemColor_clickEditedHex));

		//pre-build colors
		{
			GuiItemLayout* preLay = GuiItemLayout_new(Quad2i_init4(0, 12, 4, 1));
			GuiItem_addSubName(&layout->base, "preLay", &preLay->base);

			char nameId[64];

			int pos = 0;
			GuiItem_addSubName((GuiItem*)preLay, Std_buildNumber(0, 0, nameId), GuiItemButton_newCd(Quad2i_init4(pos++, 0, 1, 1), Rgba_initBlack(), &GuiItemColor_clickPrebuildColor));
			GuiItem_addSubName((GuiItem*)preLay, Std_buildNumber(0, 0, nameId), GuiItemButton_newCd(Quad2i_init4(pos++, 0, 1, 1), Rgba_initWhite(), &GuiItemColor_clickPrebuildColor));
			GuiItem_addSubName((GuiItem*)preLay, Std_buildNumber(0, 0, nameId), GuiItemButton_newCd(Quad2i_init4(pos++, 0, 1, 1), Rgba_initGreyLight(), &GuiItemColor_clickPrebuildColor));
			GuiItem_addSubName((GuiItem*)preLay, Std_buildNumber(0, 0, nameId), GuiItemButton_newCd(Quad2i_init4(pos++, 0, 1, 1), Rgba_initGrey(), &GuiItemColor_clickPrebuildColor));
			GuiItem_addSubName((GuiItem*)preLay, Std_buildNumber(0, 0, nameId), GuiItemButton_newCd(Quad2i_init4(pos++, 0, 1, 1), Rgba_initGreyDark(), &GuiItemColor_clickPrebuildColor));

			const int N_COLORS = 8;
			int i = 0;
			for (i = 0; i < N_COLORS; i++)
			{
				Rgba cd = Rgba_initHSL(360 / N_COLORS * i, 0.7f, 0.5f);
				GuiItem_addSubName((GuiItem*)preLay, Std_buildNumber(pos, 0, nameId), GuiItemButton_newCd(Quad2i_init4(pos, 0, 1, 1), cd, &GuiItemColor_clickPrebuildColor));
				pos++;
			}

			for (i = 0; i < pos; i++)
				GuiItemLayout_addColumn(preLay, i, 2);
		}

		_GuiItemColor_updateEdits(self);
	}

	return layout;
}

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

typedef struct GuiItemEdit_s
{
	GuiItem base;

	int text_level;
	DbValue text;
	DbValue description;

	BOOL password;
	BOOL selectMode;

	BOOL drawBorder;
	BOOL doubleBorder;
	BOOL roundedBorder;

	BOOL cursorShow;
	UINT numClickIn;

	BOOL drawHighlightIfContent;	//remove, do it through callbackHighlight ...
	GuiItemCallbackEnable* callbackHighlight;

	Rgba colorBorder;

	GuiItemCallback* clickChanged;
	GuiItemCallback* clickActivate;

	BOOL tabAway;	//after hit 'tab key' jump on another EditBox

	BOOL showDescription;

	UNI* str;

	BOOL startClickPicker;
	BOOL showPicker;
	BOOL pickerOpen;
	BOOL pickerFolder;
	BOOL pickerMultiple;
	const UNI* pickerAction;
	const UNI* pickerExts;

	BOOL drawBackground_procentage_visualize;
	BOOL drawBackground_procentage_visualize_mult100;


	DbValue highlightFind;

} GuiItemEdit;

GuiItem* GuiItemEdit_newEx(Quad2i grid, DbValue text, DbValue description, GuiItemCallback* callFinish)
{
	GuiItemEdit* self = Os_malloc(sizeof(GuiItemEdit));
	self->base = GuiItem_init(GuiItem_EDIT, grid);

	self->text_level = 1;
	self->text = text;
	self->description = description;

	self->password = FALSE;
	self->selectMode = FALSE;
	self->drawBorder = TRUE;
	self->doubleBorder = FALSE;
	self->roundedBorder = TRUE;

	self->cursorShow = TRUE;
	self->numClickIn = 1;

	self->colorBorder = Rgba_initBlack();

	self->drawHighlightIfContent = FALSE;
	self->callbackHighlight = 0;

	self->clickChanged = 0;
	self->clickActivate = 0;

	self->tabAway = TRUE;

	GuiItem_setCallClick(&self->base, callFinish);

	self->str = 0;

	self->showDescription = TRUE;

	self->startClickPicker = FALSE;
	self->showPicker = FALSE;
	self->pickerOpen = FALSE;
	self->pickerFolder = FALSE;
	self->pickerMultiple = FALSE;
	self->pickerAction = 0;
	self->pickerExts = 0;

	self->drawBackground_procentage_visualize = FALSE;
	self->drawBackground_procentage_visualize_mult100 = FALSE;

	self->base.icon_draw_back = FALSE;

	self->highlightFind = DbValue_initEmpty();

	return &self->base;
}

GuiItem* GuiItemEdit_new(Quad2i grid, DbValue text, DbValue description)
{
	return GuiItemEdit_newEx(grid, text, description, 0);
}

GuiItem* GuiItemEdit_newCopy(GuiItemEdit* src, BOOL copySub)
{
	GuiItemEdit* self = Os_malloc(sizeof(GuiItemEdit));
	*self = *src;
	GuiItem_initCopy(&self->base, &src->base, copySub);

	self->text = DbValue_initCopy(&src->text);
	self->description = DbValue_initCopy(&src->description);

	self->highlightFind = DbValue_initCopy(&src->highlightFind);

	self->str = Std_newUNI(src->str);

	if (OsWinIO_isCursorGuiItem(src))
		OsWinIO_setCursorGuiItem(self);

	return(GuiItem*)self;
}

void GuiItemEdit_saveCache(void)
{
	GuiItemEdit* self = OsWinIO_getCursorRenderItem();

	if (self && !OsWinIO_isCursorEmpty())
	{
		GuiItemEdit_setText(self, OsWinIO_getCursorRenderItemCache());
		DbValue_setFormated(&self->text, TRUE);
		GuiItem_callClick(&self->base);
	}

	OsWinIO_resetCursorGuiItem();
}

BOOL GuiItemEdit_setCursor(GuiItemEdit* self, BOOL selectAll)
{
	BOOL activeCursor = OsWinIO_isCursorGuiItem(self);
	if (!activeCursor)
	{
		DbValue_hasChanged(&self->text);

		DbValue_setFormated(&self->text, FALSE);

		GuiItemEdit_saveCache(); //save old
		OsWinIO_setCursorGuiItem(self); //set new
		OsWinIO_setCursorText(DbValue_resultNoPost(&self->text));

		if (selectAll)
		{
			OsWinIO_setCursorRenderItemPosX(0);
			OsWinIO_setCursorRenderItemPosY(Std_sizeUNI(DbValue_resultNoPost(&self->text)));
		}

		OsWinIO_resetActiveRenderItem();
	}

	return !activeCursor;
}

void GuiItemEdit_clickActivate(GuiItem* self)
{
	GuiItemEdit_setCursor((GuiItemEdit*)self, TRUE);
}

void GuiItemEdit_delete(GuiItemEdit* self)
{
	if (OsWinIO_getCursorRenderItem() == self)
		GuiItemEdit_saveCache();

	Std_deleteUNI(self->str);
	DbValue_free(&self->text);
	DbValue_free(&self->description);
	DbValue_free(&self->highlightFind);
	
	GuiItem_free(&self->base);
	Os_free(self, sizeof(GuiItemEdit));
}

void GuiItemEdit_setShowPicker(GuiItemEdit* self, BOOL showPicker, BOOL pickerOpen, BOOL pickerFolder, BOOL pickerMultiple, const UNI* pickerAction, const UNI* pickerExts)
{
	self->showPicker = showPicker;
	self->pickerOpen = pickerOpen;
	self->pickerFolder = pickerFolder;
	self->pickerMultiple = pickerMultiple;
	self->pickerAction = pickerAction;
	self->pickerExts = pickerExts;
}

void GuiItemEdit_setTextLevel(GuiItemEdit* self, int text_level)
{
	self->text_level = text_level;
}

void GuiItemEdit_showDescription(GuiItemEdit* self, BOOL showDescription)
{
	self->showDescription = showDescription;
}

const UNI* GuiItemEdit_getText(const GuiItemEdit* self)
{
	return self ? DbValue_resultNoPost(&self->text) : 0;
}
const UNI* GuiItemEdit_getTextOrCache(const GuiItemEdit* self)
{
	return (OsWinIO_getCursorRenderItem() == self) ? OsWinIO_getCursorRenderItemCache() : GuiItemEdit_getText(self);
}

void GuiItemEdit_setText(GuiItemEdit* self, const UNI* str)
{
	DbValue_setTextCopy(&self->text, str);
}

void GuiItemEdit_setPasswordStars(GuiItemEdit* self, BOOL stars)
{
	self->password = stars;
}

void GuiItemEdit_setHighlightIfContent(GuiItemEdit* self, BOOL drawHighlightIfContent)
{
	self->drawHighlightIfContent = drawHighlightIfContent;
}

void GuiItemEdit_setHighlightCallback(GuiItemEdit* self, GuiItemCallbackEnable* callbackHighlight)
{
	self->callbackHighlight = callbackHighlight;
}

void GuiItemEdit_setHighlightFind(GuiItemEdit* self, DbValue highlightFind)
{
	DbValue_free(&self->highlightFind);
	self->highlightFind = highlightFind;
}


void GuiItemEdit_setFnChanged(GuiItemEdit* self, GuiItemCallback* clickChanged)
{
	self->clickChanged = clickChanged;
}

void GuiItemEdit_setFnActivate(GuiItemEdit* self, GuiItemCallback* clickActivate)
{
	self->clickActivate = clickActivate;
}

double GuiItemEdit_getNumber(GuiItemEdit* self)
{
	return DbValue_getNumber(&self->text);
}
void GuiItemEdit_setNumber(GuiItemEdit* self, double value)
{
	DbValue_setNumber(&self->text, value);
}

int _GuiItemEdit_getCursorPos(GuiItemEdit* self, Quad2i coord, Win* win)
{
	const int textH = _GuiItem_textSize(self->text_level, coord.size.y);
	int pos = OsFont_getCursorPos(OsWinIO_getFontDefault(), textH, self->str, (OsWinIO_getTouchPos().x - coord.start.x - textH / 2 + OsWinIO_getCursorRenderItemDrawX()));
	return pos;
}

static int _GuiItemEdit_getDrawX(GuiItemEdit* self, Quad2i coord, Win* win)
{
	if (OsWinIO_isCursorGuiItem(self))
	{
		Vec2i size = coord.size;
		int textH = _GuiItem_textSize(self->text_level, size.y);
		OsFont* font = OsWinIO_getFontDefault();

		int curr = OsWinIO_getCursorRenderItemPos().y;
		int currPix = OsFont_getCharPixelPos(font, textH, self->str, curr);

		size.x -= textH;
		int t = (currPix - OsWinIO_getCursorRenderItemDrawX());
		if (t < 0 || t >= size.x) //check current
			OsWinIO_setCursorRenderItemDrawX((currPix > size.x) ? (currPix - size.x) : 0);

		return OsWinIO_getCursorRenderItemDrawX(); //pixels
	}

	return 0;
}

static Quad2i _GuiItemEdit_getPickerCord(Quad2i coord)
{
	const int cell = OsWinIO_cellSize();

	Quad2i q = coord;
	q.start.x += q.size.x - q.size.y;
	q.size.x = 2 * cell;

	return Quad2i_addSpace(q, 5);
}

void GuiItemEdit_drawBackgroundVisualize(Image4* img, Quad2i coord, double value, BOOL mult100, Rgba cd)
{
	double v = value / (mult100 ? 1 : 100);
	Quad2i q = Quad2i_addSpace(coord, 4);
	q.size.x *= Std_fclamp(v, 0, 1);

	Image4_drawBoxQuad(img, q, cd);
}


/*static void GuiItemEdit_drawHighlight(GuiItemEdit* self, Image4* img, Quad2i coord, int posTextY, int pxs, int pxe)
{
	int textH = _GuiItem_textSize(self->text_level, coord.size.y);
	OsFont* font = OsWinIO_getFontDefault();

	int h = textH * 1.2f;
	int py = posTextY - h / 2;

	if (pxs > pxe)
		Std_flip(&pxs, &pxe);
	if (pxs == pxe)
		Image4_drawBoxStartEnd(img, Vec2i_add(Vec2i_add(coord.start, Vec2i_init2(pxe, py)), currMove), Vec2i_add(Vec2i_add(coord.start, Vec2i_init2(pxe + 2, py + h)), currMove), Rgba_aprox(self->base.back_cd, self->base.front_cd, OsWinIO_getEditboxAnim(self))); //cursor
	else
		Image4_drawBoxStartEnd(img, Vec2i_add(Vec2i_add(coord.start, Vec2i_init2(pxs, py)), currMove), Vec2i_add(Vec2i_add(coord.start, Vec2i_init2(pxe, py + h)), currMove), Rgba_aprox(self->base.back_cd, self->base.front_cd, 0.5f)); //selection
}*/

void GuiItemEdit_draw(GuiItemEdit* self, Image4* img, Quad2i coord, Win* win)
{
	const int cell = OsWinIO_cellSize();

	int textH = _GuiItem_textSize(self->text_level, coord.size.y);
	OsFont* font = OsWinIO_getFontDefault();

	if (self->showPicker)
	{
		Quad2i pickerCoord = _GuiItemEdit_getPickerCord(coord);
		Image4_drawBoxQuad(img, pickerCoord, g_theme.main);
		Image4_drawText(img, Quad2i_getMiddle(pickerCoord), TRUE, font, _UNI32("+"), textH, 0, self->base.front_cd); //text

		//resize down
		coord.size.x -= coord.size.y;
		img->rect.size.x -= coord.size.y;
		Image4_repairRect(img);
	}

	const int num_lines = (self->showDescription && coord.size.y > OsWinIO_cellSize() * 1.5f) ? 2 : 1;
	const BOOL cursorActive = OsWinIO_isCursorGuiItem(self);

	const UNI* text = self->str;

	if (!self->base.drawTable)
		coord = Quad2i_addSpace(coord, 3);

	//background
	BOOL isSelect = !Rgba_isBlack(&self->colorBorder);
	Rgba bCd = isSelect ? self->colorBorder : self->base.front_cd;
	Image4_drawBoxQuad(img, coord, self->base.back_cd);

	double v = DbValue_getNumber(&self->text);
	if (self->drawBackground_procentage_visualize)
		GuiItemEdit_drawBackgroundVisualize(img, coord, v, self->drawBackground_procentage_visualize_mult100, Rgba_aprox(self->base.back_cd, g_theme.main, 0.5f));

	if (self->base.drawTable)
	{
		Quad2i q = coord;
		q.size.x += 1;
		q.size.y += 1;

		img->rect.size.x += 1;
		img->rect.size.y += 1;
		Image4_repairRect(img);
		//img->rect = q;

		Image4_drawBorder(img, q, 1, Rgba_aprox(self->base.back_cd, self->base.front_cd, 0.5f));
	}
	else
		if (self->drawBorder)
			Image4_drawBorder(img, coord, (isSelect ? 3 : 1), bCd);

	Vec2i size = coord.size;

	img->rect.start.x += textH / 2;
	img->rect.size.x -= textH;
	Image4_repairRect(img);

	Vec2i currMove = Vec2i_init2(_GuiItemEdit_getDrawX(self, coord, win) * -1, 0);

	//text
	int extra_down;
	BOOL centerDesc = OsFont_getTextSize(font, DbValue_result(&self->description), textH, 0, &extra_down).x < (size.x - textH);
	Vec2i posDesc, posText;
	posDesc.x = centerDesc ? size.x / 2 : textH / 2;
	posText.x = textH / 2;
	if (num_lines == 1)
	{
		posDesc.y = posText.y = size.y / 2;
	}
	else
	{
		posDesc.y = cell / 2;// size.y / 4;
		//posText.y = size.y * 3 / 4;
		posText.y = cell + (size.y - cell) / 2;
	}

	//description
	if (num_lines == 2)
		Image4_drawText(img, Vec2i_add(posDesc, coord.start), centerDesc, font, DbValue_result(&self->description), textH, 0, self->base.front_cd);

	//description(ghost)
	if (num_lines == 1 && Std_sizeUNI(text) == 0)
		Image4_drawText(img, Vec2i_add(posDesc, coord.start), centerDesc, font, DbValue_result(&self->description), textH, 0, Rgba_aprox(self->base.front_cd, self->base.back_cd, 0.5f)); //ghost(one-line description)


	//hightlight find string
	if(!cursorActive)
	{
		const UNI* find = self->highlightFind.result.str;
		const UBIG findSize = Std_sizeUNI(find);
		if (findSize)
		{
			BIG startPos = Std_subUNI(text, find);
			if (startPos >= 0)
			{
				int h = textH * 1.2f;
				int py = posText.y - h / 2;

				int pxs, pxe;
				pxs = OsFont_getCharPixelPos(font, textH, text, startPos) + textH / 2;
				pxe = OsFont_getCharPixelPos(font, textH, text, startPos+findSize) + textH / 2;

				Image4_drawBoxStartEnd(img, Vec2i_add(coord.start, Vec2i_init2(pxs, py)), Vec2i_add(coord.start, Vec2i_init2(pxe, py + h)), g_theme.main);
			}
		}
	}

	//cursor
	if (cursorActive)
	{
		int h = textH * 1.2f;
		int py = posText.y - h / 2;

		int pxs, pxe;
		pxs = OsFont_getCharPixelPos(font, textH, text, OsWinIO_getCursorRenderItemPos().x) + textH / 2;
		pxe = OsFont_getCharPixelPos(font, textH, text, OsWinIO_getCursorRenderItemPos().y) + textH / 2;

		if (pxs > pxe)
			Std_flip(&pxs, &pxe);
		if (pxs == pxe)
			Image4_drawBoxStartEnd(img, Vec2i_add(Vec2i_add(coord.start, Vec2i_init2(pxe, py)), currMove), Vec2i_add(Vec2i_add(coord.start, Vec2i_init2(pxe + 2, py + h)), currMove), Rgba_aprox(self->base.back_cd, self->base.front_cd, OsWinIO_getEditboxAnim(self))); //cursor
		else
			Image4_drawBoxStartEnd(img, Vec2i_add(Vec2i_add(coord.start, Vec2i_init2(pxs, py)), currMove), Vec2i_add(Vec2i_add(coord.start, Vec2i_init2(pxe, py + h)), currMove), Rgba_aprox(self->base.back_cd, self->base.front_cd, 0.5f)); //selection
	}

	//text
	Rgba cd = self->base.front_cd;
	if (!cursorActive && DbValue_isFormatUnderline(&self->text))
		cd = Rgba_aprox(self->base.front_cd, g_theme.main, 0.6f);

	Image4_drawText(img, Vec2i_add(Vec2i_add(posText, coord.start), currMove), FALSE, font, text, textH, 0, cd); //text
}

static void _GuiItemEdit_updateStr(GuiItemEdit* self)
{
	Std_replaceUNI(&self->str, OsWinIO_isCursorGuiItem(self) ? OsWinIO_getCursorRenderItemCache() : DbValue_result(&self->text));
	if (self->password)
		Std_replaceCharacters(self->str, L'*');
}

void GuiItemEdit_update(GuiItemEdit* self, Quad2i coord, Win* win)
{
	BOOL changed = (DbValue_hasChanged(&self->text) || DbValue_hasChanged(&self->description) || DbValue_hasChanged(&self->highlightFind));

	_GuiItemEdit_updateStr(self);

	GuiItem_setRedraw(&self->base, changed);
}

void _GuiItemEdit_addDropPath(GuiItemEdit* self)
{
	GuiItemEdit_saveCache();
	GuiItemEdit_setCursor(self, FALSE);
	UNI* p = OsWinIO_getCursorRenderItemCache();

	int path_i = 0;
	UNI* path;
	while ((path = OsWinIO_getDropFile(path_i++)))
	{
		if (Std_sizeUNI(p))
			p = Std_addAfterUNI(p, _UNI32(";"));
		p = Std_addAfterUNI(p, path);

		//clean
		Std_deleteUNI(path);
	}

	OsWinIO_setCursorRenderItemCache(p);
	OsWinIO_resetDrop();
	GuiItemEdit_saveCache();
}
void _GuiItemEdit_addDropContent(GuiItemEdit* self)
{
	GuiItemEdit_saveCache();
	GuiItemEdit_setCursor(self, FALSE);
	UNI* p = OsWinIO_getCursorRenderItemCache();

	int path_i = 0;
	UNI* path;
	while ((path = OsWinIO_getDropFile(path_i++)))
	{
		char* pth = Std_newCHAR_uni(path);
		OsFile f;
		if (OsFile_init(&f, pth, OsFile_R))
		{
			char read[256];
			const int n = OsFile_read(&f, read, 256);
			read[n - 1] = 0;
			UNI* rd = Std_newUNI_char_n(read, n);

			if (Std_sizeUNI(p))
				p = Std_addAfterUNI(p, _UNI32(";"));
			p = Std_addAfterUNI(p, rd);

			Std_deleteUNI(rd);
			OsFile_free(&f);
		}
		Std_deleteCHAR(pth);

		//clean
		Std_deleteUNI(path);
	}

	OsWinIO_setCursorRenderItemCache(p);
	OsWinIO_resetDrop();
	GuiItemEdit_saveCache();
}

void GuiItemEdit_touch(GuiItemEdit* self, Quad2i coord, Win* win)
{
	Rgba back_cd = g_theme.white;
	Rgba front_cd = g_theme.black;

	Vec2i backupSelect = OsWinIO_getCursorRenderItemPos();

	if (GuiItem_isEnable(&self->base) && self->selectMode)
		back_cd = g_theme.edit;

	if (!OsWinIO_isCursorGuiItem(self))
	{
		if (self->drawHighlightIfContent && Std_sizeUNI(self->str))
			back_cd = g_theme.highlight;

		if (self->callbackHighlight && self->callbackHighlight((GuiItem*)self))
			back_cd = g_theme.highlight;
	}

	if (self->base.touch && GuiItem_isEnable(&self->base) && OsWinIO_canActiveRenderItem(self))
	{
		BOOL cursorShow = OsWinIO_isCursorGuiItem(self) ? TRUE : self->cursorShow;
		UINT numClickIn = OsWinIO_isCursorGuiItem(self) ? 1 : self->numClickIn;

		BOOL startTouch = (OsWinIO_getTouch_action() == Win_TOUCH_DOWN_S) || (OsWinIO_getTouch_action() == Win_TOUCH_FORCE_DOWN_S);
		BOOL endTouch = (OsWinIO_getTouch_action() == Win_TOUCH_DOWN_E) || (OsWinIO_getTouch_action() == Win_TOUCH_FORCE_DOWN_E);

		BOOL active = OsWinIO_isActiveRenderItem(self);
		BOOL inside = Quad2i_inside(coord, OsWinIO_getTouchPos());
		BOOL touch = (startTouch && OsWinIO_getTouchNum() >= numClickIn) || active;

		BOOL insidePicker = self->showPicker ? Quad2i_inside(_GuiItemEdit_getPickerCord(coord), OsWinIO_getTouchPos()) : FALSE;

		BOOL underline = (OsWinIO_getKeyExtra() & Win_EXTRAKEY_CTRL) && DbValue_isFormatUnderline(&self->text) && !OsWinIO_isCursorGuiItem(self);

		if (insidePicker && startTouch)
			self->startClickPicker = TRUE;

		if (inside && underline && touch)
		{
			GuiItem_clickUnderline(&self->base, &self->text, FALSE, FALSE);	//open URL, email, map, etc.
		}
		else
			if (inside && touch) //full touch
			{
				const BOOL dclick = (OsWinIO_getTouchNum() >= 1 + self->numClickIn);
				

				BOOL isActive = !GuiItemEdit_setCursor(self, dclick);
				_GuiItemEdit_updateStr(self);

				int pc = _GuiItemEdit_getCursorPos(self, coord, win);
				if (active || OsWinIO_getKeyExtra() & Win_EXTRAKEY_SHIFT)
					OsWinIO_setCursorRenderItemPosY(pc); //continue
				else
				{
					//first time
					OsWinIO_setCursorRenderItemPosX(pc);
					OsWinIO_setCursorRenderItemPosY(pc);
				}

				if (dclick && isActive)
				{
					_GuiItemEdit_updateStr(self);
					OsWinIO_setCursorRenderItemPosX(0);
					OsWinIO_setCursorRenderItemPosY(Std_sizeUNI(self->str));
				}


				back_cd = Rgba_aprox(front_cd, back_cd, 0.6f);
				OsWinIO_setActiveRenderItem(self);
			}
			else
				if (active && !inside) //select text with mouse out of box
				{
					OsWinIO_setCursorRenderItemPosY(_GuiItemEdit_getCursorPos(self, coord, win));
					back_cd = Rgba_aprox(front_cd, back_cd, 0.8f); //full color
				}
				else
					if (inside && !touch) //mid color
					{
						back_cd = Rgba_aprox(front_cd, back_cd, 0.85f);
					}

		if (endTouch && OsWinIO_isDrop(&coord))
		{
			if (self->showPicker)
				_GuiItemEdit_addDropPath(self);
			else
				_GuiItemEdit_addDropContent(self);
		}

		if (active && endTouch) //end
		{
			if (self->clickActivate)
				self->clickActivate(&self->base);

			if (insidePicker && self->startClickPicker)
				GuiItemRoot_showPicker(Quad2i_getMiddle(coord), self->pickerOpen, self->pickerFolder, self->pickerMultiple, self->pickerAction, self->pickerExts);
		}

		if (endTouch)
		{
			OsWinIO_resetActiveRenderItem();
			self->startClickPicker = FALSE;
		}

		//cursor
		if (inside)
		{
			if (cursorShow)
			{
				if (insidePicker || underline)
				{
					Win_updateCursor(win, Win_CURSOR_HAND);
				}
				else
				{
					if (self->selectMode)
						Win_updateCursor(win, Win_CURSOR_HAND);
					else
						Win_updateCursor(win, Win_CURSOR_IBEAM);
				}
			}
		}

		if (OsWinIO_isCursorGuiItemInTime(self) || !Vec2i_cmp(backupSelect, OsWinIO_getCursorRenderItemPos()))
			GuiItem_setRedraw(&self->base, TRUE);
	}

	Rgba colorBorder = self->colorBorder;
	self->colorBorder = OsWinIO_isCursorGuiItem(self) ? g_theme.edit : Rgba_initBlack();
	GuiItem_setRedraw(&self->base, !Rgba_cmp(self->colorBorder, colorBorder));

	_GuiItem_updateFinalCd(&self->base, back_cd, front_cd, coord, win);
}

Vec2i _GuiItemEdit_keyRemoveSelect(Vec2i pos, UNI** text)
{
	if (pos.x < pos.y) *text = Std_removeChars(*text, pos.x, pos.y - pos.x), pos.y = pos.x;
	else
		if (pos.x > pos.y) *text = Std_removeChars(*text, pos.y, pos.x - pos.y), pos.x = pos.y;

	return pos;
}

void GuiItemEdit_key(GuiItemEdit* self, Quad2i coord, Win* win)
{
	if (/*!self->base.touch ||*/ !GuiItem_isEnable(&self->base))
		return;

	if (!OsWinIO_isCursorGuiItem(self) || (OsWinIO_getKeyID() == 0 && OsWinIO_getKeyExtra() == Win_EXTRAKEY_NONE))
		return;

	Vec2i backupSelect = OsWinIO_getCursorRenderItemPos();
	UBIG keyExtra = OsWinIO_getKeyExtra();

	if (keyExtra & Win_EXTRAKEY_ENTER)
	{
		GuiItemEdit_saveCache();
		GuiItem_setRedraw(&self->base, TRUE);
		OsWinIO_resetActiveRenderItem();
		OsWinIO_resetKey();
		return;
	}
	else
		if (keyExtra & Win_EXTRAKEY_ESCAPE) //reload old value
		{
			OsWinIO_resetCursorGuiItem();
			OsWinIO_resetActiveRenderItem();
			GuiItem_setRedraw(&self->base, TRUE);
			return;
		}

	UNI* text = OsWinIO_getCursorRenderItemCache();
	const int MX = Std_sizeUNI(text);

	UNI key = OsWinIO_getKeyID();

	Vec2i pos = OsWinIO_getCursorRenderItemPos();

	if (keyExtra & Win_EXTRAKEY_BACKSPACE)
	{
		if (pos.x != pos.y)
			pos = _GuiItemEdit_keyRemoveSelect(pos, &text);
		else
			if (pos.x > 0)
				text = Std_removeChars(text, --pos.x, 1), pos.y = pos.x;
	}
	else
		if (keyExtra & Win_EXTRAKEY_DELETE)
		{
			if (pos.x != pos.y)
				pos = _GuiItemEdit_keyRemoveSelect(pos, &text);
			else
				if (pos.x < MX)
					text = Std_removeChars(text, pos.x, 1), pos.y = pos.x;
		}
		else
			if (keyExtra & Win_EXTRAKEY_SHIFT && (keyExtra & Win_EXTRAKEY_LEFT || keyExtra & Win_EXTRAKEY_RIGHT || keyExtra & Win_EXTRAKEY_UP || keyExtra & Win_EXTRAKEY_DOWN || keyExtra & Win_EXTRAKEY_HOME || keyExtra & Win_EXTRAKEY_END))
			{
				if (pos.y > 0 && (keyExtra & Win_EXTRAKEY_LEFT || keyExtra & Win_EXTRAKEY_UP))
					pos.y--;
				else
					if (pos.y < MX && (keyExtra & Win_EXTRAKEY_RIGHT || keyExtra & Win_EXTRAKEY_DOWN))
						pos.y++;
					else
						if (keyExtra & Win_EXTRAKEY_HOME)
							pos.y = 0;
				if (keyExtra & Win_EXTRAKEY_END)
					pos.y = MX;
			}
			else
				if (keyExtra & Win_EXTRAKEY_LEFT || keyExtra & Win_EXTRAKEY_UP)
				{
					if (pos.x != pos.y)
						pos.x = pos.y = Std_min(pos.x, pos.y);
					else
						if (pos.x > 0)
							pos.x--, pos.y--;
				}
				else
					if (keyExtra & Win_EXTRAKEY_RIGHT || keyExtra & Win_EXTRAKEY_DOWN)
					{
						if (pos.x != pos.y)
							pos.x = pos.y = Std_max(pos.x, pos.y);
						else
							if (pos.x < MX)
								pos.x++, pos.y++;
					}
					else
						if (keyExtra & Win_EXTRAKEY_HOME)
						{
							pos.x = pos.y = 0;
						}
						else
							if (keyExtra & Win_EXTRAKEY_END)
							{
								pos.x = pos.y = MX;
							}
							else
								if (keyExtra & Win_EXTRAKEY_SELECT_ALL)
								{
									pos.x = 0;
									pos.y = MX;
								}
								else
									if (keyExtra & Win_EXTRAKEY_COPY)
									{
										if (!self->password)
										{
											UNI* str = 0;
											if (pos.x < pos.y) str = Std_newUNI_copy(&text[pos.x], pos.y - pos.x);
											else
												if (pos.y < pos.x) str = Std_newUNI_copy(&text[pos.y], pos.x - pos.y);

											if (str)
												Win_clipboard_setText(str);
											Std_deleteUNI(str);
										}
									}
									else
										if (keyExtra & Win_EXTRAKEY_PASTE)
										{
											if (!self->password)
											{
												if (pos.x != pos.y)
													pos = _GuiItemEdit_keyRemoveSelect(pos, &text);

												UNI* paste = Win_clipboard_getText();
												text = Std_insertUNI(text, paste, pos.x);
												pos.x += Std_sizeUNI(paste);
												pos.y = pos.x;
												Std_deleteUNI(paste);
											}
										}
										else
											if (keyExtra & Win_EXTRAKEY_CUT)
											{
												if (!self->password)
												{
													//copy
													UNI* str = 0;
													if (pos.x < pos.y) str = Std_newUNI_copy(&text[pos.x], pos.y - pos.x);
													else
														if (pos.y < pos.x) str = Std_newUNI_copy(&text[pos.y], pos.x - pos.y);

													if (str)
														Win_clipboard_setText(str);
													Std_deleteUNI(str);

													//delete
													if (pos.x != pos.y)
														pos = _GuiItemEdit_keyRemoveSelect(pos, &text);
													else
														if (pos.x < MX)
															text = Std_removeChars(text, pos.x, 1), pos.y = pos.x;
												}
											}
											else
												if (key != 0 && (keyExtra == Win_EXTRAKEY_NONE || keyExtra & Win_EXTRAKEY_SHIFT))
												{
													if (pos.x != pos.y)
														pos = _GuiItemEdit_keyRemoveSelect(pos, &text);

													text = Std_insertChar(text, key, pos.x);
													pos.x++;
													pos.y++;
												}

	OsWinIO_setCursorRenderItemPosX(pos.x);
	OsWinIO_setCursorRenderItemPosY(pos.y);

	OsWinIO_setCursorRenderItemCache(text);

	//save edited value, If user hit save
	if (keyExtra & Win_EXTRAKEY_SAVE)
		GuiItemEdit_saveCache();

	if (!Vec2i_cmp(backupSelect, OsWinIO_getCursorRenderItemPos()))
		GuiItem_setRedraw(&self->base, TRUE);

	if (self->clickChanged)
	{
		//GuiItemEdit_setText(self, OsWinIO_getCursorRenderItemCache()); // update self->text.result
		self->clickChanged(&self->base);
	}
}

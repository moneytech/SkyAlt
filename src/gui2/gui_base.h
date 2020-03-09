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


typedef struct GuiBase_s
{
	Quad2i grid;

	Quad2i coordScreen;		//rect on screen
	Quad2i coordMove;		//scroll
	Quad2i coordMoveCut;	//scroll but cut with parent

	Rgba back_cd;
	Rgba front_cd;

	BOOL touch;
	BOOL touchRecommend;

	BOOL enableThis;
	BOOL enableFinal;

	BOOL shortOutsideEdit;
	UBIG shortKey_extra;
	UNI shortKey_id;
} GuiBase;

GuiBase GuiBase_init(Quad2i grid)
{
	GuiBase self;

	self.grid = grid;
	self.coordScreen = Quad2i_init();
	self.coordMove = Quad2i_init();
	self.coordMoveCut = Quad2i_init();

	self.back_cd = Rgba_initBlack();
	self.front_cd = Rgba_initBlack();

	self.touch = TRUE;
	self.touchRecommend = TRUE;

	self.enableFinal = TRUE;
	self.enableThis = TRUE;

	self.shortOutsideEdit = FALSE;
	self.shortKey_extra = 0;
	self.shortKey_id = 0;

	return self;
}



void GuiBase_setGrid(GuiBase* self, Quad2i grid)
{
	GuiBase_setResize(self, !Quad2i_cmp(grid, self->grid));
	self->grid = grid;
}
void GuiBase_setCoord(GuiBase* self, Quad2i coord)
{
	GuiBase_setResize(self, !Quad2i_cmp(coord, self->coordScreen));
	self->coordScreen = coord;
	self->coordMove = coord;
	self->coordMoveCut = coord;
}

Quad2i GuiBase_getCoordSreen(const GuiBase* self)
{
	return self->coordScreen;
}

Quad2i GuiBase_getCoordMove(const GuiBase* self)
{
	return self->coordMove;
}

void GuiBase_initCopy(GuiBase* self, const GuiBase* src, BOOL copySub)
{
	*self = *src;

	//subs
	self->subs = StdArr_init();
	if (copySub)
	{
		BIG i;
		for (i = 0; i < src->subs.num; i++)
			_GuiBase_addSub(self, GuiBase_newCopy(src->subs.ptrs[i], copySub));
	}

	self->name = Std_newCHAR(src->name);

	if (self->icon)	self->icon = GuiImage_newCopy(src->icon);

	self->dropMoveNameSrc = Std_newCHAR(src->dropMoveNameSrc);
	self->dropMoveNameDst = Std_newCHAR(src->dropMoveNameDst);
	self->dropInName = Std_newCHAR(src->dropInName);

	self->changeSizeValue = DbValue_initCopy(&src->changeSizeValue);
	self->dropMove = DbRows_initCopy(&src->dropMove);
	self->dropMoveIn = DbRows_initCopy(&src->dropMoveIn);

	self->attrsNames = StdArr_initCopyFn(&src->attrsNames, (StdArrCOPY)&Std_newCHAR);
	self->attrsValues = StdBigs_initCopy(&src->attrsValues);
}

void GuiBase_freeSubs(GuiBase* self)
{
	StdArr_freeFn(&self->subs, (StdArrFREE)&GuiBase_delete);
	GuiBase_setResize(self, TRUE);
}

void GuiBase_free(GuiBase* self)
{
	Std_deleteCHAR(self->name);

	if (self->icon)	GuiImage_delete(self->icon);

	DbValue_free(&self->changeSizeValue);
	DbRows_free(&self->dropMove);
	DbRows_free(&self->dropMoveIn);

	Std_deleteCHAR(self->dropMoveNameSrc);
	Std_deleteCHAR(self->dropMoveNameDst);
	Std_deleteCHAR(self->dropInName);

	StdArr_freeFn(&self->attrsNames, (StdArrFREE)&Std_deleteCHAR);
	StdBigs_free(&self->attrsValues);

	GuiBase_freeSubs(self);
	Os_memset(self, sizeof(GuiBase));
}

void GuiBase_setIcon(GuiBase* self, GuiImage* icon)
{
	if (self->icon)
		GuiImage_delete(self->icon);
	self->icon = icon;
}

int GuiBase_getIconSizeX(void)
{
	const int cell = OsWinIO_cellSize();
	return cell;
}

Quad2i GuiBase_getIconCoordBack(Quad2i coord)
{
	Quad2i q = coord;
	q.size.x = GuiBase_getIconSizeX();
	return q;
}

Quad2i GuiBase_getIconCoord(Quad2i* coord)
{
	Quad2i origCoord = *coord;

	const int cell = OsWinIO_cellSize();

	Quad2i q = *coord;
	q.size.x = cell;
	q.size.y = cell;
	q = Quad2i_addSpace(q, cell / 4);

	coord->start.x += q.size.x * 1.5;
	coord->size.x -= q.size.x * 1.5;

	return Quad2i_getIntersect(q, origCoord);
}

Quad2i GuiBase_getSubMaxGrid(GuiBase* self)
{
	Quad2i mx = Quad2i_init();

	const UBIG N_SUBS = GuiBase_numSub(self);
	if (N_SUBS)
	{
		mx = GuiBase_getSub(self, 0)->grid;
		UBIG i;
		for (i = 0; i < N_SUBS; i++)
			mx = Quad2i_extend(mx, GuiBase_getSub(self, i)->grid);
	}
	return mx;
}

Quad2i GuiBase_getSubMaxCoord(GuiBase* self)
{
	Quad2i mx = Quad2i_init();

	const UBIG N_SUBS = GuiBase_numSub(self);
	if (N_SUBS)
	{
		mx = GuiBase_getSub(self, 0)->coordScreen;
		UBIG i;
		for (i = 0; i < N_SUBS; i++)
			mx = Quad2i_extend(mx, GuiBase_getSub(self, i)->coordScreen);
	}
	return mx;
}

GuiBase* GuiBase_findChildCursor(GuiBase* self)
{
	if (OsWinIO_isCursorGuiBase(self))
		return self;

	BIG i;
	for (i = 0; i < self->subs.num; i++)
	{
		GuiBase* ret = GuiBase_findChildCursor(self->subs.ptrs[i]);
		if (ret)
			return ret;
	}
	return 0;
}

void GuiBase_setCallClick(GuiBase* self, GuiBaseCallback* click)
{
	self->click = click;
}

void GuiBase_setShortcutKey(GuiBase* self, BOOL shortOutsideEdit, UBIG shortKey_extra, UNI shortKey_id, GuiBaseCallback* key)
{
	self->shortOutsideEdit = shortOutsideEdit;
	self->shortKey_extra = shortKey_extra;
	self->shortKey_id = shortKey_id;
	self->key = key;
}

BOOL GuiBase_shortcut(GuiBase* self)
{
	if (!self->show || self->remove)
		return FALSE;

	if (self->shortKey_extra == OsWinIO_getKeyExtra() && (self->shortKey_id == 0 || self->shortKey_id == OsWinIO_getKeyID()) && (!self->shortOutsideEdit || OsWinIO_isCursorEmpty()))
	{
		GuiBase_callKey(self);
		return TRUE;
	}

	int i;
	for (i = 0; i < self->subs.num; i++)
		if (GuiBase_shortcut(self->subs.ptrs[i]))
			return TRUE;
	return FALSE;
}

int GuiBase_maxSubX(GuiBase* self, Win* win)
{
	int cell = OsWinIO_cellSize();
	int maxX = 0;
	BIG i;

	for (i = 0; i < self->subs.num; i++)
	{
		GuiBase* it = self->subs.ptrs[i];
		maxX = Std_bmax(maxX, Quad2i_end(it->grid).x * cell);
	}
	return maxX;
}

int GuiBase_maxSubY(GuiBase* self, Win* win)
{
	int cell = OsWinIO_cellSize();
	int maxY = 0;
	BIG i;

	for (i = 0; i < self->subs.num; i++)
	{
		GuiBase* it = self->subs.ptrs[i];
		maxY = Std_bmax(maxY, Quad2i_end(it->grid).y * cell);
	}
	return maxY;
}

BOOL GuiBase_setShow(GuiBase* self, BOOL show)
{
	GuiBase_setRedraw(self, (show != self->show));
	self->show = show;

	int i;
	for (i = 0; i < self->subs.num; i++)
		GuiBase_setShow(self->subs.ptrs[i], show);
	return show;
}

void GuiBase_setTouch(GuiBase* self, BOOL touch)
{
	self->touch = touch;

	if (self->touch)
		self->touch = self->touchRecommend;

	int i;
	for (i = 0; i < self->subs.num; i++)
		GuiBase_setTouch(self->subs.ptrs[i], touch);
}

void GuiBase_setTouchRecommand(GuiBase* self, BOOL touchRecomend)
{
	self->touchRecommend = touchRecomend;

	int i;
	for (i = 0; i < self->subs.num; i++)
		GuiBase_setTouchRecommand(self->subs.ptrs[i], touchRecomend);
}

void GuiBase_setBorder(GuiBase* self, BOOL on)
{
	self->border = on;

	int i;
	for (i = 0; i < self->subs.num; i++)
		GuiBase_setBorder(self->subs.ptrs[i], on);
}

void GuiBase_setEnableOne(GuiBase* self, BOOL enable)
{
	if (self->enableThis != enable)
		GuiBase_setRedraw(self, TRUE);
	self->enableThis = enable;
}

void GuiBase_setEnable(GuiBase* self, BOOL enable)
{
	GuiBase_setEnableOne(self, enable);

	int i;
	for (i = 0; i < self->subs.num; i++)
		GuiBase_setEnable(self->subs.ptrs[i], enable);
}

BOOL GuiBase_enableEnableAttr(GuiBase* self)
{
	BIG row = GuiBase_findAttribute(self, "row");
	BIG parentRow = DbRoot_findParent(row);
	return DbValue_getOptionNumber(parentRow, "enable", 1) && DbValue_getOptionNumber(row, "enable", 1);	//parent and line
}
BOOL GuiBase_enableEnableParentAttr(GuiBase* self)
{
	BIG row = GuiBase_findAttribute(self, "row");
	row = DbRoot_findParent(row);
	return DbValue_getOptionNumber(row, "enable", 1);	//only parent
}


static void _GuiBase_updateEnable(GuiBase* self)
{
	BOOL enable = self->parent ? self->parent->enableFinal : TRUE;

	BOOL value = self->enableCallback ? self->enableCallback(self) : TRUE;//DbValue_getNumber(&self->enableValue);

	//this
	enable &= (value && self->enableThis);

	//redraw
	if (self->enableFinal != enable)
		GuiBase_setRedraw(self, TRUE);

	//set
	self->enableFinal = enable;
}

void GuiBase_setEnableCallback(GuiBase* self, GuiBaseCallbackEnable* enableCallback)
{
	self->enableCallback = enableCallback;
}

BOOL GuiBase_isEnable(GuiBase* self)
{
	return self->enableFinal && self->enableThis;
}

void GuiBase_showSub(GuiBase* self, BOOL show)
{
	int i;
	for (i = 0; i < self->subs.num; i++)
	{
		GuiBase* sub = self->subs.ptrs[i];

		sub->redraw = (sub->show != show);
		sub->show = show;

		GuiBase_showSub(sub, show);
	}
}

BOOL GuiBase_replaceSub(GuiBase* self, GuiBase* old, GuiBase* newone)
{
	int i;
	for (i = 0; i < self->subs.num; i++)
	{
		if (self->subs.ptrs[i] == old)
		{
			GuiBase_delete(old);
			self->subs.ptrs[i] = newone;

			GuiBase_setResize(self, TRUE);
			return TRUE;
		}

		if (GuiBase_replaceSub(self->subs.ptrs[i], old, newone))
			return TRUE;
	}

	return FALSE;
}

BOOL GuiBase_findItem(GuiBase* self, GuiBase* item)
{
	if (self == item)
		return TRUE;

	int i;
	for (i = 0; i < self->subs.num; i++)
		if (GuiBase_findItem(GuiBase_getSub(self, i), item))
			return TRUE;
	return FALSE;
}

GuiBase* GuiBase_findSubTypeEnable(GuiBase* self, GuiBaseTYPE type)
{
	int i;
	for (i = 0; i < self->subs.num; i++)
	{
		GuiBase* it = GuiBase_getSub(self, i);
		if (it->type == type && GuiBase_isEnable(it))
			return it;

		it = GuiBase_findSubTypeEnable(it, type);
		if (it)
			return it;
	}
	return 0;
}

BOOL GuiBase_isColEmpty(GuiBase* self, int col)
{
	int i;
	for (i = 0; i < self->subs.num; i++)
	{
		GuiBase* it = GuiBase_getSub(self, i);
		if (it->grid.start.x <= col && it->grid.start.x + it->grid.size.x > col)
			return FALSE;
	}
	return TRUE;
}

BOOL GuiBase_isRowEmpty(GuiBase* self, int row)
{
	int i;
	for (i = 0; i < self->subs.num; i++)
	{
		GuiBase* it = GuiBase_getSub(self, i);
		if (it->grid.start.y <= row && it->grid.start.y + it->grid.size.y > row)
			return FALSE;
	}
	return TRUE;
}

GuiBase* GuiBase_lastSub(GuiBase* self)
{
	return self->subs.num ? self->subs.ptrs[self->subs.num - 1] : 0;
}

static void _GuiBase_updateFinalCd(GuiBase* self, Rgba back_cd, Rgba front_cd, Quad2i coord, Win* win)
{
	if (!GuiBase_isEnable(self))
	{
		back_cd = Rgba_aprox(back_cd, g_theme.background, 0.7f);
		front_cd = Rgba_aprox(front_cd, g_theme.background, 0.7f);
	}

	GuiBase_setRedraw(self, (!Rgba_cmp(self->back_cd, back_cd) || !Rgba_cmp(self->front_cd, front_cd)));

	self->back_cd = back_cd;
	self->front_cd = front_cd;
}

static int _GuiBase_textSize(int level, int sizeY)
{
	int cell = OsWinIO_cellSize();
	int y;
	if (level == 1)
		y = cell / 2.4f;
	else
		y = sizeY / 2.4f;

	return y;
}

void GuiBase_print(GuiBase* self, UINT depth)
{
	int i;
	for (i = 0; i < depth; i++)
		printf("\t");

	switch (self->type)
	{
		case GuiBase_BOX: printf("Box");
			break;
		case GuiBase_TEXT: printf("Text");
			break;
		case GuiBase_TEXT_MULTI:printf("TextMulti");
			break;
		case GuiBase_BUTTON: printf("Button");
			break;
		case GuiBase_EDIT: printf("Edit");
			break;
		case GuiBase_CHECK: printf("Check");
			break;
		case GuiBase_SLIDER: printf("Slider");
			break;
		case GuiBase_RATING: printf("Rating");
			break;

		case GuiBase_COMBO_STATIC: printf("ComboStatic");
			break;
		case GuiBase_COMBO_DYNAMIC: printf("ComboDynamic");
			break;
		case GuiBase_MENU: printf("Menu");
			break;
		case GuiBase_TABLE: printf("Table");
			break;
		case GuiBase_GROUP: printf("Group");
			break;
		case GuiBase_LAYOUT: printf("Layout");
			break;
		case GuiBase_DESIGN: printf("Design");
			break;
		case GuiBase_LIST: printf("Tree");
			break;
		case GuiBase_TODO: printf("Todo");
			break;
		case GuiBase_PARTICLES: printf("Particles");
			break;
		case GuiBase_FILE: printf("File");
			break;
		case GuiBase_SWITCH: printf("Switch");
			break;
		case GuiBase_MAP: printf("Map");
			break;
		case GuiBase_COLOR: printf("Color");
			break;
		case GuiBase_LEVEL: printf("--Level--");
			break;
		case GuiBase_TAGS: printf("Tags");
			break;
		case GuiBase_CALENDAR_SMALL: printf("Calendar");
			break;
		case GuiBase_CALENDAR_BIG: printf("CalendarBig");
			break;
		case GuiBase_TIMELINE: printf("Timeline");
			break;
		case GuiBase_KANBAN: printf("KanbanF");
			break;
		case GuiBase_CHART: printf("Chart");
			break;
	}

	printf("(%s) :", self->name);
	printf(" grid(%d %d %d %d)", self->grid.start.x, self->grid.start.y, self->grid.size.x, self->grid.size.y);
	printf(" coordScreen(%d %d %d %d)", self->coordScreen.start.x, self->coordScreen.start.y, self->coordScreen.size.x, self->coordScreen.size.y);
	printf(" coordMove(%d %d %d %d)", self->coordMove.start.x, self->coordMove.start.y, self->coordMove.size.x, self->coordMove.size.y);
	printf("\n");

	for (i = 0; i < self->subs.num; i++)
		GuiBase_print(self->subs.ptrs[i], depth + 1);
}

void GuiBase_clickUnderline(GuiBase* self, DbValue* text, BOOL formatURL, BOOL formatEmail)
{
	DbFormatTYPE type = DbValue_getFormat(text);
	const UNI* result = DbValue_result(text);

	if (formatURL || type == DbFormat_URL)
	{
		char* str = Std_newCHAR_uni(result);
		OsWeb_openWebBrowser(str);
		Std_deleteCHAR(str);
	}
	else
		if (formatEmail || type == DbFormat_EMAIL)
		{
			char* str = Std_newCHAR_uni(result);
			OsWeb_openEmail(str, 0);
			Std_deleteCHAR(str);
		}
		else
			if (type == DbFormat_LOCATION)
			{
				GuiBaseLayout* layout = GuiBaseLayout_new(Quad2i_init4(0, 0, 1, 1));
				GuiBaseLayout_addColumn(layout, 0, 20);
				GuiBaseLayout_addRow(layout, 1, 20);

				GuiBase_addSubName((GuiBase*)layout, "title", GuiBaseText_new(Quad2i_init4(0, 0, 1, 1), TRUE, DbValue_initCopy(text), DbValue_initEmpty()));

				DbValue search = DbValue_initCopy(text);
				search.formated = FALSE;
				GuiBaseMap* map = (GuiBaseMap*)GuiBase_addSubName((GuiBase*)layout, "map", GuiBaseMap_new(Quad2i_init4(0, 1, 1, 1), -1, DbRows_initEmpty(), DbValue_initEmpty(), DbValue_initEmpty(), DbValue_initEmpty(), search));
				GuiBaseMap_focusSearch(map);

				GuiBaseRoot_addDialogRel((GuiBase*)layout, self, self->coordMove, TRUE);
			}
			else
				if (type == DbFormat_PHONE)
				{
					//...
				}
}

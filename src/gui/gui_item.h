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

typedef enum
{
	GuiItem_BOX,
	GuiItem_TEXT,
	GuiItem_TEXT_MULTI,
	GuiItem_BUTTON,
	GuiItem_EDIT,

	GuiItem_CHECK,
	GuiItem_SLIDER,
	GuiItem_RATING,
	GuiItem_COMBO_STATIC,
	GuiItem_COMBO_DYNAMIC,
	GuiItem_MENU,

	GuiItem_TAGS,

	GuiItem_LIST,
	GuiItem_TODO,
	GuiItem_TABLE,
	GuiItem_GROUP,
	GuiItem_CALENDAR_SMALL,
	GuiItem_CALENDAR_BIG,
	GuiItem_TIMELINE,
	GuiItem_KANBAN,
	GuiItem_CHART,

	GuiItem_LAYOUT,
	GuiItem_DESIGN,

	GuiItem_PARTICLES,

	GuiItem_FILE,

	GuiItem_SWITCH,

	GuiItem_MAP,
	GuiItem_COLOR,

	GuiItem_LEVEL,
} GuiItemTYPE;

typedef struct GuiItem_s
{
	GuiItem* parent;

	GuiItemTYPE type;
	Quad2i grid;

	Quad2i coordScreen;		//rect on screen
	Quad2i coordMove;		//scroll
	Quad2i coordMoveCut;	//scroll but cut with parent

	Rgba back_cd;
	Rgba front_cd;

	StdArr subs;	//maybe only for Layout ...

	BOOL show;

	BOOL touch;
	BOOL touchRecommend;

	GuiItemCallbackEnable* enableCallback;
	BOOL enableThis;
	BOOL enableFinal;

	BOOL redraw;

	BOOL border;
	BOOL resize;
	BOOL remove;

	BOOL changeRow;

	BOOL shortOutsideEdit;
	UBIG shortKey_extra;
	UNI shortKey_id;

	GuiItemCallback* click;
	GuiItemCallback* key;	//shortcut

	char* name;

	GuiImage* icon;
	BOOL icon_draw_back;
	BOOL icon_alternativeColor;

	BOOL changeSizeVertical;	//always right or bottom border(edge)
	DbValue changeSizeValue;
	BOOL changeSizeMoveOut;

	BOOL dropDontRemove;
	BOOL dropVertival;
	char* dropMoveNameSrc;
	char* dropMoveNameDst;
	DbRows dropMove;
	DbRows dropMoveIn;
	char* dropInName;

	GuiItemCallbackMove* dropCallback;

	BOOL changeSizeActive;

	StdArr attrsNames;	//<char*>
	StdBigs attrsValues;

	BOOL drawTable;

	GuiItemCallback* loopTouch;

	GuiItemCallback* iconDoubleTouch;

	GuiItemCallback* iconCallback;
} GuiItem;

GuiItem GuiItem_init(GuiItemTYPE type, Quad2i grid)
{
	GuiItem self;

	self.parent = 0;
	self.type = type;
	self.grid = grid;
	self.coordScreen = Quad2i_init();
	self.coordMove = Quad2i_init();
	self.coordMoveCut = Quad2i_init();

	self.back_cd = Rgba_initBlack();
	self.front_cd = Rgba_initBlack();

	self.subs = StdArr_init();

	self.show = TRUE;
	self.touch = TRUE;
	self.touchRecommend = TRUE;
	self.enableCallback = 0;
	self.enableFinal = TRUE;
	self.enableThis = TRUE;

	self.redraw = TRUE;

	self.changeRow = TRUE;

	self.shortOutsideEdit = FALSE;
	self.shortKey_extra = 0;
	self.shortKey_id = 0;

	self.key = 0;
	self.click = 0;

	self.border = FALSE;

	self.name = 0;

	self.resize = TRUE;

	self.remove = FALSE;

	self.icon = 0;
	self.icon_draw_back = TRUE;
	self.icon_alternativeColor = FALSE;

	self.changeSizeVertical = FALSE;
	self.changeSizeValue = DbValue_initEmpty();
	self.changeSizeMoveOut = FALSE;

	self.dropDontRemove = FALSE;
	self.dropVertival = FALSE;
	self.dropMoveNameSrc = 0;
	self.dropMoveNameDst = 0;
	self.dropMove = DbRows_initEmpty();
	self.dropMoveIn = DbRows_initEmpty();
	self.dropInName = 0;
	self.dropCallback = 0;

	self.drawTable = FALSE;

	self.changeSizeActive = FALSE;

	self.attrsNames = StdArr_init();
	self.attrsValues = StdBigs_init();

	self.loopTouch = 0;
	self.iconDoubleTouch = 0;
	self.iconCallback = 0;
	return self;
}

GuiItem* GuiItem_getParent(const GuiItem* self)
{
	return self->parent;
}

void GuiItem_setLoopTouch(GuiItem* self, GuiItemCallback* loopTouch)
{
	self->loopTouch = loopTouch;
}

void GuiItem_setIconDoubleTouchCall(GuiItem* self, GuiItemCallback* iconDoubleTouch)
{
	self->iconDoubleTouch = iconDoubleTouch;
}
void GuiItem_setIconCallback(GuiItem* self, GuiItemCallback* iconCallback)
{
	self->iconCallback = iconCallback;
}

void GuiItem_setAlternativeIconCd(GuiItem* self, BOOL alternativeColor)
{
	self->icon_alternativeColor = alternativeColor;
}

GuiItem* GuiItem_getBaseParent(GuiItem* self)
{
	while (self->parent)
		self = self->parent;
	return self;
}

void* GuiItem_findParentType(GuiItem* self, GuiItemTYPE type)
{
	if (!self)
		return 0;

	if (self->type == type)
		return self;

	if (self->parent)
	{
		GuiItem* it = GuiItem_findParentType(self->parent, type);
		if (it)
			return it;
	}

	if (self->type == GuiItem_LEVEL)
		return GuiItem_findParentType(GuiItemLevel_getBackChain((GuiItemLevel*)self), type);

	return 0;
}

GuiItemList* GuiItem_findParentTypeLIST(GuiItem* self)
{
	return GuiItem_findParentType(self, GuiItem_LIST);
}

BIG _GuiItem_findAttributeName(GuiItem* self, const char* name)
{
	BIG i;
	for (i = 0; i < self->attrsNames.num; i++)
	{
		if (Std_cmpCHAR(self->attrsNames.ptrs[i], name))
			return i;
	}

	return -1;
}

void GuiItem_setAttribute(GuiItem* self, const char* name, BIG value)
{
	BIG i = _GuiItem_findAttributeName(self, name);
	if (i >= 0)
	{
		self->attrsValues.ptrs[i] = value;
	}
	else
	{
		StdArr_add(&self->attrsNames, Std_newCHAR(name));
		StdBigs_add(&self->attrsValues, value);
	}
}

BOOL _GuiItem_findAttributeSub(GuiItem* self, const char* name, BIG* out_value)
{
	BIG i = _GuiItem_findAttributeName(self, name);
	if (i >= 0)
	{
		*out_value = self->attrsValues.ptrs[i];
		return TRUE;
	}

	if (self->type == GuiItem_LEVEL)
	{
		GuiItem* parent = GuiItemLevel_getBackChain((GuiItemLevel*)self);
		if (parent)
			return _GuiItem_findAttributeSub(parent, name, out_value);
	}

	if (self->parent)
	{
		return _GuiItem_findAttributeSub(self->parent, name, out_value);
	}

	return FALSE;
}
BIG GuiItem_findAttribute(GuiItem* self, const char* name)
{
	BIG v = -1;

	if (!_GuiItem_findAttributeSub(self, name, &v))
		printf("Warning: Attribute(%s) not found\n", name);

	return v;
}

Image1 GuiItem_getColumnIcon(DbFormatTYPE format)
{
	switch (format)
	{
		case DbFormat_NUMBER_1:
		case DbFormat_NUMBER_N:
		return UiIcons_init_column_number();

		case DbFormat_CURRENCY:	return UiIcons_init_column_currency();
		case DbFormat_PERCENTAGE:	return UiIcons_init_column_percentage();
		case DbFormat_RATING:	return UiIcons_init_column_rating();
		case DbFormat_SLIDER:	return UiIcons_init_column_slider();
		case DbFormat_CHECK:	return UiIcons_init_column_check();
		case DbFormat_MENU:	return UiIcons_init_column_menu();
		case DbFormat_TAGS:	return UiIcons_init_column_tags();

		case DbFormat_LINK_1:	return UiIcons_init_column_link();
		case DbFormat_LINK_N:	return UiIcons_init_column_link();
		case DbFormat_LINK_MIRRORED:	return UiIcons_init_column_link();
		case DbFormat_LINK_JOINTED:	return UiIcons_init_column_link();
		case DbFormat_LINK_FILTERED:	return UiIcons_init_column_link();

		case DbFormat_FILE_1:	return UiIcons_init_column_file();
		case DbFormat_FILE_N:	return UiIcons_init_column_file();

		case DbFormat_TEXT:	return UiIcons_init_column_text();
		case DbFormat_PHONE: return UiIcons_init_column_phone();
		case DbFormat_URL: return UiIcons_init_column_url();
		case DbFormat_EMAIL: return UiIcons_init_column_email();
		case DbFormat_LOCATION: return UiIcons_init_column_location();

		case DbFormat_DATE: return UiIcons_init_column_date();

		case DbFormat_SUMMARY:	return UiIcons_init_column_insight();

		case DbFormat_ROW: return UiIcons_init_unknown();
	}

	return UiIcons_init_unknown();
}

void GuiItem_remove(GuiItem* self)
{
	if (self)
	{
		if (self->parent)
			StdArr_removeFind(&self->parent->subs, self);
		GuiItem_delete(self);
	}
}

void GuiItem_removeSubs(GuiItem* self)
{
	const UBIG N_SUBS = GuiItem_numSub(self);
	UBIG i;
	for (i = 0; i < N_SUBS; i++)
	{
		GuiItem* it = GuiItem_getSub(self, i);
		if (it)
		{
			it->remove = TRUE;
			GuiItem_removeSubs(it);
		}
	}
}

void* GuiItem_findChildType(GuiItem* self, GuiItemTYPE type)
{
	int i;
	for (i = 0; i < self->subs.num; i++)
	{
		GuiItem* it = self->subs.ptrs[i];
		if (it->type == type)
			return it;

		it = GuiItem_findChildType(it, type);
		if (it)
			return it;
	}

	return 0;
}

void GuiItem_setName(GuiItem* self, const char* name)
{
	Std_deleteCHAR(self->name);
	self->name = Std_newCHAR(name);
}

static void* _GuiItem_findNameInner(GuiItem* self, GuiItem* caller, const char* name, int type)
{
	if (!self->remove && (!name || Std_cmpCHAR(self->name, name)) && (type < 0 || self->type == type))
		return self;

	int i;
	for (i = 0; i < self->subs.num; i++)
	{
		if (self->subs.ptrs[i] != caller)
		{
			GuiItem* it = _GuiItem_findNameInner(self->subs.ptrs[i], self, name, type);
			if (it)
				return it;
		}
	}

	if (self->parent && self->parent != caller)
	{
		GuiItem* it = _GuiItem_findNameInner(self->parent, self, name, type);
		if (it)
			return it;
	}
	return 0;
}

void* GuiItem_findName(GuiItem* self, const char* name)
{
	return _GuiItem_findNameInner(self, self, name, -1);
}
void* GuiItem_findNameType(GuiItem* self, const char* name, GuiItemTYPE type)
{
	return _GuiItem_findNameInner(self, self, name, type);
}
void* GuiItem_findType(GuiItem* self, GuiItemTYPE type)
{
	return _GuiItem_findNameInner(self, self, 0, type);
}

void GuiItem_getDepthEx(GuiItem* self, UBIG depth, UBIG* max_depth)
{
	*max_depth = Std_max(*max_depth, depth);

	int i;
	for (i = 0; i < self->subs.num; i++)
		GuiItem_getDepthEx(self->subs.ptrs[i], depth + 1, max_depth);
}
UBIG GuiItem_getDepth(GuiItem* self)
{
	UBIG depth = 0;
	GuiItem_getDepthEx(self, 1, &depth);
	return depth;
}

GuiItem* GuiItem_findSubPos(GuiItem* self, Vec2i touch)
{
	int i;
	for (i = 0; i < self->subs.num; i++)
	{
		GuiItem* it = GuiItem_findSubPos(self->subs.ptrs[i], touch);
		if (it)
			return it;
	}

	if (Quad2i_inside(self->coordMove, touch))
		return self;

	return 0;
}

void GuiItem_setRedraw(GuiItem* self, BOOL redraw)
{
	self->redraw |= redraw;

	if (redraw)
	{
		int i;
		for (i = 0; i < self->subs.num; i++)
			GuiItem_setRedraw(self->subs.ptrs[i], redraw);
	}
}

void GuiItem_setResize(GuiItem* self, BOOL resize)
{
	self->resize |= resize;
	self->redraw |= resize;

	if (resize)
	{
		int i;
		for (i = 0; i < self->subs.num; i++)
			GuiItem_setResize(self->subs.ptrs[i], resize);
	}
}

void GuiItem_setResizeOff(GuiItem* self)
{
	self->resize = FALSE;
	int i;
	for (i = 0; i < self->subs.num; i++)
		GuiItem_setResizeOff(self->subs.ptrs[i]);
}

void GuiItem_setChangeSize(GuiItem* self, UINT changeSizeVertical, DbValue changeSizeValue, BOOL changeSizeMoveOut)
{
	self->changeSizeVertical = changeSizeVertical;
	self->changeSizeMoveOut = changeSizeMoveOut;

	DbValue_free(&self->changeSizeValue);
	self->changeSizeValue = changeSizeValue;
}

int GuiItem_getChangeSizePos(const GuiItem* self, Vec2i touchPos)
{
	float width;
	if (self->changeSizeVertical)
		width = (touchPos.x - self->coordMove.start.x);
	else
		width = (touchPos.y - self->coordMove.start.y);
	width /= OsWinIO_cellSize();

	float t = width - (int)width;
	return Std_max(1, (t < 0.5) ? width - t : width + (1 - t));
}

int GuiItem_getChangeSizeWidth(void)
{
	return Std_max(2, OsWinIO_cellSize() / 8);
}
Quad2i GuiItem_getChangeSizeRect(const GuiItem* self)
{
	const int SPACE = GuiItem_getChangeSizeWidth();
	Quad2i coord = self->coordMove;

	if (self->changeSizeVertical)
		return Quad2i_init4(coord.start.x + coord.size.x - (self->changeSizeMoveOut ? 0 : SPACE), coord.start.y, SPACE * 2, coord.size.y);
	else
		return Quad2i_init4(coord.start.x, coord.start.y + coord.size.y - (self->changeSizeMoveOut ? 0 : SPACE), coord.size.x, SPACE * 2);
}

Quad2i GuiItem_getChangeSizeRectMove(const GuiItem* self, Vec2i touchPos)
{
	int width = GuiItem_getChangeSizePos(self, touchPos);

	Quad2i rect = GuiItem_getChangeSizeRect(self);

	if (self->changeSizeVertical)
		rect.size.x = GuiItem_getChangeSizeWidth();
	else
		rect.size.y = GuiItem_getChangeSizeWidth();

	if (self->changeSizeVertical)
		rect.start.x = self->coordMove.start.x + width * OsWinIO_cellSize();
	else
		rect.start.y = self->coordMove.start.y + width * OsWinIO_cellSize();

	return rect;
}

void GuiItem_setDrop(GuiItem* self, const char* nameSrc, const char* nameDst, BOOL dropVertival, DbRows dropMove, BIG row)
{
	self->dropMoveNameSrc = Std_newCHAR(nameSrc);
	self->dropMoveNameDst = Std_newCHAR(nameDst);
	self->dropVertival = dropVertival;
	self->dropMove = dropMove;

	GuiItem_setAttribute(self, "dropRow", row);
}

void GuiItem_setDropIN(GuiItem* self, const char* name, DbRows dropMoveIn)
{
	self->dropInName = Std_newCHAR(name);
	self->dropMoveIn = dropMoveIn;
}

void GuiItem_setDropCallback(GuiItem* self, GuiItemCallbackMove* dropCallback)
{
	self->dropCallback = dropCallback;
}

BOOL GuiItem_isDropBefore(const GuiItem* self, Vec2i touchPos)
{
	Quad2i coord = self->coordMove;

	if (self->dropVertival)
	{
		int h = coord.size.x / 2;//4;
		int r = (touchPos.x - coord.start.x);
		return (r < h);
	}
	else
	{
		int h = coord.size.y / 2;//4;
		int r = (touchPos.y - coord.start.y);
		return (r < h);
	}
}

Quad2i GuiItem_getDropRectDown(const GuiItem* self)
{
	const int SPACE = OsWinIO_cellSize() / 4;
	Quad2i coord = self->coordMove;

	if (self->dropVertival)
		return Quad2i_init4(coord.start.x + coord.size.x - SPACE, coord.start.y, SPACE, coord.size.y);
	else
		return Quad2i_init4(coord.start.x, coord.start.y + coord.size.y - SPACE, coord.size.x, SPACE);
}
Quad2i GuiItem_getDropRect(const GuiItem* self, Vec2i touchPos, BOOL second)
{
	const int SPACE = OsWinIO_cellSize() / 4;
	Quad2i coord = self->coordMove;

	if (second)
	{
		return coord;
	}
	else
	{
		if (self->dropVertival)
		{
			if (GuiItem_isDropBefore(self, touchPos))
				return Quad2i_init4(coord.start.x, coord.start.y, SPACE, coord.size.y);
		}
		else
		{
			if (GuiItem_isDropBefore(self, touchPos))
				return Quad2i_init4(coord.start.x, coord.start.y, coord.size.x, SPACE);
		}
	}
	return GuiItem_getDropRectDown(self);
}

GuiItem* GuiItem_findDropName(GuiItem* self, const char* name, Vec2i pos, GuiItem* ignore, BOOL* out_in)
{
	int i;
	for (i = 0; i < self->subs.num; i++)
	{
		GuiItem* t = GuiItem_findDropName(self->subs.ptrs[i], name, pos, ignore, out_in);
		if (t)
			return t;
	}

	*out_in = Std_cmpCHAR(self->dropInName, name);
	BOOL beforeAfter = Std_cmpCHAR(self->dropMoveNameDst, name);

	BOOL testCoord = Quad2i_inside(self->coordMove, pos);
	if (*out_in && beforeAfter)	//both => IN is small rect inside
		*out_in = Quad2i_inside(Quad2i_addSpaceY(self->coordMove, OsWinIO_lineSpace()), pos);

	return (self != ignore && DbRows_is(&self->dropMove) && self->show && testCoord && ((*out_in) || beforeAfter)) ? self : 0;
}

UBIG GuiItem_numSub(const GuiItem* self)
{
	return self->subs.num;
}

GuiItem* GuiItem_getSub(GuiItem* self, UBIG i)
{
	return i < GuiItem_numSub(self) ? self->subs.ptrs[i] : 0;
}

const GuiItem* GuiItem_getSubConst(const GuiItem* self, UBIG i)
{
	return i < GuiItem_numSub(self) ? self->subs.ptrs[i] : 0;
}

void* _GuiItem_addSub(GuiItem* self, GuiItem* sub)
{
	if (sub)
	{
		StdArr_add(&self->subs, sub);

		sub->show = self->show;
		sub->touch = self->touch;
		sub->redraw = self->redraw;

		sub->parent = self;

		GuiItem_setResize(self, TRUE);
	}
	return sub;
}

void* GuiItem_addSubName(GuiItem* self, const char* name, GuiItem* sub)
{
	GuiItem_setName(sub, name);
	_GuiItem_addSub(self, sub);
	return sub;
}

void GuiItem_createBackChain(GuiItem* self, StdArr* out)
{
	if (!self)
		return;

	GuiItem_createBackChain(self->parent, out);
	StdArr_add(out, Std_newCHAR(self->name));
}

GuiItem* GuiItem_findPath(GuiItem* self, const StdArr* origPath, BIG pos)
{
	if (Std_cmpCHAR(self->name, origPath->ptrs[pos]))
	{
		if (pos == origPath->num - 1)
			return self;

		int i;
		for (i = 0; i < self->subs.num; i++)
		{
			GuiItem* it = GuiItem_findPath(GuiItem_getSub(self, i), origPath, pos + 1);
			if (it)
				return it;
		}
	}
	return 0;
}

void GuiItem_removeSub(GuiItem* self, UBIG i)
{
	if (i < GuiItem_numSub(self))
	{
		GuiItem_delete(GuiItem_getSub(self, i));
		StdArr_remove(&self->subs, i);

		GuiItem_setResize(self, TRUE);
	}
}
void GuiItem_removeSubPtr(GuiItem* self, GuiItem* ptr)
{
	if (ptr)
	{
		GuiItem_delete(ptr);
		StdArr_removeFind(&self->subs, ptr);
		GuiItem_setResize(self, TRUE);
	}
}

int GuiItem_getParentPos(const GuiItem* self, const GuiItem* item)
{
	if (!self)
		return -1;

	return StdArr_find(&self->subs, item);
}

void* GuiItem_findSubType(GuiItem* self, GuiItemTYPE type)
{
	if (!self)
		return 0;

	int i;
	for (i = 0; i < self->subs.num; i++)
	{
		GuiItem* it = self->subs.ptrs[i];
		if (it->type == type)
			return it;

		it = GuiItem_findSubType(it, type);
		if (it)
			return it;
	}

	return 0;
}

void GuiItem_callClick(GuiItem* self)
{
	if (self->click)
		self->click(self);
}
void GuiItem_callKey(GuiItem* self)
{
	if (self->key)
		self->key(self);
}

BIG GuiItem_findSub(GuiItem* self, Vec2i gridPos)
{
	BIG i;
	for (i = 0; i < self->subs.num; i++)
	{
		GuiItem* it = self->subs.ptrs[i];

		if (Quad2i_inside(it->grid, gridPos))
			return i;
	}
	return -1;
}

BIG GuiItem_findGridCover(GuiItem* self, Quad2i grid, int exclude)
{
	BIG i;
	for (i = 0; i < self->subs.num; i++)
	{
		GuiItem* it = self->subs.ptrs[i];

		if (i != exclude && Quad2i_hasCoverSoft(it->grid, grid))
			return i;
	}
	return -1;
}

void GuiItem_setGrid(GuiItem* self, Quad2i grid)
{
	GuiItem_setResize(self, !Quad2i_cmp(grid, self->grid));
	self->grid = grid;
}
void GuiItem_setCoord(GuiItem* self, Quad2i coord)
{
	GuiItem_setResize(self, !Quad2i_cmp(coord, self->coordScreen));
	self->coordScreen = coord;
	self->coordMove = coord;
	self->coordMoveCut = coord;
}

Quad2i GuiItem_getCoordSreen(const GuiItem* self)
{
	return self->coordScreen;
}

Quad2i GuiItem_getCoordMove(const GuiItem* self)
{
	return self->coordMove;
}

void GuiItem_initCopy(GuiItem* self, const GuiItem* src, BOOL copySub)
{
	*self = *src;

	//subs
	self->subs = StdArr_init();
	if (copySub)
	{
		BIG i;
		for (i = 0; i < src->subs.num; i++)
			_GuiItem_addSub(self, GuiItem_newCopy(src->subs.ptrs[i], copySub));
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

void GuiItem_freeSubs(GuiItem* self)
{
	StdArr_freeFn(&self->subs, (StdArrFREE)&GuiItem_delete);
	GuiItem_setResize(self, TRUE);
}

void GuiItem_free(GuiItem* self)
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

	GuiItem_freeSubs(self);
	Os_memset(self, sizeof(GuiItem));
}

void GuiItem_setIcon(GuiItem* self, GuiImage* icon)
{
	if (self->icon)
		GuiImage_delete(self->icon);
	self->icon = icon;
}

int GuiItem_getIconSizeX(void)
{
	const int cell = OsWinIO_cellSize();
	return cell;
}

Quad2i GuiItem_getIconCoordBack(Quad2i coord)
{
	Quad2i q = coord;
	q.size.x = GuiItem_getIconSizeX();
	return q;
}

Quad2i GuiItem_getIconCoord(Quad2i* coord)
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

Quad2i GuiItem_getSubMaxGrid(GuiItem* self)
{
	Quad2i mx = Quad2i_init();

	const UBIG N_SUBS = GuiItem_numSub(self);
	if (N_SUBS)
	{
		mx = GuiItem_getSub(self, 0)->grid;
		UBIG i;
		for (i = 0; i < N_SUBS; i++)
			mx = Quad2i_extend(mx, GuiItem_getSub(self, i)->grid);
	}
	return mx;
}

Quad2i GuiItem_getSubMaxCoord(GuiItem* self)
{
	Quad2i mx = Quad2i_init();

	const UBIG N_SUBS = GuiItem_numSub(self);
	if (N_SUBS)
	{
		mx = GuiItem_getSub(self, 0)->coordScreen;
		UBIG i;
		for (i = 0; i < N_SUBS; i++)
			mx = Quad2i_extend(mx, GuiItem_getSub(self, i)->coordScreen);
	}
	return mx;
}

GuiItem* GuiItem_findChildCursor(GuiItem* self)
{
	if (OsWinIO_isCursorGuiItem(self))
		return self;

	BIG i;
	for (i = 0; i < self->subs.num; i++)
	{
		GuiItem* ret = GuiItem_findChildCursor(self->subs.ptrs[i]);
		if (ret)
			return ret;
	}
	return 0;
}

void GuiItem_setCallClick(GuiItem* self, GuiItemCallback* click)
{
	self->click = click;
}

void GuiItem_setShortcutKey(GuiItem* self, BOOL shortOutsideEdit, UBIG shortKey_extra, UNI shortKey_id, GuiItemCallback* key)
{
	self->shortOutsideEdit = shortOutsideEdit;
	self->shortKey_extra = shortKey_extra;
	self->shortKey_id = shortKey_id;
	self->key = key;
}

BOOL GuiItem_shortcut(GuiItem* self)
{
	if (!self->show || self->remove)
		return FALSE;

	if (self->shortKey_extra == OsWinIO_getKeyExtra() && (self->shortKey_id == 0 || self->shortKey_id == OsWinIO_getKeyID()) && (!self->shortOutsideEdit || OsWinIO_isCursorEmpty()))
	{
		GuiItem_callKey(self);
		return TRUE;
	}

	int i;
	for (i = 0; i < self->subs.num; i++)
		if (GuiItem_shortcut(self->subs.ptrs[i]))
			return TRUE;
	return FALSE;
}

int GuiItem_maxSubX(GuiItem* self, Win* win)
{
	int cell = OsWinIO_cellSize();
	int maxX = 0;
	BIG i;

	for (i = 0; i < self->subs.num; i++)
	{
		GuiItem* it = self->subs.ptrs[i];
		maxX = Std_bmax(maxX, Quad2i_end(it->grid).x * cell);
	}
	return maxX;
}

int GuiItem_maxSubY(GuiItem* self, Win* win)
{
	int cell = OsWinIO_cellSize();
	int maxY = 0;
	BIG i;

	for (i = 0; i < self->subs.num; i++)
	{
		GuiItem* it = self->subs.ptrs[i];
		maxY = Std_bmax(maxY, Quad2i_end(it->grid).y * cell);
	}
	return maxY;
}

BOOL GuiItem_setShow(GuiItem* self, BOOL show)
{
	GuiItem_setRedraw(self, (show != self->show));
	self->show = show;

	int i;
	for (i = 0; i < self->subs.num; i++)
		GuiItem_setShow(self->subs.ptrs[i], show);
	return show;
}

void GuiItem_setTouch(GuiItem* self, BOOL touch)
{
	self->touch = touch;

	if (self->touch)
		self->touch = self->touchRecommend;

	int i;
	for (i = 0; i < self->subs.num; i++)
		GuiItem_setTouch(self->subs.ptrs[i], touch);
}

void GuiItem_setTouchRecommand(GuiItem* self, BOOL touchRecomend)
{
	self->touchRecommend = touchRecomend;

	int i;
	for (i = 0; i < self->subs.num; i++)
		GuiItem_setTouchRecommand(self->subs.ptrs[i], touchRecomend);
}

void GuiItem_setBorder(GuiItem* self, BOOL on)
{
	self->border = on;

	int i;
	for (i = 0; i < self->subs.num; i++)
		GuiItem_setBorder(self->subs.ptrs[i], on);
}

void GuiItem_setEnableOne(GuiItem* self, BOOL enable)
{
	if (self->enableThis != enable)
		GuiItem_setRedraw(self, TRUE);
	self->enableThis = enable;
}

void GuiItem_setEnable(GuiItem* self, BOOL enable)
{
	GuiItem_setEnableOne(self, enable);

	int i;
	for (i = 0; i < self->subs.num; i++)
		GuiItem_setEnable(self->subs.ptrs[i], enable);
}

BOOL GuiItem_enableEnableAttr(GuiItem* self)
{
	BIG row = GuiItem_findAttribute(self, "row");
	return DbValue_getOptionNumber(row, "enable", 1);
}

static void _GuiItem_updateEnable(GuiItem* self)
{
	BOOL enable = self->parent ? self->parent->enableFinal : TRUE;

	BOOL value = self->enableCallback ? self->enableCallback(self) : TRUE;//DbValue_getNumber(&self->enableValue);

	//this
	enable &= (value && self->enableThis);

	//redraw
	if (self->enableFinal != enable)
		GuiItem_setRedraw(self, TRUE);

	//set
	self->enableFinal = enable;
}

void GuiItem_setEnableCallback(GuiItem* self, GuiItemCallbackEnable* enableCallback)
{
	self->enableCallback = enableCallback;
}

BOOL GuiItem_isEnable(GuiItem* self)
{
	return self->enableFinal && self->enableThis;
}

void GuiItem_showSub(GuiItem* self, BOOL show)
{
	int i;
	for (i = 0; i < self->subs.num; i++)
	{
		GuiItem* sub = self->subs.ptrs[i];

		sub->redraw = (sub->show != show);
		sub->show = show;

		GuiItem_showSub(sub, show);
	}
}

BOOL GuiItem_replaceSub(GuiItem* self, GuiItem* old, GuiItem* newone)
{
	int i;
	for (i = 0; i < self->subs.num; i++)
	{
		if (self->subs.ptrs[i] == old)
		{
			GuiItem_delete(old);
			self->subs.ptrs[i] = newone;

			GuiItem_setResize(self, TRUE);
			return TRUE;
		}

		if (GuiItem_replaceSub(self->subs.ptrs[i], old, newone))
			return TRUE;
	}

	return FALSE;
}

BOOL GuiItem_findItem(GuiItem* self, GuiItem* item)
{
	if (self == item)
		return TRUE;

	int i;
	for (i = 0; i < self->subs.num; i++)
		if (GuiItem_findItem(GuiItem_getSub(self, i), item))
			return TRUE;
	return FALSE;
}

GuiItem* GuiItem_findSubTypeEnable(GuiItem* self, GuiItemTYPE type)
{
	int i;
	for (i = 0; i < self->subs.num; i++)
	{
		GuiItem* it = GuiItem_getSub(self, i);
		if (it->type == type && GuiItem_isEnable(it))
			return it;

		it = GuiItem_findSubTypeEnable(it, type);
		if (it)
			return it;
	}
	return 0;
}

BOOL GuiItem_isColEmpty(GuiItem* self, int col)
{
	int i;
	for (i = 0; i < self->subs.num; i++)
	{
		GuiItem* it = GuiItem_getSub(self, i);
		if (it->grid.start.x <= col && it->grid.start.x + it->grid.size.x > col)
			return FALSE;
	}
	return TRUE;
}

BOOL GuiItem_isRowEmpty(GuiItem* self, int row)
{
	int i;
	for (i = 0; i < self->subs.num; i++)
	{
		GuiItem* it = GuiItem_getSub(self, i);
		if (it->grid.start.y <= row && it->grid.start.y + it->grid.size.y > row)
			return FALSE;
	}
	return TRUE;
}

GuiItem* GuiItem_lastSub(GuiItem* self)
{
	return self->subs.num ? self->subs.ptrs[self->subs.num - 1] : 0;
}

static void _GuiItem_updateFinalCd(GuiItem* self, Rgba back_cd, Rgba front_cd, Quad2i coord, Win* win)
{
	if (!GuiItem_isEnable(self))
	{
		back_cd = Rgba_aprox(back_cd, g_theme.background, 0.7f);
		front_cd = Rgba_aprox(front_cd, g_theme.background, 0.7f);
	}

	GuiItem_setRedraw(self, (!Rgba_cmp(self->back_cd, back_cd) || !Rgba_cmp(self->front_cd, front_cd)));

	self->back_cd = back_cd;
	self->front_cd = front_cd;
}

static int _GuiItem_textSize(int level, int sizeY)
{
	int cell = OsWinIO_cellSize();
	int y;
	if (level == 1)
		y = cell / 2.4f;
	else
		y = sizeY / 2.4f;

	return y;
}

void GuiItem_print(GuiItem* self, UINT depth)
{
	int i;
	for (i = 0; i < depth; i++)
		printf("\t");

	switch (self->type)
	{
		case GuiItem_BOX: printf("Box");
			break;
		case GuiItem_TEXT: printf("Text");
			break;
		case GuiItem_TEXT_MULTI:printf("TextMulti");
			break;
		case GuiItem_BUTTON: printf("Button");
			break;
		case GuiItem_EDIT: printf("Edit");
			break;
		case GuiItem_CHECK: printf("Check");
			break;
		case GuiItem_SLIDER: printf("Slider");
			break;
		case GuiItem_RATING: printf("Rating");
			break;

		case GuiItem_COMBO_STATIC: printf("ComboStatic");
			break;
		case GuiItem_COMBO_DYNAMIC: printf("ComboDynamic");
			break;
		case GuiItem_MENU: printf("Menu");
			break;
		case GuiItem_TABLE: printf("Table");
			break;
		case GuiItem_GROUP: printf("Group");
			break;
		case GuiItem_LAYOUT: printf("Layout");
			break;
		case GuiItem_DESIGN: printf("Design");
			break;
		case GuiItem_LIST: printf("Tree");
			break;
		case GuiItem_TODO: printf("Todo");
			break;
		case GuiItem_PARTICLES: printf("Particles");
			break;
		case GuiItem_FILE: printf("File");
			break;
		case GuiItem_SWITCH: printf("Switch");
			break;
		case GuiItem_MAP: printf("Map");
			break;
		case GuiItem_COLOR: printf("Color");
			break;
		case GuiItem_LEVEL: printf("--Level--");
			break;
		case GuiItem_TAGS: printf("Tags");
			break;
		case GuiItem_CALENDAR_SMALL: printf("Calendar");
			break;
		case GuiItem_CALENDAR_BIG: printf("CalendarBig");
			break;
		case GuiItem_TIMELINE: printf("Timeline");
			break;
		case GuiItem_KANBAN: printf("KanbanF");
			break;
		case GuiItem_CHART: printf("Chart");
			break;
	}

	printf("(%s) :", self->name);
	printf(" grid(%d %d %d %d)", self->grid.start.x, self->grid.start.y, self->grid.size.x, self->grid.size.y);
	printf(" coordScreen(%d %d %d %d)", self->coordScreen.start.x, self->coordScreen.start.y, self->coordScreen.size.x, self->coordScreen.size.y);
	printf(" coordMove(%d %d %d %d)", self->coordMove.start.x, self->coordMove.start.y, self->coordMove.size.x, self->coordMove.size.y);
	printf("\n");

	for (i = 0; i < self->subs.num; i++)
		GuiItem_print(self->subs.ptrs[i], depth + 1);
}

void GuiItem_clickUnderline(GuiItem* self, DbValue* text, BOOL formatURL, BOOL formatEmail)
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
				GuiItemLayout* layout = GuiItemLayout_new(Quad2i_init4(0, 0, 1, 1));
				GuiItemLayout_addColumn(layout, 0, 20);
				GuiItemLayout_addRow(layout, 1, 20);

				GuiItem_addSubName((GuiItem*)layout, "title", GuiItemText_new(Quad2i_init4(0, 0, 1, 1), TRUE, DbValue_initCopy(text), DbValue_initEmpty()));

				DbValue search = DbValue_initCopy(text);
				search.formated = FALSE;
				GuiItemMap* map = (GuiItemMap*)GuiItem_addSubName((GuiItem*)layout, "map", GuiItemMap_new(Quad2i_init4(0, 1, 1, 1), -1, DbRows_initEmpty(), DbValue_initEmpty(), DbValue_initEmpty(), DbValue_initEmpty(), search));
				GuiItemMap_focusSearch(map);

				GuiItemRoot_addDialogRel((GuiItem*)layout, self, self->coordMove, TRUE);
			}
			else
				if (type == DbFormat_PHONE)
				{
					//...
				}
}

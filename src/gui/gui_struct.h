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
	GuiStruct_SHOW_TABLES = 0,
	GuiStruct_SHOW_COLUMNS,
	GuiStruct_SHOW_PAGES,
	GuiStruct_SHOW_VIEWS,
}GuiStruct_SHOW;

Image1 GuiStruct_getIcon(UBIG row)
{
	//if (DbRoot_isTypeViewReference(row))
	//	row = DbRoot_getOrigReference(row);

	if (DbRoot_isType_folder(row))	return UiIcons_init_folder();
	if (DbRoot_isType_remote(row))	return UiIcons_init_remote();
	if (DbRoot_isType_table(row))	return UiIcons_init_table();

	if (DbRoot_isTypeView_filter(row))	return UiIcons_init_table_filter();
	if (DbRoot_isTypeView_summary(row))	return UiIcons_init_column_insight();
	if (DbRoot_isTypeView_cards(row))	return UiIcons_init_cards();
	if (DbRoot_isTypeView_group(row))	return UiIcons_init_group();
	if (DbRoot_isTypeView_kanban(row))	return UiIcons_init_kanban();
	if (DbRoot_isTypeView_calendar(row))return UiIcons_init_calendar();
	if (DbRoot_isTypeView_timeline(row))return UiIcons_init_timeline();
	if (DbRoot_isTypeView_map(row))		return UiIcons_init_map();
	if (DbRoot_isTypeView_chart(row))	return GuiItemChart_getIcon(row);	//icon from chart type

	return Image1_init();
}

static BOOL _GuiStruct_isEnable(UBIG row, GuiStruct_SHOW showType)
{
	BOOL isTable = DbRoot_isType_table(row);
	BOOL isColumn = DbRoot_isType_column(row);
	BOOL isView = DbRoot_isTypeView(row);
	BOOL isPage = DbRoot_isType_page(row);

	return	(isTable && showType == GuiStruct_SHOW_TABLES) ||
		(isColumn && showType == GuiStruct_SHOW_COLUMNS) ||
		(isView && showType == GuiStruct_SHOW_VIEWS) ||
		(isPage && showType == GuiStruct_SHOW_PAGES);
}

static BOOL _GuiStruct_hasSubs(BIG row)
{
	return DbRoot_isType_folder(row) || DbRoot_isType_remote(row) || DbRoot_isType_table(row) || DbRoot_isTypeView_filter(row) || DbRoot_isTypeView_summary(row);
}

static BOOL _GuiStruct_canGoSubsEx(UBIG row, BIG onlyTableRow, BIG onlyColumnBtableRow, GuiStruct_SHOW showType)
{
	BOOL isRoot = DbRoot_isType_root(row);
	BOOL isFolder = DbRoot_isType_folder(row);
	BOOL isRemote = DbRoot_isType_remote(row);
	BOOL isTable = DbRoot_isType_table(row);
	//BOOL isColumn = DbRoot_isType_column(row);
	BOOL isView = DbRoot_isTypeView(row);
	//BOOL isPage = DbRoot_isType_page(row);

	if (isRoot)
		return TRUE;

	if (isFolder)
		return TRUE;

	if (isRemote)
		return TRUE;

	if (isTable && onlyTableRow >= 0 && onlyTableRow != row)
		return FALSE;

	//if (showType == GuiStruct_SHOW_TABLES)
	//	return FALSE;

	if (showType == GuiStruct_SHOW_COLUMNS)
		return isTable;

	if (showType == GuiStruct_SHOW_VIEWS)
		return isTable || isView;

	if (showType == GuiStruct_SHOW_PAGES)
		return isTable;

	return FALSE;
}

static BOOL _GuiStruct_showButtonSearch(BIG row, const UNI* search)
{
	BOOL is = TRUE;
	if (Std_sizeUNI(search))
	{
		DbValue name = DbValue_initOption(row, "name", 0);
		DbValue_hasChanged(&name);
		is = Std_subUNI_small(DbValue_result(&name), search) >= 0;
		DbValue_free(&name);
	}

	return is;
}
static BOOL _GuiStruct_showButtonSearchSub(const BIG row, const UNI* search)
{
	if (_GuiStruct_showButtonSearch(row, search))
		return TRUE;

	if (_GuiStruct_hasSubs(row))
	{
		BIG it;
		UBIG i = 0;
		while ((it = DbColumnN_jump(DbRoot_subs(), row, &i, 1)) >= 0)
		{
			if (_GuiStruct_showButtonSearchSub(it, search))
				return TRUE;

			i++;
		}
	}
	return FALSE;
}

static BOOL _GuiStruct_showColumnBtable(BIG row, BIG onlyColumnBtableRow)
{
	if (DbRoot_isType_column(row) && onlyColumnBtableRow >= 0)
	{
		DbColumn* column = DbRoot_findColumn(row);
		if (column)
		{
			DbTable* btable = DbColumn_getBTable(column);
			if (btable)
				return DbTable_getRow(btable) == onlyColumnBtableRow;
			else
				return FALSE;
		}
	}

	return TRUE;
}

static void _GuiStruct_buildListItem(GuiItemLayout* parentLayout, Quad2i grid, UBIG row, const UNI* search, BIG highlightRow, BIG ignoreRow, BOOL onlyColumnType_links, GuiStruct_SHOW showType, GuiItemCallback* click)
{
	DbColumn* column = DbRoot_findColumn(row);
	DbTable* btable = column ? DbColumn_getBTable(column) : 0;

	BOOL show = (_GuiStruct_showButtonSearch(row, search) && _GuiStruct_isEnable(row, showType) && row != ignoreRow && (!onlyColumnType_links || btable));

	char nameId[64];
	Std_buildNumber(row, 0, nameId);

	DbValue name = DbValue_initOption(row, "name", 0);
	DbValue_hasChanged(&name);
	GuiItemButton* b = (GuiItemButton*)GuiItem_addSubName((GuiItem*)parentLayout, nameId, GuiItemButton_newAlphaEx(grid, name, click));
	GuiItemButton_setTextCenter(b, FALSE);
	GuiItem_setEnable((GuiItem*)b, show);
	GuiItem_setAttribute((GuiItem*)b, "clickRow", row);
	GuiItemButton_setPressedEx(b, (highlightRow == row), FALSE, FALSE);

	Image1 icon = GuiStruct_getIcon(row);
	if (Vec2i_is(icon.size))
		GuiItem_setIcon((GuiItem*)b, GuiImage_new1(icon));
}

static void _GuiStruct_buildDashboardSub(GuiItemLayout* layoutList, UBIG row, int* y, const UNI* search, BIG highlightRow, BIG onlyTableRow, BIG onlyColumnBtableRow, BOOL onlyColumnType_links, BIG ignoreRow, GuiStruct_SHOW showType, GuiItemCallback* click);

static GuiItemLayout* _GuiStruct_buildDashboard(UBIG row, int* y, const UNI* search, BIG highlightRow, BIG onlyTableRow, BIG onlyColumnBtableRow, BOOL onlyColumnType_links, BIG ignoreRow, GuiStruct_SHOW showType, GuiItemCallback* click)
{
	GuiItemLayout* layout = GuiItemLayout_new(Quad2i_init4(0, *y, 1, 1));
	GuiItemLayout_addColumn(layout, 1, 99);

	_GuiStruct_buildListItem(layout, Quad2i_init4(0, 0, 2, 1), row, search, highlightRow, ignoreRow, onlyColumnType_links, showType, click);

	int yy = 0;
	if (_GuiStruct_canGoSubsEx(row, onlyTableRow, onlyColumnBtableRow, showType))
	{
		BIG row2 = -1;
		if (DbRoot_isType_root(row) || DbRoot_isType_folder(row) || DbRoot_isType_remote(row))
			row2 = row;
		else
			if (DbRoot_isType_table(row) && showType == GuiStruct_SHOW_COLUMNS)	//note: table has both(column & views)
				row2 = DbRoot_findChildType(row, "columns");
			else
				row2 = DbRoot_findChildType(row, "views");

		if (row2 >= 0)
		{
			GuiItemLayout* layoutList = GuiItemLayout_new(Quad2i_init4(1, 1, 3, 1));
			GuiItemLayout_addColumn(layoutList, 0, 99);
			_GuiStruct_buildDashboardSub(layoutList, row2, &yy, search, highlightRow, onlyTableRow, onlyColumnBtableRow, onlyColumnType_links, ignoreRow, showType, click);

			if (yy)
			{
				GuiItem_addSubName((GuiItem*)layout, "layout_list", (GuiItem*)layoutList);
				GuiItemLayout_addRow(layout, 1, yy);
			}
			else
				GuiItem_delete((GuiItem*)layoutList);
		}
	}

	GuiItem_setGrid((GuiItem*)layout, Quad2i_init4(0, *y, 1, 1 + yy));
	*y += 1 + yy;

	return layout;
}

static void _GuiStruct_buildDashboardSub(GuiItemLayout* layoutList, UBIG row, int* y, const UNI* search, BIG highlightRow, BIG onlyTableRow, BIG onlyColumnBtableRow, BOOL onlyColumnType_links, BIG ignoreRow, GuiStruct_SHOW showType, GuiItemCallback* click)
{
	BIG it;
	UBIG i = 0;
	while ((it = DbColumnN_jump(DbRoot_subs(), row, &i, 1)) >= 0)
	{
		char nameId[64];
		Std_buildNumber(it, 0, nameId);

		const BOOL show2 = _GuiStruct_showButtonSearchSub(it, search);
		if (show2)
		{
			if (_GuiStruct_hasSubs(it))
			{
				GuiItem_addSubName((GuiItem*)layoutList, nameId, (GuiItem*)_GuiStruct_buildDashboard(it, y, search, highlightRow, onlyTableRow, onlyColumnBtableRow, onlyColumnType_links, ignoreRow, showType, click));
			}
			else
			{
				if (_GuiStruct_showColumnBtable(it, onlyColumnBtableRow))
				{
					GuiItemLayout* lai = GuiItemLayout_new(Quad2i_init4(0, *y, 1, 1));

					GuiItemLayout_addColumn(lai, 0, 99);
					GuiItem_addSubName((GuiItem*)layoutList, nameId, (GuiItem*)lai);

					_GuiStruct_buildListItem(lai, Quad2i_init4(0, 0, 1, 1), it, search, highlightRow, ignoreRow, onlyColumnType_links, showType, click);

					(*y)++;
				}
			}
		}

		i++;
	}
}

void GuiStruct_rebuildList(GuiItem* item)
{
	GuiItemEdit* searchEd = GuiItem_findName(item, "search");
	GuiItem_setShortcutKey((GuiItem*)searchEd, FALSE, Win_EXTRAKEY_CTRL | Win_EXTRAKEY_SEARCH, 0, &GuiItemEdit_clickActivate);

	GuiItemLayout* list = GuiItem_findName(item, "list");

	GuiStruct_SHOW showType = GuiItem_findAttribute(item, "showType");
	GuiItemCallback* click = (GuiItemCallback*)GuiItem_findAttribute(item, "clickFunc");
	BOOL highlightRow = GuiItem_findAttribute(item, "highlightRow");
	BIG ignoreRow = GuiItem_findAttribute(item, "ignoreRow");
	BIG onlyTableRow = GuiItem_findAttribute(item, "onlyTableRow");
	BIG onlyColumnBtableRow = GuiItem_findAttribute(item, "onlyColumnBtableRow");
	BOOL onlyColumnType_links = GuiItem_findAttribute(item, "onlyColumnType_links");

	if (list && searchEd)
	{
		GuiItem_freeSubs((GuiItem*)list);

		const UNI* search = GuiItemEdit_getText(searchEd);
		int y = 0;
		_GuiStruct_buildDashboardSub(list, DbRoot_getRootRow(), &y, search, highlightRow, onlyTableRow, onlyColumnBtableRow, onlyColumnType_links, ignoreRow, showType, click);
	}
}

void GuiStruct_clickSearchDelete(GuiItem* item)
{
	GuiItemEdit* searchEd = GuiItem_findName(item, "search");
	GuiItemEdit_setText(searchEd, 0);
}

void GuiStruct_clickEditSearch(GuiItem* item)
{
	GuiItem* list = GuiItem_findName(item, "list");
	if (list)
		GuiItem_setResize(list, TRUE);
}

GuiItemLayout* GuiStruct_create(GuiStruct_SHOW showType, BIG highlightRow, DbValue title, GuiItemCallback* click, BIG onlyTableRow, BIG onlyColumnBtableRow, BIG ignoreRow, BOOL onlyColumnType_links)
{
	GuiItemLayout* layout = GuiItemLayout_new(Quad2i_init4(0, 0, 1, 1));
	GuiItemLayout_addColumn(layout, 0, 10);
	GuiItemLayout_addRow(layout, 2, 20);
	layout->drawBorder = TRUE;
	GuiItem_setAttribute(&layout->base, "showType", showType);
	GuiItem_setAttribute(&layout->base, "clickFunc", (BIG)click);
	GuiItem_setAttribute(&layout->base, "highlightRow", highlightRow);
	GuiItem_setAttribute(&layout->base, "ignoreRow", ignoreRow);
	GuiItem_setAttribute(&layout->base, "onlyTableRow", onlyTableRow);
	GuiItem_setAttribute(&layout->base, "onlyColumnBtableRow", onlyColumnBtableRow);
	GuiItem_setAttribute(&layout->base, "onlyColumnType_links", onlyColumnType_links);

	//title
	GuiItem_addSubName((GuiItem*)layout, "search_title", GuiItemText_new(Quad2i_init4(0, 0, 1, 1), TRUE, title, DbValue_initLang("SEARCH")));

	//search
	GuiItemEdit* search = (GuiItemEdit*)GuiItem_addSubName((GuiItem*)layout, "search", GuiItemEdit_new(Quad2i_init4(0, 1, 1, 1), DbValue_initOption(DbRoot_getRootRow(), "search", 0), DbValue_initLang("SEARCH")));
	GuiItemEdit_setHighlightIfContent(search, TRUE);
	GuiItem_setIcon((GuiItem*)search, GuiImage_new1(UiIcons_init_search()));
	GuiItemEdit_setFnChanged(search, &GuiStruct_clickEditSearch);
	GuiItem_addSubName((GuiItem*)layout, "x", GuiItemButton_newAlphaEx(Quad2i_init4(1, 1, 1, 1), DbValue_initStaticCopy(_UNI32("X")), &GuiStruct_clickSearchDelete));

	//list
	GuiItemLayout* list = GuiItemLayout_new(Quad2i_init4(0, 2, 2, 1));
	GuiItemLayout_addColumn(list, 0, 100);
	GuiItemLayout_setResize(list, &GuiStruct_rebuildList);
	GuiItem_addSubName((GuiItem*)layout, "list", (GuiItem*)list);

	return layout;
}

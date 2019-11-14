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

static void _UiRoot_setLeft(GuiItem* item, BIG row)
{
	DbRoot_setPanelLeft(row);
}

static void _UiRoot_setRight(GuiItem* item, BIG row)
{
	DbRoot_setPanelRight(row);
}

static void _UiRoot_removeLeft(GuiItem* item)
{
	_UiRoot_setLeft(item, -1);
}

static void _UiRoot_removeRight(GuiItem* item)
{
	_UiRoot_setRight(item, -1);
}

void UiRoot_clickSave(GuiItem* item)
{
	DbRoot_save();
}

void UiRoot_clickEditSearch(GuiItem* item)
{
	GuiItem* list = GuiItem_findName(item, "list");
	if (list)
		GuiItem_setResize(list, TRUE);
}

void UiRoot_clickSplitCloseLeft(GuiItem* item)
{
	BIG viewRight = DbRoot_getPanelRight();
	if (viewRight >= 0)
		DbRoot_setPanelLeft(viewRight);	//viewLeft = viewRight;

	_UiRoot_removeRight(item);
}
void UiRoot_clickSplitCloseRight(GuiItem* item)
{
	_UiRoot_removeRight(item);
}

void UiRoot_clickSplitSwap(GuiItem* item)
{
	DbRoot_swapPanelRight();
}

void UiRoot_clickListItem(GuiItem* item)
{
	BIG viewLeft = DbRoot_getPanelLeft();
	BIG viewRight = DbRoot_getPanelRight();

	BOOL left = (OsWinIO_getTouch_action() == Win_TOUCH_DOWN_E);

	BIG viewClick = GuiItem_findAttribute(item, "clickRow");

	if (viewLeft == viewRight && viewRight == viewClick)
	{
		_UiRoot_removeRight(item);
	}
	else
	{
		if (left)
			_UiRoot_setLeft(item, viewClick);
		else
			_UiRoot_setRight(item, viewClick);
	}
}

void UiRoot_clickDuplicate(GuiItem* item)
{
	BIG row = GuiItem_findAttribute(item, "row");
	DbRoot_duplicateRow(row, DbRoot_findParent(row));
}

void UiRoot_clickRemove(GuiItem* item)
{
	BIG viewLeft = DbRoot_getPanelLeft();
	BIG viewRight = DbRoot_getPanelRight();

	BIG row = GuiItem_findAttribute(item, "row");
	DbRoot_removeRow(row);

	if (viewRight == row)
	{
		_UiRoot_removeRight(item);
	}
	if (viewLeft == row && viewRight >= 0)
	{
		DbRoot_setPanelLeft(viewRight);
		_UiRoot_removeRight(item);
	}

	if (viewLeft == row)
	{
		_UiRoot_removeLeft(item);
		_UiRoot_setLeft(item, DbRoot_findParent(row));
	}

	if (viewLeft == 1)	//root
	{
		_UiRoot_removeLeft(item);
		_UiRoot_setLeft(item, 2);	//pages(global)
	}

	GuiItemRoot_resizeAll();
}

void UiRoot_clickAddSub(GuiItem* item)
{
	BIG parent = GuiItem_findAttribute(item, "row");
	BIG newType = GuiItem_findAttribute(item, "type");

	if (DbRoot_isType_root(parent))
	{
		if (newType == UI_ADD_FOLDER)
			DbRoot_createFolderRow();
	}
	else
		if (DbRoot_isType_folder(parent))
		{
			if (newType == UI_ADD_TABLE)
			{
				DbTable* table = DbRoot_createTableExample(parent);
				DbRoot_setPanelLeft(DbTable_getRow(table));
			}
			else
				if (newType == UI_ADD_PAGE)
				{
					DbRoot_createPage(parent);
					//DbRoot_setPanelLeft(...);
				}
		}
		else
			if (DbRoot_isType_table(parent) || DbRoot_isTypeView_filter(parent))
			{
				parent = DbRoot_findChildType(parent, "views");

				const UNI* name = 0;
				BIG r = -1;
				switch (newType)
				{
					case UI_ADD_VIEW_CARDS:	r = DbRoot_createView_cards(parent);		name = Lang_find("VIEW_CARDS");	break;
					case UI_ADD_VIEW_FILTER:	r = DbRoot_createView_filter(parent);	name = Lang_find("VIEW_FILTER");	break;
					case UI_ADD_VIEW_GROUP:	r = DbRoot_createView_group(parent);		name = Lang_find("VIEW_GROUP"); break;
					case UI_ADD_VIEW_KANBAN:	r = DbRoot_createView_kanban(parent);	name = Lang_find("VIEW_KANBAN");	break;
					case UI_ADD_VIEW_CALENDAR:	r = DbRoot_createView_calendar(parent);	name = Lang_find("VIEW_CALENDAR");	break;
					case UI_ADD_VIEW_TIMELINE:	r = DbRoot_createView_timeline(parent);	name = Lang_find("VIEW_TIMELINE");	break;
					case UI_ADD_VIEW_CHART:	r = DbRoot_createView_chart(parent);		name = Lang_find("VIEW_CHART"); break;
					case UI_ADD_VIEW_MAP:		r = DbRoot_createView_map(parent);		name = Lang_find("VIEW_MAP"); break;
					default:	break;
				}
				if (name)
					DbRoot_setName(r, name);

				if (r >= 0)
					DbRoot_setPanelLeft(r);
			}
}

static Image1 _UiRoot_getIcon(UBIG row)
{
	if (DbRoot_isType_folder(row))	return UiIcons_init_project();
	if (DbRoot_isType_table(row))	return UiIcons_init_table();

	if (DbRoot_isTypeView_filter(row))	return UiIcons_init_table_filter();
	if (DbRoot_isTypeView_cards(row))	return UiIcons_init_cards();
	if (DbRoot_isTypeView_group(row))	return UiIcons_init_group();
	if (DbRoot_isTypeView_kanban(row))	return UiIcons_init_kanban();
	if (DbRoot_isTypeView_calendar(row))return UiIcons_init_calendar();
	if (DbRoot_isTypeView_timeline(row))return UiIcons_init_timeline();
	if (DbRoot_isTypeView_chart(row))	return UiIcons_init_graph();
	if (DbRoot_isTypeView_map(row))		return UiIcons_init_map();

	return Image1_init();
}

static DbValue _UiRoot_getSearchValue(void)
{
	return DbValue_initOption(DbRoot_getRootRow(), "search", 0);
}
static DbValue _UiRoot_getSideValue(void)
{
	return DbValue_initOption(DbRoot_getRootRow(), "side", _UNI32("10"));
}
static DbValue _UiRoot_getSplitValue(void)
{
	return DbValue_initOption(DbRoot_getRootRow(), "split", _UNI32("10"));
}
static double _UiRoot_getSideValueNumber(void)
{
	DbValue val = _UiRoot_getSideValue();
	double v = DbValue_getNumber(&val);
	DbValue_free(&val);
	return v;
}
static double _UiRoot_getSplitValueNumber(void)
{
	DbValue val = _UiRoot_getSplitValue();
	double v = DbValue_getNumber(&val);
	DbValue_free(&val);
	return v;
}

static BIG _UiRoot_getSubsRow(BIG row)
{
	return (DbRoot_isType_root(row) || DbRoot_isType_folder(row)) ? row : DbRoot_findChildType(row, "views");
}

static BOOL _UiRoot_canGoSubs(BIG row)
{
	return DbRoot_isType_folder(row) || DbRoot_isType_table(row) || DbRoot_isTypeView_filter(row);
}

static BOOL _UiRoot_showButtonSearch(BIG row, const UNI* search)
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

static BOOL _UiRoot_showButtonSearchSub(const BIG row, const UNI* search)
{
	if (_UiRoot_showButtonSearch(row, search))
		return TRUE;

	if (_UiRoot_canGoSubs(row))
	{
		BIG row2 = _UiRoot_getSubsRow(row);
		BIG it;
		UBIG i = 0;
		while ((it = DbColumnN_jump(DbRoot_getColumnSubs(), row2, &i, 1)) >= 0)
		{
			if (_UiRoot_showButtonSearchSub(it, search))
				return TRUE;

			i++;
		}
	}

	return FALSE;
}

static void _UiRoot_buildListItem(BIG viewLeft, BIG viewRight, GuiItemLayout* parentLayout, Quad2i grid, UBIG row, UBIG parentRow, const UNI* title, const UNI* search)
{
	char subName[64];
	Std_buildNumber(row, 0, subName);

	GuiItemButton* b = (GuiItemButton*)GuiItem_addSubName((GuiItem*)parentLayout, subName, GuiItemButton_newAlphaEx(grid, DbValue_initStaticCopy(title), &UiRoot_clickListItem));
	GuiItemButton_setTextCenter(b, FALSE);
	GuiItem_setEnable((GuiItem*)b, _UiRoot_showButtonSearch(row, search));
	GuiItem_setAttribute((GuiItem*)b, "clickRow", row);

	Image1 icon = _UiRoot_getIcon(row);
	if (Vec2i_is(icon.size))
		GuiItem_setIcon((GuiItem*)b, GuiImage_new1(icon));

	BOOL both = (viewLeft >= 0 && viewRight >= 0);

	GuiItemButton_setPressedEx(b, (viewLeft == row || viewRight == row), (both && viewLeft == row), (both && viewRight == row));

	char desc[32];

	//drag & drop (before/after)
	{
		if (DbRoot_isType_folder(row))
			snprintf(desc, sizeof(desc), "folder");
		else
			if (DbRoot_isType_table(row))
				snprintf(desc, sizeof(desc), "table");
			else
				snprintf(desc, sizeof(desc), "view_%lld", DbRoot_findParentTableRow(row));

		GuiItem_setDrop((GuiItem*)b, desc, desc, FALSE, DbRows_initLink(DbRoot_getColumnSubs(), parentRow), row);
	}

	//drag & drop IN
	{
		desc[0] = 0;

		if (DbRoot_isType_folder(row))	snprintf(desc, sizeof(desc), "table");

		if (DbRoot_isType_table(row))	snprintf(desc, sizeof(desc), "view_%lld", DbRoot_findParentTableRow(row));
		if (DbRoot_isTypeView_filter(row))	snprintf(desc, sizeof(desc), "view_%lld", DbRoot_findParentTableRow(row));

		if (Std_sizeCHAR(desc))
		{
			BIG row2 = _UiRoot_getSubsRow(row);
			GuiItem_setDropIN((GuiItem*)b, desc, DbRows_initLink(DbRoot_getColumnSubs(), row2));
		}
	}
}

static void _UiRoot_buildMenuTable(GuiItemMenu* menu, UBIG row)
{
	GuiItemMenu_addItemIcon(menu, GuiImage_new1(UiIcons_init_table_filter()), DbValue_initLang("VIEW_FILTER"), &UiRoot_clickAddSub, FALSE, UI_ADD_VIEW_FILTER);
	GuiItemMenu_addItemIcon(menu, GuiImage_new1(UiIcons_init_cards()), DbValue_initLang("VIEW_CARDS"), &UiRoot_clickAddSub, FALSE, UI_ADD_VIEW_CARDS);
	GuiItemMenu_addItemIcon(menu, GuiImage_new1(UiIcons_init_group()), DbValue_initLang("VIEW_GROUP"), &UiRoot_clickAddSub, FALSE, UI_ADD_VIEW_GROUP);
	//GuiItemMenu_addItemIcon(menu, GuiImage_new1(UiIcons_init_map()), DbValue_initLang("VIEW_MAP"), &UiRoot_clickAddSub, FALSE, UI_ADD_VIEW_MAP);
	//GuiItemMenu_addItemIcon(menu, GuiImage_new1(UiIcons_init_kanban()), DbValue_initLang("VIEW_KANBAN"), &UiRoot_clickAddSub, FALSE, UI_ADD_VIEW_KANBAN);
	//GuiItemMenu_addItemIcon(menu, GuiImage_new1(UiIcons_init_graph()), DbValue_initLang("VIEW_CHART"), &UiRoot_clickAddSub, FALSE, UI_ADD_VIEW_CHART);
	//GuiItemMenu_addItemIcon(menu, GuiImage_new1(UiIcons_init_calendar()), DbValue_initLang("VIEW_CALENDAR), &UiRoot_clickAddSub, FALSE, Gui_VIEW_CALENDAR);
	//GuiItemMenu_addItemIcon(menu, GuiImage_new1(UiIcons_init_timeline()), DbValue_initLang("VIEW_TIMELINE), &UiRoot_clickAddSub, FALSE, Gui_VIEW_TIMELINE);

	GuiItem_setAttribute((GuiItem*)menu, "row", row);
}

void UiRoot_clickImport(GuiItem* item)
{
	BIG row = GuiItem_findAttribute(item, "row");
	BIG type = GuiItem_findAttribute(item, "type");
	GuiItemRoot_addDialogLayout(UiDialogImport_new(row, type));
}

void UiRoot_clickExport(GuiItem* item)
{
	BIG row = GuiItem_findAttribute(item, "row");
	BIG type = GuiItem_findAttribute(item, "type");
	GuiItemRoot_addDialogLayout(UiDialogExport_new(row, type));
}

void UiRoot_buildMenuImportExport(GuiItemMenu* menu, BIG row, BOOL importt, BOOL exportt, BOOL folder, BOOL addTable)
{
	if (addTable)
	{
		GuiItemMenu_addItemIcon(menu, GuiImage_new1(UiIcons_init_table()), DbValue_initLang("ADD_TABLE"), &UiRoot_clickAddSub, FALSE, UI_ADD_TABLE);
		//GuiItemMenu_addItemIcon(menu, GuiImage_new1(UiIcons_init_page()), DbValue_initLang("ADD_PAGE), &UiRoot_clickAddSub, FALSE, UI_ADD_PAGE);
		GuiItemMenu_addItemEmpty(menu);
	}

	if (importt)
	{
		GuiItemMenu_addItemIcon(menu, GuiImage_new1(UiIcons_init_import()), DbValue_initLang("IMPORT_CSV"), &UiRoot_clickImport, FALSE, UI_IMPORT_CSV);
		GuiItemMenu_addItemIcon(menu, GuiImage_new1(UiIcons_init_import()), DbValue_initLang("IMPORT_TSV"), &UiRoot_clickImport, FALSE, UI_IMPORT_TSV);
		GuiItemMenu_addItemIcon(menu, GuiImage_new1(UiIcons_init_import()), DbValue_initLang("IMPORT_FILES"), &UiRoot_clickImport, FALSE, UI_IMPORT_FILES);
		GuiItemMenu_addItemEmpty(menu);
	}
	if (exportt)
	{
		GuiItemMenu_addItemIcon(menu, GuiImage_new1(UiIcons_init_export()), DbValue_initLang("EXPORT_CSV"), &UiRoot_clickExport, FALSE, UI_EXPORT_CSV);
		GuiItemMenu_addItemIcon(menu, GuiImage_new1(UiIcons_init_export()), DbValue_initLang("EXPORT_TSV"), &UiRoot_clickExport, FALSE, UI_EXPORT_TSV);
		GuiItemMenu_addItemIcon(menu, GuiImage_new1(UiIcons_init_export()), DbValue_initLang("EXPORT_HTML"), &UiRoot_clickExport, FALSE, UI_EXPORT_HTML);
		//GuiItemMenu_addItem(menu, DbValue_initLang("EXPORT_SQL"), GuiItemCall_newRow(&UiRoot_clickExportSql, self, row), FALSE);
		GuiItemMenu_addItemEmpty(menu);
	}

	if (folder)
	{
		GuiItemMenu_addItemIcon(menu, GuiImage_new1(UiIcons_init_duplicate()), DbValue_initLang("DUPLICATE"), &UiRoot_clickDuplicate, FALSE, -1);
		GuiItemMenu_addItemIcon(menu, GuiImage_new1(UiIcons_init_trash()), DbValue_initLang("REMOVE"), &UiRoot_clickRemove, TRUE, -1);
		GuiItem_setAttribute((GuiItem*)menu, "row", row);
	}

	GuiItemMenu_setUnderline(menu, FALSE);
}

static void _UiRoot_buildDashboardSub(GuiItemLayout* layoutList, BIG viewLeft, BIG viewRight, UBIG row, int* y, const UNI* search);

static GuiItemLayout* _UiRoot_buildDashboard(BIG viewLeft, BIG viewRight, UBIG row, UBIG parentRow, int* y, const UNI* search)
{
	GuiItemLayout* layout = GuiItemLayout_new(Quad2i_init4(0, *y, 1, 1));
	GuiItemLayout_addColumn(layout, 1, 99);

	{
		char subName[64];
		Std_buildNumber(row, 0, subName);

		UNI name[64];
		DbRoot_getName(row, name, 64);

		_UiRoot_buildListItem(viewLeft, viewRight, layout, Quad2i_init4(0, 0, 2, 1), row, parentRow, name, search);

		if (DbRoot_isType_table(row) || DbRoot_isTypeView_filter(row))
		{
			GuiItemMenu* menu = (GuiItemMenu*)GuiItem_addSubName((GuiItem*)layout, subName, GuiItemMenu_new(Quad2i_init4(2, 0, 1, 1), DbValue_initStaticCopy(_UNI32("+")), FALSE));
			GuiItemMenu_setUnderline(menu, FALSE);
			_UiRoot_buildMenuTable(menu, row);
		}
		else
			if (DbRoot_isType_folder(row))
			{
				GuiItemButton* b;
				b = (GuiItemButton*)GuiItem_addSubName((GuiItem*)layout, subName, GuiItemButton_newAlphaEx(Quad2i_init4(2, 0, 1, 1), DbValue_initStaticCopy(_UNI32("+")), &UiRoot_clickAddSub));
				GuiItem_setAttribute((GuiItem*)b, "row", row);
				GuiItem_setAttribute((GuiItem*)b, "type", UI_ADD_TABLE);
			}


		GuiItemMenu* menu = (GuiItemMenu*)GuiItem_addSubName((GuiItem*)layout, subName, GuiItemMenu_new(Quad2i_init4(3, 0, 1, 1), DbValue_initStaticCopy(_UNI32("\u2261")), FALSE));
		if (DbRoot_isType_folder(row))
			UiRoot_buildMenuImportExport(menu, row, TRUE, FALSE, TRUE, TRUE);
		else
			UiRoot_buildMenuImportExport(menu, row, FALSE, TRUE, TRUE, FALSE);
	}

	int yy = 0;

	BIG row2 = _UiRoot_getSubsRow(row);
	if (row2 >= 0)
	{
		GuiItemLayout* layoutList = GuiItemLayout_new(Quad2i_init4(1, 1, 3, 1));
		_UiRoot_buildDashboardSub(layoutList, viewLeft, viewRight, row2, &yy, search);

		if (yy)
		{
			GuiItem_addSubName((GuiItem*)layout, "layoutSub", (GuiItem*)layoutList);
			GuiItemLayout_addColumn(layoutList, 0, 99);
			GuiItemLayout_addRow(layout, 1, yy);
		}
		else
			GuiItem_delete((GuiItem*)layoutList);
	}

	GuiItem_setGrid((GuiItem*)layout, Quad2i_init4(0, *y, 1, 1 + yy));
	*y += 1 + yy;

	return layout;
}

static void _UiRoot_buildDashboardSub(GuiItemLayout* layoutList, BIG viewLeft, BIG viewRight, UBIG row, int* y, const UNI* search)
{
	BIG it;
	UBIG i = 0;
	while ((it = DbColumnN_jump(DbRoot_getColumnSubs(), row, &i, 1)) >= 0)
	{
		const BOOL show2 = _UiRoot_showButtonSearchSub(it, search);
		if (show2)
		{
			if (_UiRoot_canGoSubs(it))
			{
				GuiItem_addSubName((GuiItem*)layoutList, "layoutSub", (GuiItem*)_UiRoot_buildDashboard(viewLeft, viewRight, it, row, y, search));
			}
			else
			{
				GuiItemLayout* lai = GuiItemLayout_new(Quad2i_init4(0, *y, 1, 1));
				GuiItemLayout_addColumn(lai, 0, 99);
				GuiItem_addSubName((GuiItem*)layoutList, "layoutSub", (GuiItem*)lai);

				UNI nameIt[64];
				DbRoot_getName(it, nameIt, 64);
				_UiRoot_buildListItem(viewLeft, viewRight, lai, Quad2i_init4(0, 0, 1, 1), it, row, nameIt, search);

				GuiItemMenu* menu = (GuiItemMenu*)GuiItem_addSubName((GuiItem*)lai, "menu", GuiItemMenu_new(Quad2i_init4(1, 0, 1, 1), DbValue_initStaticCopy(_UNI32("\u2261")), FALSE));
				GuiItemMenu_addItem(menu, DbValue_initLang("DUPLICATE"), &UiRoot_clickDuplicate);
				GuiItemMenu_addItemEx(menu, DbValue_initLang("REMOVE"), &UiRoot_clickRemove, TRUE, -1);
				GuiItem_setAttribute((GuiItem*)menu, "row", it);
				GuiItemMenu_setUnderline(menu, FALSE);

				(*y)++;
			}
		}

		i++;
	}
}

void UiRoot_clickSearchDelete(GuiItem* item)
{
	GuiItemEdit* searchEd = GuiItem_findName(item, "search");
	GuiItemEdit_setText(searchEd, 0);
}

void UiRoot_rebuildList(GuiItem* item)
{
	GuiItemLayout* list = GuiItem_findName(item, "list");
	GuiItemEdit* searchEd = GuiItem_findName(item, "search");

	if (list && searchEd)
	{
		GuiItem_freeSubs((GuiItem*)list);

		BIG viewLeft = DbRoot_getPanelLeft();
		BIG viewRight = DbRoot_getPanelRight();

		const UNI* search = GuiItemEdit_getText(searchEd);
		int y = 0;

		_UiRoot_buildDashboardSub(list, viewLeft, viewRight, DbRoot_getRootRow(), &y, search);

		//+Table
		GuiItem* b = GuiItem_addSubName((GuiItem*)list, "+folder", GuiItemButton_newAlphaEx(Quad2i_init4(0, y++, 1, 1), DbValue_initLang("ADD_FOLDER"), &UiRoot_clickAddSub));
		GuiItemButton_setTextCenter((GuiItemButton*)b, FALSE);
		GuiItem_setEnable(b, !Std_sizeUNI(search));
		GuiItem_setAttribute((GuiItem*)b, "row", DbRoot_getRootRow());	//root
		GuiItem_setAttribute((GuiItem*)b, "type", UI_ADD_FOLDER);

		//+Import
		GuiItemMenu* io = (GuiItemMenu*)GuiItem_addSubName((GuiItem*)list, "+import", GuiItemMenu_new(Quad2i_init4(0, y++, 1, 1), DbValue_initLang("ADD_IMPORT"), FALSE));
		UiRoot_buildMenuImportExport(io, -1, TRUE, FALSE, FALSE, FALSE);
		GuiItemMenu_setCenter(io, FALSE);
		GuiItem_setAttribute((GuiItem*)io, "row", -1);	//root
		GuiItem_setEnable(b, !Std_sizeUNI(search));
	}
}

void UiRoot_rebuildPanel(GuiItem* item, BIG row, BOOL left)
{
	item = GuiItem_findName(item, left ? "panelL" : "panelR");

	if (DbRoot_isType_folder(row))
		UiRootProject_build((GuiItemLayout*)item, row);

	if (DbRoot_isType_table(row) || DbRoot_isTypeView_filter(row))
		UiRootTable_build((GuiItemLayout*)item, row, DbValue_initOption(row, left ? "scrollVL" : "scrollVR", 0), DbValue_initOption(row, left ? "scrollHL" : "scrollHR", 0), DbValue_initOption(row, left ? "gridL" : "gridR", _UNI32("0 0 0 0")));

	if (DbRoot_isTypeView_group(row))
		UiRootGroup_build((GuiItemLayout*)item, row, DbValue_initOption(row, left ? "scrollHL" : "scrollHR", 0));

	if (DbRoot_isTypeView_cards(row))
		UiRootCard_build((GuiItemLayout*)item, row);

	//	if (DbRoot_isTypeView_chart(row))
	//		UiRootChart_build((GuiItemLayout*)item, row);

	//	if (DbRoot_isTypeView_kanban(row))
	//		UiRootKanban_build((GuiItemLayout*)item, row);

	//	if (DbRoot_isTypeView_map(row))
	//		UiRootMap_build((GuiItemLayout*)item, row);*/
}

void UiRoot_rebuildPanelLeft(GuiItem* item)
{
	if (DbRoot_getPanelLeft() < 0)
		_UiRoot_setLeft(item, 0);

	UiRoot_rebuildPanel(item, DbRoot_getPanelLeft(), TRUE);
}
void UiRoot_rebuildPanelRight(GuiItem* item)
{
	if (DbRoot_getPanelRight() < 0)
		_UiRoot_setRight(item, 0);

	UiRoot_rebuildPanel(item, DbRoot_getPanelRight(), FALSE);
}

void UiRoot_rebuildBase(GuiItem* item)
{
	BIG viewLeft = DbRoot_getPanelLeft();
	BIG viewRight = DbRoot_getPanelRight();

	int panelsCells = (Win_getImage(UiScreen_getWin()).size.x / OsWinIO_cellSize()) - _UiRoot_getSideValueNumber();
	int leftCells = Std_max(5, _UiRoot_getSplitValueNumber());
	int rightCells = Std_max(5, panelsCells - _UiRoot_getSplitValueNumber());

	GuiItemLayout* layout = (GuiItemLayout*)item;
	GuiItemLayout_addColumn(layout, 0, Std_max(2, _UiRoot_getSideValueNumber()));
	GuiItemLayout_addColumn(layout, 1, leftCells);
	GuiItemLayout_addColumn(layout, 3, rightCells);
	GuiItemLayout_addRow(layout, 0, 100);

	//rewrite current
	BOOL sidePanel = _UiRoot_getSideValueNumber() > 3;
	GuiItem_setShow(GuiItem_findName(item, "save"), sidePanel);
	GuiItem_setGrid(GuiItem_findName(item, "logo"), Quad2i_init4(0, 0, 1 + !sidePanel, 1));

	//free
	GuiItem_remove(GuiItem_findName(item, "panelL"));	//left
	GuiItem_remove(GuiItem_findName(item, "panelR"));	//right
	GuiItem_remove(GuiItem_findName(item, "split"));

	//Left panel
	if (viewLeft >= 0)
	{
		GuiItemLayout* panelLayout = GuiItemLayout_new(Quad2i_init4(1, 0, (viewRight >= 0) ? 1 : 3, 1));
		GuiItemLayout_setBackgroundWhite(panelLayout, TRUE);
		GuiItem_addSubName((GuiItem*)layout, "panelL", (GuiItem*)panelLayout);
		GuiItemLayout_setResize(panelLayout, &UiRoot_rebuildPanelLeft);

		if (viewRight >= 0)
			GuiItem_setChangeSize((GuiItem*)panelLayout, TRUE, _UiRoot_getSplitValue(), TRUE);
	}

	//Right panel
	if (viewRight >= 0)
	{
		GuiItemLayout* panelLayout = GuiItemLayout_new(Quad2i_init4(3, 0, 1, 1));
		GuiItemLayout_setBackgroundWhite(panelLayout, TRUE);
		GuiItem_addSubName((GuiItem*)layout, "panelR", (GuiItem*)panelLayout);
		GuiItemLayout_setResize(panelLayout, &UiRoot_rebuildPanelRight);

		//Split (X)
		GuiItemLayout* layMid = GuiItemLayout_new(Quad2i_init4(2, 0, 1, 1));
		GuiItem_addSubName((GuiItem*)layout, "split", (GuiItem*)layMid);
		GuiItemLayout_addRow(layMid, 0, 100);
		GuiItemLayout_addRow(layMid, 2, 2);
		GuiItemLayout_addRow(layMid, 4, 100);
		GuiItem_addSubName((GuiItem*)layMid, ">>", GuiItemButton_newAlphaEx(Quad2i_init4(0, 1, 1, 1), DbValue_initStaticCopy(_UNI32(">>")), &UiRoot_clickSplitCloseRight));
		GuiItem_addSubName((GuiItem*)layMid, "<<", GuiItemButton_newAlphaEx(Quad2i_init4(0, 3, 1, 1), DbValue_initStaticCopy(_UNI32("<<")), &UiRoot_clickSplitCloseLeft));
		GuiItem_addSubName((GuiItem*)layMid, "<>", GuiItemButton_newAlphaEx(Quad2i_init4(0, 5, 1, 1), DbValue_initStaticCopy(_UNI32("<>")), &UiRoot_clickSplitSwap));
	}

	//change Window/Fullscreen mode
	GuiItemMenu* menu = GuiItem_findName(item, "logo");
	if (menu)
	{
		DbValue* itV = _GuiItemMenu_findItem(menu, Lang_find(!UiScreen_isFullScreen() ? "WINDOW_MODE" : "FULLSCREEN_MODE"));
		if (itV)
			itV->lang_id = (UiScreen_isFullScreen() ? "WINDOW_MODE" : "FULLSCREEN_MODE");
	}
}

GuiItemLayout* UiRoot_new(void)
{
	GuiItemLayout* layout = GuiItemLayout_new(Quad2i_init());
	GuiItemLayout_setResize(layout, &UiRoot_rebuildBase);

	GuiItem_setShortcutKey((GuiItem*)layout, FALSE, Win_EXTRAKEY_CTRL | Win_EXTRAKEY_SAVE, 0, &UiRoot_clickSave);

	GuiItemLayout* sideLayout = GuiItemLayout_new(Quad2i_init4(0, 0, 1, 1));
	GuiItemLayout_addColumn(sideLayout, 0, 100);
	GuiItemLayout_addColumn(sideLayout, 1, 100);
	GuiItemLayout_addRow(sideLayout, 2, 100);
	GuiItem_addSubName((GuiItem*)layout, "side", (GuiItem*)sideLayout);

	//logo
	GuiItem_addSubName((GuiItem*)sideLayout, "logo", UiRootMenu_new(Quad2i_init4(0, 0, 1, 1)));
	//save
	GuiItem_addSubName((GuiItem*)sideLayout, "save", GuiItemButton_newBlackEx(Quad2i_init4(1, 0, 1, 1), DbValue_initLang("SAVE"), &UiRoot_clickSave));

	//search
	GuiItemLayout* searchLayout = GuiItemLayout_new(Quad2i_init4(0, 1, 2, 1));
	GuiItemLayout_addColumn(searchLayout, 0, 100);
	GuiItem_addSubName((GuiItem*)sideLayout, "searchLayout", (GuiItem*)searchLayout);
	{
		GuiItemEdit* search = (GuiItemEdit*)GuiItem_addSubName((GuiItem*)searchLayout, "search", GuiItemEdit_new(Quad2i_init4(0, 0, 1, 1), _UiRoot_getSearchValue(), DbValue_initLang("SEARCH")));
		GuiItemEdit_setHighlightIfContent(search, TRUE);
		GuiItem_setIcon((GuiItem*)search, GuiImage_new1(UiIcons_init_search()));
		GuiItemEdit_setFnChanged(search, &UiRoot_clickEditSearch);

		GuiItem_addSubName((GuiItem*)searchLayout, "X", GuiItemButton_newAlphaEx(Quad2i_init4(1, 0, 1, 1), DbValue_initStaticCopy(_UNI32("X")), &UiRoot_clickSearchDelete));
	}

	//list panel
	GuiItemLayout* projectsLayout = GuiItemLayout_new(Quad2i_init4(0, 2, 2, 1));
	GuiItem_addSubName((GuiItem*)sideLayout, "list", (GuiItem*)projectsLayout);
	GuiItemLayout_addColumn(projectsLayout, 0, 99);
	GuiItemLayout_setResize(projectsLayout, &UiRoot_rebuildList);
	GuiItemLayout_setScrollV(projectsLayout, DbValue_initOption(DbRoot_getRootRow(), "scrollS", 0));

	//collaboration info
	//if(self->showUpdateSave)
	//	GuiItem_addSub((GuiItem*)sideLayout, GuiItemButton_newClassicEx(Quad2i_init4(0, 4, 2, 1), DbValue_initLang("SAVE_AND_UPDATE), &UiRoot_clickSaveAndUpdate));

	GuiItem_setChangeSize((GuiItem*)sideLayout, TRUE, _UiRoot_getSideValue(), TRUE);

	return layout;
}

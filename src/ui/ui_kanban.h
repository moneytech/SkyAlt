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

void UiRootKanban_clickRebuild(GuiItem* item)
{
	//resize/redraw only this ...
	//GuiItemRoot_resizeAll();
}

void UiRootKanban_clickPageShowAll(GuiItem* item)
{
	AppGui* appGui = App_getGui();
	BIG row = GuiItem_findAttribute(item, "row");
	AppGui_enablePropSelectAll(appGui, row, TRUE);
}

void UiRootKanban_clickPageHideAll(GuiItem* item)
{
	AppGui* appGui = App_getGui();
	BIG row = GuiItem_findAttribute(item, "row");
	AppGui_enablePropSelectAll(appGui, row, FALSE);
}

static GuiItemLayout* UiRootKanban_buildListEx(UBIG row, UBIG hiddenProp)
{
	AppGui* appGui = App_getGui();

	GuiItemLayout* layColumn = GuiItemLayout_new(Quad2i_init());
	GuiItemLayout_addColumn(layColumn, 0, 10);
	GuiItemLayout_addRow(layColumn, 0, 10);	//list
	GuiItem_setAttribute((GuiItem*)layColumn, "row", row);

	GuiItemLayout* skin = GuiItemLayout_new(Quad2i_init4(0, 0, 1, 1));
	GuiItemLayout_addColumn(skin, 2, 99);

	GuiItem* drag = GuiItem_addSub((GuiItem*)skin, GuiItemBox_newEmpty(Quad2i_init4(0, 0, 1, 1)));
	GuiItem_setIcon(drag, GuiImage_new1(UiIcons_init_reoder()));
	GuiItem_setDrop(drag, "lanes", "lanes", FALSE, AppGui_connectLinkSub(appGui, hiddenProp), -1);
	GuiItem_addSub((GuiItem*)skin, GuiItemCheck_newEx(Quad2i_init4(1, 0, 1, 1), AppGui_connectEnable(appGui, -1), DbValue_initEmpty(), &UiRootKanban_clickRebuild));

	GuiItem* name = GuiItem_addSub((GuiItem*)skin, GuiItemText_new(Quad2i_init4(2, 0, 1, 1), FALSE, AppGui_connectName(appGui, -1), DbValue_initEmpty()));
	GuiItem_setEnableMsg(name, AppGui_connectEnable(appGui, -1), FALSE);

	GuiItemList* list = (GuiItemList*)GuiItem_addSub((GuiItem*)layColumn, GuiItemList_new(Quad2i_init4(0, 0, 1, 1), AppGui_connectLinkSub(appGui, hiddenProp), (GuiItem*)skin, DbValue_initEmpty(), DbValue_initEmpty()));
	GuiItemList_setShowBorder(list, FALSE);
	//GuiItemList_setShowRemove(list, TRUE);
	GuiItem_setCallClick((GuiItem*)list, &UiRootKanban_clickRebuild);

	//Hide all
	GuiItem_addSub((GuiItem*)layColumn, GuiItemButton_newClassicEx(Quad2i_init4(0, 1, 1, 1), DbValue_initLang(GUI_HIDE_ALL), &UiRootKanban_clickPageHideAll));

	//Show All
	GuiItem_addSub((GuiItem*)layColumn, GuiItemButton_newClassicEx(Quad2i_init4(0, 2, 1, 1), DbValue_initLang(GUI_SHOW_ALL), &UiRootKanban_clickPageShowAll));

	return layColumn;
}

void UiRootKanban_clickColumnList(GuiItem* item)
{
	GuiItemComboDynamic* cb = (GuiItemComboDynamic*)item;
	DbTable* table = GuiItemComboDynamic_getExtraTable(cb);

	GuiItemComboDynamic_setOptionsLinks(cb, GuiDbID_newArray(DbRoot_getTableInfo(), DbRoot_getColumnLinks(table)));
}

/*void UiRootKanban_clickColumnListSET(GuiItem* item)
{
	AppGui* appGui = App_getGui();

	GuiItemComboDynamic* cb = (GuiItemComboDynamic*)item;
	BIG groupRow = GuiItemComboDynamic_getExtraRow(cb);
	BIG columnRow = GuiItemComboDynamic_getValueRow(cb);

	AppGui_addPropShortLine(appGui, groupRow, columnRow);
}*/

static GuiItem* UiRootKanban_build(GuiItemLayout* layout, UBIG row)
{
	AppGui* appGui = App_getGui();

	BIG parent = AppGui_getParent(appGui, row);
	DbFilter* exeFilter = UiRoot_loadTableAndFilter(parent);
	AppGui_checkCopyPropTable(appGui, row, parent);	//check cards

	DbTable* table = AppGui_getTable(appGui, parent);
	BIG tableRow = DbTable_getRow(table);
	//BIG columnFirst = AppGui_addPropGroupColumnFirst(appGui, row);

	//GuiItemLayout* layout = GuiItemLayout_new(grid);
	GuiItemLayout_addColumn(layout, 0, 99);
	GuiItemLayout_addRow(layout, 1, 99);

	//header
	{
		GuiItemLayout* layoutMenu = GuiItemLayout_new(Quad2i_init4(0, 0, 1, 1));
		GuiItemLayout_addColumn(layoutMenu, 0, 6);	//name
		GuiItemLayout_addColumn(layoutMenu, 2, 4);
		GuiItemLayout_addColumn(layoutMenu, 4, 3);
		GuiItemLayout_addColumn(layoutMenu, 6, 3);
		GuiItemLayout_addColumn(layoutMenu, 7, 100);	//space
		GuiItemLayout_addColumn(layoutMenu, 8, 1);
		GuiItem_addSub((GuiItem*)layout, (GuiItem*)layoutMenu);

		//Name
		GuiItem* name = GuiItem_addSub((GuiItem*)layoutMenu, GuiItemEdit_newEx(Quad2i_init4(0, 0, 1, 1), AppGui_connectName(appGui, row), DbValue_initLang(GUI_NAME), &UiRootKanban_clickRebuild));
		GuiItem_setIcon(name, GuiImage_new1(UiIcons_init_name()));

		//Column
		/*GuiItemComboDynamic* cb = (GuiItemComboDynamic*)*/GuiItem_addSub((GuiItem*)layoutMenu, GuiItemComboDynamic_newEx(Quad2i_init4(2, 0, 1, 1), TRUE, AppGui_connectLinkTable(appGui, row), DbValue_initGET((DbColumn*)DbRoot_getColumnInfoName(), tableRow), GuiDbID_newArray(DbRoot_getTableInfo(), DbRoot_getColumnLinks(table)), DbValue_initLang(GUI_COLUMNS), &UiRootKanban_clickRebuild));
		//GuiItemComboDynamic_setEmptyLang(cb, GUI_CHOOSE);
		//GuiItemComboDynamic_setOpenCall(cb, &UiRootKanban_clickColumnList);
		//GuiItemComboDynamic_setExtraInfo(cb, table, row);*/

		//Hidden lines
		GuiItemMenu* hidden = (GuiItemMenu*)GuiItem_addSub((GuiItem*)layoutMenu, GuiItemMenu_new(Quad2i_init4(4, 0, 1, 1), DbValue_initLang(GUI_LANES), FALSE));
		GuiItemMenu_setContext(hidden, UiRootKanban_buildListEx(row, AppGui_getPropSelect(App_getGui(), row)));
		GuiItemMenu_setHighligthBackground(hidden, AppGui_hasShowedSub(appGui, row, FALSE));
		GuiItem_setIcon((GuiItem*)hidden, GuiImage_new1(UiIcons_init_table_hide()));

		//Card
		GuiItemMenu* card = (GuiItemMenu*)GuiItem_addSub((GuiItem*)layoutMenu, GuiItemMenu_new(Quad2i_init4(6, 0, 1, 1), DbValue_initLang(GUI_CARD), FALSE));
		GuiItemMenu_setContext(card, UiRootCard_buildList(row));
		GuiItemMenu_setHighligthBackground(card, AppGui_hasShowedSub(appGui, row, FALSE));
		GuiItem_setIcon((GuiItem*)card, GuiImage_new1(UiIcons_init_table_hide()));

		//Options
		GuiItemMenu* menu = (GuiItemMenu*)GuiItem_addSub((GuiItem*)layoutMenu, GuiItemMenu_new(Quad2i_init4(8, 0, 1, 1), DbValue_initStaticCopy(_UNI32("≡")), FALSE));
		GuiItemMenu_setUnderline(menu, FALSE);
		GuiItemMenu_addItem(menu, DbValue_initLang(GUI_REMOVE_UNUSED), &GuiItemKanban_clickRemoveEmptyLanes);
	}

	GuiItemKanban* kanban = GuiItemKanban_new(Quad2i_init4(0, 1, 1, 1), AppGui_connectLinkTable(appGui, row), AppGui_connectLinkSub(appGui, AppGui_getPropSelect(appGui, row)), (GuiItem*)GuiItemKanban_buildSkin(row, TRUE, TRUE), exeFilter);
	GuiItem_addSubName((GuiItem*)layout, "kanban", (GuiItem*)kanban);

	return (GuiItem*)layout;
}

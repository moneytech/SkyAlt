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






void UiRootChart_clickRebuild(GuiItem* item)
{
	//resize/redraw only this ...
	//GuiItemRoot_resizeAll();
}








static GuiItemLayout* _UiRootChart_buildColumnList(UBIG row)
{
	AppGui* appGui = App_getGui();
	DbTable* table = AppGui_getTable(appGui, row);
	//DbRoot* tables = DbTable_getParent(table);
	
	
	UBIG selectRow = AppGui_getPropSelect(appGui, row);

	GuiItemLayout* layout = GuiItemLayout_new(Quad2i_init());
	GuiItemLayout_addColumn(layout, 0, 20);
	GuiItemLayout_addRow(layout, 2, 5);	//list

	
	
	//Column = count
	GuiItem_addSub((GuiItem*)layout, GuiItemCheck_new(Quad2i_init4(0, 0, 1, 1), AppGui_connectShowGroupCount(appGui, row), DbValue_initLang(GUI_CHART_GROUP_COUNT)));
	
	//add
	GuiItem* add = GuiItem_addSub((GuiItem*)layout, GuiItemButton_newClassicEx(Quad2i_init4(0, 1, 1, 1), DbValue_initStaticCopy(_UNI32("+")), &UiRoot_clickAddSubLine));
	GuiItem_setAttribute((GuiItem*)add, "row", selectRow);

	//skin for list
	GuiItemLayout* skin = GuiItemLayout_new(Quad2i_init4(0, 0, 1, 1));
	GuiItemLayout_addColumn(skin, 2, 100);
	GuiItemLayout_addColumn(skin, 3, 2);
	
	GuiItem* drag = GuiItem_addSub((GuiItem*)skin, GuiItemBox_newEmpty(Quad2i_init4(0, 0, 1, 1)));
	GuiItem_setIcon(drag, GuiImage_new1(UiIcons_init_reoder()));
	GuiItem_setDrop(drag, "group", "group", FALSE, AppGui_connectLinkSub(appGui, selectRow), -1);


	//enable
	GuiItem_addSub((GuiItem*)skin, GuiItemCheck_new(Quad2i_init4(1, 0, 1, 1), AppGui_connectEnable(appGui, -1), DbValue_initEmpty()));

	//combo
	GuiItemComboDynamic* cbb = (GuiItemComboDynamic*)GuiItem_addSub((GuiItem*)skin, GuiItemComboDynamic_new(Quad2i_init4(2, 0, 1, 1), TRUE, AppGui_connectLinkTable(appGui, -1), DbValue_initGET((DbColumn*)DbRoot_getColumnInfoName(), -1), GuiDbID_newEmpty(), DbValue_initLang(GUI_COLUMNS)));
	GuiItemComboDynamic_setOpenCall(cbb, &UiRoot_clickColumnList);
	GuiItemComboDynamic_setExtraInfo(cbb, table, selectRow);
	GuiItem_setEnableMsg((GuiItem*)cbb, AppGui_connectEnable(appGui, -1), FALSE);
	
	//color
	GuiItem_addSub((GuiItem*)skin, (GuiItem*)GuiItemColor_new(Quad2i_init4(3, 0, 1, 1), AppGui_connectColor(appGui, -1), FALSE));
	
	
	//list
	GuiItemList* list = (GuiItemList*)GuiItem_addSub((GuiItem*)layout, GuiItemList_new(Quad2i_init4(0, 2, 1, 1), AppGui_connectLinkSub(appGui, selectRow), (GuiItem*)skin, DbValue_initEmpty(), DbValue_initEmpty()));
	GuiItemList_setShowRemove(list, TRUE);
	GuiItemList_setShowBorder(list, FALSE);
	GuiItem_setCallClick((GuiItem*)list, &UiRootTable_clickRebuild);	//nová položka v shorting list

	
	//First Column = Title
	GuiItem* firstColumn = GuiItem_addSub((GuiItem*)layout, GuiItemCheck_new(Quad2i_init4(0, 3, 1, 1), AppGui_connectShowTitle(appGui, row), DbValue_initLang(GUI_CHART_FIRST_TITLE)));
	
	
	
	
	GuiItem_setEnableMsg((GuiItem*)firstColumn, AppGui_connectShowGroupCount(appGui, row), TRUE);
	GuiItem_setEnableMsg((GuiItem*)add, AppGui_connectShowGroupCount(appGui, row), TRUE);
	GuiItem_setEnableMsg((GuiItem*)list, AppGui_connectShowGroupCount(appGui, row), TRUE);

	return layout;
}



static GuiItem* UiRootChart_build(GuiItemLayout* layout, UBIG row)
{
	AppGui* appGui = App_getGui();
	
	BIG parent = AppGui_getParent(appGui, row);
	DbFilter* exeFilter = UiRoot_loadTableAndFilter(parent);

	//UBIG selectRow = AppGui_getPropSelect(appGui, row);
	
	//GuiItemLayout* layout = GuiItemLayout_new(grid);
	GuiItemLayout_addColumn(layout, 0, 99);
	GuiItemLayout_addRow(layout, 1, 99);


	//header
	{
	GuiItemLayout* layoutMenu = GuiItemLayout_new(Quad2i_init4(0, 0, 1, 1));
	GuiItemLayout_addColumn(layoutMenu, 0, 6);	//name
	GuiItemLayout_addColumn(layoutMenu, 2, 6);
	GuiItemLayout_addColumn(layoutMenu, 4, 6);
	GuiItemLayout_addColumn(layoutMenu, 6, 6);
	GuiItemLayout_addColumn(layoutMenu, 8, 6);
	GuiItemLayout_addColumn(layoutMenu, 10, 6);
	GuiItemLayout_addColumn(layoutMenu, 12, 6);
	GuiItem_addSub((GuiItem*)layout, (GuiItem*)layoutMenu);

	//name
	GuiItem* name = GuiItem_addSub((GuiItem*)layoutMenu, GuiItemEdit_newEx(Quad2i_init4(0, 0, 1, 1), AppGui_connectName(appGui, row), DbValue_initLang(GUI_NAME), &UiRootChart_clickRebuild));
	GuiItem_setIcon(name, GuiImage_new1(UiIcons_init_name()));
	
	//groups
	GuiItemMenu* groupp = (GuiItemMenu*)GuiItem_addSub((GuiItem*)layoutMenu, GuiItemMenu_new(Quad2i_init4(2, 0, 1, 1), DbValue_initLang(GUI_GROUP), FALSE));
	GuiItemMenu_setContext(groupp, _UiRoot_buildShortingList(row, TRUE));
	GuiItemMenu_setHighligthBackground(groupp, AppGui_hasGroupLines(appGui, row));
	GuiItem_setIcon((GuiItem*)groupp, GuiImage_new1(UiIcons_init_group()));
	GuiItemMenu_setCenter(groupp, FALSE);

	//list
	GuiItemMenu* columnsList = (GuiItemMenu*)GuiItem_addSub((GuiItem*)layoutMenu, GuiItemMenu_new(Quad2i_init4(4, 0, 1, 1), DbValue_initLang(GUI_COLUMNS), FALSE));
	GuiItemMenu_setContext(columnsList, _UiRootChart_buildColumnList(row));
	GuiItemMenu_setHighligthBackground(columnsList, AppGui_hasSelectLines(appGui, row));
	GuiItem_setIcon((GuiItem*)columnsList, GuiImage_new1(UiIcons_init_chart_columns()));
	GuiItemMenu_setCenter(columnsList, FALSE);

	//graph types
	GuiItem_addSub((GuiItem*)layoutMenu, GuiItemComboStatic_newEx(Quad2i_init4(6, 0, 1, 1), AppGui_connectType2(appGui, row), DbValue_initLang(GUI_CHART_TYPE_OPTIONS), DbValue_initEmpty(), &UiRootChart_clickRebuild));
	GuiItem_addSub((GuiItem*)layoutMenu, GuiItemComboStatic_newEx(Quad2i_init4(8, 0, 1, 1), AppGui_connectType3(appGui, row), DbValue_initLang(GUI_CHART_TYPE2_OPTIONS), DbValue_initEmpty(), &UiRootChart_clickRebuild));
	
	//show values
	GuiItem_addSub((GuiItem*)layoutMenu, GuiItemCheck_new(Quad2i_init4(10, 0, 1, 1), AppGui_connectShowValues(appGui, row), DbValue_initLang(GUI_CHART_SHOW_VALUES)));

	//width
	GuiItem_addSub((GuiItem*)layoutMenu, GuiItemComboStatic_new(Quad2i_init4(12, 0, 1, 1), AppGui_connectWidth(appGui, row), DbValue_initLang(GUI_CHART_WIDTH_OPTIONS), DbValue_initEmpty()));
	
	}

	DbTable* table = AppGui_getTable(appGui, row);
	GuiDbID* ids = (exeFilter && !DbFilter_isEmpty(exeFilter)) ? GuiDbID_newArray(table, StdBigs_initCopy(DbFilter_getRows(exeFilter))) : GuiDbID_newTable(table);

	
	
	GuiItem* chart = GuiItemChart_new(	Quad2i_init4(0, 1, 1, 1), ids, AppGui_connectType2(appGui, row), AppGui_connectType3(appGui, row), AppGui_connectShowTitle(appGui, row), AppGui_connectShowValues(appGui, row), AppGui_connectShowGroupCount(appGui, row), AppGui_connectWidth(appGui, row), 
						AppGui_connectLinkSub(appGui, AppGui_getPropSelect(appGui, row)), AppGui_connectLinkTable(appGui, -1), AppGui_connectEnable(appGui, -1), AppGui_connectColor(appGui, -1),
						AppGui_connectLinkSub(appGui, AppGui_getPropGroup(appGui, row)), AppGui_connectLinkTable(appGui, -1), AppGui_connectEnable(appGui, -1), AppGui_connectAscending(appGui, -1));
	GuiItem_addSub((GuiItem*)layout, (GuiItem*)chart);
	
	
	DbFilter_delete(exeFilter);
	return (GuiItem*)layout;
}

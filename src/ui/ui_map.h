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




void UiRootMap_clickRebuild(GuiItem* item)
{
	//resize/redraw only this ...
	//GuiItemRoot_resizeAll();
}




void UiRootMap_clickPointsFocus(GuiItem* item)
{
	GuiItemMap* map = GuiItem_findName(item, "map");
	if(map)
	{
		GuiItemMap_refocus(map);
	}	
}


void UiRootMap_clickColumnList(GuiItem* item)
{
	GuiItemComboDynamic* cb = (GuiItemComboDynamic*)item;
	DbTable* table = GuiItemComboDynamic_getExtraTable(cb);
	
	GuiItemComboDynamic_setOptionsLinks(cb, GuiDbID_newArray(DbRoot_getTableInfo(), DbRoot_getColumnLinks(table)));
}



static GuiItemLayout* _UiRootMap_buildTitlesList(UBIG row)
{
	AppGui* appGui = App_getGui();
	DbTable* table = AppGui_getTable(appGui, row);


	UBIG propRow = AppGui_getPropTitles(appGui, row);

	GuiItemLayout* layout = GuiItemLayout_new(Quad2i_init());
	GuiItemLayout_addColumn(layout, 0, 10);
	GuiItemLayout_addRow(layout, 2, 10);	//list


	//enable all
	GuiItem_addSub((GuiItem*)layout, GuiItemCheck_new(Quad2i_init4(0, 0, 1, 1), AppGui_connectEnable(appGui, propRow), DbValue_initLang(GUI_ENABLE)));


	//add
	GuiItemButton* add = (GuiItemButton*)GuiItem_addSub((GuiItem*)layout, GuiItemButton_newClassicEx(Quad2i_init4(0, 1, 1, 1), DbValue_initStaticCopy(_UNI32("+")), &UiRoot_clickAddSubLine));
	GuiItem_setAttribute((GuiItem*)add, "row", propRow);

	//skin for list
	GuiItemLayout* skin = GuiItemLayout_new(Quad2i_init4(0, 0, 1, 1));
	GuiItemLayout_addColumn(skin, 2, 10);
	//GuiItemLayout_addColumn(skin, 3, 10);

	GuiItem* drag = GuiItem_addSub((GuiItem*)skin, GuiItemBox_newEmpty(Quad2i_init4(0, 0, 1, 1)));
	GuiItem_setIcon(drag, GuiImage_new1(UiIcons_init_reoder()));
	GuiItem_setDrop(drag, "title", "title", FALSE, AppGui_connectLinkSub(appGui, propRow), -1);

	GuiItem_addSub((GuiItem*)skin, GuiItemCheck_new(Quad2i_init4(1, 0, 1, 1), AppGui_connectEnable(appGui, -1), DbValue_initEmpty()));

	GuiItemComboDynamic* cbb = (GuiItemComboDynamic*)GuiItem_addSub((GuiItem*)skin, GuiItemComboDynamic_new(Quad2i_init4(2, 0, 1, 1), FALSE, AppGui_connectLinkTable(appGui, -1), DbValue_initGET((DbColumn*)DbRoot_getColumnInfoName(), -1), GuiDbID_newEmpty(), DbValue_initEmpty()));
	GuiItemComboDynamic_setOpenCall(cbb, &UiRoot_clickColumnList);
	GuiItemComboDynamic_setExtraInfo(cbb, table, propRow);
	//GuiItem* order = GuiItem_addSub((GuiItem*)skin, GuiItemComboStatic_new(Quad2i_init4(3, 0, 1, 1), AppGui_connectAscending(appGui, -1), DbValue_initStatic(Lang_getColumnOrder()), DbValue_initEmpty()));

	GuiItem_setEnableMsg((GuiItem*)cbb, AppGui_connectEnable(appGui, -1), FALSE);
	//GuiItem_setEnableMsg(order, AppGui_connectEnable(appGui, -1), FALSE);


	//list
	GuiItemList* list = (GuiItemList*)GuiItem_addSub((GuiItem*)layout, GuiItemList_new(Quad2i_init4(0, 2, 1, 1), AppGui_connectLinkSub(appGui, propRow), (GuiItem*)skin, DbValue_initEmpty(), DbValue_initEmpty()));
	GuiItemList_setShowRemove(list, TRUE);
	GuiItemList_setShowBorder(list, FALSE);
	GuiItem_setCallClick((GuiItem*)list, &UiRootTable_clickRebuild);	//nová položka v shorting list


	GuiItem_setEnableMsg((GuiItem*)add, AppGui_connectEnable(appGui, propRow), FALSE);
	GuiItem_setEnableMsg((GuiItem*)list, AppGui_connectEnable(appGui, propRow), FALSE);


	return layout;
}



static GuiItem* UiRootMap_build(GuiItemLayout* layout, UBIG row)
{
	AppGui* appGui = App_getGui();
	
	BIG parent = AppGui_getParent(appGui, row);
	DbFilter* exeFilter = UiRoot_loadTableAndFilter(parent);
	

	//AppGui_checkCopyPropTable(appGui, row, AppGui_getParent(appGui, row));	//for titles
	
	
	DbTable* table = AppGui_getTable(appGui, AppGui_getParent(appGui, row));
	
	UBIG tableRow = DbTable_getRow(table);
	
	//GuiItemLayout* layout = GuiItemLayout_new(grid);
	GuiItemLayout_addColumn(layout, 0, 99);
	GuiItemLayout_addRow(layout, 1, 99);


	UBIG propTitlesRow = AppGui_getPropTitles(appGui, row);

	//BIG columnFirst = AppGui_addPropGroupColumnFirst(appGui, row);
	
	
	//header
	{
	GuiItemLayout* layoutMenu = GuiItemLayout_new(Quad2i_init4(0, 0, 1, 1));
	GuiItemLayout_addColumn(layoutMenu, 0, 6);
	GuiItemLayout_addColumn(layoutMenu, 2, 6);
	GuiItemLayout_addColumn(layoutMenu, 4, 6);
	GuiItemLayout_addColumn(layoutMenu, 6, 6);
	GuiItemLayout_addColumn(layoutMenu, 7, 99);	//space
	GuiItemLayout_addColumn(layoutMenu, 8, 6);
	GuiItem_addSub((GuiItem*)layout, (GuiItem*)layoutMenu);

	//name
	GuiItem* name = GuiItem_addSub((GuiItem*)layoutMenu, GuiItemEdit_newEx(Quad2i_init4(0, 0, 1, 1), AppGui_connectName(appGui, row), DbValue_initLang(GUI_NAME), &UiRootMap_clickRebuild));
	GuiItem_setIcon(name, GuiImage_new1(UiIcons_init_name()));

	//address
	GuiItem_addSub((GuiItem*)layoutMenu, GuiItemComboDynamic_newEx(Quad2i_init4(2, 0, 1, 1), TRUE, AppGui_connectLinkTableA(appGui, row), DbValue_initGET((DbColumn*)DbRoot_getColumnInfoName(), tableRow), GuiDbID_newArray(DbRoot_getTableInfo(), DbRoot_getColumnLinks(table)), DbValue_initLang(GUI_ADDRESS), &UiRootMap_clickPointsFocus));


	//titles
	//GuiItem_addSub((GuiItem*)layoutMenu, GuiItemComboDynamic_newEx(Quad2i_init4(4, 0, 1, 1), AppGui_connectLinkTableA(appGui, row), DbValue_initGET((DbColumn*)DbRoot_getColumnInfoName(), tableRow), GuiDbID_newArray(DbRoot_getTableInfo(), DbRoot_getColumnLinks(table)), DbValue_initLang(GUI_COLUMNS), &UiRootMap_clickPointsFocus));
	/*GuiItemMenu* hidden = (GuiItemMenu*)GuiItem_addSub((GuiItem*)layoutMenu, GuiItemMenu_new(Quad2i_init4(4, 0, 1, 1), DbValue_initLang(GUI_TITLES), FALSE));
	GuiItemMenu_setContext(hidden, UiRootCard_buildListOff(row));
	GuiItemMenu_setHighligthBackground(hidden, AppGui_hasShowedSub(appGui, row, TRUE));
	//GuiItem_setIcon((GuiItem*)hidden, GuiImage_new1(UiIcons_init_table_hide()));*/

	//titles
	GuiItemMenu* titles = (GuiItemMenu*)GuiItem_addSub((GuiItem*)layoutMenu, GuiItemMenu_new(Quad2i_init4(4, 0, 1, 1), DbValue_initLang(GUI_TITLES), FALSE));
	GuiItemMenu_setContext(titles, _UiRootMap_buildTitlesList(row));
	GuiItemMenu_setHighligthBackground(titles, AppGui_hasLine(appGui, propTitlesRow));
	GuiItemMenu_setCenter(titles, FALSE);


	
	
	//advanced
/*	{
	GuiItemLayout* layoutAdv = GuiItemLayout_new(Quad2i_init());
	GuiItemLayout_addColumn(layoutAdv, 0, 3);
	GuiItemLayout_addColumn(layoutAdv, 1, 4);
	GuiItemLayout_addColumn(layoutAdv, 2, 1);
	GuiItemLayout_addColumn(layoutAdv, 3, 4);
	
	
	//type(icon or point)
	GuiItem_addSub((GuiItem*)layoutAdv, GuiItemText_new(Quad2i_init4(0, 0, 1, 1), FALSE, DbValue_initLang(GUI_TYPE), DbValue_initEmpty()));
	GuiItem_addSub((GuiItem*)layoutAdv, GuiItemComboStatic_newEx(Quad2i_init4(1, 0, 3, 1), AppGui_connectType2(appGui, row), DbValue_initLang(GUI_MAP_TYPE_OPTIONS), DbValue_initEmpty(), 0));
	
	//radius
	GuiItem_addSub((GuiItem*)layoutAdv, GuiItemText_new(Quad2i_init4(0, 2, 1, 3), FALSE, DbValue_initLang(GUI_RADIUS), DbValue_initEmpty()));
	GuiItem_addSub((GuiItem*)layoutAdv, GuiItemComboDynamic_newEx(Quad2i_init4(1, 2, 3, 1), FALSE, AppGui_connectLinkTableB(appGui, row), DbValue_initGET((DbColumn*)DbRoot_getColumnInfoName(), tableRow), GuiDbID_newArray(DbRoot_getTableInfo(), DbRoot_getColumnLinks(table)), DbValue_initLang(GUI_COLUMNS), &UiRootMap_clickPointsFocus));
	GuiItem_addSub((GuiItem*)layoutAdv, GuiItemEdit_newEx(Quad2i_init4(1, 3, 3, 2), AppGui_connectRadius(appGui, row), DbValue_initLang(GUI_MULTIPLIER), &UiRootMap_clickRebuild));
		
	
	//color
	GuiItem_addSub((GuiItem*)layoutAdv, GuiItemText_new(Quad2i_init4(0, 6, 1, 4), FALSE, DbValue_initLang(GUI_COLOR), DbValue_initEmpty()));
	GuiItem_addSub((GuiItem*)layoutAdv, (GuiItem*)GuiItemColor_new(Quad2i_init4(1, 6, 1, 1), AppGui_connectColorCdStart(appGui, row), FALSE));
	GuiItem_addSub((GuiItem*)layoutAdv, GuiItemText_new(Quad2i_init4(2, 6, 1, 1), TRUE, DbValue_initStaticCopy(_UNI32("=>")), DbValue_initEmpty()));
	GuiItem_addSub((GuiItem*)layoutAdv, (GuiItem*)GuiItemColor_new(Quad2i_init4(3, 6, 1, 1), AppGui_connectColorCdEnd(appGui, row), FALSE));
	
	GuiItem_addSub((GuiItem*)layoutAdv, GuiItemComboDynamic_newEx(Quad2i_init4(1, 7, 3, 1), FALSE, AppGui_connectLinkTableC(appGui, row), DbValue_initGET((DbColumn*)DbRoot_getColumnInfoName(), tableRow), GuiDbID_newArray(DbRoot_getTableInfo(), DbRoot_getColumnLinks(table)), DbValue_initLang(GUI_COLUMNS), &UiRootMap_clickPointsFocus));	
	
	GuiItem_addSub((GuiItem*)layoutAdv, GuiItemEdit_newEx(Quad2i_init4(1, 8, 1, 2), AppGui_connectColorValueStart(appGui, row), DbValue_initLang(GUI_START), &UiRootMap_clickRebuild));
	GuiItem_addSub((GuiItem*)layoutAdv, GuiItemEdit_newEx(Quad2i_init4(3, 8, 1, 2), AppGui_connectColorValueEnd(appGui, row), DbValue_initLang(GUI_END), &UiRootMap_clickRebuild));
	

	
	GuiItemMenu* advanced = (GuiItemMenu*)GuiItem_addSub((GuiItem*)layoutMenu, GuiItemMenu_new(Quad2i_init4(6, 0, 1, 1), DbValue_initLang(GUI_ADVANCED), FALSE));
	GuiItemMenu_setContext(advanced, layoutAdv);
	GuiItemMenu_setCenter(advanced, FALSE);
	}*/
	

	
	GuiItem_addSub((GuiItem*)layoutMenu, GuiItemButton_newClassicEx(Quad2i_init4(8, 0, 1, 1), DbValue_initLang(GUI_MAP_FOCUS), &UiRootMap_clickPointsFocus));
	
	
	}


//přidat ostatní connect ...
	
	GuiDbID* ids = (exeFilter && !DbFilter_isEmpty(exeFilter)) ? GuiDbID_newArray(table, StdBigs_initCopy(DbFilter_getRows(exeFilter))) : GuiDbID_newTable(table);
	GuiItem_addSubName((GuiItem*)layout, "map", GuiItemMap_new(Quad2i_init4(0, 1, 1, 1), ids,	AppGui_connectLinkTableA(appGui, row),
													AppGui_connectLatitude(appGui, row), AppGui_connectLongitude(appGui, row), AppGui_connectZoom(appGui, row),
													AppGui_connectType2(appGui, row), AppGui_connectWidth(App_getGui(), row), AppGui_connectLinkSub(App_getGui(), propTitlesRow), AppGui_connectEnable(App_getGui(), row)));
	

	
	DbFilter_delete(exeFilter);	
		
	return (GuiItem*)layout;
}

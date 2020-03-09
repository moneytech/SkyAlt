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

GuiItemLayout* UiRootTable_buildShortingList(UBIG row, const char* subType, const char* translationEnableAll, BOOL showAscending, BOOL showColor, DbRows columnsCombo, BOOL warningIfListEmpty)
{
	GuiItemLayout* layout = GuiItemLayout_new(Quad2i_init());
	GuiItemLayout_addColumn(layout, 0, 20);

	DbTable* table = DbRoot_findParentTable(row);
	if (table)
	{
		BIG groupRow = DbRows_findSubType(row, subType);
		GuiItem_setAttribute((GuiItem*)layout, "row", groupRow);

		int y = 0;
		//enable all
		if (translationEnableAll)
			GuiItem_addSubName((GuiItem*)layout, "enable", GuiItemCheck_new(Quad2i_init4(0, y++, 1, 1), DbValue_initOptionEnable(groupRow), DbValue_initLang(translationEnableAll)));

		GuiItemLayout_addRow(layout, y, 10);	//list

		//skin for list
		GuiItemLayout* skin = GuiItemLayout_new(Quad2i_init4(0, 0, 1, 1));
		GuiItem_setAttribute((GuiItem*)skin, "row", groupRow);
		{
			int x = 3;
			GuiItemLayout_addColumn(skin, 2, 10);
			if (showAscending)
				GuiItemLayout_addColumn(skin, x++, 10);
			if (showColor)
				GuiItemLayout_addColumn(skin, x++, 3);

			GuiItem* drag = GuiItem_addSubName((GuiItem*)skin, "drag", GuiItemBox_newEmpty(Quad2i_init4(0, 0, 1, 1)));
			GuiItem_setIcon(drag, GuiImage_new1(UiIcons_init_reoder()));
			//GuiItem_setDrop(drag, "short", "short", FALSE, DbRows_initLink1(DbRoot_ref(), groupRow), -1);
			GuiItem_setDrop(drag, "short", "short", FALSE, DbRows_initLinkN(DbRoot_subs(), -1), -1);

			GuiItem_addSubName((GuiItem*)skin, "on", GuiItemCheck_new(Quad2i_init4(1, 0, 1, 1), DbValue_initOptionEnable(-1), DbValue_initEmpty()));

			GuiItemComboDynamic* cbb = (GuiItemComboDynamic*)GuiItem_addSubName((GuiItem*)skin, "column", GuiItemComboDynamic_new(Quad2i_init4(2, 0, 1, 1), TRUE, DbRows_initLink1(DbRoot_ref(), -1), DbValue_initOption(-1, "name", 0), DbRows_getTable(&columnsCombo) ? columnsCombo : DbRows_initSubs(table, "columns", FALSE), DbValue_initEmpty()));
			GuiItem_setEnableCallback((GuiItem*)cbb, &GuiItem_enableEnableAttr);

			x = 3;
			if (showAscending)
			{
				GuiItem* order = GuiItem_addSubName((GuiItem*)skin, "ascending", GuiItemComboStatic_new(Quad2i_init4(x++, 0, 1, 1), DbValue_initOption(-1, "ascending", 0), Lang_find("LANGUAGE_ORDER"), DbValue_initEmpty()));
				GuiItem_setEnableCallback((GuiItem*)order, &GuiItem_enableEnableAttr);
			}

			if (showColor)
				GuiItem_addSubName((GuiItem*)skin, "cd", (GuiItem*)GuiItemColor_new(Quad2i_init4(x++, 0, 1, 1), DbValue_initOption(-1, "cd", 0), FALSE));
		}

		//list
		GuiItemList* list = (GuiItemList*)GuiItem_addSubName((GuiItem*)layout, "list", GuiItemList_new(Quad2i_init4(0, y++, 1, 1), DbRows_initLinkN(DbRoot_subs(), groupRow), (GuiItem*)skin, DbValue_initEmpty()));
		GuiItemList_setShowRemove(list, TRUE);
		GuiItemList_setShowBorder(list, FALSE);
		GuiItemList_setShowWarningIfEmpty(list, warningIfListEmpty);

		//add
		GuiItemButton* add = (GuiItemButton*)GuiItem_addSubName((GuiItem*)layout, "+", GuiItemButton_newClassicEx(Quad2i_init4(0, y++, 1, 1), DbValue_initStaticCopy(_UNI32("+")), &GuiItemTable_clickAddSubLine));
		GuiItemButton_setWarningCd(add, (warningIfListEmpty && !DbRows_hasFilterSubActive(row, subType)));

		GuiItem_setEnableCallback((GuiItem*)add, &GuiItem_enableEnableAttr);
		GuiItem_setEnableCallback((GuiItem*)list, &GuiItem_enableEnableAttr);
		//GuiItem_setEnableMsg((GuiItem*)add, DbValue_initOptionEnable(groupRow), FALSE);
		//GuiItem_setEnableMsg((GuiItem*)list, DbValue_initOptionEnable(groupRow), FALSE);
	}

	return layout;
}

void UiRootTable_clickSelectRemove(GuiItem* item)
{
	BIG row = GuiItem_getRow(item);
	DbRoot_removeRow(row);
}

void UiRootTable_rebuildShortingLayout(GuiItem* item)
{
	GuiItem_freeSubs(item);
	GuiItemLayout* layout = (GuiItemLayout*)item;
	//GuiItemLayout* layout = GuiItemLayout_new(Quad2i_init());
	GuiItemLayout_addColumn(layout, 0, 2);
	GuiItemLayout_addColumn(layout, 1, 18);

	BIG row = GuiItem_findAttribute(item, "row");
	//BIG groupRow = GuiItem_findAttribute(item, "groupRow");
	BIG shortRow = DbRows_findSubType(row, "short");

	DbTable* table = DbRoot_findParentTable(row);
	if (table)
	{
		//BIG groupRow = DbRows_findSubType(row, subType);
		GuiItem_setAttribute((GuiItem*)layout, "row", shortRow);

		int y = 0;
		//enable all
		GuiItem_addSubName((GuiItem*)layout, "enable", GuiItemCheck_new(Quad2i_init4(0, y++, 2, 1), DbValue_initOptionEnable(shortRow), DbValue_initLang("SHORT_ENABLE")));

		//GuiItemLayout_addRow(layout, y, 10);	//list

		UBIG i = 0;
		BIG it;
		while ((it = DbColumnN_jump(DbRoot_subs(), shortRow, &i, 1)) >= 0)
		{
			//skin
			GuiItemLayout* skin = GuiItemLayout_new(Quad2i_init4(0, y++, 2, 1));
			GuiItem_setAttribute((GuiItem*)skin, "row", it);
			GuiItemLayout_addColumn(skin, 2, 10);
			GuiItemLayout_addColumn(skin, 3, 10);

			//drag
			GuiItem* drag = GuiItem_addSubName((GuiItem*)skin, "drag", GuiItemBox_newEmpty(Quad2i_init4(0, 0, 1, 1)));
			GuiItem_setRow(drag, it, 0);
			GuiItem_setIcon(drag, GuiImage_new1(UiIcons_init_reoder()));
			GuiItem_setDrop(drag, "short", "short", FALSE, DbRows_initLinkN(DbRoot_subs(), shortRow), -1);

			//on/off
			GuiItem_addSubName((GuiItem*)skin, "on", GuiItemCheck_new(Quad2i_init4(1, 0, 1, 1), DbValue_initOptionEnable(-1), DbValue_initEmpty()));

			//column
			GuiItemComboDynamic* cbb = (GuiItemComboDynamic*)GuiItem_addSubName((GuiItem*)skin, "column", GuiItemComboDynamic_new(Quad2i_init4(2, 0, 1, 1), TRUE, DbRows_initLink1(DbRoot_ref(), -1), DbValue_initOption(-1, "name", 0), DbRows_initSubs(table, "columns", FALSE), DbValue_initEmpty()));
			GuiItem_setEnableCallback((GuiItem*)cbb, &GuiItem_enableEnableAttr);

			//ascending
			GuiItem* order = GuiItem_addSubName((GuiItem*)skin, "ascending", GuiItemComboStatic_new(Quad2i_init4(3, 0, 1, 1), DbValue_initOption(-1, "ascending", 0), Lang_find("LANGUAGE_ORDER"), DbValue_initEmpty()));
			GuiItem_setEnableCallback((GuiItem*)order, &GuiItem_enableEnableAttr);

			//remove
			GuiItem_addSubName((GuiItem*)skin, "remove", GuiItemButton_newClassicEx(Quad2i_init4(4, 0, 1, 1), DbValue_initStaticCopyCHAR("X"), &GuiItemTable_clickSelectRemove));

			GuiItem_setRow((GuiItem*)skin, it, 0);
			GuiItem_setEnableCallback((GuiItem*)skin, &GuiItem_enableEnableParentAttr);
			char nameId[64];
			snprintf(nameId, 64, "%lld_line", i);
			GuiItem_addSubName((GuiItem*)layout, nameId, (GuiItem*)skin);

			i++;
		}

		//add
		GuiItemButton* add = (GuiItemButton*)GuiItem_addSubName((GuiItem*)layout, "+", GuiItemButton_newBlackEx(Quad2i_init4(0, y++, 1, 1), DbValue_initStaticCopy(_UNI32("+")), &GuiItemTable_clickAddSubLine));
		//GuiItemButton_setWarningCd(add, (warningIfListEmpty && !DbRows_hasFilterSubActive(row, subType)));
		GuiItem_setEnableCallback((GuiItem*)add, &GuiItem_enableEnableAttr);
	}
}
GuiItemLayout* UiRootTable_buildShortingLayout(Quad2i grid, BIG row)
{
	GuiItemLayout* layout = GuiItemLayout_new(grid);
	GuiItem_setAttribute((GuiItem*)layout, "row", row);
	//GuiItem_setAttribute((GuiItem*)layout, "groupRow", DbRows_findSubType(row, subType));
	GuiItemLayout_setResize(layout, &UiRootTable_rebuildShortingLayout);
	return layout;
}

GuiItem* UiRootTable_build(GuiItemLayout* layout, UBIG row, DbValue scrollV, DbValue scrollH, DbValue selectGrid, DbValue search)
{
	BIG thisRow = row;
	//if (DbRoot_isTypeViewReference(row))
	//	row = DbRoot_getOrigReference(row);

	BOOL isFilter = !DbRoot_isType_table(row);

	GuiItemLayout_addColumn(layout, 0, 99);
	GuiItemLayout_addRow(layout, 1, 99);

	//header
	GuiItemLayout* layoutMenu = GuiItemLayout_new(Quad2i_init4(0, 0, 1, 1));
	GuiItemLayout_setDrawBackground(layoutMenu, FALSE);
	GuiItemLayout_addColumn(layoutMenu, 0, 6);
	//GuiItemLayout_addColumn(layoutMenu, 1, 99);	//big space
	GuiItemLayout_addColumn(layoutMenu, 2, 4);
	GuiItemLayout_addColumn(layoutMenu, 4, 4);
	GuiItemLayout_addColumn(layoutMenu, 6, 4);
	GuiItemLayout_addColumn(layoutMenu, 8, 4);
	GuiItemLayout_addColumn(layoutMenu, 9, 99);	//big space
	GuiItemLayout_addColumn(layoutMenu, 10, 4);
	GuiItemLayout_addColumn(layoutMenu, 13, 2);
	GuiItem_addSubName((GuiItem*)layout, "menu", (GuiItem*)layoutMenu);

	//name
	GuiItem_addSubName((GuiItem*)layoutMenu, "header", UiRoot_createMenuNameHeader(Quad2i_init4(0, 0, 1, 1), thisRow));

	if (!DbRoot_isReference(row))
	{
		//Row Height
		GuiItem* rowH = GuiItem_addSubName((GuiItem*)layoutMenu, "height", GuiItemComboStatic_newEx(Quad2i_init4(2, 0, 1, 1), DbValue_initOption(row, "height", 0), Lang_find("TABLE_STYLE_OPTIONS"), DbValue_initEmpty(), 0));
		GuiItem_setIcon(rowH, GuiImage_new1(UiIcons_init_table_row_height()));

		//Hidden
		GuiItemMenu* columns = (GuiItemMenu*)GuiItem_addSubName((GuiItem*)layoutMenu, "columns", GuiItemMenu_new(Quad2i_init4(4, 0, 1, 1), DbValue_initLang("COLUMNS"), FALSE));
		GuiItemMenu_setContext(columns, GuiItemTable_buildShowedList(Quad2i_init(), row));
		GuiItemMenu_setTransparent(columns, FALSE);
		GuiItemMenu_setHighligthBackground(columns, DbRows_hasColumnsSubDeactive(row, "columns"), 0.5f);
		GuiItem_setIcon((GuiItem*)columns, GuiImage_new1(UiIcons_init_table_hide()));
		GuiItemMenu_setCenter(columns, FALSE);

		//Short
		GuiItemMenu* shortt = (GuiItemMenu*)GuiItem_addSubName((GuiItem*)layoutMenu, "short_menu", GuiItemMenu_new(Quad2i_init4(6, 0, 1, 1), DbValue_initLang("SHORT"), FALSE));
		GuiItemMenu_setContext(shortt, UiRootTable_buildShortingLayout(Quad2i_init(), row));
		GuiItemMenu_setTransparent(shortt, FALSE);
		GuiItemMenu_setHighligthBackground(shortt, DbRows_hasFilterSubActive(row, "short"), 0.5f);
		GuiItem_setIcon((GuiItem*)shortt, GuiImage_new1(UiIcons_init_table_short()));
		GuiItemMenu_setCenter(shortt, FALSE);

		//Filter
		if (isFilter)
		{
			GuiItemMenu* filterr = (GuiItemMenu*)GuiItem_addSubName((GuiItem*)layoutMenu, "filter_menu", GuiItemMenu_new(Quad2i_init4(8, 0, 1, 1), DbValue_initLang("FILTER"), FALSE));
			GuiItemMenu_setContext(filterr, GuiItemTable_buildSelectList(Quad2i_init(), row, DbRoot_findParentTable(row)));
			GuiItemMenu_setTransparent(filterr, FALSE);
			GuiItemMenu_setHighligthBackground(filterr, DbRows_hasFilterSubActive(row, "select"), 0.5f);
			GuiItem_setIcon((GuiItem*)filterr, GuiImage_new1(UiIcons_init_table_filter()));
			GuiItemMenu_setCenter(filterr, FALSE);
		}
	}

	//Search
	{
		GuiItemEdit* searchEdit = (GuiItemEdit*)GuiItem_addSubName((GuiItem*)layoutMenu, "search", GuiItemEdit_newEx(Quad2i_init4(10, 0, 1, 1), search, DbValue_initLang("SEARCH"), &GuiItemTable_clickFindForward));
		GuiItemEdit_setHighlightIfContent(searchEdit, TRUE);
		GuiItem_setIcon((GuiItem*)searchEdit, GuiImage_new1(UiIcons_init_search()));
		GuiItem_setShortcutKey((GuiItem*)searchEdit, FALSE, Win_EXTRAKEY_CTRL | Win_EXTRAKEY_SEARCH, 0, &GuiItemEdit_clickActivate);

		GuiItem_addSubName((GuiItem*)layoutMenu, "search_backward", GuiItemButton_newAlphaEx(Quad2i_init4(11, 0, 1, 1), DbValue_initStaticCopy(_UNI32("↑")), &GuiItemTable_clickFindBackward));
		GuiItem_addSubName((GuiItem*)layoutMenu, "search_forward", GuiItemButton_newAlphaEx(Quad2i_init4(12, 0, 1, 1), DbValue_initStaticCopy(_UNI32("↓")), &GuiItemTable_clickFindForward));
	}

	//replace
	{
		GuiItemMenu* replace = (GuiItemMenu*)GuiItem_addSubName((GuiItem*)layoutMenu, "replace", GuiItemMenu_new(Quad2i_init4(13, 0, 1, 1), DbValue_initLang("REPLACE"), FALSE));
		GuiItemMenu_setTransparent(replace, FALSE);
		GuiItemLayout* lay = GuiItemLayout_new(Quad2i_init());
		GuiItemLayout_addColumn(lay, 0, 5);
		GuiItemLayout_addColumn(lay, 1, 5);

		GuiItem_addSubName((GuiItem*)lay, "find_str", GuiItemEdit_new(Quad2i_init4(0, 0, 2, 1), DbValue_initOption(row, "find_str", 0), DbValue_initLang("FIND")));
		GuiItem_addSubName((GuiItem*)lay, "replace_str", GuiItemEdit_new(Quad2i_init4(0, 1, 2, 1), DbValue_initOption(row, "replace_str", 0), DbValue_initLang("REPLACE")));

		GuiItem_addSubName((GuiItem*)lay, "replace_cell", GuiItemCheck_new(Quad2i_init4(0, 2, 2, 1), DbValue_initOption(row, "replace_cell", 0), DbValue_initLang("REPLACE_CELL")));

		GuiItem_addSubName((GuiItem*)lay, "button_replace_one", GuiItemButton_newClassicEx(Quad2i_init4(0, 4, 1, 1), DbValue_initLang("REPLACE"), &GuiItemTable_clickFindReplaceOne));
		GuiItem_addSubName((GuiItem*)lay, "button_replace_all", GuiItemButton_newClassicEx(Quad2i_init4(1, 4, 1, 1), DbValue_initLang("REPLACE_ALL"), &GuiItemTable_clickFindReplaceAll));

		GuiItemMenu_setContext(replace, lay);
		//GuiItemMenu_setCloseAuto(replace, FALSE);
	}

	//Table
	{
		DbRows filter = DbRows_initFilter(thisRow);
		GuiItemTable* tableG = GuiItemTable_new(Quad2i_init4(0, 1, 1, 1), row, filter, TRUE, TRUE, scrollV, scrollH, selectGrid, search);
		GuiItem_addSubName((GuiItem*)layout, "table", (GuiItem*)tableG);
	}

	return (GuiItem*)layout;
}

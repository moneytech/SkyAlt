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

GuiItem* UiRootSummary_build(GuiItemLayout* layout, UBIG row, DbValue scrollV, DbValue scrollH, DbValue selectGrid, DbValue search)
{
	DbTable* origTable = DbRoot_findParentTable(DbRoot_findParent(row));
	//DbTable* table = DbRoot_findParentTable(row);

	BIG thisRow = row;
	//if (DbRoot_isTypeViewReference(row))
	//	row = DbRoot_getOrigReference(row);

	GuiItemLayout_addColumn(layout, 0, 99);
	GuiItemLayout_addRow(layout, 1, 99);

	//header
	GuiItemLayout* layoutMenu = GuiItemLayout_new(Quad2i_init4(0, 0, 1, 1));
	GuiItemLayout_setDrawBackground(layoutMenu, FALSE);
	GuiItemLayout_addColumn(layoutMenu, 0, 6);
	//GuiItemLayout_addColumn(layoutMenu, 1, 99);	//big space
	GuiItemLayout_addColumn(layoutMenu, 2, 4);
	GuiItemLayout_addColumn(layoutMenu, 4, 4);
	GuiItemLayout_addColumn(layoutMenu, 6, 6);
	GuiItemLayout_addColumn(layoutMenu, 8, 99);	//big space
	GuiItemLayout_addColumn(layoutMenu, 9, 4);
	GuiItemLayout_addColumn(layoutMenu, 10, 4);
	GuiItem_addSubName((GuiItem*)layout, "menu", (GuiItem*)layoutMenu);

	//name
	GuiItem_addSubName((GuiItem*)layoutMenu, "header", UiRoot_createMenuNameHeader(Quad2i_init4(0, 0, 1, 1), thisRow));

	if (!DbRoot_isReference(row))
	{
		//Hidden
		GuiItemMenu* columns = (GuiItemMenu*)GuiItem_addSubName((GuiItem*)layoutMenu, "columns", GuiItemMenu_new(Quad2i_init4(2, 0, 1, 1), DbValue_initLang("COLUMNS"), FALSE));
		GuiItemMenu_setContext(columns, GuiItemTable_buildShowedList(Quad2i_init(), row));
		GuiItemMenu_setHighligthBackground(columns, DbRows_hasColumnsSubDeactive(row, "columns"), 0.5f);
		GuiItemMenu_setTransparent(columns, FALSE);
		GuiItem_setIcon((GuiItem*)columns, GuiImage_new1(UiIcons_init_table_hide()));
		GuiItemMenu_setCenter(columns, FALSE);

		//Short
		BOOL hasShort = DbRows_hasFilterSubActive(row, "short");
		GuiItemMenu* shortt = (GuiItemMenu*)GuiItem_addSubName((GuiItem*)layoutMenu, "short", GuiItemMenu_new(Quad2i_init4(4, 0, 1, 1), DbValue_initLang("SHORT"), FALSE));
		GuiItemMenu_setContext(shortt, UiRootTable_buildShortingLayout(Quad2i_init(), row));
		GuiItemMenu_setTransparent(shortt, FALSE);
		GuiItemMenu_setHighligthBackground(shortt, hasShort, 0.5f);
		GuiItem_setIcon((GuiItem*)shortt, GuiImage_new1(UiIcons_init_table_short()));
		GuiItemMenu_setCenter(shortt, FALSE);

		//Group by
		{
			BIG groupColumnRow = DbColumnN_getFirstRow(DbRoot_subs(), DbRows_findOrCreateSubType(row, "group"));
			///BIG groupColumnRow = DbRoot_ref_row(DbRows_findOrCreateSubType(row, "group"));
			GuiItem_addSubName((GuiItem*)layoutMenu, "group_by", GuiItemComboDynamic_new(Quad2i_init4(6, 0, 1, 1), TRUE, DbRows_initLink1(DbRoot_ref(), groupColumnRow), DbValue_initOption(-1, "name", 0), DbRows_initSubs(origTable, "columns", FALSE), DbValue_initLang("GROUP_BY")));
		}

		//Row Height
		GuiItem* rowH = GuiItem_addSubName((GuiItem*)layoutMenu, "height", GuiItemComboStatic_newEx(Quad2i_init4(9, 0, 1, 1), DbValue_initOption(row, "height", 0), Lang_find("TABLE_STYLE_OPTIONS"), DbValue_initEmpty(), 0));
		GuiItem_setIcon(rowH, GuiImage_new1(UiIcons_init_table_row_height()));

		//Search
		/*{
			GuiItemEdit* searchEdit = (GuiItemEdit*)GuiItem_addSubName((GuiItem*)layoutMenu, "search", GuiItemEdit_newEx(Quad2i_init4(10, 0, 1, 1), search, DbValue_initLang("SEARCH"), 0));
			GuiItemEdit_setHighlightIfContent(searchEdit, TRUE);
			GuiItem_setIcon((GuiItem*)searchEdit, GuiImage_new1(UiIcons_init_search()));
			GuiItem_setShortcutKey((GuiItem*)searchEdit, FALSE, Win_EXTRAKEY_CTRL | Win_EXTRAKEY_SEARCH, 0, &GuiItemEdit_clickActivate);
		}*/

		//create filter
		//DbRows filter = DbRows_initFilter(thisRow);
		//DbRows_forceEmptyFilter(&filter);
		{
			/*DbValue_hasChanged(&search);
			const UNI* searchStr = DbValue_result(&search);
			if (Std_sizeUNI(searchStr))
				DbRows_addSearchFilter(&filter, table, searchStr);*/
		}
	}

	//Table
	{
		//DbRows filter = DbRows_initFilterEx(thisRow, DbRoot_findParentTable(DbRoot_findParent(row)));
		//DbRows_forceEmptyFilter(&filter);
		//DbRows_hasChanged(&filter);	//execute filter
		//DbTable* newTable = DbRoot_fillSummaryTable(row, filter.filter);

		DbRows finalFilter = DbRows_initFilter(thisRow);

		//short
		//if (hasShort)
		{
			//DbFilter_clearGroups(finalFilter.filter);

			//DbFilter* f = DbFilter_new("x", newTable);
			//DbFilter_addShortCopy(f, filter.filter);
			//DbRows_addFilter(&finalFilter, f);
		}

		//search
		//DbValue_hasChanged(&search);
		//const UNI* searchStr = DbValue_result(&search);
		//if (Std_sizeUNI(searchStr))
		//	DbRows_addSearchFilter(&finalFilter, newTable, searchStr);

		GuiItemTable* tableG = GuiItemTable_new(Quad2i_init4(0, 1, 1, 1), row, finalFilter, TRUE, TRUE, scrollV, scrollH, selectGrid, search);
		//GuiItemTable_setModeSummary(tableG, TRUE);
		GuiItem_addSubName((GuiItem*)layout, "table", (GuiItem*)tableG);
	}

	return (GuiItem*)layout;
}

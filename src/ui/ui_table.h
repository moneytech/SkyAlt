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

void UiRoot_clickAddSubLine(GuiItem* item)
{
	BIG propRow = GuiItem_findAttribute(item, "row");

	DbRows rows = DbRows_initLink(DbRoot_getColumnSubs(), propRow);
	DbRows_addNewRow(&rows);

	DbRows_free(&rows);
}

static void _UiRootTable_setPageShow(BIG row, BOOL show)
{
	DbValues values = DbRows_getOptions(row, "columns", "enable", FALSE);

	BIG i;
	for (i = 0; i < values.num; i++)
		DbValue_setNumber(&values.values[i], show);

	DbValues_free(&values);
}

void UiRootTable_clickPageShowAll(GuiItem* item)
{
	_UiRootTable_setPageShow(GuiItem_findAttribute(item, "row"), TRUE);
}

void UiRootTable_clickPageHideAll(GuiItem* item)
{
	_UiRootTable_setPageShow(GuiItem_findAttribute(item, "row"), FALSE);
}

GuiItemLayout* UiRootTable_buildShowedList(BIG row, BOOL isFilter)
{
	BIG columnsRow = DbRows_findSubType(row, "columns");

	GuiItemLayout* layColumn = GuiItemLayout_new(Quad2i_init());
	GuiItemLayout_addColumn(layColumn, 0, 10);
	GuiItemLayout_addRow(layColumn, 0, 10);	//list

	GuiItem_setAttribute((GuiItem*)layColumn, "row", row);

	GuiItemLayout* skin = GuiItemLayout_new(Quad2i_init4(0, 0, 1, 1));
	GuiItemLayout_addColumn(skin, 2, 99);

	GuiItem* drag = GuiItem_addSubName((GuiItem*)skin, "drag", GuiItemBox_newEmpty(Quad2i_init4(0, 0, 1, 1)));
	GuiItem_setIcon(drag, GuiImage_new1(UiIcons_init_reoder()));
	GuiItem_setDrop(drag, "columns", "columns", FALSE, DbRows_initLink(DbRoot_getColumnSubs(), columnsRow), -1);

	GuiItem_addSubName((GuiItem*)skin, "on", GuiItemCheck_newEx(Quad2i_init4(1, 0, 1, 1), DbValue_initOptionEnable(-1), DbValue_initEmpty(), 0));

	if (!isFilter)
	{
		GuiItem* name = GuiItem_addSubName((GuiItem*)skin, "name", GuiItemText_new(Quad2i_init4(2, 0, 1, 1), FALSE, DbValue_initOption(-1, "name", 0), DbValue_initEmpty()));
		GuiItem_setEnableMsg(name, DbValue_initOptionEnable(-1), FALSE);
	}
	else
	{
		GuiItem* skinName = GuiItemText_new(Quad2i_init4(2, 0, 1, 1), FALSE, DbValue_initOption(-1, "name", 0), DbValue_initEmpty());
		GuiItemList* listName = (GuiItemList*)GuiItem_addSubName((GuiItem*)skin, "list", GuiItemList_new(Quad2i_init4(2, 0, 1, 1), DbRows_initLink(DbRoot_getColumnSubs(), -1), skinName, DbValue_initEmpty()));
		GuiItemList_setShowBorder(listName, FALSE);
		GuiItemList_setShowScroll(listName, FALSE);
		GuiItem_setEnableMsg((GuiItem*)listName, DbValue_initOptionEnable(-1), FALSE);
	}

	GuiItemList* list = (GuiItemList*)GuiItem_addSubName((GuiItem*)layColumn, "list", GuiItemList_new(Quad2i_init4(0, 0, 1, 1), DbRows_initLink(DbRoot_getColumnSubs(), columnsRow), (GuiItem*)skin, DbValue_initEmpty()));
	GuiItemList_setShowBorder(list, FALSE);
	GuiItem_setCallClick((GuiItem*)list, 0);

	//Hide all
	GuiItem_addSubName((GuiItem*)layColumn, "hideAll", GuiItemButton_newClassicEx(Quad2i_init4(0, 1, 1, 1), DbValue_initLang("HIDE_ALL"), &UiRootTable_clickPageHideAll));

	//Show All
	GuiItem_addSubName((GuiItem*)layColumn, "showAll", GuiItemButton_newClassicEx(Quad2i_init4(0, 2, 1, 1), DbValue_initLang("SHOW_ALL"), &UiRootTable_clickPageShowAll));

	return layColumn;
}

GuiItemLayout* UiRootTable_buildShortingList(UBIG row, BOOL group)
{
	GuiItemLayout* layout = GuiItemLayout_new(Quad2i_init());
	GuiItemLayout_addColumn(layout, 0, 20);
	GuiItemLayout_addRow(layout, 2, 10);	//list

	DbTable* table = DbRoot_findParentTable(row);
	if (table)
	{
		BIG groupRow = DbRows_findSubType(row, group ? "group" : "short");

		//enable all
		GuiItem_addSubName((GuiItem*)layout, "enable", GuiItemCheck_new(Quad2i_init4(0, 0, 1, 1), DbValue_initOptionEnable(groupRow), DbValue_initLang(group ? "GROUP_ENABLE" : "SHORT_ENABLE")));

		//add
		GuiItemButton* add = (GuiItemButton*)GuiItem_addSubName((GuiItem*)layout, "+", GuiItemButton_newClassicEx(Quad2i_init4(0, 1, 1, 1), DbValue_initStaticCopy(_UNI32("+")), &UiRoot_clickAddSubLine));
		GuiItem_setAttribute((GuiItem*)add, "row", groupRow);

		//skin for list
		GuiItemLayout* skin = GuiItemLayout_new(Quad2i_init4(0, 0, 1, 1));
		GuiItemLayout_addColumn(skin, 2, 10);
		GuiItemLayout_addColumn(skin, 3, 10);

		GuiItem* drag = GuiItem_addSubName((GuiItem*)skin, "drag", GuiItemBox_newEmpty(Quad2i_init4(0, 0, 1, 1)));
		GuiItem_setIcon(drag, GuiImage_new1(UiIcons_init_reoder()));
		GuiItem_setDrop(drag, "short", "short", FALSE, DbRows_initLink(DbRoot_getColumnSubs(), groupRow), -1);

		GuiItem_addSubName((GuiItem*)skin, "on", GuiItemCheck_new(Quad2i_init4(1, 0, 1, 1), DbValue_initOptionEnable(-1), DbValue_initEmpty()));

		GuiItemComboDynamic* cbb = (GuiItemComboDynamic*)GuiItem_addSubName((GuiItem*)skin, "column", GuiItemComboDynamic_new(Quad2i_init4(2, 0, 1, 1), FALSE, DbRows_initLink(DbRoot_getColumnSubs(), -1), DbValue_initOption(-1, "name", 0), DbRows_initSubs(table, "columns", FALSE), DbValue_initEmpty()));
		GuiItem* order = GuiItem_addSubName((GuiItem*)skin, "ascending", GuiItemComboStatic_new(Quad2i_init4(3, 0, 1, 1), DbValue_initOption(-1, "ascending", 0), Lang_find("LANGUAGE_ORDER"), DbValue_initEmpty()));
		GuiItem_setEnableMsg((GuiItem*)cbb, DbValue_initOptionEnable(-1), FALSE);
		GuiItem_setEnableMsg(order, DbValue_initOptionEnable(-1), FALSE);

		//list
		GuiItemList* list = (GuiItemList*)GuiItem_addSubName((GuiItem*)layout, "list", GuiItemList_new(Quad2i_init4(0, 2, 1, 1), DbRows_initLink(DbRoot_getColumnSubs(), groupRow), (GuiItem*)skin, DbValue_initEmpty()));
		GuiItemList_setShowRemove(list, TRUE);
		GuiItemList_setShowBorder(list, FALSE);

		GuiItem_setEnableMsg((GuiItem*)add, DbValue_initOptionEnable(groupRow), FALSE);
		GuiItem_setEnableMsg((GuiItem*)list, DbValue_initOptionEnable(groupRow), FALSE);
	}

	return layout;
}

void UiRootTable_clickRemove(GuiItem* item)
{
	BIG row = GuiItem_getRow(item);
	DbRoot_removeRow(row);
}


char* _UiRootTable_getNameId(char* nameId, const char* format, BIG value)
{
	snprintf(nameId, 64, format, value);
	return nameId;
}

void UiRootTable_rebuildSelectList(GuiItem* item)
{
	GuiItem_freeSubs(item);

	GuiItemLayout* lines = (GuiItemLayout*)item;
	BIG selectRow = GuiItem_findAttribute(item, "row");

	DbTable* table = DbRoot_findParentTable(selectRow);

	GuiItemLayout_addColumn(lines, 2, 3);
	GuiItemLayout_addColumn(lines, 3, 10);
	GuiItemLayout_addColumn(lines, 4, 4);
	GuiItemLayout_addColumn(lines, 5, 15);

	int y = 0;
	UBIG i = 0;
	BIG it;
	while ((it = DbColumnN_jump(DbRoot_getColumnSubs(), selectRow, &i, 1)) >= 0)
	{
		char nameId[64];

		//drag
		GuiItem* drag = GuiItem_addSubName((GuiItem*)lines, _UiRootTable_getNameId(nameId, "%lld_drag", i), GuiItemBox_newEmpty(Quad2i_init4(0, y, 1, 1)));
		GuiItem_setRow(drag, it, 0);
		GuiItem_setIcon(drag, GuiImage_new1(UiIcons_init_reoder()));
		GuiItem_setDrop(drag, "select", "select", FALSE, DbRows_initLink(DbRoot_getColumnSubs(), selectRow), -1);

		//on/off
		GuiItem_addSubName((GuiItem*)lines, _UiRootTable_getNameId(nameId, "%lld_on", i), GuiItemCheck_new(Quad2i_init4(1, y, 1, 1), DbValue_initOptionEnable(it), DbValue_initEmpty()));

		//and/or
		GuiItem* andOr = 0;
		if (y)
			andOr = GuiItem_addSubName((GuiItem*)lines, _UiRootTable_getNameId(nameId, "%lld_ascending", i), GuiItemComboStatic_new(Quad2i_init4(2, y, 1, 1), DbValue_initOption(it, "ascending", 0), Lang_find("BOOLEAN_OPTIONS"), DbValue_initEmpty()));

		//column
		DbRows rows = DbRows_initLink(DbRoot_getColumnSubs(), DbRoot_findOrCreateChildType(it, "column"));
		BIG columnRow = DbRows_getRow(&rows, 0);
		DbColumn* column = DbRoot_findColumn(columnRow);
		GuiItem* cbb = GuiItem_addSubName((GuiItem*)lines, _UiRootTable_getNameId(nameId, "%lld_name", i), GuiItemComboDynamic_new(Quad2i_init4(3, y, 1, 1), FALSE, rows, DbValue_initOption(-1, "name", 0), DbRows_initSubs(table, "columns", TRUE), DbValue_initEmpty()));

		GuiItem* type = 0;
		GuiItem* edit = 0;
		//funcType
		if (column)
		{
			DbValue tp = DbValue_initOption(it, "func", 0);
			const int fnIndex = DbValue_getNumber(&tp);
			UNI* options = DbFilterSelectFunc_getList(DbColumnFormat_findColumn(column));
			type = GuiItem_addSubName((GuiItem*)lines, _UiRootTable_getNameId(nameId, "%lld_func", i), GuiItemComboStatic_new(Quad2i_init4(4, y, 1, 1), tp, options, DbValue_initEmpty()));
			Std_deleteUNI(options);

			const char* funcName = DbFilterSelectFunc_getName(DbColumnFormat_findColumn(column), fnIndex);
			if (!Std_cmpCHAR(funcName, "FILTER_FUNC_EMPTY") && !Std_cmpCHAR(funcName, "FILTER_FUNC_NOT_EMPTY"))
			{
				Quad2i editGrid = Quad2i_init4(5, y, 1, 1);

				DbFormatTYPE format = DbColumnFormat_findColumn(column);
				if (format == DbFormat_MENU)
				{
					BIG optionsRow = DbRows_findSubType(columnRow, "options");
					edit = GuiItemComboDynamic_new(editGrid, FALSE, DbRows_initLink(DbRoot_getColumnSubs(), DbRoot_findOrCreateChildType(it, "option")), DbValue_initOption(-1, "name", 0), DbRows_initLink(DbRoot_getColumnSubs(), optionsRow), DbValue_initEmpty());
				}
				else
					if (format == DbFormat_TAGS)
					{
						BIG optionsRow = DbRows_findSubType(columnRow, "options");
						edit = GuiItemComboDynamic_new(editGrid, FALSE, DbRows_initLink(DbRoot_getColumnSubs(), DbRoot_findOrCreateChildType(it, "option")), DbValue_initOption(-1, "name", 0), DbRows_initLink(DbRoot_getColumnSubs(), optionsRow), DbValue_initEmpty());
					}
					else
						if (format == DbFormat_DATE)
						{
							//copy timeFormat from Column settings
							const OsDateTimeTYPE timeFormat = DbValue_getOptionNumber(DbColumn_getRow(column), "timeFormat", 0);
							DbValue_setOptionNumber(it, "timeFormat", timeFormat);

							DbValue v = DbValue_initOption(it, "value", 0);
							v.optionFormat = DbFormat_DATE;
							edit = GuiItemButton_newWhiteEx(editGrid, v, &GuiItemTable_clickSelectCalendar);
							GuiItem_setAttribute(edit, "timeFormat", timeFormat);
						}
						else
							if (format == DbFormat_CHECK)
							{
								edit = GuiItemCheck_new(editGrid, DbValue_initOption(it, "value", 0), DbValue_initEmpty());
							}
							else
								edit = GuiItemEdit_new(editGrid, DbValue_initOption(it, "value", 0), DbValue_initEmpty());
			}
			if (edit)
				GuiItem_addSubName((GuiItem*)lines, _UiRootTable_getNameId(nameId, "%lld_value", i), edit);
		}

		//remove
		GuiItem* remove = GuiItem_addSubName((GuiItem*)lines, _UiRootTable_getNameId(nameId, "%lld_remove", i), GuiItemButton_newClassicEx(Quad2i_init4(6, y, 1, 1), DbValue_initStaticCopyCHAR("X"), &UiRootTable_clickRemove));
		GuiItem_setRow(remove, it, 0);

		if (andOr)	GuiItem_setEnableMsg(andOr, DbValue_initOptionEnable(it), FALSE);
		if (cbb)	GuiItem_setEnableMsg(cbb, DbValue_initOptionEnable(it), FALSE);
		if (type)	GuiItem_setEnableMsg(type, DbValue_initOptionEnable(it), FALSE);
		if (edit)	GuiItem_setEnableMsg(edit, DbValue_initOptionEnable(it), FALSE);

		y++;
		i++;
	}

	GuiItemLayout_addRow((GuiItemLayout*)GuiItem_getParent(item), 2, y + 1);
}

GuiItemLayout* UiRootTable_buildSelectList(UBIG row)
{
	GuiItemLayout* layout = GuiItemLayout_new(Quad2i_init());
	GuiItemLayout_addColumn(layout, 0, 30);

	DbTable* table = DbRoot_findParentTable(row);
	if (table)
	{
		BIG selectRow = DbRows_findSubType(row, "select");
		GuiItem_setAttribute((GuiItem*)layout, "row", selectRow);

		//enable all
		GuiItem_addSubName((GuiItem*)layout, "on", GuiItemCheck_new(Quad2i_init4(0, 0, 1, 1), DbValue_initOptionEnable(selectRow), DbValue_initLang("SELECT_ENABLE")));

		//add
		GuiItemButton* add = (GuiItemButton*)GuiItem_addSubName((GuiItem*)layout, "+", GuiItemButton_newClassicEx(Quad2i_init4(0, 1, 1, 1), DbValue_initStaticCopy(_UNI32("+")), &UiRoot_clickAddSubLine));

		//lines
		GuiItemLayout* lines = GuiItemLayout_new(Quad2i_init4(0, 2, 1, 1));
		GuiItem_addSubName((GuiItem*)layout, "list", (GuiItem*)lines);
		GuiItemLayout_setResize(lines, &UiRootTable_rebuildSelectList);

		int y = 0;
		UBIG i = 0;
		BIG it;
		while ((it = DbColumnN_jump(DbRoot_getColumnSubs(), selectRow, &i, 1)) >= 0)
		{
			i++;
			y++;
		}
		GuiItemLayout_addRow(layout, 2, y + 1);

		//maxRecords
		GuiItem* maxRecords = GuiItem_addSubName((GuiItem*)layout, "maxRecords", GuiItemEdit_newEx(Quad2i_init4(0, 3, 1, 2), DbValue_initOption(selectRow, "maxRecords", 0), DbValue_initLang("MAX_RECORDS"), 0));

		GuiItem_setEnableMsg((GuiItem*)add, DbValue_initOptionEnable(selectRow), FALSE);
		GuiItem_setEnableMsg((GuiItem*)lines, DbValue_initOptionEnable(selectRow), FALSE);
		GuiItem_setEnableMsg((GuiItem*)maxRecords, DbValue_initOptionEnable(selectRow), FALSE);
	}

	return layout;
}

GuiItem* UiRootTable_build(GuiItemLayout* layout, UBIG row, DbValue scrollV, DbValue scrollH, DbValue selectGrid)
{
	BOOL isFilter = !DbRoot_isType_table(row);

	GuiItemLayout_addColumn(layout, 0, 99);
	GuiItemLayout_addRow(layout, 1, 99);

	//header
	GuiItemLayout* layoutMenu = GuiItemLayout_new(Quad2i_init4(0, 0, 1, 1));
	GuiItemLayout_setDrawBackground(layoutMenu, FALSE);
	GuiItemLayout_addColumn(layoutMenu, 0, 6);
	GuiItemLayout_addColumn(layoutMenu, 1, 99);	//big space
	GuiItemLayout_addColumn(layoutMenu, 2, 3);
	GuiItemLayout_addColumn(layoutMenu, 4, 3);
	GuiItemLayout_addColumn(layoutMenu, 6, 3);
	GuiItemLayout_addColumn(layoutMenu, 8, 99);	//big space
	GuiItemLayout_addColumn(layoutMenu, 9, 4);
	GuiItemLayout_addColumn(layoutMenu, 11, 4);
	GuiItem_addSubName((GuiItem*)layout, "menu", (GuiItem*)layoutMenu);

	//name
	GuiItem* name = GuiItem_addSubName((GuiItem*)layoutMenu, "name", GuiItemEdit_newEx(Quad2i_init4(0, 0, 1, 1), DbValue_initOption(row, "name", 0), DbValue_initLang("NAME"), 0));
	GuiItem_setIcon(name, GuiImage_new1(UiIcons_init_name()));

	//Hidden
	GuiItemMenu* hidden = (GuiItemMenu*)GuiItem_addSubName((GuiItem*)layoutMenu, "hidden", GuiItemMenu_new(Quad2i_init4(2, 0, 2, 1), DbValue_initLang("HIDDEN"), FALSE));
	GuiItemMenu_setContext(hidden, UiRootTable_buildShowedList(row, isFilter));
	GuiItemMenu_setHighligthBackground(hidden, DbRows_hasColumnsSubDeactive(row, "columns"));
	GuiItem_setIcon((GuiItem*)hidden, GuiImage_new1(UiIcons_init_table_hide()));
	GuiItemMenu_setCenter(hidden, FALSE);

	//Short
	GuiItemMenu* shortt = (GuiItemMenu*)GuiItem_addSubName((GuiItem*)layoutMenu, "short", GuiItemMenu_new(Quad2i_init4(4, 0, 2, 1), DbValue_initLang("SHORT"), FALSE));
	GuiItemMenu_setContext(shortt, UiRootTable_buildShortingList(row, FALSE));
	GuiItemMenu_setHighligthBackground(shortt, DbRows_hasFilterSubActive(row, "short"));
	GuiItem_setIcon((GuiItem*)shortt, GuiImage_new1(UiIcons_init_table_short()));
	GuiItemMenu_setCenter(shortt, FALSE);

	//Filter
	if (isFilter)
	{
		GuiItemMenu* filterr = (GuiItemMenu*)GuiItem_addSubName((GuiItem*)layoutMenu, "filtert", GuiItemMenu_new(Quad2i_init4(6, 0, 2, 1), DbValue_initLang("FILTER"), FALSE));
		GuiItemMenu_setContext(filterr, UiRootTable_buildSelectList(row));
		GuiItemMenu_setHighligthBackground(filterr, DbRows_hasFilterSubActive(row, "select"));
		GuiItem_setIcon((GuiItem*)filterr, GuiImage_new1(UiIcons_init_table_filter()));
		GuiItemMenu_setCenter(filterr, FALSE);
	}

	//Row Height
	GuiItem* rowH = GuiItem_addSubName((GuiItem*)layoutMenu, "height", GuiItemComboStatic_newEx(Quad2i_init4(9, 0, 2, 1), DbValue_initOption(row, "height", 0), Lang_find("TABLE_STYLE_OPTIONS"), DbValue_initEmpty(), 0));
	GuiItem_setIcon(rowH, GuiImage_new1(UiIcons_init_table_row_height()));

	//Search
	{
		GuiItemEdit* searchEdit = (GuiItemEdit*)GuiItem_addSubName((GuiItem*)layoutMenu, "search", GuiItemEdit_newEx(Quad2i_init4(11, 0, 2, 1), DbValue_initOption(row, "search", 0), DbValue_initLang("SEARCH"), 0));
		GuiItemEdit_setHighlightIfContent(searchEdit, TRUE);
		GuiItem_setIcon((GuiItem*)searchEdit, GuiImage_new1(UiIcons_init_search()));
	}

	//Table
	{
		DbRows filter = DbRows_initFilter(row);

		GuiItemTable* tableG = GuiItemTable_new(Quad2i_init4(0, 1, 1, 1), row, filter, TRUE, TRUE, scrollV, scrollH, selectGrid);
		GuiItem_addSubName((GuiItem*)layout, "table", (GuiItem*)tableG);
	}

	return (GuiItem*)layout;
}

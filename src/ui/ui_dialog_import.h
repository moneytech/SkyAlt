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

void UiDialogImport_clickType(GuiItem* item)
{
	//copy type to switch
	BIG type = GuiItemComboStatic_getValue((GuiItemComboStatic*)GuiItem_findName(item, "importAs"));
	GuiItemSwitch_setNumber(GuiItem_findName(item, "switch"), type);
}

typedef enum
{
	UI_IMPORT_CSV,
	UI_IMPORT_TSV,
	UI_IMPORT_FILES,
}UI_IMPORT;

void UiDialogImport_clickImport(GuiItem* item)
{
	BIG row = GuiItem_findAttribute(item, "row");
	if (row < 0)
		row = DbRoot_createFolderRow();

	const UNI* pathUNI = GuiItemEdit_getText((GuiItemEdit*)GuiItem_findName(item, "path"));
	DbTable* table = DbRoot_createTable(row);
	DbRows rows = DbRows_initTable(table);

	BIG type = GuiItemComboStatic_getValue((GuiItemComboStatic*)GuiItem_findName(item, "importAs"));
	if (type == UI_IMPORT_CSV || type == UI_IMPORT_TSV)
	{
		BOOL firstRowColumnNames = GuiItemCheck_isActive((GuiItemCheck*)GuiItem_findName(item, "rowColumnNames"));
		BOOL recognizeColumnType = GuiItemCheck_isActive((GuiItemCheck*)GuiItem_findName(item, "recognizeColumnType"));


//rozpoznat koncovku ...
//nahrát více souborù ...

		{
			char* path = Std_newCHAR_uni(pathUNI);
			/*char* path_folder;
			char* path_name;
			OsFile_getParts(path, &path_folder, &path_name);
			UNI* pathNameUNI = Std_newUNI_char(path_name);*/

			if (type == UI_IMPORT_CSV)
				IOCsv_read(path, firstRowColumnNames, recognizeColumnType, &rows);	//csv
			else
				IOTsv_read(path, firstRowColumnNames, recognizeColumnType, &rows);	//tsv

			/*Std_deleteUNI(pathNameUNI);
			Std_deleteCHAR(path_folder);
			Std_deleteCHAR(path_name);*/
			Std_deleteCHAR(path);
		}
	}
	else
		if (type == UI_IMPORT_FILES)
		{
			BOOL subDirs = GuiItemCheck_isActive((GuiItemCheck*)GuiItem_findName(item, "subDirs"));

			char* path = Std_newCHAR_uni(pathUNI);
			if (path)
				IOFiles_read(path, subDirs, &rows);
			Std_deleteCHAR(path);
		}

	DbRows_free(&rows);

	DbRoot_setPanelLeft(DbTable_getRow(table));
	GuiItem_closeParentLevel(item);	//hide
}

GuiItemLayout* UiDialogImport_newBase(BIG row, UINT importType)
{
	//layout
	GuiItemLayout* layout = GuiItemLayout_new(Quad2i_init4(0, 0, 1, 1));
	//self->layout = layout;
	GuiItemLayout_addColumn(layout, 0, 100);
	GuiItemLayout_addColumn(layout, 1, 100);
	GuiItem_setAttribute((GuiItem*)layout, "row", row);

	//path
	GuiItemEdit* path = (GuiItemEdit*)GuiItem_addSubName((GuiItem*)layout, "path", GuiItemEdit_new(Quad2i_init4(0, 0, 2, 2), DbValue_initEmpty(), DbValue_initLang("PATH")));
	GuiItemEdit_setShowPicker(path, TRUE, TRUE, FALSE, TRUE, Lang_find("IMPORT"), 0);

	_UNI32("*.csv;*.csv");

	//options
	GuiItem_addSubName((GuiItem*)layout, "importAs", GuiItemComboStatic_newEx(Quad2i_init4(0, 3, 2, 1), DbValue_initNumber(importType), Lang_find("IMPORT_OPTIONS"), DbValue_initEmpty(), &UiDialogImport_clickType));

	GuiItemSwitch* sw = (GuiItemSwitch*)GuiItem_addSubName((GuiItem*)layout, "switch", GuiItemSwitch_new(Quad2i_init4(0, 4, 2, 2), DbValue_initNumber(importType)));
	GuiItem_setName((GuiItem*)sw, "switch");
	{
		GuiItemLayout* mnLayout = (GuiItemLayout*)GuiItemSwitch_addItem(sw, 0, (GuiItem*)GuiItemLayout_new(Quad2i_init4(0, 0, 2, 3)));
		GuiItemLayout_addColumn(mnLayout, 0, 99);
		GuiItem_addSubName((GuiItem*)mnLayout, "rowColumnNames", GuiItemCheck_new(Quad2i_init4(0, 0, 1, 1), DbValue_initNumber(1), DbValue_initLang("FIRST_ROW_COLUMN_NAMES")));
		GuiItem_addSubName((GuiItem*)mnLayout, "recognizeColumnType", GuiItemCheck_new(Quad2i_init4(0, 1, 1, 1), DbValue_initNumber(1), DbValue_initLang("RECOGNIZE_COLUMN_TYPE")));
	}
	{
		GuiItemLayout* mnLayout = (GuiItemLayout*)GuiItemSwitch_addItem(sw, 1, (GuiItem*)GuiItemLayout_new(Quad2i_init4(0, 0, 2, 3)));
		GuiItemLayout_addColumn(mnLayout, 0, 99);
		GuiItem_addSubName((GuiItem*)mnLayout, "rowColumnNames", GuiItemCheck_new(Quad2i_init4(0, 0, 1, 1), DbValue_initNumber(1), DbValue_initLang("FIRST_ROW_COLUMN_NAMES")));
		GuiItem_addSubName((GuiItem*)mnLayout, "recognizeColumnType", GuiItemCheck_new(Quad2i_init4(0, 1, 1, 1), DbValue_initNumber(1), DbValue_initLang("RECOGNIZE_COLUMN_TYPE")));
	}
	{
		GuiItemLayout* mnLayout = (GuiItemLayout*)GuiItemSwitch_addItem(sw, 2, (GuiItem*)GuiItemLayout_new(Quad2i_init4(0, 0, 2, 3)));
		GuiItemLayout_addColumn(mnLayout, 0, 99);
		GuiItem_addSubName((GuiItem*)mnLayout, "subDirs", GuiItemCheck_new(Quad2i_init4(0, 0, 1, 1), DbValue_initNumber(1), DbValue_initLang("SUB_DIRS")));
	}

	//Button
	GuiItem_addSubName((GuiItem*)layout, "import", GuiItemButton_newClassicEx(Quad2i_init4(0, 7, 2, 2), DbValue_initLang("IMPORT"), &UiDialogImport_clickImport));

	return layout;
}

GuiItemLayout* UiDialogImport_new(BIG row, UINT importType)
{
	return GuiItemRoot_createDialogLayout(Vec2i_init2(10, 10), DbValue_initLang("IMPORT"), (GuiItem*)UiDialogImport_newBase(row, importType), 0);
}

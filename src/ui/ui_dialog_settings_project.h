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

GuiItemLayout* UiDialogSettingsProject_new(void)
{
	//layout
	GuiItemLayout* layout = GuiItemLayout_new(Quad2i_init4(0, 0, 1, 1));
	GuiItemLayout_addColumn(layout, 0, 10);
	GuiItemLayout_addColumn(layout, 2, 10);

	//Show/disable Audio ...
	//Show/disable Video ...
	//Show/disable PDF ...
	//Show/disable Map ...

	//Show/disable Web ...
	//Show/disable Email ...

	//info
	{
		UBIG ram = DbRoot_bytes(TRUE);
		UBIG ramMax = OsThread_getMaxRam();

		GuiItem_addSubName((GuiItem*)layout, "tables", GuiItemText_new(Quad2i_init4(0, 0, 1, 2), TRUE, DbValue_initNumber(DbRoot_numEx(FALSE)), DbValue_initLang("TABLES")));
		//GuiItem_addSubName((GuiItem*)layout, "filters", GuiItemText_new(Quad2i_init4(2, 0, 1, 2), TRUE, DbValue_initNumber(DbRoot_numFilters(tables, FALSE)), DbValue_initLang("FILTERS)));
		GuiItem_addSubName((GuiItem*)layout, "columns", GuiItemText_new(Quad2i_init4(0, 3, 1, 2), TRUE, DbValue_initNumber(DbRoot_numColumns(FALSE)), DbValue_initLang("COLUMNS")));
		GuiItem_addSubName((GuiItem*)layout, "records", GuiItemText_new(Quad2i_init4(2, 3, 1, 2), TRUE, DbValue_initNumber(DbRoot_numRecords(FALSE)), DbValue_initLang("RECORDS")));

		//GuiItem_addSubName((GuiItem*)layout, "pages", GuiItemText_new(Quad2i_init4(0, 6, 1, 2), TRUE, DbValue_initNumber(AppGui_numPages(a->gui)), DbValue_initLang("PAGES)));

		GuiItem_addSubName((GuiItem*)layout, "table_cache", GuiItemText_new(Quad2i_init4(0, 13, 1, 2), TRUE, DbValue_initStatic(Std_newNumberSize(ram)), DbValue_initLang("TABLE_CACHE")));
		GuiItem_addSubName((GuiItem*)layout, "%ram", GuiItemText_new(Quad2i_init4(2, 13, 1, 2), TRUE, DbValue_initStatic(Std_addAfterUNI(Std_newNumber(((double)ram) / ramMax), _UNI32("%"))), DbValue_initLang("RAM")));

		GuiItem_addSubName((GuiItem*)layout, "disk_space", GuiItemText_new(Quad2i_init4(0, 16, 1, 2), TRUE, DbValue_initStatic(Std_newNumberSize(FileProject_getProjectSize())), DbValue_initLang("DISK_SIZE")));
		//GuiItem_addSubName((GuiItem*)layout, "temp_file_size", GuiItemText_new(Quad2i_init4(2, 16, 1, 2), TRUE, DbValue_initStatic(Std_newNumberSize(FileSystem_tempFileSize(&a->tables->fileSystem))), DbValue_initLang("TEMP_FILE_SIZE)));
	}

	return GuiItemRoot_createDialogLayout(Vec2i_init2(10, 20), DbValue_initLang("PROJECT_SETTINGS"), (GuiItem*)layout, 0);
}

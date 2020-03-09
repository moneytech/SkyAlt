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

static GuiItem* UiRootFolder_build(GuiItemLayout* layout, UBIG row)
{
	GuiItemLayout_addColumn(layout, 1, 99);
	GuiItemLayout_addColumn(layout, 2, 99);
	GuiItemLayout_addRow(layout, 0, 2);
	GuiItemLayout_addRow(layout, 1, 5);

	//header
	GuiItem* header = GuiItem_addSubName((GuiItem*)layout, "header", UiRoot_createMenuNameHeader(Quad2i_init4(0, 0, 3, 1), row));
	GuiItem_setIcon(GuiItem_findName(header, "name"), 0);
	GuiItemEdit_setTextLevel(GuiItem_findName(header, "name"), 2);
	GuiItemEdit_showDescription(GuiItem_findName(header, "name"), FALSE);

	//description
	GuiItem_addSubName((GuiItem*)layout, "description", GuiItemEdit_newEx(Quad2i_init4(0, 1, 3, 1), DbValue_initOption(row, "description", 0), DbValue_initLang("DESCRIPTION"), 0));

	//"From Menu"
	GuiItem* it;
	it = GuiItem_addSubName((GuiItem*)layout, "add_table", GuiItemButton_newClassicEx(Quad2i_init4(0, 3, 2, 2), DbValue_initLang("ADD_TABLE"), &UiRoot_clickAddSub));
	GuiItem_setAttribute(it, "type", UI_ADD_TABLE);
	GuiItem_setAttribute(it, "row", row);
	//it = GuiItem_addSubName((GuiItem*)layout, "add_page", GuiItemButton_newClassicEx(Quad2i_init4(0, 5, 2, 2), DbValue_initLang("ADD_PAGE"), &UiRoot_clickAddSub));
	//GuiItem_setAttribute(it, "type", UI_ADD_PAGE);
	//GuiItem_setAttribute(it, "row", row);

	it = GuiItem_addSubName((GuiItem*)layout, "import_csv", GuiItemButton_newClassicEx(Quad2i_init4(2, 3, 1, 2), DbValue_initLang("IMPORT_CSV"), &UiRoot_clickImport));
	GuiItem_setAttribute(it, "type", UI_IMPORT_CSV);
	GuiItem_setAttribute(it, "row", row);
	it = GuiItem_addSubName((GuiItem*)layout, "import_tsv", GuiItemButton_newClassicEx(Quad2i_init4(2, 5, 1, 2), DbValue_initLang("IMPORT_TSV"), &UiRoot_clickImport));
	GuiItem_setAttribute(it, "type", UI_IMPORT_TSV);
	GuiItem_setAttribute(it, "row", row);
	it = GuiItem_addSubName((GuiItem*)layout, "import_files", GuiItemButton_newClassicEx(Quad2i_init4(2, 7, 1, 2), DbValue_initLang("IMPORT_FILES"), &UiRoot_clickImport));
	GuiItem_setAttribute(it, "type", UI_IMPORT_FILES);
	GuiItem_setAttribute(it, "row", row);

	return (GuiItem*)layout;
}

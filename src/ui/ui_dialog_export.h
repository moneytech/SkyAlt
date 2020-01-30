/*
 * Copyright (c) 2018 Milan Suk
 *
 * Use of this software is governed by the Business Source License included
 * in the LICENSE file and at www.mariadb.com/bsl11.
 *
 * Change Date: 2025-02-01
 *
 * On the date above, in accordance with the Business Source License, use
 * of this software will be governed by version 2 or later of the General
 * Public License.
 */

void UiDialogExport_clickType(GuiItem* item)
{
	//copy type to switch
	BIG type = GuiItemComboStatic_getValue((GuiItemComboStatic*)GuiItem_findName(item, "exportAs"));
	GuiItemSwitch_setNumber(GuiItem_findName(item, "switch"), type);
}

typedef enum
{
	UI_EXPORT_CSV,
	UI_EXPORT_TSV,
	UI_EXPORT_HTML,
}UI_EXPORT;

void UiDialogExport_clickExport(GuiItem* item)
{
	BIG row = GuiItem_findAttribute(item, "row");
	const UNI* pathUNI = GuiItemEdit_getText(GuiItem_findName(item, "path"));

	DbRows filter = DbRows_initFilter(row);
	DbRows_hasChanged(&filter);
	DbValues columns = DbRows_getSubs(row, "columns", TRUE, -1);

	char* path = Std_newCHAR_uni(pathUNI);
	BIG type = GuiItemComboStatic_getValue((GuiItemComboStatic*)GuiItem_findName(item, "exportAs"));
	if (type == UI_EXPORT_CSV || type == UI_EXPORT_TSV)
	{
		BOOL firstRowColumnNames = GuiItemCheck_isActive((GuiItemCheck*)GuiItem_findName(item, "rowColumnNames"));
		if (type == UI_EXPORT_CSV)
			IOCsv_write(path, firstRowColumnNames, &filter, &columns);
		else
			IOTsv_write(path, firstRowColumnNames, &filter, &columns);
	}
	else
		if (type == UI_EXPORT_HTML)
		{
			BOOL renderImages = GuiItemCheck_isActive((GuiItemCheck*)GuiItem_findName(item, "renderImages"));
			IOHtml_write(path, renderImages, &filter, &columns);
		}

	Std_deleteCHAR(path);
	DbRows_free(&filter);
	DbValues_free(&columns);

	GuiItem_closeParentLevel(item);	//hide
}

GuiItemLayout* UiDialogExport_new(BIG row, UINT exportType)
{
	//layout
	GuiItemLayout* layout = GuiItemLayout_new(Quad2i_init4(0, 0, 1, 1));
	//self->layout = layout;
	GuiItemLayout_addColumn(layout, 0, 7);
	GuiItemLayout_addColumn(layout, 1, 7);
	GuiItem_setAttribute((GuiItem*)layout, "row", row);

	//path
	GuiItemEdit* path = (GuiItemEdit*)GuiItem_addSubName((GuiItem*)layout, "path", GuiItemEdit_new(Quad2i_init4(0, 0, 2, 2), DbValue_initEmpty(), DbValue_initLang("PATH")));
	GuiItemEdit_setShowPicker(path, TRUE, FALSE, FALSE, FALSE, Lang_find("EXPORT"), 0);

	//options
	GuiItem_addSubName((GuiItem*)layout, "exportAs", GuiItemComboStatic_newEx(Quad2i_init4(0, 3, 2, 1), DbValue_initNumber(exportType), Lang_find("EXPORT_OPTIONS"), DbValue_initEmpty(), &UiDialogExport_clickType));

	GuiItemSwitch* sw = GuiItem_addSubName((GuiItem*)layout, "switch", GuiItemSwitch_new(Quad2i_init4(0, 4, 2, 2), DbValue_initNumber(exportType)));
	{
		GuiItemLayout* mnLayout = (GuiItemLayout*)GuiItemSwitch_addItem(sw, 0, (GuiItem*)GuiItemLayout_new(Quad2i_init4(0, 0, 2, 3)));
		GuiItemLayout_addColumn(mnLayout, 0, 99);
		GuiItem_addSubName((GuiItem*)mnLayout, "rowColumnNames", GuiItemCheck_new(Quad2i_init4(0, 0, 1, 1), DbValue_initNumber(1), DbValue_initLang("FIRST_ROW_COLUMN_NAMES")));
	}
	{
		GuiItemLayout* mnLayout = (GuiItemLayout*)GuiItemSwitch_addItem(sw, 1, (GuiItem*)GuiItemLayout_new(Quad2i_init4(0, 0, 2, 3)));
		GuiItemLayout_addColumn(mnLayout, 0, 99);
		GuiItem_addSubName((GuiItem*)mnLayout, "rowColumnNames", GuiItemCheck_new(Quad2i_init4(0, 0, 1, 1), DbValue_initNumber(1), DbValue_initLang("FIRST_ROW_COLUMN_NAMES")));
	}
	{
		GuiItemLayout* mnLayout = (GuiItemLayout*)GuiItemSwitch_addItem(sw, 2, (GuiItem*)GuiItemLayout_new(Quad2i_init4(0, 0, 2, 3)));
		GuiItemLayout_addColumn(mnLayout, 0, 99);
		GuiItem_addSubName((GuiItem*)mnLayout, "renderImages", GuiItemCheck_new(Quad2i_init4(0, 0, 1, 1), DbValue_initNumber(1), DbValue_initLang("RENDER_IMAGES")));
	}

	//Button
	GuiItem_addSubName((GuiItem*)layout, "export", GuiItemButton_newClassicEx(Quad2i_init4(0, 7, 2, 2), DbValue_initLang("EXPORT"), &UiDialogExport_clickExport));

	return GuiItemRoot_createDialogLayout(Vec2i_init2(10, 10), DbValue_initLang("EXPORT"), (GuiItem*)layout, 0);
}

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

void UiDialogExportImage_clickExport(GuiItem* item)
{
	BIG row = GuiItem_findAttribute(item, "row");
	const UNI* pathUNI = GuiItemEdit_getText(GuiItem_findName(item, "path"));
	int w = GuiItemEdit_getNumber(GuiItem_findName(item, "w"));
	int h = GuiItemEdit_getNumber(GuiItem_findName(item, "h"));

	if (w > 0 && h > 0)
	{
		//prepare
		DbRows filter = DbRows_initFilter(row);
		DbRows_hasChanged(&filter);
		GuiItemChart* chart = (GuiItemChart*)GuiItemChart_new(Quad2i_init(), row, filter, DbValue_initEmpty());

		//render
		Image4 img = GuiItemChart_renderImage(chart, Vec2i_init2(w, h), UiScreen_getWin());

		//save
		char* path = Std_newCHAR_uni(pathUNI);
		Image4_saveJpeg(&img, path);
		Std_deleteCHAR(path);

		//clear
		Image4_free(&img);
		GuiItem_delete((GuiItem*)chart);
		DbRows_free(&filter);
	}

	GuiItem_closeParentLevel(item);	//hide
}

GuiItemLayout* UiDialogExportImage_new(BIG row)
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
	GuiItem_addSubName((GuiItem*)layout, "w", GuiItemEdit_new(Quad2i_init4(0, 3, 1, 2), DbValue_initNumber(1280), DbValue_initLang("RES_WIDTH")));
	GuiItem_addSubName((GuiItem*)layout, "h", GuiItemEdit_new(Quad2i_init4(1, 3, 1, 2), DbValue_initNumber(720), DbValue_initLang("RES_HEIGHT")));

	//Button
	GuiItem_addSubName((GuiItem*)layout, "export", GuiItemButton_newClassicEx(Quad2i_init4(0, 7, 2, 2), DbValue_initLang("EXPORT"), &UiDialogExportImage_clickExport));

	return GuiItemRoot_createDialogLayout(Vec2i_init2(10, 10), DbValue_initLang("EXPORT"), (GuiItem*)layout, 0);
}

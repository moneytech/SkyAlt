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

void UiDialogOpen_clickOpen(GuiItem* item)
{
	const UNI* pathUNI = GuiItemEdit_getText(GuiItem_findName(item, "path"));
	const UNI* password = GuiItemEdit_getText(GuiItem_findName(item, "password"));

	UiScreen_openProject(pathUNI, password);
}

void UiDialogOpen_clickEditPath(GuiItem* item)
{
	const UNI* path = OsWinIO_getCursorRenderItemCache();// GuiItemEdit_getText(GuiItem_findName(item, "path"));
	GuiItem_setEnable(GuiItem_findName(item, "password"), FileProject_hasPassword(path));
}

GuiItemLayout* UiDialogOpen_new(const UNI* path)
{
	//layout
	GuiItemLayout* layout = GuiItemLayout_new(Quad2i_init4(0, 0, 1, 1));
	GuiItemLayout_addColumn(layout, 0, 5);
	GuiItemLayout_addColumn(layout, 2, 5);

	//path
	GuiItemEdit* pathGui = (GuiItemEdit*)GuiItem_addSubName((GuiItem*)layout, "path", GuiItemEdit_new(Quad2i_init4(0, 0, 3, 2), DbValue_initStaticCopy(path), DbValue_initLang("PROJECT_NAME")));
	GuiItemEdit_setShowPicker(pathGui, TRUE, FALSE, FALSE, FALSE, Lang_find("CREATE"), _UNI32("*.sky"));
	GuiItemEdit_setFnChanged(pathGui, &UiDialogOpen_clickEditPath);

	//password
	GuiItemEdit* password = (GuiItemEdit*)GuiItem_addSubName((GuiItem*)layout, "password", GuiItemEdit_new(Quad2i_init4(0, 3, 3, 2), DbValue_initEmpty(), DbValue_initLang("PASSWORD")));	//password
	GuiItemEdit_setPasswordStars(password, TRUE);
	GuiItem_setEnable((GuiItem*)password, FileProject_hasPassword(path));

	//Buttons
	GuiItem* b = GuiItem_addSubName((GuiItem*)layout, "button", GuiItemButton_newClassicEx(Quad2i_init4(0, 6, 3, 2), DbValue_initLang("OPEN"), &UiDialogOpen_clickOpen));
	GuiItem_setShortcutKey(b, FALSE, Win_EXTRAKEY_ENTER, 0, &UiDialogOpen_clickOpen);

	return GuiItemRoot_createDialogLayout(Vec2i_init2(10, 10), DbValue_initLang("OPEN"), (GuiItem*)layout, 0);
}

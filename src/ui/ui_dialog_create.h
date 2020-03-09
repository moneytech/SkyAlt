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

void GuiRoot_clickEditPassword(GuiItem* item)
{
	const UNI* password = OsWinIO_getCursorRenderItemCache();// GuiItemEdit_getText(GuiItem_findName(item, "password"));
	GuiItem_setEnable(GuiItem_findName(item, "cycles"), Std_sizeUNI(password) > 0);
}

void UiDialogCreate_clickCreate(GuiItem* item)
{
	const UNI* pathUNI = GuiItemEdit_getText(GuiItem_findName(item, "path"));
	const UNI* password = GuiItemEdit_getText(GuiItem_findName(item, "password"));
	BIG cycles = Std_sizeUNI(password) ? Std_max(1, GuiItemSlider_getNumber(GuiItem_findName(item, "cycles"))) : 0;

	if (Std_sizeUNI(pathUNI))
		UiScreen_createProject(pathUNI, password, cycles);
	else
		Logs_addError("ERR_INVALID_NAME");
}

GuiItemLayout* UiDialogCreate_new(void)
{
	//layout
	GuiItemLayout* layout = GuiItemLayout_new(Quad2i_init4(0, 0, 1, 1));
	GuiItemLayout_addColumn(layout, 0, 20);
	GuiItemLayout_addColumn(layout, 2, 20);

	//path
	GuiItemEdit* path = (GuiItemEdit*)GuiItem_addSubName((GuiItem*)layout, "path", GuiItemEdit_new(Quad2i_init4(0, 0, 3, 2), DbValue_initStatic(0), DbValue_initLang("PROJECT_NAME")));		//name
	GuiItemEdit_setShowPicker(path, TRUE, FALSE, FALSE, FALSE, Lang_find("CREATE"), 0);

	//password
	GuiItemEdit* password = GuiItem_addSubName((GuiItem*)layout, "password", GuiItemEdit_new(Quad2i_init4(0, 3, 3, 2), DbValue_initEmpty(), DbValue_initLang("PASSWORD")));	//password
	GuiItemEdit_setPasswordStars(password, TRUE);
	GuiItemEdit_setFnChanged(password, &GuiRoot_clickEditPassword);

	//cycles
	GuiItemSlider* cycles = GuiItem_addSubName((GuiItem*)layout, "cycles", GuiItemSlider_newEx(Quad2i_init4(0, 6, 3, 2), DbValue_initNumber(100000), DbValue_initNumber(FileKey_CYCLES * 5), DbValue_initNumber(1), DbValue_initNumber(FileKey_CYCLES), DbValue_initLang("PASSWORD_SECURITY"), DbValue_initLang("PASSWORD_LIGHT"), DbValue_initLang("PASSWORD_VILLAIN"), 0));
	GuiItem_setEnable((GuiItem*)cycles, FALSE);

	//Buttons
	GuiItem* b = GuiItem_addSubName((GuiItem*)layout, "button", GuiItemButton_newClassicEx(Quad2i_init4(0, 9, 3, 2), DbValue_initLang("NEW"), &UiDialogCreate_clickCreate));
	GuiItem_setShortcutKey(b, FALSE, Win_EXTRAKEY_ENTER, 0, &UiDialogCreate_clickCreate);

	return GuiItemRoot_createDialogLayout(Vec2i_init2(15, 12), DbValue_initLang("NEW"), (GuiItem*)layout, 0);
}

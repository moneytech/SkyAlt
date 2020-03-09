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

void UiDialogChangePassword_clickEditPassword(GuiItem* item)
{
	const UNI* password = OsWinIO_getCursorRenderItemCache();	//GuiItemEdit_getText(GuiItem_findName(item, "password"));
	GuiItem_setEnable(GuiItem_findName(item, "cycles"), Std_sizeUNI(password) > 0);
}

void UiDialogChangePassword_clickSaveProject(GuiItem* item)
{
	const UNI* password = GuiItemEdit_getText(GuiItem_findName(item, "password"));
	BIG cycles = password ? Std_max(1, GuiItemSlider_getNumber(GuiItem_findName(item, "cycles"))) : 0;

	FileProject_changePassword(cycles, password);

	GuiItem_closeParentLevel(item);	//hide
}

GuiItemLayout* UiDialogChangePassword_new(void)
{
	//layout
	GuiItemLayout* layout = GuiItemLayout_new(Quad2i_init4(0, 0, 1, 1));
	GuiItemLayout_addColumn(layout, 0, 5);
	GuiItemLayout_addColumn(layout, 2, 5);

	//passwords
	GuiItemEdit* passwordOld = GuiItem_addSubName((GuiItem*)layout, "oldPassword", GuiItemEdit_new(Quad2i_init4(0, 0, 3, 2), DbValue_initEmpty(), DbValue_initLang("PASSWORD")));	//password
	GuiItemEdit* passwordNew = GuiItem_addSubName((GuiItem*)layout, "newPassword", GuiItemEdit_new(Quad2i_init4(0, 3, 3, 2), DbValue_initEmpty(), DbValue_initLang("PASSWORD")));	//password
	GuiItemEdit_setPasswordStars(passwordOld, TRUE);
	GuiItemEdit_setPasswordStars(passwordNew, TRUE);
	GuiItemEdit_setFnChanged(passwordNew, &UiDialogChangePassword_clickEditPassword);

	//cycles
	GuiItemSlider* cycles = GuiItem_addSubName((GuiItem*)layout, "cycles", GuiItemSlider_newEx(Quad2i_init4(0, 6, 3, 2), DbValue_initNumber(100000), DbValue_initNumber(FileKey_CYCLES * 5), DbValue_initNumber(1), DbValue_initNumber(FileKey_CYCLES), DbValue_initLang("PASSWORD_SECURITY"), DbValue_initLang("PASSWORD_LIGHT"), DbValue_initLang("PASSWORD_VILLAIN"), 0));
	GuiItem_setEnable((GuiItem*)cycles, FALSE);

	//Buttons
	GuiItem* b = GuiItem_addSubName((GuiItem*)layout, "nutton", GuiItemButton_newClassicEx(Quad2i_init4(0, 9, 3, 2), DbValue_initLang("CHANGE_PASSWORD"), &UiDialogChangePassword_clickSaveProject));
	GuiItem_setShortcutKey(b, FALSE, Win_EXTRAKEY_ENTER, 0, &UiDialogChangePassword_clickSaveProject);

	return GuiItemRoot_createDialogLayout(Vec2i_init2(10, 12), DbValue_initLang("CHANGE_PASSWORD"), (GuiItem*)layout, 0);
}

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

void UiDialogUpdate_clickAutoUpdate(GuiItem* item)
{
	UiIniSettings_setUpdate(GuiItemCheck_isActive(GuiItem_findName(item, "update")));
}

void UiDialogUpdate_clickCheck(GuiItem* item)
{
	UiAutoUpdate_run();

	BIG ver = UiAutoUpdate_getVersionUNI();
	GuiItem_setEnable(GuiItem_findName(item, "button"), UiAutoUpdate_hasUpdate());
	GuiItemText* latest = GuiItem_findName(item, "latestVer");
	if (ver < 0)
		GuiItemText_setText(latest, Lang_find("CHECKING"));
	else
		GuiItemText_setNumber(latest, ver);
}

void UiDialogUpdate_clickUpdate(GuiItem* item)
{
	if (UiAutoUpdate_hasUpdate())
	{
		if (UiAutoUpdate_updateFile())
			Logs_addInfo("INF_UPDATES_NOTE");
		else
			Logs_addError("ERR_UPDATE_FAIL");
	}

	GuiItem_closeParentLevel(item);	//hide
}

GuiItemLayout* UiDialogUpdate_new(void)
{
	DbValue currentVer = DbValue_initNumber(STD_BUILD);
	DbValue latestVer = DbValue_initStatic(UiAutoUpdate_hasUpdate() ? Std_newNumber(g_autoupdate->version) : Std_newUNI(_UNI32("---")));

	//layout
	GuiItemLayout* layout = GuiItemLayout_new(Quad2i_init4(0, 0, 1, 1));
	GuiItemLayout_addColumn(layout, 0, 5);
	GuiItemLayout_addColumn(layout, 1, 1);
	GuiItemLayout_addColumn(layout, 2, 5);

	//auto-update
	GuiItem_addSubName((GuiItem*)layout, "update", GuiItemCheck_newEx(Quad2i_init4(0, 0, 3, 1), DbValue_initNumber(UiIniSettings_getUpdate()), DbValue_initLang("UPDATES_START"), &UiDialogUpdate_clickAutoUpdate));

	//versions
	GuiItem_addSubName((GuiItem*)layout, "currentVer", GuiItemText_new(Quad2i_init4(0, 2, 1, 2), TRUE, currentVer, DbValue_initLang("CURRENT_VERSION")));
	GuiItem_addSubName((GuiItem*)layout, "latestVer", GuiItemText_new(Quad2i_init4(2, 2, 1, 2), TRUE, latestVer, DbValue_initLang("LATEST_VERSION")));

	//check
	GuiItem_addSubName((GuiItem*)layout, "check", GuiItemButton_newAlphaEx(Quad2i_init4(0, 5, 3, 1), DbValue_initLang("CHECK"), &UiDialogUpdate_clickCheck));

	//web link
	GuiItem_addSubName((GuiItem*)layout, "web", GuiItemText_new(Quad2i_init4(0, 7, 3, 1), TRUE, DbValue_initStaticCopy(_UNI32("www. ... .com")), DbValue_initEmpty()));

	//ok & cancel
	GuiItem* updateButton = GuiItem_addSubName((GuiItem*)layout, "button", GuiItemButton_newClassicEx(Quad2i_init4(0, 9, 3, 1), DbValue_initLang("UPDATES_DO"), &UiDialogUpdate_clickUpdate));
	GuiItem_setEnable(updateButton, UiAutoUpdate_hasUpdate());
	GuiItem_setShortcutKey(updateButton, FALSE, Win_EXTRAKEY_ENTER, 0, &UiDialogUpdate_clickUpdate);

	return GuiItemRoot_createDialogLayout(Vec2i_init2(11, 11), DbValue_initLang("UPDATES"), (GuiItem*)layout, 0);
}

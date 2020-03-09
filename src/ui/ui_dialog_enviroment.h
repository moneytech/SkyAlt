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

void UiDialogEnviroment_clickAutoUpdate(GuiItem* item)
{
	UiIniSettings_setUpdate(GuiItemCheck_isActive(GuiItem_findName(item, "update")));
}

void UiDialogEnviroment_clickNetworkStartup(GuiItem* item)
{
	UiIniSettings_setNetworkOnline(GuiItemComboStatic_getValue(GuiItem_findName(item, "network_startup")) == 0);
}

void UiDialogEnviroment_clickChooseLanguage(GuiItem* item)
{
	Lang_setPos(Std_max(0, GuiItemComboStatic_getValue(GuiItem_findName(item, "language"))));
	UiScreen_askResizeHard();
}

void UiDialogEnviroment_clickDateFormat(GuiItem* item)
{
	UiIniSettings_setDateFormat(Std_max(0, GuiItemComboStatic_getValue(GuiItem_findName(item, "date"))));
}

void UiDialogEnviroment_clickChooseTheme(GuiItem* item)
{
	UiIniSettings_setTheme(Std_max(0, GuiItemComboStatic_getValue(GuiItem_findName(item, "theme"))));
	UiScreen_updateTheme();
	UiScreen_askResizeHard();
}

void UiDialogEnviroment_clickThreads(GuiItem* item)
{
	const UINT n = GuiItemEdit_getNumber(GuiItem_findName(item, "threads"));
	UiIniSettings_setThreads(n);
}

void UiDialogEnviroment_clickDpi(GuiItem* item)
{
	const UINT dpi = GuiItemEdit_getNumber(GuiItem_findName(item, "dpi"));
	UiScreen_setDPI(dpi);
}

GuiItemLayout* UiDialogEnviroment_new(void)
{
	//layout
	GuiItemLayout* layout = GuiItemLayout_new(Quad2i_init4(0, 0, 1, 1));
	GuiItemLayout_addColumn(layout, 0, 10);

	int y = -3;
	//Language
	GuiItem_addSubName((GuiItem*)layout, "language", GuiItemComboStatic_newEx(Quad2i_init4(0, y += 3, 1, 2), DbValue_initNumber(Lang_getPos()), Lang_find("LANGUAGE_LIST"), DbValue_initLang("LANGUAGE"), &UiDialogEnviroment_clickChooseLanguage));

	//Date
	GuiItem_addSubName((GuiItem*)layout, "date", GuiItemComboStatic_newEx(Quad2i_init4(0, y += 3, 1, 2), DbValue_initNumber(UiIniSettings_getDateFormat()), Lang_find("CALENDAR_FORMAT_DATE"), DbValue_initLang("DATE_FORMAT"), &UiDialogEnviroment_clickDateFormat));

	//theme
	GuiItem_addSubName((GuiItem*)layout, "theme", GuiItemComboStatic_newEx(Quad2i_init4(0, y += 3, 1, 2), DbValue_initNumber(UiIniSettings_getTheme()), Lang_find("THEME_OPTIONS"), DbValue_initLang("THEME"), &UiDialogEnviroment_clickChooseTheme));

	//threads
	GuiItem_addSubName((GuiItem*)layout, "threads", GuiItemEdit_newEx(Quad2i_init4(0, y += 3, 1, 2), DbValue_initNumber(UiIniSettings_getThreads()), DbValue_initLang("THREADS"), &UiDialogEnviroment_clickThreads));

	//dpi
	GuiItem_addSubName((GuiItem*)layout, "dpi", GuiItemEdit_newEx(Quad2i_init4(0, y += 3, 1, 2), DbValue_initNumber(UiIniSettings_getDPI()), DbValue_initLang("DPI"), &UiDialogEnviroment_clickDpi));

	//auto-update
	GuiItem_addSubName((GuiItem*)layout, "update", GuiItemCheck_newEx(Quad2i_init4(0, y += 3, 1, 1), DbValue_initNumber(UiIniSettings_getUpdate()), DbValue_initLang("UPDATES_AUTO"), &UiDialogEnviroment_clickAutoUpdate));

	//network
	GuiItemComboStatic* startup = (GuiItemComboStatic*)GuiItem_addSubName((GuiItem*)layout, "network_startup", GuiItemComboStatic_newEx(Quad2i_init4(0, y += 2, 1, 2), DbValue_initNumber(!UiIniSettings_isNetworkOnline()), 0, DbValue_initLang("NET_STARTUP"), &UiDialogEnviroment_clickNetworkStartup));
	GuiItemComboStatic_addItem(startup, DbValue_initLang("NET_ONLINE"));
	GuiItemComboStatic_addItem(startup, DbValue_initLang("NET_OFFLINE"));

	return GuiItemRoot_createDialogLayout(Vec2i_init2(10, y + 2), DbValue_initLang("SETTINGS_ENVIROMENT"), (GuiItem*)layout, 0);
}

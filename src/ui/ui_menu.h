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

void UiRootMenu_clickNewProject(GuiItem* item)
{
	GuiItemRoot_addDialogLayout(UiScreen_needSave() ? UiDialogSave_new(UiDialogSave_NEW) : UiDialogCreate_new());
}
void UiRootMenu_clickOpenProject(GuiItem* item)
{
	GuiItemRoot_addDialogLayout(UiScreen_needSave() ? UiDialogSave_new(UiDialogSave_OPEN) : UiDialogOpen_new(0));
}
void UiRootMenu_clickSaveProject(GuiItem* item)
{
	DbRoot_save();
}

void UiRootMenu_clickChangePassword(GuiItem* item)
{
	GuiItemRoot_addDialogLayout(UiDialogChangePassword_new());
}

void UiRootMenu_clickUpdate(GuiItem* item)
{
	GuiItemRoot_addDialogLayout(UiDialogUpdate_new());
}
void UiRootMenu_clickSettingsProject(GuiItem* item)
{
	GuiItemRoot_addDialogLayout(UiDialogSettingsProject_new());
}
void UiRootMenu_clickSettingsEnviroment(GuiItem* item)
{
	GuiItemRoot_addDialogLayout(UiDialogSettingsEnviroment_new());
}
void UiRootMenu_clickLogs(GuiItem* item)
{
	GuiItemRoot_addDialogLayout(UiDialogLog_new());
}

void UiRootMenu_clickShortcuts(GuiItem* item)
{
	GuiItemRoot_addDialogLayout(UiDialogShortcuts_new());
}
void UiRootMenu_clickLicense(GuiItem* item)
{
	GuiItemRoot_addDialogLayout(UiDialogLicense_new(TRUE));
}

void UiRootMenu_clickChangelog(GuiItem* item)
{
	GuiItemRoot_addDialogLayout(UiDialogChangelog_new());
}

void UiRootMenu_clickAbout(GuiItem* item)
{
	GuiItemRoot_addDialogLayout(UiDialogAbout_new());
}

void UiRootMenu_clickFullscreen(GuiItem* item)
{
	UiScreen_switchFullScreenWindow();
}

void UiRootMenu_clickQuit(GuiItem* item)
{
	UiScreen_closeAsk();
}

GuiItem* UiRootMenu_new(Quad2i coord)
{
	GuiItemMenu* menu = (GuiItemMenu*)GuiItemMenu_newImage(coord, GuiImage_new1(UiLogo_init()));

	GuiItemMenu_addItem(menu, DbValue_initLang("NEW"), &UiRootMenu_clickNewProject);
	GuiItemMenu_addItem(menu, DbValue_initLang("OPEN"), &UiRootMenu_clickOpenProject);
	GuiItemMenu_addItem(menu, DbValue_initLang("SAVE"), &UiRootMenu_clickSaveProject);
	GuiItemMenu_addItem(menu, DbValue_initLang("CHANGE_PASSWORD"), &UiRootMenu_clickChangePassword);

	GuiItemMenu_addItem(menu, DbValue_initLang("PROJECT_SETTINGS"), &UiRootMenu_clickSettingsProject);

	GuiItemMenu_addItemEmpty(menu);
	GuiItemMenu_addItem(menu, DbValue_initLang("SETTINGS_ENVIROMENT"), &UiRootMenu_clickSettingsEnviroment);
	//GuiItemMenu_addItem(menu, DbValue_initLang("UPDATES), &UiRootMenu_clickUpdate);
	GuiItemMenu_addItem(menu, DbValue_initLang("LOGS"), &UiRootMenu_clickLogs);

	GuiItemMenu_addItemEmpty(menu);
	//GuiItemMenu_addItem(menu, DbValue_initLang("SHORTCUTS), &UiRootMenu_clickShortcuts);
	GuiItemMenu_addItem(menu, DbValue_initLang("LICENSE"), &UiRootMenu_clickLicense);
	GuiItemMenu_addItem(menu, DbValue_initLang("CHANGELOG"), &UiRootMenu_clickChangelog);
	GuiItemMenu_addItem(menu, DbValue_initLang("ABOUT"), &UiRootMenu_clickAbout);
	GuiItemMenu_addItemEmpty(menu);
	GuiItemMenu_addItem(menu, DbValue_initLang(UiScreen_isFullScreen() ? "WINDOW_MODE" : "FULLSCREEN_MODE"), &UiRootMenu_clickFullscreen);
	GuiItemMenu_addItemEmpty(menu);
	GuiItemMenu_addItem(menu, DbValue_initLang("QUIT"), &UiRootMenu_clickQuit);

	return (GuiItem*)menu;
}

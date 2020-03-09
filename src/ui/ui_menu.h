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

void UiRootMenu_clickNewProject(GuiItem* item)
{
	if (!UiScreen_needSave())
		UiScreen_setWelcome();

	GuiItemRoot_addDialogLayout(UiScreen_needSave() ? UiDialogSave_new(UiDialogSave_NEW) : UiDialogCreate_new());
}
void UiRootMenu_clickOpenProject(GuiItem* item)
{
	if (!UiScreen_needSave())
		UiScreen_setWelcome();
	GuiItemRoot_addDialogLayout(UiScreen_needSave() ? UiDialogSave_new(UiDialogSave_OPEN) : UiDialogOpen_new(0));
}
void UiRootMenu_clickOpenExamples(GuiItem* item)
{
	if (!UiScreen_needSave())
		UiScreen_setWelcome();
	GuiItemRoot_addDialogLayout(UiScreen_needSave() ? UiDialogSave_new(UiDialogSave_OPEN_EXAMPLE) : UiDialogOpenExamples_new(0));
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
void UiRootMenu_clickProjectInfo(GuiItem* item)
{
	GuiItemRoot_addDialogLayout(UiDialogProjectInfo_new());
}
void UiRootMenu_clickEnviroment(GuiItem* item)
{
	GuiItemRoot_addDialogLayout(UiDialogEnviroment_new());
}
void UiRootMenu_clickProjectOptimalization(GuiItem* item)
{
	GuiItemRoot_addDialogLayout(UiDialogProjectOptimalization_new());
}

void UiRootMenu_clickLogs(GuiItem* item)
{
	GuiItemRoot_addDialogLayout(UiDialogLog_new());
}

void UiRootMenu_clickShortcuts(GuiItem* item)
{
	GuiItemRoot_addDialogLayout(UiDialogShortcuts_new());
}
void UiRootMenu_clickEula(GuiItem* item)
{
	GuiItemRoot_addDialogLayout(UiDialogEula_new(TRUE));
}
void UiRootMenu_clickLicense(GuiItem* item)
{
	GuiItemRoot_addDialogLayout(UiDialogLicense_new());
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

void UiRootMenu_clickReportProblem(GuiItem* item)
{
	OsWeb_openEmail("issues@skyalt.com", "Problem");
}

GuiItem* UiRootMenu_new(Quad2i coord)
{
	GuiItemMenu* menu = (GuiItemMenu*)GuiItemMenu_newImage(coord, GuiImage_new1(UiLogo_init()), FALSE);

	GuiItemMenu_addItem(menu, DbValue_initLang("NEW"), &UiRootMenu_clickNewProject);
	GuiItemMenu_addItem(menu, DbValue_initLang("OPEN"), &UiRootMenu_clickOpenProject);
	GuiItemMenu_addItem(menu, DbValue_initLang("OPEN_EXAMPLE"), &UiRootMenu_clickOpenExamples);
	GuiItemMenu_addItem(menu, DbValue_initLang("SAVE"), &UiRootMenu_clickSaveProject);
	GuiItemMenu_addItem(menu, DbValue_initLang("CHANGE_PASSWORD"), &UiRootMenu_clickChangePassword);

	GuiItemMenu_addItem(menu, DbValue_initLang("PROJECT_INFO"), &UiRootMenu_clickProjectInfo);
	GuiItemMenu_addItem(menu, DbValue_initLang("PROJECT_OPTIMALIZATION"), &UiRootMenu_clickProjectOptimalization);

	GuiItemMenu_addItemEmpty(menu);
	GuiItemMenu_addItem(menu, DbValue_initLang("SETTINGS_ENVIROMENT"), &UiRootMenu_clickEnviroment);
	//GuiItemMenu_addItem(menu, DbValue_initLang("UPDATES), &UiRootMenu_clickUpdate);
	GuiItemMenu_addItem(menu, DbValue_initLang("LOGS"), &UiRootMenu_clickLogs);

	GuiItemMenu_addItemEmpty(menu);
	//GuiItemMenu_addItem(menu, DbValue_initLang("SHORTCUTS), &UiRootMenu_clickShortcuts);
	GuiItemMenu_addItem(menu, DbValue_initLang("EULA_SHORT"), &UiRootMenu_clickEula);
	GuiItemMenu_addItem(menu, DbValue_initLang("LICENSE"), &UiRootMenu_clickLicense);
	GuiItemMenu_addItem(menu, DbValue_initLang("CHANGELOG"), &UiRootMenu_clickChangelog);
	GuiItemMenu_addItem(menu, DbValue_initLang("ABOUT"), &UiRootMenu_clickAbout);
	GuiItemMenu_addItemEmpty(menu);
	GuiItemMenu_addItem(menu, DbValue_initLang(UiScreen_isFullScreen() ? "WINDOW_MODE" : "FULLSCREEN_MODE"), &UiRootMenu_clickFullscreen);
	GuiItemMenu_addItemEmpty(menu);
	GuiItemMenu_addItem(menu, DbValue_initLang("REPORT_PROBLEM"), &UiRootMenu_clickReportProblem);
	GuiItemMenu_addItemEmpty(menu);
	GuiItemMenu_addItem(menu, DbValue_initLang("QUIT"), &UiRootMenu_clickQuit);

	return (GuiItem*)menu;
}

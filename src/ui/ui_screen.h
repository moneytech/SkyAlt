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

typedef struct UiScreen_s
{
	Win* win;

	LimitTime limitData;
	LimitTime limitDraw;

	BOOL close;
	BOOL reloadProject;	//index changed

	UNI* newPathOpen;
	UNI* newPathCreate;
	UNI* newPassword;
	BIG newCycles;

	BOOL indexChanged;
}UiScreen;

UiScreen* g_UiScreen = 0;

void UiScreen_updateTheme(void)
{
	switch (UiIniSettings_getTheme())
	{
		case 0:	GuiItemTheme_goOcean();		break;
		case 1:	GuiItemTheme_goRed();		break;
		case 2:	GuiItemTheme_goBlue();		break;
		case 3:	GuiItemTheme_goGreen();		break;
		case 4:	GuiItemTheme_goGrey();		break;
	}
}

void UiScreen_setTheme(int theme)
{
	if (theme < 0 || theme > 4)	//rotation(ctrl+t)
		theme = 0;

	UiIniSettings_setTheme(theme);
	UiScreen_askResize();
}

void UiScreen_changeTheme(int add)
{
	UiScreen_setTheme(UiIniSettings_getTheme() + add);
}

static void _UiScreen_updateDPI(void)
{
	OsWinIO_setDPI(UiIniSettings_getDPI());
}

void UiScreen_setDPI(int dpi)
{
	UiIniSettings_setDPI(dpi);
	UiScreen_askResize();
}

void UiScreen_changeDPI(int add)
{
	UiScreen_setDPI(OsWinIO_getDPI() - add);
}

BOOL UiScreen_isIndexChanged(void)
{
	return g_UiScreen->indexChanged;
}
void UiScreen_setIndexChanged(BOOL indexChanged)
{
	if (g_UiScreen->indexChanged != indexChanged)
		GuiItemRoot_resizeAll();
	g_UiScreen->indexChanged = indexChanged;
}
void UiScreen_reloadProject(void)
{
	g_UiScreen->reloadProject = TRUE;
}

Win* UiScreen_getWin(void)
{
	return g_UiScreen->win;
}

BOOL UiScreen_needSave(void)
{
	return (DbRoot_is() && DbRoot_numAllChanges());
}

void UiScreen_closeHard(void)
{
	g_UiScreen->close = TRUE;
}
void UiScreen_closeAsk(void)
{
	if (UiScreen_needSave())
		GuiItemRoot_addDialogLayout(UiDialogSave_new(UiDialogSave_CLOSE));
	else
		UiScreen_closeHard();
}

static void UiScreen_updateWindowTitle(void)
{
	char* titleName;

	if (DbRoot_is())
	{
		const char* project = FileProject_getPath();

		UiIniSettings_setProject(project);	//update .ini

		UNI* lic = License_getTitle();

		titleName = Std_newCHAR(STD_TITLE);
		titleName = Std_addAfterCHAR(titleName, "(");
		titleName = Std_addAfterCHAR_uni(titleName, lic);
		titleName = Std_addAfterCHAR(titleName, ")");

		titleName = Std_addAfterCHAR(titleName, " - ");

		titleName = Std_addAfterCHAR(titleName, project);

		Std_deleteUNI(lic);
	}
	else
		titleName = UiScreen_getNameVersion();

	Win_setTitle(UiScreen_getWin(), titleName);
	Std_deleteCHAR(titleName);
}

void UiScreen_openProject(const UNI* path, const UNI* password)
{
	g_UiScreen->newPathOpen = Std_newUNI(path);
	g_UiScreen->newPassword = Std_newUNI(password);
	g_UiScreen->newCycles = 0;
}
void UiScreen_createProject(const UNI* path, const UNI* password, BIG cycles)
{
	g_UiScreen->newPathCreate = Std_newUNI(path);
	g_UiScreen->newPassword = Std_newUNI(password);
	g_UiScreen->newCycles = cycles;
}

BOOL UiScreen_tryNewProject(void)
{
	BOOL changed = Std_sizeUNI(g_UiScreen->newPathCreate) || Std_sizeUNI(g_UiScreen->newPathOpen) || g_UiScreen->reloadProject;

	if (Std_sizeUNI(g_UiScreen->newPathCreate))
	{
		char* path = Std_newCHAR_uni(g_UiScreen->newPathCreate);

		FileProject_newCreate(path, g_UiScreen->newPassword, g_UiScreen->newCycles);
		DbRoot_newCreate();
		DbRoot_save();

		if (!StdProgress_is())
			OsFileDir_removeFile(path);

		Std_deleteCHAR(path);
	}

	if (Std_sizeUNI(g_UiScreen->newPathOpen))
	{
		char* path = Std_newCHAR_uni(g_UiScreen->newPathOpen);
		if (FileProject_newOpen(path, g_UiScreen->newPassword))
			DbRoot_newOpen();

		Std_deleteCHAR(path);
	}

	Std_deleteUNI(g_UiScreen->newPathCreate);
	Std_deleteUNI(g_UiScreen->newPathOpen);
	Std_deleteUNI(g_UiScreen->newPassword);
	g_UiScreen->newPathCreate = 0;
	g_UiScreen->newPathOpen = 0;
	g_UiScreen->newPassword = 0;

	if (g_UiScreen->reloadProject)
	{
		DbRoot_newOpen();
		g_UiScreen->reloadProject = FALSE;
		g_UiScreen->indexChanged = FALSE;
	}

	if (changed)
		UiScreen_updateWindowTitle();

	return changed && DbRoot_is();
}

void UiScreen_askResize(void)
{
	Win_pleaseResize(UiScreen_getWin());
}
void UiScreen_askResizeHard(void)
{
	GuiItemRoot_resizeAll();
}
char* UiScreen_getNameVersion(void)
{
	char nm[32];
	snprintf(nm, sizeof(nm), "%s build.%d", STD_TITLE, STD_BUILD);
	return Std_newCHAR(nm);
}

UNI* UiScreen_getNameVersionUNI(void)
{
	char* str = UiScreen_getNameVersion();
	UNI* uni = Std_newUNI_char(str);
	Std_deleteCHAR(str);
	return uni;
}

void UiScreen_setWelcome(void)
{
	GuiItemRoot_setContentLayout(UiWelcome_new());
}

void UiScreen_setStartup(void)
{
	GuiItemRoot_setContentLayout(UiStartup_new());
}

void UiScreen_setRoot(void)
{
	GuiItemRoot_setContentLayout(UiRoot_new());

	if (License_exist())
	{
		if (License_isTimeValid())
		{
			if (License_getRemainingDays() < LICENSE_DAYS_WARNING)
				Logs_addInfo("LICENSE_EXPIRATION_SOON");
		}
		else
			Logs_addError("COMMERCIAL_EXPIRED");
	}
}

BOOL UiScreen_tick(void* selff, Quad2i* redrawRect);

void UiScreen_delete(void)
{
	if (g_UiScreen)
	{
		Quad2i coord;
		Win_getWindowCoord(g_UiScreen->win, &coord);
		UiIniSettings_setWindow(coord);
		UiIniSettings_setLanguage(Lang_getLangName());

		Win_delete(g_UiScreen->win);
		Os_free(g_UiScreen, sizeof(UiScreen));
		g_UiScreen = 0;
	}
}

BOOL UiScreen_new(void)
{
	if (g_UiScreen)
		UiScreen_delete();

	g_UiScreen = Os_malloc(sizeof(UiScreen));

	g_UiScreen->close = FALSE;
	g_UiScreen->limitData = LimitTime_initFps(MAX_DATA_FPS);
	g_UiScreen->limitDraw = LimitTime_initFps(MAX_RENDER_FPS);

	Quad2i coord = UiIniSettings_getWindow();
	g_UiScreen->win = Win_new(&coord, &UiScreen_tick, g_UiScreen);

	g_UiScreen->newPathOpen = 0;
	g_UiScreen->newPathCreate = 0;
	g_UiScreen->newPassword = 0;
	g_UiScreen->newCycles = 0;

	g_UiScreen->indexChanged = FALSE;
	g_UiScreen->reloadProject = FALSE;

	UiScreen_updateWindowTitle();

	return TRUE;
}

void UiScreen_start(void)
{
	if (UiIniSettings_isLicenseAccepted())
	{
		UiScreen_setWelcome();
	}
	else
	{
		GuiItemRoot_setContentLayout(UiDialogEula_new(FALSE));
	}

	g_ini->file_existed = TRUE;
	UiScreen_updateWindowTitle();

	Win_start(g_UiScreen->win);
}

BOOL UiScreen_isFullScreen(void)
{
	return Win_isFullscreen(g_UiScreen->win);
}

void UiScreen_switchFullScreenWindow(void)
{
	Win_setFullscreen(g_UiScreen->win, !UiScreen_isFullScreen());
}

BOOL UiScreen_tick(void* selff, Quad2i* redrawRect)
{
	UiScreen* self = selff;

	if (UiScreen_tryNewProject())
		UiScreen_setRoot();

	//touch
	if (OsWinIO_isTouch())
	{
		LimitTime_reset(&self->limitData);	//faster update

		if (OsWinIO_getTouch_action() == Win_TOUCH_WHEEL)
		{
			if (OsWinIO_getKeyExtra() == Win_EXTRAKEY_CTRL)	//change DPI
			{
				UiScreen_changeDPI(OsWinIO_getTouch_wheel() * 3);
				OsWinIO_resetTouch();
			}
		}
	}

	//key
	if (OsWinIO_isKey())
	{
		if (OsWinIO_getKeyExtra() & Win_EXTRAKEY_ZOOM_0)	UiScreen_changeDPI(OsWinIO_getDPI() - OsScreen_getDPI());
		if (OsWinIO_getKeyExtra() & Win_EXTRAKEY_ZOOM_A)	UiScreen_changeDPI(-3);
		if (OsWinIO_getKeyExtra() & Win_EXTRAKEY_ZOOM_S)	UiScreen_changeDPI(+3);

		if (OsWinIO_getKeyExtra() & Win_EXTRAKEY_THEME)	UiScreen_changeTheme(1);

		if (OsWinIO_getKeyExtra() & Win_EXTRAKEY_FULLSCREEN)	UiScreen_switchFullScreenWindow();

		if (OsWinIO_getKeyExtra() & Win_EXTRAKEY_LOG)		GuiItemRoot_addDialogLayout(UiDialogLog_new());

		GuiItemRoot_key(UiScreen_getWin());

		LimitTime_reset(&self->limitData);
	}

	if (Win_isResize(self->win))
	{
		_UiScreen_updateDPI();
		UiScreen_updateTheme();

		UiScreen_askResizeHard();

		LimitTime_reset(&self->limitData);
		LimitTime_reset(&self->limitDraw);

		*redrawRect = Quad2i_init2(Vec2i_init(), Win_getImage(self->win).size);

		Win_resetResize(self->win);
	}

	//render
	BOOL doDraw = LimitTime_isTimeout(&self->limitDraw);	//smazat ...
	BOOL doUpdate = LimitTime_isTimeout(&self->limitData);

	if (UiAutoUpdate_isRunFinished())
		if (UiAutoUpdate_hasUpdate())
			;//GuiItemRoot_setContext(&UiDialogUpdate_new()->base.base, 0);

	//draw
	GuiItemRoot_tick(doUpdate, doDraw, UiScreen_getWin(), redrawRect);

	//log
	if (Logs_isChanged())
		GuiItemRoot_addDialogLayout(UiDialogLog_new());

	//exit
	if (OsWinIO_isExit())
	{
		UiScreen_closeAsk();
	}

	if (UiIniSettings_getLicenseVersion() < 0)
		self->close = TRUE;

	OsWinIO_tick();

	if (DbRoot_is())
	{
		if (DbRoot_tryUpdateFileIndexes())
		{
			g_UiScreen->indexChanged = TRUE;
			GuiItemRoot_resizeAll();
		}
	}

	return !self->close;
}

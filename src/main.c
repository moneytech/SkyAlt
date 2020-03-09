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

#include "os.h"
#include "std.h"
#include "language.h"
#include "log.h"
#include "file.h"
#include "db.h"
#include "gui.h"
#include "ui.h"
#include "media.h"
#include "map.h"
#include "license.h"



const char* __asan_default_options() { return "malloc_context_size=100";/*divide by ':'*/ }	//fsanitize will print longer scope

int main(int argc, char** argv)
{
#ifndef _DEBUG
	Os_showConsole(FALSE);
#endif
	StdProgress_initGlobal();
	Logs_initGlobal();
	OsODBC_initGlobal();
	OSMedia_initGlobal();
	Win_init();
	OsCrypto_initGlobal();
	OsNet_init();
	OsHTTPS_initGlobal();
	OsXml_initGlobal();

	GuiItemTheme_init();
	if (!OsWinIO_new())
		return -1;
	UiIniSettings_new();
	Map_new();
	UiAutoUpdate_new();
	if (UiIniSettings_getUpdate())
		;//UiAutoUpdate_run();

	License_init();

	//UiIcons_convertFolder("../icons");
	//UiLogo_convertZlib();
	//MapTiles_createCache();
	//MapPoly_createCache();
	//OsXml_testGpx("D:/social_data/nike/2010-11-16 102541-Nike-Activity.gpx");

	//License_makeFile(_UNI32("SkyAlt inc."), 36, 12);

	if (Lang_initGlobal(UiIniSettings_getLanguage()))
	{
		UiAutoUpdate_cleanOld();
		//DbRoot_initProgress();

		UiScreen_new();
		FileCache_new();
		MediaNetwork_new(UiIniSettings_isNetworkOnline());
		Map_updateNet();
		MediaLibrary_new();
		GuiItemRoot_new();

		//open project(from parameter or .ini)
		{
			UNI* lastProject = 0;
			if (argc > 1)	//open file: "./ChangeOver 1.alt pass"
				lastProject = Std_newUNI_char(argv[1]);
			else
				lastProject = Std_newUNI_char(UiIniSettings_getProject());

			if (Std_isUNI(lastProject))
			{
				Vec2i pos = Vec2i_init2(10, 10);
				OsWinIO_setDropFromInside(lastProject, &pos);
			}
			Std_deleteUNI(lastProject);
		}

		UiScreen_start();

		//<< waiting for close >>

		GuiItemRoot_delete();
		DbRoot_delete();
		FileProject_delete();
		MediaLibrary_delete();
		MediaNetwork_delete();
		FileCache_delete();
		UiScreen_delete();
	}
	else
	{
		Os_showConsole(TRUE);
		Os_consoleWaitForKey();
	}

	License_free();
	Map_delete();
	UiIniSettings_delete();

	UiAutoUpdate_delete();
	OsWinIO_delete();

	//DbRoot_freeProgress();

	OsXml_freeGlobal();
	OsHTTPS_freeGlobal();
	OsNet_free();
	OsCrypto_freeGlobal();
	OSMedia_freeGlobal();
	OsODBC_freeGlobal();
	Logs_freeGlobal();
	Lang_freeGlobal();
	StdProgress_freeGlobal();

	Os_showMemleaks();

	return 0;
}
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

#include "os.h"
#include "std.h"
#include "language.h"
#include "log.h"
#include "file.h"
#include "db.h"
#include "gui.h"
#include "ui.h"
#include "media.h"

 //#include "ui/ui_logo.h"
 //#include "ui/ui_icons.h"

const char* __asan_default_options() { return "malloc_context_size=100";/*divide by ':'*/ }	//fsanitize will print longer scope

int main(int argc, char** argv)
{
#ifndef _DEBUG
	Os_showConsole(FALSE);
#endif
	Logs_initGlobal();
	Win_init();
	OsCrypto_initGlobal();
	OsNet_init();
	OsHTTPS_initGlobal();
	GuiItemTheme_init();
	if (!OsWinIO_new())
		return -1;
	UiIniSettings_new();
	UiAutoUpdate_new();
	if (UiIniSettings_getUpdate())
		;//UiAutoUpdate_run();

	//UiIcons_convertFolder("../icons");
	//UiLogo_convertZlib();

	if (Lang_initGlobal(UiIniSettings_getLanguage()))
	{
		UiAutoUpdate_cleanOld();
		DbRoot_initProgress();

		UiScreen_new();
		FileCache_new();
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

		//UiApp_new();
		UiScreen_start();

		//<< waiting for close >>

		GuiItemRoot_delete();
		DbRoot_delete();
		MediaLibrary_delete();
		FileCache_delete();
		UiScreen_delete();
	}
	else
	{
		//show console + err
		printf("Error: Missing Language(Translation)\n");
		Os_showConsole(TRUE);
	}

	UiIniSettings_delete();

	UiAutoUpdate_delete();
	OsWinIO_delete();

	DbRoot_freeProgress();

	OsHTTPS_freeGlobal();
	OsNet_free();
	OsCrypto_freeGlobal();
	Logs_freeGlobal();
	Lang_freeGlobal();

	Os_showMemleaks();

	return 0;
}
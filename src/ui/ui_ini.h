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

typedef struct UiIniSettings_s
{
	BOOL file_existed;
	int license_ver;
	int update;
	int theme;
	int threads;
	int dpi;
	Quad2i win;
	char* language;	//en, cs
	char* project;

	OsDateTYPE dateFormat;
} UiIniSettings;

UiIniSettings* g_ini = 0;

const char* UiIniSettings_getLanguage(void)
{
	return g_ini->language;
}
int UiIniSettings_getLanguagePos(void)
{
	return Std_max(0, Lang_findLangNamePos(g_ini->language));
}

DbValue UiIniSettings_getLanguageValueMsg(void)
{
	return DbValue_initNumber(UiIniSettings_getLanguagePos());
}

void UiIniSettings_setLanguage(const char* language)
{
	Std_deleteCHAR(g_ini->language);
	g_ini->language = Std_newCHAR(language);
}

void UiIniSettings_setDateFormat(OsDateTYPE dateFormat)
{
	g_ini->dateFormat = dateFormat;
}

OsDateTYPE UiIniSettings_getDateFormat(void)
{
	return g_ini->dateFormat;
}

void UiIniSettings_setProject(const char* projectPath)
{
	Std_rewriteCHAR(&g_ini->project, projectPath);
}

void UiIniSettings_setLicenseVersion(int version)
{
	g_ini->license_ver = version;
}
void UiIniSettings_setLicenseAccept(void)
{
	UiIniSettings_setLicenseVersion(STD_LICENSE_VER);
}

void UiIniSettings_setTheme(int theme)
{
	g_ini->theme = theme;
}

void UiIniSettings_setWindow(Quad2i coord)
{
	g_ini->win = coord;
}

void UiIniSettings_setDPI(int dpi)
{
	g_ini->dpi = dpi;
}

void UiIniSettings_setUpdate(int update)
{
	g_ini->update = update;
}

UINT UiIniSettings_getTheme(void)
{
	return g_ini->theme;
}

UBIG UiIniSettings_getNumThreads(void)
{
	UBIG n = g_ini->threads;
	if (n == 0)
		n = OsThread_getNumCPUCores();
	return n;
}

UINT UiIniSettings_getThreads(void)
{
	return g_ini->threads;
}

void UiIniSettings_setThreads(int threads)
{
	g_ini->threads = threads;
}

Quad2i UiIniSettings_getWindow(void)
{
	return g_ini->win;
}

int UiIniSettings_getLicenseVersion(void)
{
	return g_ini->license_ver;
}
BOOL UiIniSettings_isLicenseAccepted(void)
{
	return UiIniSettings_getLicenseVersion() >= STD_LICENSE_VER;
}

UINT UiIniSettings_getDPI(void)
{
	return g_ini->dpi;
}

const char* UiIniSettings_getProject(void)
{
	return g_ini->project;
}

UINT UiIniSettings_getUpdate(void)
{
	return g_ini->update;
}

BOOL UiIniSettings_new(void)
{
	g_ini = Os_malloc(sizeof(UiIniSettings));

	g_ini->file_existed = OsFile_existFile(STD_INI_PATH);

	g_ini->license_ver = 0;
	g_ini->update = 1;
	g_ini->theme = 0;

	g_ini->threads = 0;	//auto: get from Operation system
	g_ini->dpi = 0;

	g_ini->win = Quad2i_init();

	g_ini->language = 0;
	g_ini->project = 0;

	int timeZone = Os_timeZone();
	g_ini->dateFormat = (timeZone <= -3 && timeZone >= -10) ? OsDate_US : OsDate_EU;

	UiIniSettings_setDPI(OsScreen_getDPI());
	Quad2i def;
	OsScreen_getDefaultCoord(&def);
	UiIniSettings_setWindow(def);

	OsFile file;
	if (OsFile_init(&file, STD_INI_PATH, OsFile_R))
	{
		char* line;
		while ((line = OsFile_readLine(&file)))
		{
			int value1, value2, value3, value4;
			char str[64];

			if (sscanf(line, "license = %d", &value1) == 1)
				g_ini->license_ver = Std_abs(value1);

			if (sscanf(line, "update = %d", &value1) == 1)
				g_ini->update = value1;

			if (sscanf(line, "theme = %d", &value1) == 1)
				g_ini->theme = value1;

			if (sscanf(line, "threads = %d", &value1) == 1)
				g_ini->threads = Std_clamp(value1, 0, 256);

			if (sscanf(line, "dpi = %d", &value1) == 1)
				g_ini->dpi = Std_clamp(value1, 0, 256);

			if (sscanf(line, "window = %d %d %d %d", &value1, &value2, &value3, &value4) == 4)
			{
				Vec2i maxRes;
				OsScreen_getMonitorResolution(&maxRes);

				value1 = Std_clamp(value1, 0, maxRes.x - 1);
				value2 = Std_clamp(value2, 0, maxRes.y - 1);
				value3 = Std_clamp(value3, 1, maxRes.x);
				value4 = Std_clamp(value4, 1, maxRes.y);

				UiIniSettings_setWindow(Quad2i_init4(value1, value2, value3, value4));
			}

			if (sscanf(line, "language = %63s", str) == 1)
				UiIniSettings_setLanguage(str);

			if (sscanf(line, "date = %d", &value1) == 1)
				g_ini->dateFormat = Std_clamp(value1, 0, 3);

			if (sscanf(line, "project = %63s", str) == 1)
				UiIniSettings_setProject(str);

			Std_deleteCHAR(line);
		}

		OsFile_free(&file);
	}
	return TRUE;
}

void UiIniSettings_delete(void)
{
	if (g_ini)
	{
		OsFile file;
		if (OsFile_init(&file, STD_INI_PATH, OsFile_W))
		{
			char line[128];

			snprintf(line, sizeof(line), "license = %d\r\n", Std_max(0, g_ini->license_ver));
			OsFile_write(&file, line, Std_sizeCHAR(line));

			snprintf(line, sizeof(line), "update = %d\r\n", g_ini->update);
			OsFile_write(&file, line, Std_sizeCHAR(line));

			snprintf(line, sizeof(line), "theme = %d\r\n", g_ini->theme);
			OsFile_write(&file, line, Std_sizeCHAR(line));
			snprintf(line, sizeof(line), "threads = %d\r\n", g_ini->threads);
			OsFile_write(&file, line, Std_sizeCHAR(line));

			snprintf(line, sizeof(line), "dpi = %d\r\n", g_ini->dpi);
			OsFile_write(&file, line, Std_sizeCHAR(line));

			snprintf(line, sizeof(line), "window = %d %d %d %d\r\n", g_ini->win.start.x, g_ini->win.start.y, g_ini->win.size.x, g_ini->win.size.y);
			OsFile_write(&file, line, Std_sizeCHAR(line));

			const char* lang = UiIniSettings_getLanguage();
			if (lang)
			{
				snprintf(line, sizeof(line), "language = %s\r\n", lang);
				OsFile_write(&file, line, Std_sizeCHAR(line));
			}

			snprintf(line, sizeof(line), "date = %d\r\n", g_ini->dateFormat);
			OsFile_write(&file, line, Std_sizeCHAR(line));

			const char* prj = UiIniSettings_getProject();
			if (prj)
			{
				snprintf(line, sizeof(line), "project = %s\r\n", prj);
				OsFile_write(&file, line, Std_sizeCHAR(line));
			}

			OsFile_free(&file);
		}

		Std_deleteCHAR(g_ini->project);

		Os_free(g_ini, sizeof(UiIniSettings));
		g_ini = 0;
	}
}

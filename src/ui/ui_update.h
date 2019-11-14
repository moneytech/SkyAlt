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

typedef struct UiAutoUpdate_s
{
	OsThread thread;
	UCHAR* exe;
	UBIG exe_size;
	UINT version;
	volatile BOOL checkingFinished;
} UiAutoUpdate;

UiAutoUpdate* g_autoupdate = 0;

THREAD_FUNC(_UiAutoUpdate_loop, param);

BOOL UiAutoUpdate_isRunning(void)
{
	return OsThread_tick(&g_autoupdate->thread);
}

BOOL UiAutoUpdate_hasUpdate(void)
{
	return !UiAutoUpdate_isRunning() && g_autoupdate->exe_size > 0;
}

BOOL UiAutoUpdate_run(void)
{
	if (!UiAutoUpdate_isRunning())
		return OsThread_init(&g_autoupdate->thread, g_autoupdate, _UiAutoUpdate_loop) == 0;
	return FALSE;
}

BOOL UiAutoUpdate_isRunFinished(void)
{
	BOOL fin = g_autoupdate->checkingFinished;
	g_autoupdate->checkingFinished = FALSE;
	return fin;
}

BOOL UiAutoUpdate_new(void)
{
	g_autoupdate = Os_malloc(sizeof(UiAutoUpdate));

	g_autoupdate->exe = 0;
	g_autoupdate->exe_size = 0;
	g_autoupdate->checkingFinished = FALSE;
	g_autoupdate->version = 0;
	g_autoupdate->thread = OsThread_initEmpty();

	return TRUE;
}

void UiAutoUpdate_delete(void)
{
	if (g_autoupdate)
	{
		OsThread_free(&g_autoupdate->thread, TRUE);

		Os_free(g_autoupdate->exe, g_autoupdate->exe_size);

		Os_free(g_autoupdate, sizeof(UiAutoUpdate));
		g_autoupdate = 0;
	}
}

BIG UiAutoUpdate_getVersionUNI(void)
{
	if (UiAutoUpdate_isRunning())
		return -1; //work in progress

	return g_autoupdate->version;
}


static BOOL _UiAutoUpdate_getFileHash(void* file, UBIG file_size, OsCryptoSha2* hash, UINT version)
{
	//BOOL OsFile_initHash(const char* path, OsCryptoSha2* out);

	if (OsCryptoSha2_exe(file, file_size, hash))
	{
		//replace first 4 Bytes with version
		Os_memcpy(&hash->m_key[0], &version, 4); //add version, so nobody will be able to substitute an older version(of exe) like the new-one
		return TRUE;
	}
	return FALSE;
}

BOOL UiAutoUpdate_cleanOld()
{
	char* path = OsFileDir_currentProgramDir();
	char* pathOld = Std_addCHAR(path, ".old");

	BOOL ok = OsFileDir_removeFile(pathOld);

	Std_deleteCHAR(pathOld);
	Std_deleteCHAR(path);
	return ok;
}

BOOL UiAutoUpdate_updateFile(void)
{
	BOOL ok = FALSE;

	char* path = OsFileDir_currentProgramDir();
	char* pathOld = Std_addCHAR(path, ".old");

	OsFile_unlink(path);	//maybe not needed? ...

	if (OsFileDir_renameFile(path, pathOld)) //renames program.exe -> program.old
	{
		OsFile ff;
		if (OsFile_init(&ff, path, OsFile_W))
		{
			if (OsFile_write(&ff, g_autoupdate->exe, g_autoupdate->exe_size) == g_autoupdate->exe_size)
			{
				printf("Program file has been upgraded to build.%d!\n", g_autoupdate->version); //translation ...
				ok = TRUE;
			}
			OsFile_free(&ff);

			//chmod(path, S_IRWXG);	//...
		}
	}

	Std_deleteCHAR(pathOld);
	Std_deleteCHAR(path);
	return ok;
}

static BIG UiAutoUpdate_download(const char* EXT, const char* OS, UINT version, UCHAR** output)
{
	BIG size = -1;
	OsHTTPS stats;
	if (OsHTTPS_init(&stats, STD_UPDATE_SERVER))
	{
		char post[128];
		snprintf(post, sizeof(post), "/bin/SkyAlt-%s-%d%s", OS, version, EXT); //SkyAlt-Linux-1.sig

		size = OsHTTPS_get(&stats, post, (char**)output); //sends request
		OsHTTPS_free(&stats);
	}
	return size;
}

THREAD_FUNC(_UiAutoUpdate_loop, param)
{
	//find max version
	int version = STD_BUILD + 1;
	{
		UCHAR* find = 0;
		while (UiAutoUpdate_isRunning() && UiAutoUpdate_download(".sig", STD_OS, version, &find) == sizeof(OsCryptoECDSASign))
		{
			version++;
			Os_free(find, sizeof(OsCryptoECDSASign));
			find = 0;
		}
		Os_free(find, 0);
		version--;
	}

	//download
	BIG sign_size = 0;
	BIG exe_size = 0;
	UCHAR* exe = 0;
	UCHAR* sign = 0;
	if (UiAutoUpdate_isRunning() && version != STD_BUILD)
	{
		sign_size = UiAutoUpdate_download(".sig", STD_OS, version, &sign);
		exe_size = UiAutoUpdate_download(STD_FILE_EXE, STD_OS, version, &exe);
	}

	//check
	if (UiAutoUpdate_isRunning() && exe_size > 0 && sign_size == sizeof(OsCryptoECDSASign))
	{
		OsCryptoSha2 hash;
		if (_UiAutoUpdate_getFileHash(exe, exe_size, &hash, version))
		{
			OsCryptoECDSA ecdsa;
			if (OsCryptoECDSA_initFromPublic(&ecdsa, (OsCryptoECDSAPublic*)Os_getUpdatePublicKey()))
			{
				if (OsCryptoECDSA_verify(&ecdsa, (UCHAR*)&hash, sizeof(OsCryptoSha2), (OsCryptoECDSASign*)sign, sign_size))
				{
					g_autoupdate->version = version;

					g_autoupdate->exe = exe;
					g_autoupdate->exe_size = exe_size;

					exe = 0;
				}
				OsCryptoECDSA_free(&ecdsa);
			}
		}
	}

	Os_free(sign, sign_size);
	Os_free(exe, exe_size);
	g_autoupdate->thread.m_running = FALSE;
	g_autoupdate->checkingFinished = TRUE;

	if (!g_autoupdate->exe)
		g_autoupdate->version = STD_BUILD;

	if (UiAutoUpdate_hasUpdate())
		printf("New build.%d is available\n", g_autoupdate->version);

	return 0;
}

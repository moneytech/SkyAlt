/*
 * Copyright (c) 2018 Milan Suk
 *
 * Use of this software is governed by the Business Source License included
 * in the LICENSE file and at www.mariadb.com/bsl11.
 *
 * Change Date: 2025-02-01
 *
 * On the date above, in accordance with the Business Source License, use
 * of this software will be governed by version 2 or later of the General
 * Public License.
 */

#define FileProject_UPDATE_INDEX_TIMEOUT_SEC 30	//1/2 minute

typedef struct FileProject_s
{
	char* projectPath;
	FileKey* key;
	StdArr users;	//<FileUser*>
	FileRow uid;

	double updateIndex_time;
} FileProject;

FileProject* g_FileProject = 0;

BOOL FileProject_is(void)
{
	return g_FileProject != 0;
}

UBIG FileProject_numUsers(void)
{
	return g_FileProject->users.num;
}
FileUser* FileProject_getUser(UBIG i)
{
	return g_FileProject->users.ptrs[i];
}

FileUser* FileProject_findUser(FileRow id)
{
	int i;
	for (i = 0; i < g_FileProject->users.num; i++)
	{
		FileUser* user = FileProject_getUser(i);
		if (FileUser_isId(user, id))
			return user;
	}
	return 0;
}
FileUser* FileProject_getUserMe(void)
{
	return FileProject_findUser(g_FileProject->uid);
}

FileUser* FileProject_addUser(FileRow userId)
{
	FileUser* user = FileProject_findUser(userId);
	if (!user)
	{
		user = FileUser_new(userId);
		StdArr_add(&g_FileProject->users, user);
	}
	return user;
}

static void _FileProject_updateUserList(void)
{
	char** folders;
	const BIG N = OsFileDir_getFileList(g_FileProject->projectPath, FALSE, TRUE, FALSE, &folders);
	int i;
	for (i = 0; i < N; i++)
	{
		int len = Std_sizeCHAR(folders[i]);
		if (len == (8 * 2))
			FileProject_addUser(FileUser_convertToId(folders[i]));

		Std_deleteCHAR(folders[i]);
	}
	Os_free(folders, N * sizeof(char*));
}

void FileProject_deleteEx(FileProject* self)
{
	StdArr_freeFn(&self->users, (StdArrFREE)&FileUser_delete);

	Std_deleteCHAR(self->projectPath);

	if (self->key)
		FileKey_delete(self->key);

	Os_free(self, sizeof(FileProject));
}
void FileProject_delete(void)
{
	if (g_FileProject)
		FileProject_deleteEx(g_FileProject);
	g_FileProject = 0;
}

FileRow FileProject_createUID(void)
{
	FileRow fr;
	fr.row = 0;
	fr.user = Os_getUID();
	return fr;
}

BOOL _FileProject_createDeviceFolder(const char* projectPath)
{
	char* userPath = FileUser_convertPath(projectPath, g_FileProject->uid);

	BOOL ok = OsFileDir_makeDir(userPath);
	if (ok)
	{
		FileProject_addUser(g_FileProject->uid);

		//create 'files' dir
		userPath = Std_addAfterCHAR(userPath, "/files");
		ok = OsFileDir_makeDir(userPath);
	}
	Std_deleteCHAR(userPath);

	return ok;
}

char* FileProject_cleanPath(const char* path)
{
	char* ret = Std_newCHAR(path);

	char* find = 0;
	char* t = ret;
	while (t && (t = (char*)Std_findSubCHAR(t, ".sky")))
	{
		find = t;
		t += Std_sizeCHAR(".sky");
	}

	if (find)
		*find = 0;	//cut .sky

	return ret;
}

static void _FileProject_addMapTiles(void)
{
	char* path = FileUser_getMapPath(FileProject_getUserMe());
	Map_loadUserTiles(path);
	Std_deleteCHAR(path);
}

BOOL FileProject_newOpen(const char* path, const UNI* password)
{
	char* projectPath = FileProject_cleanPath(path);

	{
		char* vStr = Std_addCHAR(projectPath, ".sky");
		UBIG size;
		UCHAR* version = OsFile_initRead(vStr, &size, 0);
		int v = version ? version[0] : -1;
		Os_free(version, size);
		Std_deleteCHAR(vStr);

		if (v == -1)
			printf("Warning: Project root/version file is missing\n");	//SLOG ...
		else
		if (size != 1 || v != '1')
		{
			printf("Error: Project version is invalid\n");	//SLOG ...
			Std_deleteCHAR(projectPath);
			return 0;
		}
	}

	FileProject_delete();
	g_FileProject = Os_malloc(sizeof(FileProject));
	g_FileProject->users = StdArr_init();
	g_FileProject->key = 0;
	g_FileProject->projectPath = projectPath;

	g_FileProject->uid = FileProject_createUID();
	g_FileProject->key = FileKey_newLoad(projectPath, password);
	g_FileProject->updateIndex_time = 0;

	BOOL ok = TRUE;
	_FileProject_updateUserList();
	if (!FileProject_getUserMe())
		ok &= _FileProject_createDeviceFolder(projectPath);

	ok &= g_FileProject->key && FileProject_getUserMe();

	if (ok)
		_FileProject_addMapTiles();

	if (!ok)
		FileProject_delete();

	return ok;
}

BOOL FileProject_newCreate(const char* path, const UNI* password, BIG cycles)
{
	char* projectPath = FileProject_cleanPath(path);

	FileProject_delete();
	g_FileProject = Os_malloc(sizeof(FileProject));
	g_FileProject->users = StdArr_init();
	g_FileProject->key = 0;
	g_FileProject->projectPath = projectPath;
	g_FileProject->uid = FileProject_createUID();
	g_FileProject->updateIndex_time = 0;

	BOOL ok = FALSE;
	if (OsFileDir_makeDir(projectPath))
	{
		g_FileProject->key = FileKey_newCreate(projectPath, password, cycles);
		ok = (g_FileProject->key != 0);
	}

	if (ok)
	{
		char* vStr = Std_addCHAR(projectPath, ".sky");
		ok = OsFile_initWrite(vStr, "1", 1);
		Std_deleteCHAR(vStr);
	}

	if (ok)
		ok = _FileProject_createDeviceFolder(projectPath);

	if (ok)
		_FileProject_addMapTiles();

	if (!ok)
		FileProject_delete();

	return ok;
}

FileRow FileProject_getUID(void)
{
	return g_FileProject->uid;
}

UNI* FileProject_getUID_string(void)
{
	char str[(8 * 2) + 1];
	Std_setHEX_char(&str[0], (UCHAR*)&g_FileProject->uid, 8);
	return Std_newUNI_char(str);
}

const char* FileProject_getPathEx(FileProject* self)
{
	return self->projectPath;
}

const char* FileProject_getPath(void)
{
	return g_FileProject ? FileProject_getPathEx(g_FileProject) : 0;
}

FileKey* FileProject_getKey(void)
{
	return g_FileProject ? g_FileProject->key : 0;
}

void FileProject_encryptDirect(const UBIG block, unsigned char* plain_text, unsigned char* cipher_text, int size)
{
	FileKey_aesEncryptDirect(FileProject_getKey(), block, plain_text, cipher_text, size);
}
void FileProject_decryptDirect(const UBIG block, unsigned char* cipher_text, unsigned char* plain_text, int size)
{
	FileKey_aesDecryptDirect(FileProject_getKey(), block, cipher_text, plain_text, size);
}

UBIG FileProject_getProjectSize(void)
{
	UBIG sum = 0;
	int i;
	for (i = 0; i < g_FileProject->users.num; i++)
		sum += FileUser_getProjectSize(g_FileProject->users.ptrs[i]);

	return sum;
}

StdArr FileProject_openColumns(FileRow fileId, BOOL write)
{
	StdArr files = StdArr_init();
	int i;
	for (i = 0; i < g_FileProject->users.num; i++)
	{
		FileFile* file = FileFile_newColumn(g_FileProject->users.ptrs[i], fileId, write);
		if (file)
			StdArr_add(&files, file);
	}
	return files;
}

UBIG FileProject_bytesColumns(FileRow fileId)
{
	UBIG bytes = 0;
	int i;
	for (i = 0; i < g_FileProject->users.num; i++)
		bytes += FileFile_bytesColumn(g_FileProject->users.ptrs[i], fileId);
	return bytes;
}

FileFile* FileProject_openFile(FileRow fileId, BOOL write)
{
	FileFile* file = 0;
	int i;
	for (i = 0; i < g_FileProject->users.num; i++)
	{
		file = FileFile_newFile(g_FileProject->users.ptrs[i], fileId, write);
		if (file)
			break;
	}
	return file;
}

BOOL FileProject_updateIndex(void)
{
	_FileProject_updateUserList();

	BOOL changed = FALSE;
	int i;
	for (i = 0; i < g_FileProject->users.num; i++)
	{
		if (!FileUser_isId(g_FileProject->users.ptrs[i], g_FileProject->uid))	//not me
			changed |= FileUser_updateIndex(g_FileProject->users.ptrs[i]);
	}
	return changed;
}

void FileProject_resetIndex(void)
{
	int i;
	for (i = 0; i < g_FileProject->users.num; i++)
	{
		if (!FileUser_isId(g_FileProject->users.ptrs[i], g_FileProject->uid))	//not me
			FileUser_resetIndex(g_FileProject->users.ptrs[i]);
	}
}

BOOL FileProject_tryUpdateIndex(void)
{
	BOOL changed = FALSE;

	const double t = Os_time();
	if (t - g_FileProject->updateIndex_time > FileProject_UPDATE_INDEX_TIMEOUT_SEC)
	{
		changed = FileProject_updateIndex();
		g_FileProject->updateIndex_time = t;
	}

	if (changed)
		FileProject_resetIndex();

	return changed;
}

BOOL FileProject_changePassword(UBIG cycles, const UNI* password)
{
	BOOL ok = FALSE;

	char* backupPath = Std_addCHAR(g_FileProject->projectPath, "-");
	while (OsFile_existFolder(backupPath))
		backupPath = Std_addAfterCHAR(backupPath, "-");

	char* keyPath = FileKey_getPathKey(g_FileProject->projectPath, TRUE);
	char* keyPath2 = FileKey_getPathKey(g_FileProject->projectPath, FALSE);

	FileProject* oldWorkspace = g_FileProject;
	if (FileProject_newCreate(backupPath, password, cycles))
	{
		ok = TRUE;
		int i;
		for (i = 0; i < g_FileProject->users.num; i++)
		{
			FileUser* newUser = FileUser_new(FileUser_getId(g_FileProject->users.ptrs[i]));
			FileUser_createFolder(newUser);

			ok &= FileUser_changePassword(oldWorkspace, g_FileProject->users.ptrs[i], newUser);

			FileUser_delete(newUser);
		}
	}

	if (ok)
	{
		ok = OsFileDir_removeDir(g_FileProject->projectPath);
		ok = OsFileDir_renameDir(backupPath, g_FileProject->projectPath);
	}
	else
		OsFileDir_removeDir(backupPath);

	//replace key
	FileKey_delete(oldWorkspace->key);
	oldWorkspace->key = FileKey_newCopy(g_FileProject->key);

	//clean
	FileProject_delete();
	g_FileProject = oldWorkspace;

	Std_deleteCHAR(keyPath);
	Std_deleteCHAR(keyPath2);
	Std_deleteCHAR(backupPath);

	return ok;
}

BOOL FileProject_isExistCHAR(const char* path)
{
	BOOL exist = FALSE;

	char* projectPath = FileProject_cleanPath(path);

	if (OsFile_existFolder(projectPath))
	{
		projectPath = Std_addAfterCHAR(projectPath, ".sky");
		exist = OsFile_existFile(projectPath);
	}

	Std_deleteCHAR(projectPath);

	return exist;
}

BOOL FileProject_isExist(const UNI* path)
{
	char* pathChar = Std_newCHAR_uni(path);
	BOOL exist = FileProject_isExistCHAR(pathChar);
	Std_deleteCHAR(pathChar);
	return exist;
}

BOOL FileProject_hasPassword(const UNI* path)
{
	BOOL has = FALSE;

	char* projectPath;
	{
		char* pathChar = Std_newCHAR_uni(path);
		projectPath = FileProject_cleanPath(pathChar);
		Std_deleteCHAR(pathChar);
	}

	if (FileProject_isExistCHAR(projectPath))
	{
		char* keyPath = FileKey_getPathKey(projectPath, TRUE);

		OsFile f;
		if (OsFile_init(&f, keyPath, OsFile_R))
		{
			UBIG version;
			if (OsFile_read(&f, &version, 8) == 8)
			{
				if (version == 1)
				{
					BIG cycles;
					if (OsFile_read(&f, &cycles, 8) == 8)
					{
						has = FileKey_hasPassword(cycles);
					}
				}
				//else if(version == 2)
			}
			OsFile_free(&f);
		}

		Std_deleteCHAR(projectPath);
		Std_deleteCHAR(keyPath);
	}

	return has;
}

void FileProject_removeMyColumn(FileRow file)
{
	char* path = FileUser_getPathColumn(FileProject_getUserMe(), file);
	OsFileDir_removeFile(path);
	Std_deleteCHAR(path);
}
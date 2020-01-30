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

char* FileUser_convertPath(const char* path, FileRow id)
{
	char str[(8 * 2) + 1];
	Std_setHEX_char(&str[0], (UCHAR*)&id, 8);

	char* ret = Std_newCHAR(path);
	ret = Std_addAfterCHAR(ret, "/");
	ret = Std_addAfterCHAR(ret, str);

	return ret;
}

UBIG FileUser_checkName(const char* str)
{
	return Std_sizeCHAR(str) == (8 * 2);
}

FileRow FileUser_convertToId(const char* str)
{
	FileRow id = FileRow_initEmpty();

	if (FileUser_checkName(str))
		Std_getFromHEX(str, 8 * 2, (UCHAR*)&id);

	return id;
}

typedef struct FileUser_s
{
	FileRow userId;
	FileIndexes index;
} FileUser;

FileUser* FileUser_new(FileRow userId)
{
	FileUser* self = Os_malloc(sizeof(FileUser));

	self->userId = userId;
	self->index = FileIndexes_init();
	FileIndexes_update(&self->index, self, TRUE);

	return self;
}

void FileUser_delete(FileUser* self)
{
	FileIndexes_free(&self->index);
	Os_free(self, sizeof(FileUser));
}

FileRow FileUser_getId(const FileUser* self)
{
	return self->userId;
}

BOOL FileUser_isId(const FileUser* self, FileRow uid)
{
	return FileRow_cmp(self->userId, uid);
}

char* FileUser_getPath(const FileUser* self)
{
	return FileUser_convertPath(FileProject_getPath(), self->userId);
}
char* FileUser_getPathF(const FileUser* self)
{
	char* path = FileUser_getPath(self);
	path = Std_addAfterCHAR(path, "/files");
	return path;
}

char* FileUser_getPathColumn(const FileUser* self, FileRow fileId)
{
	fileId.user = 0;

	char* userPath = FileUser_getPath(self);
	char* ret = FileUser_convertPath(userPath, fileId);
	Std_deleteCHAR(userPath);
	return ret;
}

char* FileUser_getPathFile(const FileUser* self, FileRow fileId)
{
	char* userPath = FileUser_getPathF(self);
	char* ret = FileUser_convertPath(userPath, fileId);
	Std_deleteCHAR(userPath);
	return ret;
}

BOOL FileUser_createFolder(FileUser* self)
{
	char* path = FileUser_getPath(self);
	BOOL ok = OsFileDir_makeDir(path);
	Std_deleteCHAR(path);
	return ok;
}

FileFile* FileUser_createColumn(const FileUser* self)
{
	FileRow ptr = FileRow_initEmpty();
	BOOL found = TRUE;
	while (found)
	{
		ptr = FileRow_initRandom();
		char* path = FileUser_getPathColumn(self, ptr);
		found = OsFile_existFile(path);
		Std_deleteCHAR(path);
	}
	return FileRow_is(ptr) ? FileFile_newColumn(self, ptr, TRUE) : 0;
}

FileFile* FileUser_createFile(const FileUser* self)
{
	FileRow ptr = FileRow_initEmpty();
	BOOL found = TRUE;
	while (found)
	{
		ptr = FileRow_initRandom();
		char* path = FileUser_getPathFile(self, ptr);
		found = OsFile_existFile(path);
		Std_deleteCHAR(path);
	}
	return FileRow_is(ptr) ? FileFile_newFile(self, ptr, TRUE) : 0;
}

FileRow* FileUser_getFileList(FileUser* self, BIG* out_num)
{
	char* pth = FileUser_getPath(self);

	//get files in folder
	char** names;
	*out_num = OsFileDir_getFileList(pth, TRUE, FALSE, FALSE, &names);

	//create list
	FileRow* fileIds = Os_malloc(*out_num * sizeof(FileRow));
	UBIG i;
	for (i = 0; i < *out_num; i++)
		fileIds[i] = FileUser_convertToId(names[i]);

	Os_free(names, *out_num * sizeof(char*));

	Std_deleteCHAR(pth);
	return fileIds;
}

UBIG FileUser_getProjectSize(FileUser* self)
{
	UBIG sum = 0;
	char* pth = FileUser_getPath(self);

	//get files in folder
	char** paths;
	const BIG N = OsFileDir_getFileList(pth, TRUE, FALSE, TRUE, &paths);

	//sum bytes
	UBIG i;
	for (i = 0; i < N; i++)
		sum += OsFile_bytes(paths[i]);

	//clear
	for (i = 0; i < N; i++)
		Std_deleteCHAR(paths[i]);
	Os_free(paths, N * sizeof(char*));

	Std_deleteCHAR(pth);
	return sum;
}

static BOOL _FileUser_changePasswordColumn(FileProject* oldWorkspace, FileUser* self, FileUser* newUser)
{
	char* pth = FileUser_getPath(self);

	BOOL ok = TRUE;
	char** names;
	const BIG N = OsFileDir_getFileList(pth, TRUE, FALSE, FALSE, &names);
	BIG i;

	//copy files
	for (i = 0; i < N && ok; i++)
	{
		if (FileUser_checkName(names[i]))
		{
			char* srcPath = Std_addCHAR(FileProject_getPathEx(oldWorkspace), names[i]);
			char* dstPath = Std_addCHAR(FileProject_getPath(), names[i]);
			ok &= FileFile_copyColumn(FileUser_convertToId(names[i]), dstPath, srcPath, newUser, self);
			Std_deleteCHAR(srcPath);
			Std_deleteCHAR(dstPath);
		}
	}

	//clear
	for (i = 0; i < N; i++)
		Std_deleteCHAR(names[i]);
	Os_free(names, N * sizeof(char*));

	Std_deleteCHAR(pth);

	return ok;
}
static BOOL _FileUser_changePasswordFile(FileProject* oldWorkspace, FileUser* self, FileUser* newUser)
{
	char* pth = FileUser_getPathF(self);

	BOOL ok = TRUE;
	char** names;
	const BIG N = OsFileDir_getFileList(pth, TRUE, FALSE, FALSE, &names);
	BIG i;

	//copy files
	for (i = 0; i < N && ok; i++)
	{
		if (FileUser_checkName(names[i]))
		{
			char* srcPath = Std_addCHAR(FileProject_getPathEx(oldWorkspace), names[i]);
			char* dstPath = Std_addCHAR(FileProject_getPath(), names[i]);
			ok &= FileFile_copyFile(FileUser_convertToId(names[i]), dstPath, srcPath, newUser, self);
			Std_deleteCHAR(srcPath);
			Std_deleteCHAR(dstPath);
		}
	}

	//clear
	for (i = 0; i < N; i++)
		Std_deleteCHAR(names[i]);
	Os_free(names, N * sizeof(char*));

	Std_deleteCHAR(pth);

	return ok;
}

BOOL FileUser_changePassword(FileProject* oldWorkspace, FileUser* self, FileUser* newUser)
{
	BOOL ok = TRUE;

	ok &= _FileUser_changePasswordColumn(oldWorkspace, self, newUser);
	ok &= _FileUser_changePasswordFile(oldWorkspace, self, newUser);

	return ok;
}

BOOL FileUser_updateIndex(FileUser* self)
{
	FileIndexes_update(&self->index, self, FALSE);
	BOOL changed = FileIndexes_isChanged(&self->index);
	//FileIndexes_reset(&self->index);
	return changed;
}
void FileUser_resetIndex(FileUser* self)
{
	FileIndexes_reset(&self->index);
}

char* FileUser_getMapPath(FileUser* self)
{
	char* mapPatch = FileUser_getPath(self);
	mapPatch = Std_addAfterCHAR(mapPatch, "/map_std.tiles");
	return mapPatch;
}
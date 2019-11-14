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

UBIG IOFiles_readNet(const char* url, DbValue* file, int index, volatile StdProgress* progress)
{
	char* buff;
	const BIG N = OsHTTPS_downloadWithStatus(url, &progress->done, &progress->running, &buff);
	if (N >= 0)
	{
		UNI ext[8];
		OsFile_getExtensionUNI(url, ext);

		DbValue_importData(file, N, (UCHAR*)buff, index, ext, progress);
	}
	return Std_bmax(0, N);
}

static UBIG _IOFiles_readFile(const char* path, DbValue* file, int index, volatile StdProgress* progress)
{
	UBIG n = 0;

	OsFile f;
	if (OsFile_init(&f, path, OsFile_R))
	{
		UNI ext[8];
		OsFile_getExtensionUNI(path, ext);

		DbValue_importFile(file, &f, index, ext, progress);

		n = OsFile_getSeekPos(&f);
		OsFile_free(&f);
	}
	return n;
}

static UBIG _IOFiles_readFileRow(const char* path, UBIG rowFolder, DbRows* table, DbValue* name, DbValue* folderType, DbValue* file, DbRows* subs, volatile StdProgress* progress)
{
	UBIG n = 0;
	UNI* pathUni = Std_newUNI_char(path);

	UBIG row = DbRows_addNewRow(table);
	DbValue_setRow(name, row, 0);
	DbValue_setRow(folderType, row, 0);
	DbValue_setRow(file, row, 0);
	DbRows_setBaseRow(subs, rowFolder);

	DbValue_setNumber(folderType, 0);
	DbValue_setTextCopy(name, pathUni);
	DbRows_addLinkRow(subs, row);

	n = _IOFiles_readFile(path, file, 0, progress);

	Std_deleteUNI(pathUni);

	return n;
}

UBIG IOFiles_readFolder(const char* path, BOOL subDirs, DbRows* table, DbValue* name, DbValue* folderType, DbValue* file, DbRows* subs, UBIG* done_bytes, const UBIG all_bytes, volatile StdProgress* progress)
{
	BIG i;
	UNI* pathUni = Std_newUNI_char(path);

	UBIG rowFolder = DbRows_addNewRow(table);
	DbValue_setRow(name, rowFolder, 0);
	DbValue_setRow(folderType, rowFolder, 0);

	DbValue_setNumber(folderType, 1);
	DbValue_setTextCopy(name, pathUni);

	if (OsFile_existFile(path))
	{
		//only one file
		*done_bytes += _IOFiles_readFileRow(path, rowFolder, table, name, folderType, file, subs, progress);
		progress->done = 1;
	}
	else
	{
		//files in Folder
		char** files;
		int num_files = OsFileDir_getFileList(path, TRUE, FALSE, TRUE, &files);
		for (i = 0; i < num_files && progress->running; i++)
		{
			*done_bytes += _IOFiles_readFileRow(files[i], rowFolder, table, name, folderType, file, subs, progress);
			Std_deleteCHAR(files[i]);

			progress->done = *done_bytes / (double)all_bytes;
		}
		Os_free(files, num_files * sizeof(char*));

		//Folders
		if (subDirs)
		{
			char** folders;
			int num_folders = OsFileDir_getFileList(path, FALSE, TRUE, TRUE, &folders);
			for (i = 0; i < num_folders && progress->running; i++)
			{
				UBIG row = IOFiles_readFolder(folders[i], subDirs, table, name, folderType, file, subs, done_bytes, all_bytes, progress);
				DbRows_setBaseRow(subs, rowFolder);
				DbRows_addLinkRow(subs, row);

				Std_deleteCHAR(folders[i]);
			}
			Os_free(folders, num_folders * sizeof(char*));
		}
	}

	progress->done = 1;

	Std_deleteUNI(pathUni);

	return rowFolder;
}

BIG IOFiles_readSingle(const char* path, DbValue* value, int index, volatile StdProgress* progress)
{
	BIG n = -1;
	if (OsFile_existFile(path))
	{
		n = _IOFiles_readFile(path, value, index, progress);
	}
	else
		Logs_addError("ERR_INVALID_PATH");

	return n;
}

void IOFiles_read(const char* paths, BOOL subDirs, DbRows* tableOut, volatile StdProgress* progress)
{
	DbValue name = DbValue_initGET(DbTable_createColumnFormat(tableOut->table, DbFormat_TEXT, _UNI32("Name"), 0), -1);
	DbValue folderType = DbValue_initGET(DbTable_createColumnFormat(tableOut->table, DbFormat_CHECK, _UNI32("Folder"), 0), -1);
	DbValue file = DbValue_initGET(DbTable_createColumnFormat(tableOut->table, DbFormat_FILE_1, _UNI32("File"), 0), -1);
	DbRows subs = DbRows_initLink((DbColumnN*)DbTable_createColumnFormat(tableOut->table, DbFormat_LINK_N, _UNI32("Subs"), tableOut->table), -1);

	UBIG all_bytes = 0;
	int N = Std_separNumItemsCHAR(paths, ';');
	int i;
	for (i = 0; i < N; i++)
	{
		char* path = Std_separGetItemCHAR(paths, i, ';');
		all_bytes += OsFile_existFile(path) ? OsFile_bytes(path) : OsFileDir_getFolderBytes(path, subDirs);
		Std_deleteCHAR(path);
	}

	UBIG done_bytes = 0;
	progress->done = 0;

	for (i = 0; i < N; i++)
	{
		char* path = Std_separGetItemCHAR(paths, i, ';');
		if (OsFile_exist(path))
		{
			IOFiles_readFolder(path, subDirs, tableOut, &name, &folderType, &file, &subs, &done_bytes, all_bytes, progress);
		}
		else
			Logs_addError("ERR_INVALID_PATH");

		Std_deleteCHAR(path);
	}

	DbValue_free(&name);
	DbValue_free(&folderType);
	DbValue_free(&file);
	DbRows_free(&subs);
}

static void _IOFiles_writeFile(const char* path, DbValue* column, volatile StdProgress* progress)
{
	if (DbValue_getFileSize(column, 0))
	{
		UBIG row = DbValue_getRow(column);

		UNI nameUni[64];
		DbColumn_getName(column->column, nameUni, 64);

		char ext[8];
		DbValue_getFileExt_char(column, 0, ext);

		char* colName = Std_newCHAR_uni(nameUni);
		char filePath[256];
		snprintf(filePath, sizeof(filePath), "%s/%s_%lld.%.7s", path, colName, row, ext);
		Std_deleteCHAR(colName);

		OsFile f;
		if (OsFile_init(&f, filePath, OsFile_W))
		{
			DbValue_exportFile(column, &f, 0, progress);
			OsFile_free(&f);
		}
	}
}

BOOL IOFiles_writeSingle(const char* path, DbValue* data, volatile StdProgress* progress)
{
	_IOFiles_writeFile(path, data, progress); //single file
	return progress->running;
}

BOOL IOFiles_write(const char* path, DbRows* rows, DbValue* data, volatile StdProgress* progress)
{
	const UBIG N = DbRows_getSize(rows);
	UBIG i;
	for (i = 0; i < N && progress->running; i++)
	{
		DbValue_setRow(data, DbRows_getRow(rows, i), 0);
		_IOFiles_writeFile(path, data, progress);
	}

	return progress->running;
}

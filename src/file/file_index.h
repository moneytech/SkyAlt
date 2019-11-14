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

typedef struct FileIndex_s
{
	FileRow fileId;

	UBIG bytes_old;
	UBIG bytes_new;
} FileIndex;

FileIndex FileIndex_init(FileRow fileId)
{
	FileIndex self;
	self.fileId = fileId;
	self.bytes_old = 0;
	self.bytes_new = 0;
	return self;
}

BOOL FileIndex_isChanged(const FileIndex* self)
{
	return self->bytes_old != self->bytes_new;
}

void FileIndex_reset(FileIndex* self)
{
	self->bytes_old = self->bytes_new;
}

void FileIndex_update(FileIndex* self, const FileUser* user, BOOL first)
{
	char* path = FileUser_getPathColumn(user, self->fileId);
	self->bytes_new = OsFile_bytes(path);
	if (first)
		self->bytes_old = self->bytes_new;
	Std_deleteCHAR(path);
}

typedef struct FileIndexes_s
{
	FileIndex* indexes;
	UBIG num;
} FileIndexes;

FileIndexes FileIndexes_init(void)
{
	FileIndexes self;
	self.indexes = 0;
	self.num = 0;
	return self;
}

void FileIndexes_free(FileIndexes* self)
{
	Os_free(self->indexes, self->num * sizeof(FileIndex));
	Os_memset(self, sizeof(FileIndexes));
}

BOOL FileIndexes_isChanged(const FileIndexes* self)
{
	BIG i;
	for (i = 0; i < self->num; i++)
		if (FileIndex_isChanged(&self->indexes[i]))
			return TRUE;
	return FALSE;
}

void FileIndexes_reset(FileIndexes* self)
{
	BIG i;
	for (i = 0; i < self->num; i++)
		FileIndex_reset(&self->indexes[i]);
}

static BIG _FileIndexes_find(FileIndexes* self, FileRow fileId)
{
	BIG i;
	for (i = 0; i < self->num; i++)
		if (FileRow_cmp(self->indexes[i].fileId, fileId))
			return i;
	return -1;
}

void FileIndexes_update(FileIndexes* self, const FileUser* user, BOOL first)
{
	BIG i;

	//get file list
	char* userPath = FileUser_getPath(user);
	char** names;
	BIG num_files = OsFileDir_getFileList(userPath, TRUE, FALSE, FALSE, &names);

	//add new files
	for (i = 0; i < num_files; i++)
	{
		if (Std_sizeCHAR(names[i]) == (8 * 2))
		{
			FileRow fileId;
			Std_getFromHEX(names[i], 8 * 2, (UCHAR*)&fileId);

			if (_FileIndexes_find(self, fileId) < 0)
			{
				self->num++;
				self->indexes = Os_realloc(self->indexes, self->num * sizeof(FileIndex));
				self->indexes[self->num - 1] = FileIndex_init(fileId);
			}
		}
		else
			printf("File with wrong name(lenght)\n");

		Std_deleteCHAR(names[i]);
	}
	Os_free(names, num_files * sizeof(char*));
	Std_deleteCHAR(userPath);

	//udpate file sizes
	for (i = 0; i < self->num; i++)
		FileIndex_update(&self->indexes[i], user, first);
}

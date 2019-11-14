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

 //collaborate
#include "os.h"
#include "std.h"
#include "log.h"
#include "language.h"

//header
#include "file.h"

#define FileCache_DELAY 10	//10seconds
#define FileCache_NUM 256	//256x ptrs
#define FileCache_MAX_BYTES (128*1024*1024)	//128MB

BOOL FileRow_is(const FileRow self)
{
	return self.row > 0;
}

FileRow FileRow_init(BIG row)
{
	FileRow self;
	self.user = FileProject_getUID().user;
	self.row = row;
	return self;
}
FileRow FileRow_initEmpty()
{
	return FileRow_init(-1);
}

FileRow FileRow_initRandom(void)
{
	FileRow row;
	while (!OsCrypto_random(8, &row) || !FileRow_is(row));
	return row;
}

BOOL FileRow_cmp(const FileRow a, const FileRow b)
{
	return a.user == b.user && a.row == b.row;
}

void FileRow_invalidate(FileRow* self)
{
	if (FileRow_is(*self))
		self->row = -self->row;
}

BIG FileRow_getBIG(FileRow self)
{
	if (!FileRow_is(self))
		self.row = -self.row;

	return *(BIG*)&self;
}

double FileRow_getDouble(FileRow self)
{
	return *(double*)&self;
}

typedef struct FileKey_s FileKey;
FileKey* FileProject_getKey(void);

char* FileUser_getPathColumn(const FileUser* self, FileRow fileId);
char* FileUser_getPath(const FileUser* self);

const char* FileProject_getPathEx(FileProject* self);

BOOL FileFile_copyFile(FileRow fileId, const char* dstPath, const char* srcPath, FileUser* dst, FileUser* src);
BOOL FileFile_copyColumn(FileRow fileId, const char* dstPath, const char* srcPath, FileUser* dst, FileUser* src);

#include "file/file_head.h"
#include "file/file_key.h"
#include "file/file_index.h"

#include "file/file_user.h"
#include "file/file_file.h"

#include "file/file_project.h"
#include "file/file_cache.h"
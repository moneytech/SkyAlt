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

#define FileKey_CYCLES 5000000	//5M

typedef struct FileRow_s
{
	BIG	row : 40,
		user : 24;
} FileRow;
FileRow FileRow_init(BIG row);
FileRow FileRow_initEmpty();
BOOL FileRow_cmp(const FileRow a, const FileRow b);
BOOL FileRow_is(const FileRow self);
void FileRow_invalidate(FileRow* self);
BIG FileRow_getBIG(FileRow self);
double FileRow_getDouble(FileRow self);

typedef struct FileUser_s FileUser;

typedef struct FileFile_s FileFile;
FileRow FileFile_getId(const FileFile* self);
FileFile* FileFile_newColumn(const FileUser* user, FileRow row, BOOL write);
FileFile* FileFile_newFile(const FileUser* user, FileRow row, BOOL write);
void FileFile_delete(FileFile* self);
BOOL FileFile_isColumnExist(const FileUser* user, FileRow row);
UBIG FileFile_getSeekPos(FileFile* self);
UBIG FileFile_seekEnd(FileFile* self);

void FileFile_writeItemHeader(FileFile* self, const double date, const UBIG num_items);
void FileFile_writeItem_1(FileFile* self, FileRow row, double value);
void FileFile_writeItem_n(FileFile* self, FileRow row, const double* values);
void FileFile_writeItemText_32(FileFile* self, FileRow row, const UNI* str);

BOOL FileFile_readItemHeader(FileFile* self, double* out_date, UBIG* out_num_items);
BOOL FileFile_readItem_1(FileFile* self, FileRow* out_row, double* out_value);
BOOL FileFile_readItem_n(FileFile* self, FileRow* out_row, double** out_values);
BOOL FileFile_readItemText_32(FileFile* self, FileRow* out_row, UNI** out_str);

BOOL FileFile_importData(FileFile* self, UCHAR* data, const UBIG data_size, const UNI* ext, volatile StdProgress* progress);
BOOL FileFile_import(FileFile* self, OsFile* file, const UNI* ext, volatile StdProgress* progress);
UBIG FileFile_export(FileFile* self, OsFile* file, volatile StdProgress* progress);

FileFile* FileUser_createColumn(const FileUser* self);
FileFile* FileUser_createFile(const FileUser* self);
BOOL FileUser_updateIndex(FileUser* self);

typedef struct FileProject_s FileProject;
BOOL FileProject_isExist(const UNI* path);
BOOL FileProject_hasPassword(const UNI* path);
BOOL FileProject_newOpen(const char* path, const UNI* password, volatile StdProgress* progress);
BOOL FileProject_newCreate(const char* path, const UNI* password, BIG cycles, volatile StdProgress* progress);
void FileProject_delete(void);
FileRow FileProject_getUID(void);
UNI* FileProject_getUID_string(void);
UBIG FileProject_numUsers(void);
FileUser* FileProject_getUser(UBIG i);
FileUser* FileProject_getUserMe(void);
StdArr FileProject_openColumns(FileRow FileRow, BOOL write);
FileFile* FileProject_openFile(FileRow FileRow, BOOL write);
UBIG FileProject_bytesColumns(FileRow FileRow);
BOOL FileProject_changePassword(UBIG cycles, const UNI* password, volatile StdProgress* progress);
const char* FileProject_getPath(void);
UBIG FileProject_getProjectSize(void);
BOOL FileProject_updateIndex(void);

typedef struct FileHead_s	//16B align
{
	UBIG size;
	UCHAR ext[8];
} FileHead;
FileHead FileHead_initEmpty(void);
FileHead FileHead_init(UBIG size, const UNI* ext);
void FileHead_free(FileHead* self);
void FileHead_copyExt(const FileHead* self, UNI out_ext[8]);
BOOL FileHead_isExt(const FileHead* self, const char* ext);
void FileHead_getInfoEx(const char* ext, const UBIG size, char out[64], BOOL bytesQuotes);
void FileHead_getInfo(const FileHead* self, char out[64], BOOL bytesQuotes);
UNI* FileHead_getInfoUNI(const FileHead* self, BOOL bytesQuotes);

typedef struct FileCache_s FileCache;
BOOL FileCache_new(void);
void FileCache_clear(void);
void FileCache_delete(void);
UBIG FileCache_readData(FileRow FileRow, BIG pos, UBIG size, UCHAR* out);
BOOL FileCache_readHead(FileRow FileRow, FileHead* out_head);
void FileCache_maintenance(void);

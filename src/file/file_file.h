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

typedef struct FileFileItem_s	//16B size
{
	FileRow row;	//negative value => remove
	double value;
} FileFileItem;

typedef struct FileFileItemHead_s	//16B size
{
	double date;
	UBIG num_items;
} FileFileItemHead;

FileFileItem FileFileItem_init(FileRow row, double value)
{
	FileFileItem self;
	self.row = row;
	self.value = value;
	return self;
}

#define FileFileBuffer_BYTES 4096
typedef struct FileFileBuffer_s
{
	OsFile* file;
	UCHAR* buffer;
	int buffer_pos;
	int buffer_size;
	UBIG seek_start;
	BOOL write;
} FileFileBuffer;

FileFileBuffer FileFileBuffer_initEmpty(void)
{
	FileFileBuffer self;
	Os_memset(&self, sizeof(FileFileBuffer));
	return self;
}

FileFileBuffer FileFileBuffer_init(OsFile* file, BOOL write)
{
	FileFileBuffer self;
	self.write = write;
	self.file = file;
	self.buffer = Os_malloc(FileFileBuffer_BYTES);
	self.buffer_pos = 0;
	self.buffer_size = 0;

	self.seek_start = write ? OsFile_seekEnd(file) : 0;

	return self;
}

BOOL FileFileBuffer_writeFinish(FileFileBuffer* self)
{
	BOOL ok = TRUE;
	if (self->buffer_pos)
	{
		OsFile_seekAbs(self->file, self->seek_start);	//maybe not needed? ...

		ok = (OsFile_write(self->file, self->buffer, self->buffer_pos) == self->buffer_pos);

		self->seek_start += self->buffer_pos;
		self->buffer_pos = 0;
	}
	return ok;
}

void FileFileBuffer_free(FileFileBuffer* self)
{
	if (self->write)
		FileFileBuffer_writeFinish(self);
	Os_free(self->buffer, FileFileBuffer_BYTES);
	Os_memset(self, sizeof(FileFileBuffer));
}

UBIG FileFileBuffer_getFilePos(FileFileBuffer* self)
{
	return self->seek_start + self->buffer_pos;
}

UBIG FileFileBuffer_write16(FileFileBuffer* self, UCHAR data[16])
{
	Os_memcpy(&self->buffer[self->buffer_pos], data, 16);
	self->buffer_pos += 16;

	if (self->buffer_pos == FileFileBuffer_BYTES)
		FileFileBuffer_writeFinish(self);

	return FileFileBuffer_getFilePos(self);
}

BOOL FileFileBuffer_read16(FileFileBuffer* self, UBIG pos, UCHAR data[16])
{
	if (pos < self->seek_start || pos >= self->seek_start + self->buffer_size)
	{
		self->seek_start = pos;
		self->buffer_pos = 0;
		self->buffer_size = 0;
		if (OsFile_seekAbs(self->file, pos))
			self->buffer_size = OsFile_read(self->file, self->buffer, FileFileBuffer_BYTES);
	}

	Os_memcpy(data, &self->buffer[pos - self->seek_start], 16);
	self->buffer_pos += 16;

	return pos < self->seek_start + self->buffer_size;
}

typedef struct FileFile_s
{
	OsFile file;
	const FileUser* user;
	FileRow fileId;
	BOOL write;

	FileFileBuffer buffer;
} FileFile;

void FileFile_delete(FileFile* self)
{
	FileFileBuffer_free(&self->buffer);

	OsFile_free(&self->file);
	Os_free(self, sizeof(FileFile));
}

static FileFile* _FileFile_new(const FileUser* user, FileRow fileId, BOOL write, const char* path)
{
	FileFile* self = Os_malloc(sizeof(FileFile));
	self->file = OsFile_initEmpty();
	self->user = user;
	self->fileId = fileId;
	self->write = write;

	self->buffer = FileFileBuffer_initEmpty();

	if (OsFile_init(&self->file, path, write ? OsFile_A : OsFile_R))   //OsFile_A(seek doesn't work) -> OsFile_RW
	{
		self->buffer = FileFileBuffer_init(&self->file, write);
	}
	else
	{
		FileFile_delete(self);
		self = 0;
	}

	return self;
}

FileFile* FileFile_newColumn(const FileUser* user, FileRow row, BOOL write)
{
	char* path = FileUser_getPathColumn(user, row);
	FileFile* self = _FileFile_new(user, row, write, path);
	Std_deleteCHAR(path);
	return self;
}

FileFile* FileFile_newFile(const FileUser* user, FileRow row, BOOL write)
{
	char* path = FileUser_getPathFile(user, row);
	FileFile* self = _FileFile_new(user, row, write, path);
	Std_deleteCHAR(path);
	return self;
}

BOOL FileFile_isColumnExist(const FileUser* user, FileRow row)
{
	char* path = FileUser_getPathColumn(user, row);
	BOOL exist = OsFile_exist(path);
	Std_deleteCHAR(path);
	return exist;
}

UBIG FileFile_bytesColumn(const FileUser* user, FileRow fileId)
{
	char* path = FileUser_getPathColumn(user, fileId);
	UBIG bytes = OsFile_bytes(path);
	Std_deleteCHAR(path);
	return bytes;
}

FileRow FileFile_getId(const FileFile* self)
{
	return self->fileId;
}

OsFile* FileFile_getFile(FileFile* self)
{
	return &self->file;
}

UBIG FileFile_seekEnd(FileFile* self)
{
	return OsFile_seekEnd(&self->file);
}

UBIG FileFile_getSeekPos(FileFile* self)
{
	return OsFile_getSeekPos(&self->file);
}

BOOL FileFile_read16Pos(FileFile* self, UBIG file_pos, UCHAR out_plain[16])
{
	UCHAR cipher[16];
	BOOL ok = FileFileBuffer_read16(&self->buffer, file_pos, cipher);
	if (ok)
	{
		FileKey_aesDecryptDirect(FileProject_getKey(), (FileRow_getBIG(self->fileId) + file_pos), cipher, out_plain, 16);	//AES_IV = FileName + pos
	}
	return ok;
}

BOOL FileFile_read16(FileFile* self, UCHAR out_plain[16])
{
	return FileFile_read16Pos(self, FileFileBuffer_getFilePos(&self->buffer), out_plain);
}

BIG FileFile_write16Pos(FileFile* self, UBIG file_pos, const UCHAR in_plain[16])
{
	UCHAR cipher[16];
	FileKey_aesEncryptDirect(FileProject_getKey(), (FileRow_getBIG(self->fileId) + file_pos), (void*)in_plain, cipher, 16);	//AES_IV = FileName + pos

	return FileFileBuffer_write16(&self->buffer, cipher);
}

BIG FileFile_write16(FileFile* self, const UCHAR in_plain[16])
{
	return FileFile_write16Pos(self, FileFileBuffer_getFilePos(&self->buffer), in_plain);
}

BOOL FileFile_copyFile(FileRow fileId, const char* dstPath, const char* srcPath, FileUser* dst, FileUser* src)
{
	BOOL ok = FALSE;

	FileFile* fr = FileFile_newFile(src, fileId, FALSE);
	if (fr)
	{
		FileFile* fw = FileFile_newFile(dst, fileId, TRUE);
		if (fw)
		{
			ok = TRUE;
			UCHAR plain[16];
			while (FileFile_read16(fr, plain))
				FileFile_write16(fw, plain);
			FileFile_delete(fw);
		}

		FileFile_delete(fr);
	}

	return ok;
}

BOOL FileFile_copyColumn(FileRow fileId, const char* dstPath, const char* srcPath, FileUser* dst, FileUser* src)
{
	BOOL ok = FALSE;

	FileFile* fr = FileFile_newColumn(src, fileId, FALSE);
	if (fr)
	{
		FileFile* fw = FileFile_newColumn(dst, fileId, TRUE);
		if (fw)
		{
			ok = TRUE;
			UCHAR plain[16];
			while (FileFile_read16(fr, plain))
				FileFile_write16(fw, plain);
			FileFile_delete(fw);
		}

		FileFile_delete(fr);
	}

	return ok;
}

static BIG FileFile_getCopyN(UBIG i, UBIG size)
{
	return Std_bmin(16, size - i);
}

BIG FileFile_exportArray(FileFile* self, UBIG pos, UBIG size, UCHAR* data)
{
	UBIG i = 0;
	UCHAR plain[16];
	while (i < size)
	{
		if (!FileFile_read16Pos(self, pos + i, plain))
			break;

		Os_memcpy(&data[i], plain, FileFile_getCopyN(i, size));
		i += 16;
	}

	return i;
}

static BOOL _FileFile_importHead(FileFile* self, FileHead head)
{
	return FileFile_write16(self, (UCHAR*)&head) == sizeof(FileHead);
}

BOOL FileFile_exportHead(FileFile* self, FileHead* out_head)
{
	return FileFile_read16Pos(self, 0, (UCHAR*)out_head);
}

BOOL FileFile_importData(FileFile* self, UCHAR* data, const UBIG data_size, const UNI* ext)
{
	UBIG i = 0;

	if (_FileFile_importHead(self, FileHead_init(data_size, ext)))
	{
		while (i < data_size)
		{
			UCHAR plain[16];
			Os_memset(plain, 16);
			Os_memcpy(plain, &data[i], FileFile_getCopyN(i, data_size));

			if (FileFile_write16(self, plain) != sizeof(FileHead) + i + 16)
				break;

			StdProgress_setEx("IMPORTING", i, data_size);
			i += 16;
		}

		return i >= data_size;
	}

	return FALSE;
}

BOOL FileFile_import(FileFile* self, OsFile* file, const UNI* ext)
{
	UBIG i = 0;

	const UBIG size = OsFile_seekEnd(file);
	OsFile_seekStart(file);

	if (_FileFile_importHead(self, FileHead_init(size, ext)))
	{
		while (i < size && StdProgress_is())
		{
			UCHAR plain[16];
			Os_memset(plain, 16);
			OsFile_read(file, plain, FileFile_getCopyN(i, size));

			if (FileFile_write16(self, plain) != sizeof(FileHead) + i + 16)
				break;

			StdProgress_setEx("IMPORTING", i, size);
			i += 16;
		}

		return i >= size;
	}

	return FALSE;
}

UBIG FileFile_export(FileFile* self, OsFile* file)
{
	FileHead head;
	FileFile_exportHead(self, &head);

	UCHAR plain[16];
	BIG i = 0;	//first 16 is ".extension"
	while (i < head.size && FileFile_read16Pos(self, i + 16, plain) && StdProgress_is())
	{
		OsFile_write(file, plain, FileFile_getCopyN(i, head.size));

		StdProgress_setEx("EXPORTING", i, head.size);
		i += 16;
	}

	return i >= head.size;
}

static void _FileFile_writeItem(FileFile* self, FileFileItem* item)
{
	FileFile_write16(self, (UCHAR*)item);
}

static void _FileFile_writeItemArray(FileFile* self, FileRow row, const UCHAR* arr, const UBIG BYTES)
{
	//write size
	FileFileItem item = FileFileItem_init(row, BYTES);
	_FileFile_writeItem(self, &item);

	//write data
	BIG i;
	for (i = 0; i < BYTES; i += 16)
	{
		UCHAR plain[16];
		Os_memset(plain, 16);
		Os_memcpy(plain, &arr[i], Std_min(16, BYTES - i));

		FileFile_write16(self, plain);
	}
}

void FileFile_writeItemHeader(FileFile* self, const double date, const UBIG num_items)
{
	FileFileItemHead item;
	item.date = date;
	item.num_items = num_items;
	FileFile_write16(self, (UCHAR*)&item);
}

void FileFile_writeItem_1(FileFile* self, FileRow row, double value)
{
	if (!FileRow_is(row))
		value = 0;
	FileFileItem item = FileFileItem_init(row, value);
	_FileFile_writeItem(self, &item);
}
void FileFile_writeItem_n(FileFile* self, FileRow row, const double* values)
{
	if (!FileRow_is(row))
		values = 0;
	_FileFile_writeItemArray(self, row, (UCHAR*)values, (values ? values[0] + 1 : 0) * sizeof(double));
}

void FileFile_writeItemText_32(FileFile* self, FileRow row, const UNI* str)
{
	if (!FileRow_is(row))
		str = 0;
	_FileFile_writeItemArray(self, row, (UCHAR*)str, Std_bytesUNI(str));
}

static BOOL _FileFile_readItem(FileFile* self, FileFileItem* out_item)
{
	return FileFile_read16(self, (UCHAR*)out_item);
}
static BOOL _FileFile_readItemArray(FileFile* self, FileRow* out_row, UCHAR** out_array)
{
	*out_array = 0;
	FileFileItem item;

	//read size
	BOOL ok = _FileFile_readItem(self, &item);
	*out_row = item.row;

	//read data
	if (ok && item.value)	//item.value is size
	{
		*out_array = Os_calloc(1, Std_round16(item.value));
		BIG i;
		for (i = 0; i < item.value && ok; i += 16)
			ok &= FileFile_read16(self, &(*out_array)[i]);
	}

	return ok;
}

BOOL FileFile_readItemHeader(FileFile* self, double* out_date, UBIG* out_num_items)
{
	FileFileItemHead item;
	BOOL ok = FileFile_read16(self, (UCHAR*)&item);
	if (ok)
	{
		*out_date = item.date;
		*out_num_items = item.num_items;
	}
	return ok;
}

BOOL FileFile_readItem_1(FileFile* self, FileRow* out_row, double* out_value)
{
	FileFileItem item;
	BOOL ok = _FileFile_readItem(self, &item);
	*out_row = item.row;
	*out_value = item.value;
	return ok;
}

BOOL FileFile_readItem_n(FileFile* self, FileRow* out_row, double** out_values)
{
	return _FileFile_readItemArray(self, out_row, (UCHAR**)out_values);
}

BOOL FileFile_readItemText_32(FileFile* self, FileRow* out_row, UNI** out_str)
{
	return _FileFile_readItemArray(self, out_row, (UCHAR**)out_str);
}

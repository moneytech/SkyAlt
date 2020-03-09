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

typedef struct FileCacheBlock_s
{
	UCHAR* data;
	FileRow fileId;

	UBIG start;
	UBIG size;
	double timeUse;
}FileCacheBlock;

FileCacheBlock FileCacheBlock_init(FileFile* file, UBIG start, UBIG size)
{
	FileCacheBlock self;

	self.size = Std_round16(size);
	self.data = Os_malloc(self.size);

	self.timeUse = Os_time();

	self.fileId = file->fileId;
	self.start = start;

	FileFile_exportArray(file, start, size, self.data);

	return self;
}

void FileCacheBlock_free(FileCacheBlock* self)
{
	Os_free(self->data, self->size);
	Os_memset(self, sizeof(FileCacheBlock));
}

BOOL FileCacheBlock_isEmpty(const FileCacheBlock* self)
{
	return (self->data == 0);
}

BOOL FileCacheBlock_is(const FileCacheBlock* self, FileRow fileId, UBIG pos)
{
	return FileRow_cmp(self->fileId, fileId) && pos >= self->start && pos < self->start + self->size;
}

UBIG FileCacheBlock_copy(const FileCacheBlock* self, UBIG start, UBIG size, UCHAR* dst)
{
	UBIG max_end = self->start + self->size;

	start = start - self->start;
	size = (max_end > start + size) ? size : (max_end - start);

	Os_memcpy(dst, &self->data[start], size);

	return size;
}

typedef struct FileCache_s
{
	OsLock lock;

	FileCacheBlock* blocks;
	int num_blocks;
}FileCache;

FileCache* g_FileCache = 0;

FileCacheBlock* FileCache_getBlock(int i)
{
	return &g_FileCache->blocks[i];
}
void FileCache_clear(void)
{
	OsLock_lock(&g_FileCache->lock);

	int i;
	for (i = 0; i < g_FileCache->num_blocks; i++)
		FileCacheBlock_free(FileCache_getBlock(i));

	OsLock_unlock(&g_FileCache->lock);
}

void FileCache_delete(void)
{
	if (g_FileCache)
	{
		UBIG oldN = g_FileCache->num_blocks;
		FileCache_clear();
		Os_free(g_FileCache->blocks, oldN * sizeof(void*));

		OsLock_free(&g_FileCache->lock);
		Os_free(g_FileCache, sizeof(FileCache));
		g_FileCache = 0;
	}
}

BOOL FileCache_new(void)
{
	if (g_FileCache)
		FileCache_delete();

	g_FileCache = Os_malloc(sizeof(FileCache));

	OsLock_init(&g_FileCache->lock);

	g_FileCache->blocks = Os_calloc(FileCache_NUM, sizeof(FileCacheBlock));
	g_FileCache->num_blocks = FileCache_NUM;

	return TRUE;
}

UBIG FileCache_size(void)
{
	UBIG sum = 0;

	int i;
	for (i = 0; i < g_FileCache->num_blocks; i++)
		sum += FileCache_getBlock(i)->size;

	return sum;
}

static FileCacheBlock* _FileCache_maintenanceForceOneBlock(void)
{
	FileCacheBlock* oldest = 0;
	int i;
	for (i = 0; i < g_FileCache->num_blocks; i++)
	{
		FileCacheBlock* b = FileCache_getBlock(i);
		if (!FileCacheBlock_isEmpty(b))
		{
			if (!oldest || b->timeUse < oldest->timeUse)
				oldest = b;
		}
		else
		{
			oldest = b;
			break;
		}
	}

	FileCacheBlock_free(oldest);
	return oldest;
}

static FileCacheBlock* _FileCache_createBlock(FileRow fileId, UBIG pos, UBIG size)
{
	FileCacheBlock* block = 0;

	FileFile* file = FileProject_openFile(fileId, FALSE);
	if (file)
	{
		block = _FileCache_maintenanceForceOneBlock();	//find or free one
		*block = FileCacheBlock_init(file, pos, size);
		FileFile_delete(file);
	}
	return block;
}

static FileCacheBlock* _FileCache_findOrCreateBlock(FileRow fileId, UBIG pos, UBIG size)
{
	int i;
	for (i = 0; i < g_FileCache->num_blocks; i++)
	{
		FileCacheBlock* b = FileCache_getBlock(i);
		if (FileCacheBlock_is(b, fileId, pos))
		{
			b->timeUse = Os_time();
			return b;
		}
	}

	return _FileCache_createBlock(fileId, pos, size);
}

void FileCache_maintenance(void)
{
	OsLock_lock(&g_FileCache->lock);

	double time = Os_time();

	int i;
	for (i = 0; i < g_FileCache->num_blocks; i++)
	{
		FileCacheBlock* b = FileCache_getBlock(i);
		if (!FileCacheBlock_isEmpty(b) && (time - b->timeUse) > FileCache_DELAY)
			FileCacheBlock_free(b);
	}

	while (FileCache_size() > FileCache_MAX_BYTES)
		_FileCache_maintenanceForceOneBlock();

	OsLock_unlock(&g_FileCache->lock);
}

UBIG FileCache_readData(FileRow fileId, BIG pos, UBIG size, UCHAR* out)
{
	UBIG origSize = size;

	pos += 16;

	OsLock_lock(&g_FileCache->lock);

	while (size > 0)
	{
		FileCacheBlock* b = _FileCache_findOrCreateBlock(fileId, pos, size);
		if (!b)
			break;

		UBIG done = FileCacheBlock_copy(b, pos, size, out);

		pos += done;
		size -= done;
		out += done;
	}

	OsLock_unlock(&g_FileCache->lock);

	return (origSize - size);
}

BOOL FileCache_readHead(FileRow fileId, FileHead* out_head)
{
	return FileCache_readData(fileId, -16, 16, (UCHAR*)out_head) == 16;
}

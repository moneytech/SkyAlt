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

typedef struct MediaImage_s
{
	//Sources
	FileRow fileId;
	char* url;

	FileHead head;
	BIG offset;

	UCHAR* buff;

	BOOL loaded;
	BOOL loadedNotice;	//TRUE => MediaLibray will tell GUI to redraw

	UNI* info;
	double timeUse;
	Vec2i rectSize;
	Image4 img;
}MediaImage;

Vec2i _MediaImage_getSizeRatioEx(Vec2i origSize, Vec2i imgSize)
{
	return Vec2i_subRatio(imgSize, origSize);
}

BIG _MediaImage_read(void* selff, UCHAR* buff, int buff_size)
{
	BIG ret = -1;

	MediaImage* self = selff;

	if (self->offset >= self->head.size)
		return -1;	//AVERROR_EOF

	if (FileRow_is(self->fileId))
	{
		UBIG n = FileCache_readData(self->fileId, self->offset, buff_size, buff);
		self->offset += n;
		ret = n;
	}
	else
		if (self->buff)
		{
			UBIG n = Std_bmin(buff_size, (self->head.size - self->offset));
			Os_memcpy(buff, &self->buff[self->offset], n);
			self->offset += n;
			ret = n;
		}

	return ret;
}
BIG _MediaImage_seek(void* selff, BIG offset)
{
	MediaImage* self = selff;

	if (offset < 0)
		return self->head.size;

	self->offset = offset;
	return self->offset;
}

static MediaImage* _MediaImage_new(Vec2i rectSize, FileRow fileId, FileHead head)
{
	MediaImage* self = Os_calloc(1, sizeof(MediaImage));
	self->loaded = FALSE;
	self->loadedNotice = FALSE;

	self->info = 0;
	self->timeUse = Os_time();
	self->img = Image4_init();
	self->fileId = fileId;
	self->head = head;
	self->offset = 0;
	self->rectSize = rectSize;

	return self;
}

MediaImage* MediaImage_newFile(FileRow fileId, Vec2i rectSize)
{
	FileHead head;
	FileCache_readHead(fileId, &head);

	return _MediaImage_new(rectSize, fileId, head);
}

MediaImage* MediaImage_newBuffer(const char* url, const char* ext, UCHAR* buff, UBIG bytes, Vec2i rectSize)
{
	FileHead head;
	head.size = bytes;
	Std_copyCHAR((char*)head.ext, 8, ext);
	MediaImage* self = _MediaImage_new(rectSize, FileRow_initEmpty(), head);
	if (self)
	{
		self->fileId = FileRow_initEmpty();
		self->url = Std_newCHAR(url);
		self->buff = buff;
	}
	return self;
}

void MediaImage_delete(MediaImage* self)
{
	Std_deleteCHAR(self->url);
	Os_free(self->buff, self->head.size);

	Std_deleteUNI(self->info);
	Image4_free(&self->img);

	Os_free(self, sizeof(MediaImage));
}

BOOL MediaImage_isFile(const MediaImage* self, FileRow fileId, Vec2i imgSize)
{
	return FileRow_cmp(self->fileId, fileId) && Vec2i_cmp(self->rectSize, imgSize);
}
BOOL MediaImage_isUrl(const MediaImage* self, const char* url, const char* ext, Vec2i imgSize)
{
	return Std_cmpCHAR(self->url, url) && Std_cmpCHAR((char*)self->head.ext, ext) && Vec2i_cmp(self->rectSize, imgSize);
}

static UNI* _MediaImage_getInfo(MediaImage* self, Vec2i size)
{
	char inf[64];
	FileHead_getInfoEx((char*)self->head.ext, self->head.size, inf, TRUE);

	char str[64];
	snprintf(str, sizeof(str), "%s %dx%d", inf, size.x, size.y);
	return Std_newUNI_char(str);
}

BOOL MediaImage_cook(MediaImage* self)
{
	BOOL cook = !self->loaded;

	if (cook)
	{
		//double t = Os_time();

		char* name = Std_addCHAR(".", (char*)self->head.ext);
		OSMedia* media = OSMedia_newOpen(&_MediaImage_read, &_MediaImage_seek, self, name);
		Std_deleteCHAR(name);

		if (media)
		{
			Vec2i ratioSize = _MediaImage_getSizeRatioEx(*OSMedia_getOrigSize(media), self->rectSize);
			self->img = Image4_initSize(ratioSize);

			OSMedia_loadVideo(media, &self->img);
			self->info = _MediaImage_getInfo(self, *OSMedia_getOrigSize(media));

			OSMedia_delete(media);
		}

		self->loaded = TRUE;
		self->loadedNotice = TRUE;

		//printf("Loaded in: %f\n", Os_time()-t);
	}
	return cook;
}

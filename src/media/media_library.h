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

typedef struct MediaLibrary_s
{
	OsLock lock;

	OsThread images_thread;
	OsThread audio_thread;

	StdArr images;
	StdArr flacs;
	StdArr opuss;

	float volume;
}MediaLibrary;

MediaLibrary* g_MediaLibrary = 0;

THREAD_FUNC(MediaLibrary_loopImages, param);
THREAD_FUNC(MediaLibrary_loopAudio, param);

void MediaLibrary_delete(void)
{
	if (g_MediaLibrary)
	{
		OsThread_free(&g_MediaLibrary->images_thread, TRUE);
		OsThread_free(&g_MediaLibrary->audio_thread, TRUE);
		OsLock_free(&g_MediaLibrary->lock);

		StdArr_freeFn(&g_MediaLibrary->images, (StdArrFREE)&MediaImage_delete);
		//StdArr_freeFn(&g_MediaLibrary->flacs, (StdArrFREE)&MediaFlac_delete);
		//StdArr_freeFn(&g_MediaLibrary->opuss, (StdArrFREE)&MediaOpus_delete);
		//StdArr_freeFn(&g_MediaLibrary->texts, (StdArrFREE)&MediaText_delete);

		Os_free(g_MediaLibrary, sizeof(MediaLibrary));
		g_MediaLibrary = 0;
	}
}

BOOL MediaLibrary_new(void)
{
	if (g_MediaLibrary)
		MediaLibrary_delete();

	g_MediaLibrary = Os_malloc(sizeof(MediaLibrary));

	g_MediaLibrary->images = StdArr_init();
	//g_MediaLibrary->flacs = StdArr_init();
	//g_MediaLibrary->opuss = StdArr_init();
	//g_MediaLibrary->texts = StdArr_init();

	g_MediaLibrary->volume = 0.5f;

	OsLock_init(&g_MediaLibrary->lock);
	OsThread_init(&g_MediaLibrary->images_thread, g_MediaLibrary, &MediaLibrary_loopImages);
	OsThread_init(&g_MediaLibrary->audio_thread, g_MediaLibrary, &MediaLibrary_loopAudio);
	return TRUE;
}

static BOOL _MediaLibrary_isBuffer(const char* url, const char* ext, Vec2i imgSize)
{
	int i;
	for (i = 0; i < g_MediaLibrary->images.num; i++)
		if (MediaImage_isUrl(g_MediaLibrary->images.ptrs[i], url, ext, imgSize))
			return TRUE;
	return FALSE;
}

static BOOL _MediaLibrary_is(FileRow fileId, Vec2i imgSize, BOOL* out_image, BOOL* out_audio, BOOL* out_text)
{
	*out_image = FALSE;
	*out_audio = FALSE;
	*out_text = FALSE;

	int i;

	for (i = 0; i < g_MediaLibrary->images.num; i++)
		if (MediaImage_isFile(g_MediaLibrary->images.ptrs[i], fileId, imgSize))
			*out_image = TRUE;

	/*for(i=0; i < g_MediaLibrary->flacs.num; i++)
		if(MediaFlac_is(g_MediaLibrary->flacs.ptrs[i], fileId))
			*out_audio = TRUE;*/

			/*for(i=0; i < g_MediaLibrary->opuss.num; i++)
				if(MediaOpus_is(g_MediaLibrary->opuss.ptrs[i], fileId))
					*out_audio = TRUE;*/

					/*for(i=0; i < g_MediaLibrary->texts.num; i++)
						if(MediaText_is(g_MediaLibrary->texts.ptrs[i], fileId))
							*out_text = TRUE;*/

	return (*out_image || *out_audio || *out_text);
}

BOOL MediaLibrary_hasImageBuffer(const char* url, const char* ext, Vec2i imgSize)
{
	OsLock_lock(&g_MediaLibrary->lock);

	BOOL found = _MediaLibrary_isBuffer(url, ext, imgSize);

	OsLock_unlock(&g_MediaLibrary->lock);

	return found;
}

BOOL MediaLibrary_addImageBuffer(const char* url, const char* ext, UCHAR* buff, UBIG bytes, Vec2i rectSize)
{
	BOOL added = FALSE;
	OsLock_lock(&g_MediaLibrary->lock);

	if (!_MediaLibrary_isBuffer(url, ext, rectSize))
	{
		MediaImage* img = MediaImage_newBuffer(url, ext, buff, bytes, rectSize);
		if (img)
		{
			StdArr_add(&g_MediaLibrary->images, img);
			added = TRUE;
		}
	}

	OsLock_unlock(&g_MediaLibrary->lock);
	return added;
}

BOOL MediaLibrary_add(FileRow fileId, Vec2i img_rectSize, BOOL* out_image, BOOL* out_audio, BOOL* out_text)
{
	if (img_rectSize.x <= 0 || img_rectSize.y <= 0)
		return FALSE;

	*out_image = FALSE;
	*out_audio = FALSE;
	*out_text = FALSE;

	OsLock_lock(&g_MediaLibrary->lock);

	if (!_MediaLibrary_is(fileId, img_rectSize, out_image, out_audio, out_text))
	{
		FileHead head;
		if (FileCache_readHead(fileId, &head))
		{
			if (FileHead_isExt(&head, "jpg") || FileHead_isExt(&head, "jpeg") || FileHead_isExt(&head, "png"))
			{
				MediaImage* img = MediaImage_newFile(fileId, img_rectSize);
				if (img)
				{
					StdArr_add(&g_MediaLibrary->images, img);
					*out_image = TRUE;
				}
			}
			/*else
			if(FileHead_isExt(&head, "opus"))
			{
				MediaOpus* opus = MediaOpus_new(fileId);
				if(opus)
				{
					StdArr_add(&g_MediaLibrary->opuss, opus);
					*out_audio = TRUE;
				}
			}
			else
			if(FileHead_isExt(&head, "flac"))
			{
				MediaFlac* flac = MediaFlac_new(fileId);
				if(flac)
				{
					StdArr_add(&g_MediaLibrary->flacs, flac);
					*out_audio = TRUE;
				}
			}*/
			else
				if (FileHead_isExt(&head, "txt"))
				{
					//MediaText* text = MediaText_new(fileId, g_MediaLibrary->cache);
					//if(text)
					//{
					//	StdArr_add(&g_MediaLibrary->texts, text);
					*out_text = TRUE;
					//}
				}

			FileHead_free(&head);
		}
	}

	OsLock_unlock(&g_MediaLibrary->lock);

	return (*out_image || *out_audio || *out_text);
}

BOOL MediaLibrary_imageUpdate(FileRow fileId, Vec2i imgSize)
{
	BOOL loadedNotice = FALSE;
	OsLock_lock(&g_MediaLibrary->lock);

	int i;
	for (i = 0; i < g_MediaLibrary->images.num; i++)
	{
		MediaImage* it = (MediaImage*)g_MediaLibrary->images.ptrs[i];
		if (MediaImage_isFile(it, fileId, imgSize))
		{
			it->timeUse = Os_time();

			loadedNotice = it->loadedNotice;
			it->loadedNotice = FALSE;
		}
	}

	OsLock_unlock(&g_MediaLibrary->lock);
	return loadedNotice;
}

BOOL MediaLibrary_imageBufferUpdate(const char* url, const char* ext, Vec2i imgSize)
{
	BOOL loadedNotice = FALSE;
	OsLock_lock(&g_MediaLibrary->lock);

	int i;
	for (i = 0; i < g_MediaLibrary->images.num; i++)
	{
		MediaImage* it = (MediaImage*)g_MediaLibrary->images.ptrs[i];
		if (MediaImage_isUrl(it, url, ext, imgSize))
		{
			it->timeUse = Os_time();

			loadedNotice = it->loadedNotice;
			it->loadedNotice = FALSE;
		}
	}

	OsLock_unlock(&g_MediaLibrary->lock);
	return loadedNotice;
}

void MediaLibrary_imageDrawExt(FileRow fileId, Image4* img, Quad2i coord, int textH, Rgba cd)
{
	OsLock_lock(&g_MediaLibrary->lock);

	UNI ext[9] = { 0 };

	int i;
	for (i = 0; i < g_MediaLibrary->images.num; i++)
	{
		MediaImage* it = (MediaImage*)g_MediaLibrary->images.ptrs[i];
		if (it->loaded && MediaImage_isFile(it, fileId, coord.size))
		{
			Std_copyUNI_char(ext, 9, (char*)((MediaImage*)g_MediaLibrary->images.ptrs[i])->head.ext);
			break;
		}
	}

	/*
	for(i=0; i < g_MediaLibrary->flacs.num; i++)
		if(MediaFlac_is(g_MediaLibrary->flacs.ptrs[i], fileId))
			info = ((MediaFlac*)g_MediaLibrary->flacs.ptrs[i])->info;

	for(i=0; i < g_MediaLibrary->opuss.num; i++)
		if(MediaOpus_is(g_MediaLibrary->opuss.ptrs[i], fileId))
			info = ((MediaOpus*)g_MediaLibrary->opuss.ptrs[i])->info;
	*/

	OsLock_unlock(&g_MediaLibrary->lock);

	Std_convertUpUNI(ext);
	OsFont* font = OsWinIO_getFontDefault();
	Image4_drawText(img, Quad2i_getMiddle(coord), TRUE, font, ext, textH, 0, cd);
}

void MediaLibrary_imageDrawInfo(FileRow fileId, Image4* img, Quad2i coord, int textH, Rgba cd)
{
	OsLock_lock(&g_MediaLibrary->lock);

	const UNI* info = 0;
	int i;
	for (i = 0; i < g_MediaLibrary->images.num; i++)
	{
		MediaImage* it = (MediaImage*)g_MediaLibrary->images.ptrs[i];
		if (it->loaded && MediaImage_isFile(it, fileId, coord.size))
		{
			info = ((MediaImage*)g_MediaLibrary->images.ptrs[i])->info;
			break;
		}
	}

	/*
	for(i=0; i < g_MediaLibrary->flacs.num; i++)
		if(MediaFlac_is(g_MediaLibrary->flacs.ptrs[i], fileId))
			info = ((MediaFlac*)g_MediaLibrary->flacs.ptrs[i])->info;

	for(i=0; i < g_MediaLibrary->opuss.num; i++)
		if(MediaOpus_is(g_MediaLibrary->opuss.ptrs[i], fileId))
			info = ((MediaOpus*)g_MediaLibrary->opuss.ptrs[i])->info;
	*/

	OsLock_unlock(&g_MediaLibrary->lock);

	OsFont* font = OsWinIO_getFontDefault();
	Image4_drawText(img, Quad2i_getMiddle(coord), TRUE, font, info, textH, 0, cd);
}

BOOL MediaLibrary_imageDraw(FileRow fileId, Image4* img, Quad2i coord)
{
	BOOL found = FALSE;
	OsLock_lock(&g_MediaLibrary->lock);

	int i;
	for (i = 0; i < g_MediaLibrary->images.num; i++)
	{
		MediaImage* it = (MediaImage*)g_MediaLibrary->images.ptrs[i];
		if (it->loaded && MediaImage_isFile(it, fileId, coord.size))
		{
			Image4_copyImage4(img, Quad2i_getSubStart(coord, it->img.size), &it->img);

			found = TRUE;
			break;
		}
	}

	OsLock_unlock(&g_MediaLibrary->lock);
	return found;
}
BOOL MediaLibrary_imageBufferDraw(const char* url, const char* ext, Image4* img, Quad2i coord)
{
	BOOL found = FALSE;
	OsLock_lock(&g_MediaLibrary->lock);

	int i;
	for (i = 0; i < g_MediaLibrary->images.num; i++)
	{
		MediaImage* it = (MediaImage*)g_MediaLibrary->images.ptrs[i];
		if (it->loaded && MediaImage_isUrl(it, url, ext, coord.size))
		{
			Image4_copyImage4(img, Quad2i_getSubStart(coord, it->img.size), &it->img);

			found = TRUE;
			break;
		}
	}

	OsLock_unlock(&g_MediaLibrary->lock);
	return found;
}

void MediaLibrary_setVolume(float volume)
{
	g_MediaLibrary->volume = volume;
}

BOOL MediaLibrary_isPlayingSomething(void)
{
	BOOL play = FALSE;

	OsLock_lock(&g_MediaLibrary->lock);

	/*int i;
	for(i=0; i < g_MediaLibrary->flacs.num; i++)
		play |= ((MediaFlac*)g_MediaLibrary->flacs.ptrs[i])->play;

	for(i=0; i < g_MediaLibrary->opuss.num; i++)
		play |= ((MediaOpus*)g_MediaLibrary->opuss.ptrs[i])->play;*/

	OsLock_unlock(&g_MediaLibrary->lock);

	return play;
}

void MediaLibrary_stopAllPlay(void)
{
	OsLock_lock(&g_MediaLibrary->lock);

	/*
	int i;
	for(i=0; i < g_MediaLibrary->flacs.num; i++)
		((MediaFlac*)g_MediaLibrary->flacs.ptrs[i])->play = FALSE;

	for(i=0; i < g_MediaLibrary->opuss.num; i++)
		((MediaOpus*)g_MediaLibrary->opuss.ptrs[i])->play = FALSE;*/

	OsLock_unlock(&g_MediaLibrary->lock);
}

void MediaLibrary_play(FileRow fileId, BOOL play)
{
	OsLock_lock(&g_MediaLibrary->lock);

	/*int i;
	for(i=0; i < g_MediaLibrary->flacs.num; i++)
		if(MediaFlac_is(g_MediaLibrary->flacs.ptrs[i], fileId))
			((MediaFlac*)g_MediaLibrary->flacs.ptrs[i])->play = play;

	for(i=0; i < g_MediaLibrary->opuss.num; i++)
		if(MediaOpus_is(g_MediaLibrary->opuss.ptrs[i], fileId))
			((MediaOpus*)g_MediaLibrary->opuss.ptrs[i])->play = play;*/

	OsLock_unlock(&g_MediaLibrary->lock);
}

BOOL MediaLibrary_isPlay(FileRow fileId)
{
	BOOL play = FALSE;

	OsLock_lock(&g_MediaLibrary->lock);

	/*int i;
	for(i=0; i < g_MediaLibrary->flacs.num; i++)
		if(MediaFlac_is(g_MediaLibrary->flacs.ptrs[i], fileId))
			play |= ((MediaFlac*)g_MediaLibrary->flacs.ptrs[i])->play;

	for(i=0; i < g_MediaLibrary->opuss.num; i++)
		if(MediaOpus_is(g_MediaLibrary->opuss.ptrs[i], fileId))
			play |= ((MediaOpus*)g_MediaLibrary->opuss.ptrs[i])->play;*/

	OsLock_unlock(&g_MediaLibrary->lock);

	return play;
}

void MediaLibrary_setSeek(FileRow fileId, float t)
{
	OsLock_lock(&g_MediaLibrary->lock);

	/*int i;
	for(i=0; i < g_MediaLibrary->flacs.num; i++)
		if(MediaFlac_is(g_MediaLibrary->flacs.ptrs[i], fileId))
			MediaFlac_setSeek(g_MediaLibrary->flacs.ptrs[i], t);

	for(i=0; i < g_MediaLibrary->opuss.num; i++)
		if(MediaOpus_is(g_MediaLibrary->opuss.ptrs[i], fileId))
			MediaOpus_setSeek(g_MediaLibrary->opuss.ptrs[i], t);*/

	OsLock_unlock(&g_MediaLibrary->lock);
}

float MediaLibrary_getSeek(FileRow fileId)
{
	float t = 0;

	OsLock_lock(&g_MediaLibrary->lock);

	/*int i;
	for(i=0; i < g_MediaLibrary->flacs.num; i++)
		if(MediaFlac_is(g_MediaLibrary->flacs.ptrs[i], fileId))
			t = MediaFlac_getSeek(g_MediaLibrary->flacs.ptrs[i]);

	for(i=0; i < g_MediaLibrary->opuss.num; i++)
		if(MediaOpus_is(g_MediaLibrary->opuss.ptrs[i], fileId))
			t = MediaOpus_getSeek(g_MediaLibrary->opuss.ptrs[i]);*/

	OsLock_unlock(&g_MediaLibrary->lock);

	return t;
}

/*UNI* MediaLibrary_audioInfo(FileRow fileId)
{
	UNI* str = 0;

	OsLock_lock(&g_MediaLibrary->lock);

	int i;
	for(i=0; i < g_MediaLibrary->flacs.num; i++)
		if(MediaFlac_is(g_MediaLibrary->flacs.ptrs[i], fileId))
			str = Std_newUNI(((MediaFlac*)g_MediaLibrary->flacs.ptrs[i])->info);

	for(i=0; i < g_MediaLibrary->opuss.num; i++)
		if(MediaOpus_is(g_MediaLibrary->opuss.ptrs[i], fileId))
			str = Std_newUNI(((MediaOpus*)g_MediaLibrary->opuss.ptrs[i])->info);

	OsLock_unlock(&g_MediaLibrary->lock);

	return str;
}*/

/*UNI* MediaLibrary_getText(FileID fileId)
{
	UNI* str = 0;

	OsLock_lock(&g_MediaLibrary->lock);

	int i;
	for(i=0; i < g_MediaLibrary->texts.num; i++)
		if(MediaText_is(g_MediaLibrary->texts.ptrs[i], fileId))
			str = Std_newUNI(((MediaText*)g_MediaLibrary->texts.ptrs[i])->text);

	OsLock_unlock(&g_MediaLibrary->lock);

	return str;
}*/

void MediaLibrary_maintenanceAudio(void)
{
	OsLock_lock(&g_MediaLibrary->lock);

	/*
	double time = Os_time();
	int i;
	for(i=g_MediaLibrary->flacs.num-1; i >=0 ; i--)
	{
		MediaFlac* flac = g_MediaLibrary->flacs.ptrs[i];
		if(!flac->play && (time - flac->timeUse) > Media_DELAY)
		{
			MediaFlac_delete(flac);
			StdArr_remove(&g_MediaLibrary->flacs, i);
		}
	}

	for(i=g_MediaLibrary->opuss.num-1; i >=0 ; i--)
	{
		MediaOpus* opus = g_MediaLibrary->opuss.ptrs[i];
		if(!opus->play && (time - opus->timeUse) > Media_DELAY)
		{
			MediaOpus_delete(opus);
			StdArr_remove(&g_MediaLibrary->opuss, i);
		}
	}*/

	OsLock_unlock(&g_MediaLibrary->lock);
}

void MediaLibrary_maintenanceImages(void)
{
	OsLock_lock(&g_MediaLibrary->lock);

	double time = Os_time();
	int i;
	for (i = g_MediaLibrary->images.num - 1; i >= 0; i--)
	{
		MediaImage* img = g_MediaLibrary->images.ptrs[i];
		if ((time - img->timeUse) > Media_DELAY)
		{
			MediaImage_delete(img);
			StdArr_remove(&g_MediaLibrary->images, i);
		}
	}

	OsLock_unlock(&g_MediaLibrary->lock);
}

MediaImage* MediaLibrary_getSafeImage(int i)
{
	OsLock_lock(&g_MediaLibrary->lock);
	MediaImage* img = (i < g_MediaLibrary->images.num) ? g_MediaLibrary->images.ptrs[i] : 0;
	OsLock_unlock(&g_MediaLibrary->lock);
	return img;
}

/*MediaFlac* MediaLibrary_getSafeFlac(int i)
{
	OsLock_lock(&g_MediaLibrary->lock);
	MediaFlac* img = (i < g_MediaLibrary->flacs.num) ? g_MediaLibrary->flacs.ptrs[i] : 0;
	OsLock_unlock(&g_MediaLibrary->lock);
	return img;
}

MediaOpus* MediaLibrary_getSafeOpus(int i)
{
	OsLock_lock(&g_MediaLibrary->lock);
	MediaOpus* img = (i < g_MediaLibrary->opuss.num) ? g_MediaLibrary->opuss.ptrs[i] : 0;
	OsLock_unlock(&g_MediaLibrary->lock);
	return img;
}*/

/*MediaText* MediaLibrary_getSafeText(int i)
{
	OsLock_lock(&g_MediaLibrary->lock);
	MediaText* txt = (i < g_MediaLibrary->texts.num) ? g_MediaLibrary->texts.ptrs[i] : 0;
	OsLock_unlock(&g_MediaLibrary->lock);
	return txt;
}*/

THREAD_FUNC(MediaLibrary_loopImages, param)
{
	while (OsThread_tick(&g_MediaLibrary->images_thread))
	{
		const int n_max = 3;
		int n = 3;
		int i;
		for (i = g_MediaLibrary->images.num - 1; i >= 0 && n > 0; i--)	//backwards => load newest first
		{
			if (MediaImage_cook(MediaLibrary_getSafeImage(i)))
				n--;
		}

		FileCache_maintenance();
		MediaLibrary_maintenanceImages();

		if (n == n_max)
			OsThread_sleep(1);
	}
	return 0;
}

THREAD_FUNC(MediaLibrary_loopAudio, param)
{
	OsAudioSpeaker* speaker = 0;
	int* buff = Os_malloc(OsAudio_RECOMMAND_BLOCK_SIZE * 2);
	short* buff2 = Os_malloc(OsAudio_RECOMMAND_BLOCK_SIZE);

	while (OsThread_tick(&g_MediaLibrary->audio_thread))
	{
		BOOL isPlay = MediaLibrary_isPlayingSomething();

		if (speaker)
			OsAudioSpeaker_pause(speaker, !isPlay);

		if (isPlay)
		{
			Os_memset(buff, OsAudio_RECOMMAND_BLOCK_SIZE * 2);
			Os_memset(buff2, OsAudio_RECOMMAND_BLOCK_SIZE);

			int n = 0;
			int i;
			/*for(i=0; i < g_MediaLibrary->opuss.num; i++)
				n += MediaOpus_cook((MediaOpus*)g_MediaLibrary->opuss.ptrs[i], buff);

			for(i=0; i < g_MediaLibrary->flacs.num; i++)
				n += MediaFlac_cook((MediaFlac*)g_MediaLibrary->flacs.ptrs[i], buff);*/

			if (n)
			{
				for (i = 0; i < OsAudio_RECOMMAND_BLOCK_SIZE / 2; i++)
					buff2[i] = buff[i] / n;
			}

			if (!speaker)
				speaker = OsAudioSpeaker_new(OsAudio_RECOMMAND_BLOCK_SIZE, 48000, 16, 2);
			OsAudioSpeaker_write(speaker, buff2, OsAudio_RECOMMAND_BLOCK_SIZE, g_MediaLibrary->volume);
		}

		MediaLibrary_maintenanceAudio();
		OsThread_sleep(1);
	}

	if (speaker)
		OsAudioSpeaker_delete(speaker);

	Os_free(buff, OsAudio_RECOMMAND_BLOCK_SIZE * 2);
	Os_free(buff2, OsAudio_RECOMMAND_BLOCK_SIZE);

	return 0;
}

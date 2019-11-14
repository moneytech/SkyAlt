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

typedef struct MediaNetworkCache_s
{
	double timeUse;
	char* url;
	UCHAR* data;
	BIG bytes;
	double priority;
}MediaNetworkCache;

MediaNetworkCache* MediaNetworkCache_new(const char* url)
{
	MediaNetworkCache* self = Os_malloc(sizeof(MediaNetworkCache));
	self->timeUse = Os_time();
	self->url = Std_newCHAR(url);
	self->data = 0;
	self->bytes = -1;
	return self;
}
void MediaNetworkCache_delete(MediaNetworkCache* self)
{
	Os_free(self->data, self->bytes);
	Std_deleteCHAR(self->url);
	Os_free(self, sizeof(MediaNetworkCache));
}
BOOL MediaNetworkCache_isTimeValid(const MediaNetworkCache* self, double actual_time)
{
	return (actual_time - self->timeUse) > Media_DELAY;
}
void MediaNetworkCache_updateTime(MediaNetworkCache* self)
{
	self->timeUse = Os_time();
}
void MediaNetworkCache_removeTime(MediaNetworkCache* self)
{
	self->timeUse = Os_time() - 2 * Media_DELAY;
}

BIG MediaNetworkCache_copy(MediaNetworkCache* self, UCHAR* dst, UBIG start, UBIG max_bytes)
{
	if (self->bytes >= 0)
	{
		if (start < self->bytes)
		{
			UBIG bytes = Std_min(max_bytes, self->bytes - start);
			Os_memcpy(dst, &self->data[start], bytes);
			return bytes;
		}
		else
			return 0;
	}
	return -1;
}

typedef struct MediaNetwork_s
{
	OsLock lock;
	OsThread thread;
	StdArr cache;
}MediaNetwork;

MediaNetwork* g_MediaNetwork = 0;
UBIG g_MediaNetwork_download_bytes = 0;
UBIG g_MediaNetwork_download_files = 0;

THREAD_FUNC(MediaNetwork_loop, param);

void MediaNetwork_delete(void)
{
	OsThread_free(&g_MediaNetwork->thread, TRUE);
	OsLock_free(&g_MediaNetwork->lock);

	StdArr_freeFn(&g_MediaNetwork->cache, (StdArrFREE)&MediaNetworkCache_delete);

	Os_free(g_MediaNetwork, sizeof(MediaNetwork));
	g_MediaNetwork = 0;
}

BOOL MediaNetwork_new(void)
{
	if (g_MediaNetwork)
		MediaNetwork_delete();

	g_MediaNetwork = Os_malloc(sizeof(MediaNetwork));

	g_MediaNetwork->cache = StdArr_init();

	OsLock_init(&g_MediaNetwork->lock);
	OsThread_init(&g_MediaNetwork->thread, g_MediaNetwork, &MediaNetwork_loop);
	return TRUE;
}

BOOL MediaNetwork_is(void)
{
	return g_MediaNetwork != 0;
}

static MediaNetworkCache* _MediaNetwork_getCache(BIG i)
{
	return g_MediaNetwork->cache.ptrs[i];
}

static BIG _MediaNetwork_find(const char* url)
{
	BIG i;
	for (i = 0; i < g_MediaNetwork->cache.num; i++)
	{
		if (Std_cmpCHAR(_MediaNetwork_getCache(i)->url, url))
			return i;
	}
	return -1;
}

static UBIG _MediaNetwork_add(const char* url, double priority)
{
	BIG i = _MediaNetwork_find(url);
	if (i < 0)
	{
		StdArr_add(&g_MediaNetwork->cache, MediaNetworkCache_new(url));
		i = g_MediaNetwork->cache.num - 1;
	}

	_MediaNetwork_getCache(i)->priority = Std_fclamp(priority, 0, 1);

	return i;
}

BIG MediaNetwork_download(const char* url, UCHAR* dst, UBIG start, UBIG max_bytes, double priority)
{
	BIG ret_bytes = -1;

	OsLock_lock(&g_MediaNetwork->lock);

	MediaNetworkCache* cache = _MediaNetwork_getCache(_MediaNetwork_add(url, priority));
	MediaNetworkCache_updateTime(cache);

	ret_bytes = MediaNetworkCache_copy(cache, dst, start, max_bytes);

	OsLock_unlock(&g_MediaNetwork->lock);

	return ret_bytes;
}

BOOL MediaNetwork_undownload(const char* url)
{
	BOOL found = FALSE;

	OsLock_lock(&g_MediaNetwork->lock);

	BIG i = _MediaNetwork_find(url);
	if (i >= 0)
	{
		MediaNetworkCache_removeTime(_MediaNetwork_getCache(i));
		found = TRUE;
	}

	OsLock_unlock(&g_MediaNetwork->lock);

	return found;
}

static MediaNetworkCache* _MediaNetwork_getNonDownload(void)
{
	MediaNetworkCache* ret = 0;
	double maxPriority = -1;

	OsLock_lock(&g_MediaNetwork->lock);

	int i;
	for (i = 0; i < g_MediaNetwork->cache.num; i++)
	{
		MediaNetworkCache* cache = _MediaNetwork_getCache(i);
		if (cache->bytes < 0 && cache->priority > maxPriority)
		{
			maxPriority = cache->priority;
			ret = cache;
		}
	}

	OsLock_unlock(&g_MediaNetwork->lock);

	return ret;
}

static void _MediaNetwork_maintenance(void)
{
	OsLock_lock(&g_MediaNetwork->lock);

	double time = Os_time();
	int i;
	for (i = g_MediaNetwork->cache.num - 1; i >= 0; i--)
	{
		MediaNetworkCache* cache = _MediaNetwork_getCache(i);
		if (MediaNetworkCache_isTimeValid(cache, time))
		{
			MediaNetworkCache_delete(cache);
			StdArr_remove(&g_MediaNetwork->cache, i);
		}
	}

	OsLock_unlock(&g_MediaNetwork->lock);
}

THREAD_FUNC(MediaNetwork_loop, param)
{
	while (OsThread_tick(&g_MediaNetwork->thread))
	{
		MediaNetworkCache* cache = _MediaNetwork_getNonDownload();
		if (cache)
		{
			float done;
			BIG bytes = OsHTTPS_downloadWithStatus(cache->url, &done, &g_MediaNetwork->thread.m_running, (char**)&cache->data);
			if (g_MediaNetwork->thread.m_running)
			{
				if (bytes >= 0)
				{
					cache->bytes = bytes;
					MediaNetworkCache_updateTime(cache);

					g_MediaNetwork_download_bytes += bytes;
					g_MediaNetwork_download_files++;
				}
				else
					MediaNetworkCache_removeTime(cache);	//error = remove it
			}
		}

		_MediaNetwork_maintenance();

		if (!cache)
			OsThread_sleep(1);
	}
	return 0;
}

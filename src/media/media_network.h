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

typedef struct MediaNetworkDelay_s
{
	char* url;
	double timeUse;
	double delay;
}MediaNetworkDelay;

MediaNetworkDelay* MediaNetworkDelay_new(const char* url, double delay)
{
	MediaNetworkDelay* self = Os_malloc(sizeof(MediaNetworkDelay));
	self->url = Std_newCHAR(url);
	self->delay = delay;
	self->timeUse = 0;
	return self;
}
void MediaNetworkDelay_delete(MediaNetworkDelay* self)
{
	Std_deleteCHAR(self->url);
	Os_free(self, sizeof(MediaNetworkDelay));
}
BOOL MediaNetworkDelay_is(const MediaNetworkDelay* self, const char* url)
{
	return Std_startWithCHAR(url, self->url);
}
BOOL MediaNetworkDelay_canBeUse(MediaNetworkDelay* self)
{
	return (Os_time() - self->timeUse) > self->delay;
}
void MediaNetworkDelay_use(MediaNetworkDelay* self)
{
	self->timeUse = Os_time();
}

typedef struct MediaNetwork_s
{
	OsLock lock;
	OsThread thread;
	StdArr cache;
	StdArr delays;

	double last_download;
	double bandwidthStart;
	UBIG bandwidthBytes;
}MediaNetwork;

MediaNetwork* g_MediaNetwork = 0;
UBIG g_MediaNetwork_download_bytes = 0;
UBIG g_MediaNetwork_download_files = 0;

THREAD_FUNC(MediaNetwork_loop, param);

void MediaNetwork_delete(void)
{
	if (g_MediaNetwork)
	{
		OsThread_free(&g_MediaNetwork->thread, TRUE);
		OsLock_free(&g_MediaNetwork->lock);

		StdArr_freeFn(&g_MediaNetwork->cache, (StdArrFREE)&MediaNetworkCache_delete);
		StdArr_freeFn(&g_MediaNetwork->delays, (StdArrFREE)&MediaNetworkDelay_delete);

		Os_free(g_MediaNetwork, sizeof(MediaNetwork));
		g_MediaNetwork = 0;
	}
}

BOOL MediaNetwork_new(BOOL online)
{
	if (g_MediaNetwork)
		MediaNetwork_delete();

	g_MediaNetwork = Os_malloc(sizeof(MediaNetwork));

	g_MediaNetwork->cache = StdArr_init();
	g_MediaNetwork->delays = StdArr_init();

	g_MediaNetwork->last_download = 0;
	g_MediaNetwork->bandwidthStart = Os_time();
	g_MediaNetwork->bandwidthBytes = 0;

	OsLock_init(&g_MediaNetwork->lock);

	g_MediaNetwork->thread = OsThread_initEmpty();
	MediaNetwork_run(online);

	return TRUE;
}

BOOL MediaNetwork_is(void)
{
	return OsThread_isRunning(&g_MediaNetwork->thread);
}

void MediaNetwork_run(BOOL run)
{
	if (run && !MediaNetwork_is())
		OsThread_init(&g_MediaNetwork->thread, g_MediaNetwork, &MediaNetwork_loop);
	else
		if (!run && MediaNetwork_is())
			OsThread_free(&g_MediaNetwork->thread, TRUE);
}

static MediaNetworkCache* _MediaNetwork_getCache(BIG i)
{
	return g_MediaNetwork->cache.ptrs[i];
}
static MediaNetworkDelay* _MediaNetwork_getDelay(BIG i)
{
	return g_MediaNetwork->delays.ptrs[i];
}

void MediaNetwork_addDelay(const char* url, double delay)
{
	OsLock_lock(&g_MediaNetwork->lock);

	//no duplicity
	BOOL found = FALSE;
	BIG i;
	for (i = 0; i < g_MediaNetwork->delays.num; i++)
	{
		if (Std_cmpCHAR(_MediaNetwork_getDelay(i)->url, url))
		{
			_MediaNetwork_getDelay(i)->delay = delay;
			found = TRUE;
		}
	}

	//add
	if (!found)
		StdArr_add(&g_MediaNetwork->delays, MediaNetworkDelay_new(url, delay));

	OsLock_unlock(&g_MediaNetwork->lock);
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

static BOOL _MediaNetwork_isDelayPassed(const char* url)
{
	BIG i;
	for (i = 0; i < g_MediaNetwork->delays.num; i++)
	{
		if (MediaNetworkDelay_is(_MediaNetwork_getDelay(i), url))
			return MediaNetworkDelay_canBeUse(_MediaNetwork_getDelay(i));
	}
	return TRUE;	//no delays record find so it can be use
}
static void _MediaNetwork_useDelay(const char* url)
{
	BIG i;
	for (i = 0; i < g_MediaNetwork->delays.num; i++)
	{
		if (MediaNetworkDelay_is(_MediaNetwork_getDelay(i), url))
			MediaNetworkDelay_use(_MediaNetwork_getDelay(i));
	}
}

BIG MediaNetwork_download(const char* url, double priority, UCHAR** buff)
{
	BIG ret_bytes = -1;

	g_MediaNetwork->last_download = Os_time();

	if (MediaNetwork_is())
	{
		OsLock_lock(&g_MediaNetwork->lock);

		MediaNetworkCache* cache = _MediaNetwork_getCache(_MediaNetwork_add(url, priority));
		MediaNetworkCache_updateTime(cache);

		if (cache->bytes >= 0)
		{
			ret_bytes = cache->bytes;
			*buff = cache->data;

			cache->data = 0;
			cache->bytes = 0;
			MediaNetworkCache_removeTime(cache);
		}

		OsLock_unlock(&g_MediaNetwork->lock);
	}
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

BOOL MediaNetwork_isDownloadActive(void)
{
	return (Os_time() - g_MediaNetwork->last_download) < 1;
}

double MediaNetwork_getBandwidth(void)
{
	OsLock_lock(&g_MediaNetwork->lock);

	double t = Os_time();
	double bandwidth = g_MediaNetwork->bandwidthBytes / (t - g_MediaNetwork->bandwidthStart);

	//reset
	g_MediaNetwork->bandwidthBytes = 0;
	g_MediaNetwork->bandwidthStart = t;

	OsLock_unlock(&g_MediaNetwork->lock);

	return bandwidth;
}

static void _MediaNetwork_addBandwidth(UBIG bytes)
{
	OsLock_lock(&g_MediaNetwork->lock);

	g_MediaNetwork->bandwidthBytes += bytes;

	OsLock_unlock(&g_MediaNetwork->lock);
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
		if (cache->bytes < 0 && cache->priority > maxPriority&& _MediaNetwork_isDelayPassed(cache->url))
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
				_MediaNetwork_useDelay(cache->url);

				if (bytes >= 0)
				{
					cache->bytes = bytes;
					MediaNetworkCache_updateTime(cache);

					g_MediaNetwork_download_bytes += bytes;
					g_MediaNetwork_download_files++;

					_MediaNetwork_addBandwidth(bytes);
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

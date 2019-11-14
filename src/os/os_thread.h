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

THREAD_FUNC(OsThread_loop, param)
{
	OsThread* self = (OsThread*)param;
	self->func(self->func_param);
	return 0;
}

OsThread OsThread_initEmpty(void)
{
	OsThread self;
	self.m_thread = 0;
	self.m_running = FALSE;
	return self;
}

OsThread OsThread_initRoot(void)
{
	OsThread self = OsThread_initEmpty();
	return self;
}

const char* OsThread_init(OsThread* self, void* param, OsThread_loopFUNC func)
{
	const char* err = 0;
	self->m_running = TRUE;
	self->func = func;
	self->func_param = param;

#ifdef WIN32
	if (!(self->m_thread = CreateThread(NULL, 0, &OsThread_loop, self, 0, NULL)))
		err = "CreateThread";
#else
	if (pthread_create(&self->m_thread, 0, &OsThread_loop, self) != 0)
		err = "pthread_create";
#endif
	return err;
}

void OsThread_setGameOver(OsThread* self)
{
	self->m_running = FALSE;
}

BOOL OsThread_waitUntilIsFinished(OsThread* self)
{
	if (self->m_thread)
#ifdef WIN32
		WaitForSingleObject(self->m_thread, INFINITE);
#else
		if (pthread_join(self->m_thread, NULL))
			return FALSE;
#endif
	return TRUE;
}

const char* OsThread_free(OsThread* self, BOOL gameOver)
{
	const char* err = 0;
	if (gameOver)
		OsThread_setGameOver(self);
	OsThread_waitUntilIsFinished(self);
	return err;
}

BOOL OsThread_isRunning(const OsThread* self)
{
	return self->m_running;
}

void OsThread_sleep(int ms)
{
#ifdef _WIN32
	Sleep(ms); //mili-seconds
#else
	usleep(ms * 1000); //micro-seconds
#endif
}

UINT OsThread_getNumCPUCores(void)
{
	UINT n;
#ifdef _WIN32
	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);
	n = sysinfo.dwNumberOfProcessors;
#else
	n = sysconf(_SC_NPROCESSORS_ONLN);
#endif
	return Std_max(1, n);
}

UBIG OsThread_getID(void)
{
#ifdef _WIN32
	return(UBIG)GetCurrentThread(); //GetCurrentThreadId(); //...
#else
	return pthread_self();
#endif
}

UBIG OsThread_isID(const OsThread* self, UBIG id)
{
	return(UBIG)self->m_thread == id;
}

UBIG OsThread_getMaxRam(void)
{
#ifdef _WIN32
	MEMORYSTATUSEX statex;
	statex.dwLength = sizeof(statex);
	GlobalMemoryStatusEx(&statex);
	return statex.ullTotalPhys;
#else
	struct sysinfo info;
	return sysinfo(&info) == 0 ? info.totalram : 1;
#endif
}

BOOL OsThread_tick(OsThread* self)
{
	return OsThread_isRunning(self);
}

BOOL OsLock_init(OsLock* self)
{
#ifdef _WIN32
	self->m_mutex = CreateMutex(0, FALSE, 0);
	return self->m_mutex != 0;
#else
	self->m_mutex = malloc(sizeof(pthread_mutex_t));
	return pthread_mutex_init((pthread_mutex_t*)self->m_mutex, NULL) == 0;
#endif
}

void OsLock_free(OsLock* self)
{
#ifdef _WIN32
	CloseHandle(self->m_mutex);
	self->m_mutex = 0;
#else
	pthread_mutex_destroy((pthread_mutex_t*)self->m_mutex);
	free(self->m_mutex);
	self->m_mutex = 0;
#endif
}

void OsLock_lock(OsLock* self)
{
#ifdef _WIN32
	WaitForSingleObject(self->m_mutex, INFINITE);
#else
	pthread_mutex_lock((pthread_mutex_t*)self->m_mutex);
#endif
}

void OsLock_unlock(OsLock* self)
{
#ifdef _WIN32
	ReleaseMutex(self->m_mutex);
#else
	pthread_mutex_unlock((pthread_mutex_t*)self->m_mutex);
#endif
}

BOOL OsLock_tryLock(OsLock* self)
{
#ifdef _WIN32
	return WaitForSingleObject(self->m_mutex, 0) != WAIT_TIMEOUT;
#else
	return pthread_mutex_trylock((pthread_mutex_t*)self->m_mutex) == 0;
#endif
}

BOOL OsLockEvent_init(OsLockEvent* self)
{
#ifdef _WIN32
	self->m_event = CreateEvent(0, 0, 0, 0);
	return self->m_event != 0;
#else
	BOOL ok = TRUE;
	self->m_signalled = FALSE;
	ok &= pthread_mutex_init((pthread_mutex_t*)self->m_mutex, NULL) == 0;
	ok &= pthread_cond_init((pthread_cond_t*)self->m_cond, 0) == 0;
	return ok;
#endif
}
void OsLockEvent_free(OsLockEvent* self)
{
#ifdef _WIN32
	CloseHandle(self->m_event);
	self->m_event = 0;
#else
	pthread_mutex_destroy((pthread_mutex_t*)self->m_mutex);
	pthread_cond_destroy((pthread_cond_t*)self->m_cond);
#endif
}
BOOL OsLockEvent_wait(OsLockEvent* self, int timeout_ms)
{
#ifdef _WIN32
	return WaitForSingleObject(self->m_event, (timeout_ms > 0) ? timeout_ms : INFINITE) == WAIT_OBJECT_0;	//true = triggered, false = timeout/failed
#else
	BOOL trig = TRUE;
	struct timeval now;
	gettimeofday(&now, NULL);

	struct timespec timeToWait;
	timeToWait.tv_sec = now.tv_sec + 5;
	timeToWait.tv_nsec = (now.tv_usec + 1000UL * timeout_ms) * 1000UL;

	pthread_mutex_lock((pthread_mutex_t*)self->m_mutex);
	while (!self->m_signalled)
	{
		if (timeout_ms > 0)
			trig = pthread_cond_timedwait((pthread_cond_t*)self->m_cond, (pthread_mutex_t*)self->m_mutex, &timeToWait) != ETIMEDOUT;
		else
			pthread_cond_wait((pthread_cond_t*)self->m_cond, (pthread_mutex_t*)self->m_mutex);
	}
	self->m_signalled = FALSE;
	pthread_mutex_unlock((pthread_mutex_t*)self->m_mutex);
	return trig;
#endif
}
void OsLockEvent_trigger(OsLockEvent* self)
{
#ifdef _WIN32
	SetEvent(self->m_event);
#else
	pthread_mutex_lock((pthread_mutex_t*)self->m_mutex);
	self->m_signalled = TRUE;
	pthread_mutex_unlock((pthread_mutex_t*)self->m_mutex);
	pthread_cond_signal((pthread_cond_t*)self->m_cond);
#endif
}

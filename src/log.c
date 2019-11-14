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
#include "language.h"
#include "file.h"
#include "db.h"

//header
#include "log.h"

typedef struct Log_s
{
	OsDate date;
	const char* id;
	const char* type;	//GUI_ERROR, etc.
} Log;

Log* Log_new(const char* id, const char* type)
{
	Log* self = Os_malloc(sizeof(Log));
	self->date = OsDate_initActual();
	self->id = id;
	self->type = type;
	return self;
}

void Log_delete(Log* self)
{
	Os_free(self, sizeof(Log));
}

typedef struct Logs_s
{
	OsLock lock;

	StdArr logs;
	BOOL changed;
} Logs;

Logs g_logs;

void Logs_initGlobal(void)
{
	g_logs.logs = StdArr_init();
	g_logs.changed = FALSE;
	OsLock_init(&g_logs.lock);
}

void Logs_freeGlobal(void)
{
	OsLock_free(&g_logs.lock);
	StdArr_freeFn(&g_logs.logs, (StdArrFREE)&Log_delete);
	Os_memset(&g_logs, sizeof(Logs));
}

void Logs_clear(void)
{
	OsLock_lock(&g_logs.lock);

	g_logs.changed = (g_logs.logs.num > 0);
	StdArr_freeFn(&g_logs.logs, (StdArrFREE)&Log_delete);
	g_logs.logs = StdArr_init();

	OsLock_unlock(&g_logs.lock);
}

static void _Logs_add(Log* log)
{
	OsLock_lock(&g_logs.lock);

	StdArr_insert(&g_logs.logs, 0, log);
	g_logs.changed = TRUE;

	OsLock_unlock(&g_logs.lock);
}

void Logs_addInfo(const char* id)
{
	_Logs_add(Log_new(id, "INFO"));
}

void Logs_addWarning(const char* id)
{
	_Logs_add(Log_new(id, "WARNING"));
}

void Logs_addError(const char* id)
{
	_Logs_add(Log_new(id, "ERROR"));
}

BOOL Logs_isChanged(void)
{
	BOOL ret = g_logs.changed;
	g_logs.changed = FALSE;
	return ret;
}

static Log* Logs_getI(UBIG i)
{
	return g_logs.logs.ptrs[i];
}

UBIG Logs_num(void)
{
	return g_logs.logs.num;
}
UNI* Logs_dateStr(UBIG i)
{
	OsLock_lock(&g_logs.lock);
	UNI* ret = (i < g_logs.logs.num) ? OsDate_getStringDateTimeUNI(&Logs_getI(i)->date, OsDate_ISO, OsDate_HMS, 0) : 0;
	OsLock_unlock(&g_logs.lock);
	return ret;
}
const char* Logs_id(UBIG i)
{
	OsLock_lock(&g_logs.lock);
	const char* ret = (i < g_logs.logs.num) ? Logs_getI(i)->id : 0;
	OsLock_unlock(&g_logs.lock);
	return ret;
}
const char* Logs_type(UBIG i)
{
	OsLock_lock(&g_logs.lock);
	const char* ret = (i < g_logs.logs.num) ? Logs_getI(i)->type : 0;
	OsLock_unlock(&g_logs.lock);
	return ret;
}
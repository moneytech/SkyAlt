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

typedef struct StdProgress_s
{
	const char* trans;	//must be STATIC, because it's multi-thread!

	volatile float done;
	volatile BOOL running;
}StdProgress;

volatile StdProgress g_progress;

void StdProgress_initGlobal(void)
{
	g_progress.trans = 0;
	g_progress.done = 0;
	g_progress.running = TRUE;
}

void StdProgress_freeGlobal(void)
{
	Os_memset((void*)&g_progress, sizeof(StdProgress));
}

float StdProgress_get(void)
{
	return g_progress.done;
}

void StdProgress_set(const char* trans, float done)
{
	g_progress.trans = trans;
	g_progress.done = done;
}
void StdProgress_setEx(const char* trans, double part, double maxx)
{
	StdProgress_set(trans, maxx ? part / maxx : 0);
}

void StdProgress_setExx(const char* trans, double part, double maxx, double progressStart)
{
	StdProgress_setEx(trans, part, maxx);
	g_progress.done += progressStart;
}

void StdProgress_run(BOOL run)
{
	g_progress.running = run;
	g_progress.done = 0;
}

BOOL StdProgress_is(void)
{
	return g_progress.running;
}

const char* StdProgress_getTranslationID(void)
{
	return g_progress.trans;
}

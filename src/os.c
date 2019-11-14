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

 //header
#include "os.h"

//collaborate
#include "std.h"

#include "os/os_base.h"
#include "os/os_thread.h"
#include "os/os_time.h"
#include "os/os_crypto.h"
#include "os/os_file.h"
#include "os/os_net.h"
#include "os/os_font.h"
#include "os/os_screen.h"
#include "os/os_win_io.h"
#include "os/os_zlib.h"
#include "os/os_audio.h"
#include "os/os_media.h"

#ifdef _WIN32
#include "os/os_window_win.h"
#elif __linux__
#include "os/os_window_gtk.h"
#elif __APPLE__

#endif

UCHAR g_UiAutoUpdate_pubKey[] = { 4, 242, 20, 244, 152, 63, 154, 68, 0, 132, 88, 0, 144, 199, 155, 167, 121, 45, 87, 185, 222, 87, 127, 175, 231, 229, 83, 181, 149, 155, 132, 66, 189, 251, 203, 105, 235, 211, 218, 89, 39, 62, 127, 193, 185, 197, 36, 236, 216, 39, 137, 176, 168, 114, 126, 188, 243, 0, 201, 167, 241, 59, 170, 28, 206 };

UCHAR* Os_getUpdatePublicKey(void)
{
	return g_UiAutoUpdate_pubKey;
}

double Os_sqrt(double x)
{
	return sqrt(x);
}
double Os_pow(double x, double y)
{
	return pow(x, y);
}

double Os_atof(const char* str)
{
	while (*str && Std_isNotDigit(*str))		//skip chars before number
		str++;

	char* end;
	return strtod(str, &end);
}

void Os_gcvt(double value, int digits, char* str)
{
#ifdef _WIN32
	_gcvt(value, digits, str);
#elif __linux__
	gcvt(value, digits, str);
#endif
}

double Os_cos(double x)
{
	return cos(x);
}
double Os_tan(double x)
{
	return tan(x);
}
double Os_atan(double x)
{
	return atan(x);
}
double Os_log(double x)
{
	return log(x);
}
double Os_exp(double x)
{
	return exp(x);
}
int Os_isalnum(int c)
{
	return isalnum(c);
}

void Os_memsetEx(void* ptr, int value, UBIG size)
{
	memset(ptr, value, size);
}

void Os_memset(void* ptr, UBIG size)
{
	Os_memsetEx(ptr, 0, size);
}

void* Os_calloc(UBIG count, UBIG item_size)
{
#ifdef _DEBUG
	return _calloc_dbg(count, item_size, _NORMAL_BLOCK, 0, 0);
#else
	return calloc(count, item_size);
#endif
}

void* Os_malloc(UBIG size)
{
	return Os_calloc(1, size);
}

void* Os_realloc(void* ptr, UBIG size)
{
#ifdef _DEBUG
	return _realloc_dbg(ptr, size, _NORMAL_BLOCK, 0, 0);
#else
	return realloc(ptr, size);
#endif
}

void Os_free(void* ptr, UBIG size)
{
	if (ptr)
		Os_memset(ptr, size);

#ifdef _DEBUG
	_free_dbg(ptr, _NORMAL_BLOCK);
#else
	free(ptr);
#endif
}

void* Os_memcpy(void* dst, const void* src, UBIG size)
{
	return memcpy(dst, src, size);
}
void* Os_memmove(void* dst, void* src, UBIG size)
{
	return memmove(dst, src, size);
}
int Os_memcmp(void* a, void* b, UBIG size)
{
	return memcmp(a, b, size);
}

void Os_showConsole(BOOL show)
{
#ifdef _WIN32
	ShowWindow(GetConsoleWindow(), show ? SW_SHOW : SW_HIDE);
#endif
}

void Os_showMemleaks(void)
{
#ifdef _DEBUG
	_CrtDumpMemoryLeaks();
#endif
}
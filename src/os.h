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

#ifdef _WIN32
#pragma warning(disable : 4018)	// >=
#pragma warning(disable : 4244)	//float -> int
#endif

#define M_PI 3.14159265358979323846

#define OK 0

typedef int BOOL;
typedef unsigned char UCHAR;
typedef unsigned int UINT;
typedef int UNI;
typedef unsigned short USHORT;
typedef unsigned long long UBIG;
typedef long long BIG;

#ifdef _WIN32
#define _UNI32(STR) U##STR
#elif __linux__
#define _UNI32(STR) L##STR
#endif

#define TRUE 1
#define FALSE 0

#define STD_BUILD 2	//! ...
#define STD_TITLE "SkyAlt"

#define STD_LICENSE_VER 1

#define STD_WEBSITE _UNI32("https://www.skyalt.com")

#define MAX_DATA_FPS 3
#define MAX_RENDER_FPS 30
#define MAX_EXE_FPS 60
#define MAX_WORKSPACE_UPDATE 0.1	//every 10sec

#define NET_USER_AGENT "SkyAlt/0.2"

#define TEXT_TAB_SPACES 2

#ifdef _DEBUG
#define MAX_COMPUTING_WAIT 1000
#else
#define MAX_COMPUTING_WAIT 400
#endif

#ifdef _WIN32
#define STD_OS "Windows"
#elif __linux__
#define STD_OS "Linux"
#elif __APPLE__
#define STD_OS "MacOS"
#endif

#ifdef _WIN32
#define STD_FILE_EXE ".exe"
#elif __linux__
#define STD_FILE_EXE ""
#elif __APPLE__
#define STD_FILE_EXE ""
#endif

#ifdef _WIN32
#define STD_DROP_FILE_HEADER _UNI32("file:///")
#define STD_DROP_FILE_HEADER_CHAR "file:///"
#elif __linux__
#define STD_DROP_FILE_HEADER _UNI32("file://")
#define STD_DROP_FILE_HEADER_CHAR "file://"
#elif __APPLE__

#endif

#define STD_UPDATE_SERVER "https://www.skyalt.com/updates"
#define STD_INI_PATH "SkyAlt.ini"
#define STD_INI_MAP_PATH "map.ini"
#define STD_INI_MAP_URL "https://raw.githubusercontent.com/MilanSuk/SkyAlt_web/master/map.ini"	//https://www.skyalt.com/map.ini doesn't work! Some redirect.

UCHAR* Os_getUpdatePublicKey(void);

#ifdef _WIN32
extern int sscanf(char const* const _Buffer, char const* const _Format, ...);
extern int printf(char const* const _Format, ...);
extern int sprintf(char* const _Buffer, char const* const _Format, ...);
extern int snprintf(char* const _Buffer, UBIG const _BufferCount, char const* const _Format, ...);
extern int fprintf(void* const _Stream, char const* const _Format, ...);
#elif __linux__
#include <stdio.h>
//extern int sscanf(char const* const _Buffer, char const* const _Format, ...);
//extern int printf(char const* const _Format, ...);
//extern int sprintf(char* const _Buffer, char const* const _Format, ...);
//extern int snprintf(char* restrict buffer, size_t bufsz, const char* restrict format, ...);
//extern int fprintf(void* restrict stream, const char* restrict format, ...);
#endif

double Os_sqrt(double x);
double Os_pow(double x, double y);
void Os_gcvt(double value, int digits, char* str);
double Os_atof(const char* str);
double Os_cos(double x);
double Os_acos(double x);
double Os_sin(double x);
double Os_asin(double x);
double Os_tan(double x);
double Os_atan(double x);
double Os_log(double x);
double Os_exp(double x);
int Os_isalnum(int c);

void* Os_calloc(UBIG count, UBIG item_size);
void* Os_malloc(UBIG size);
void* Os_realloc(void* ptr, UBIG size);
void Os_free(void* ptr, UBIG size);
void Os_memset(void* ptr, UBIG size);
void Os_memsetEx(void* ptr, int value, UBIG size);
void* Os_memcpy(void* dst, const void* src, UBIG size);
void* Os_memmove(void* dst, void* src, UBIG size);
int Os_memcmp(void* a, void* b, UBIG size);

void Os_qsort(void* base, UBIG num, int item_size, int (cmpfunc)(const void* context, const void* a, const void* b), void* context);

void Os_showConsole(BOOL show);

void Os_showMemleaks(void);

#define OsFile_R "rb"
#define OsFile_W "wb"
#define OsFile_RW "rb+"
#define OsFile_A "ab"

typedef struct OsCryptoSha2_s OsCryptoSha2;

typedef struct OsFile_s
{
	void* m_file;
}OsFile;

OsFile OsFile_initEmpty(void);
BOOL OsFile_init(OsFile* self, const char* path, const char* mode);
BOOL OsFile_initTemp(OsFile* self);
void OsFile_free(OsFile* self);

void OsFile_flush(OsFile* self);
BOOL OsFile_is(const OsFile* self);

UBIG OsFile_read(OsFile* self, void* ptr, UBIG NUM);
UBIG OsFile_write(OsFile* self, const void* ptr, UBIG NUM);
UBIG OsFile_getSeekPos(OsFile* self);
void OsFile_seekStart(OsFile* self);
UBIG OsFile_seekEnd(OsFile* self);
BOOL OsFile_seekAbs(OsFile* self, UBIG pos);
BOOL OsFile_seekRel(OsFile* self, BIG pos);
BOOL OsFile_setEndOfFile(OsFile* self);

UCHAR* OsFile_initRead(const char* path, UBIG* out_size, UBIG alloc_extra_bytes);
BOOL OsFile_initWrite(const char* path, void* data, const UBIG bytes);
BOOL OsFile_initHash(const char* path, OsCryptoSha2* out);

char* OsFile_readLine(OsFile* self);
void OsFile_writeUNIch(OsFile* self, const UNI l);
void OsFile_writeUNI(OsFile* self, const UNI* str);
void OsFile_writeNumber(OsFile* self, double value);

BOOL OsFile_existFile(const char* path);
BOOL OsFile_existFolder(const char* path);
BOOL OsFile_exist(const char* path);
BIG OsFile_bytes(const char* path);
BIG OsFileDir_getFileList(const char* path, BOOL file_names, BOOL subdir_names, BOOL complete_path, char*** out);
UBIG OsFileDir_getFolderBytes(const char* path, BOOL subdirs);

BOOL OsFileDir_renameFile(const char* oldname, const char* newname);
BOOL OsFileDir_renameDir(const char* oldname, const char* newname);

BOOL OsFileDir_removeFile(const char* path);
BOOL OsFileDir_removeDir(const char* path);
BOOL OsFileDir_removeDirContent(const char* path);

BOOL OsFileDir_makeDir(const char* path);
char* OsFileDir_currentDir(void);
char* OsFileDir_currentProgramDir(void);

void OsFile_unlink(const char* path);

void OsFile_getExtension(const char* path, UCHAR ext[8]);
void OsFile_getExtensionUNI(const char* path, UNI ext[8]);
void OsFile_getParts(const char* path, char** out_folder, char** out_name);

BOOL OsFile_cmp(const char* pathA, const char* pathB, BIG maxErrors);

void OsFile_testSeek(void);
void OsFile_testOpen(void);

double Os_time(void);
int Os_timeZone(void);
double Os_timeUTC(void);
double Os_printTime(const char* text, double startTime);

typedef enum
{
	OsDate_EU = 0,
	OsDate_US,
	OsDate_ISO,
	OsDate_TEXT,
}OsDateTYPE;
typedef enum
{
	OsDate_NONE = 0,	//hide
	OsDate_HM,
	OsDate_HMS,
}OsDateTimeTYPE;
typedef struct OsDate_s	//8Bytes!!!
{
	short m_year;	//from year 1900
	char m_month;	//[0,11]
	char m_day;		//[0,30]
	char m_hour;	//[0,23]
	char m_min;		//[0,59]
	char m_sec;		//[0,59]
}OsDate;
OsDate OsDate_initEmpty(void);
OsDate OsDate_initDay(char day, char month, short year);
OsDate OsDate_initActual(void);

BOOL OsDate_is(const OsDate* self);
BOOL OsDate_cmpOnlyDate(const OsDate* a, const OsDate* b);
BOOL OsDate_cmpOnlyTime(const OsDate* a, const OsDate* b);

BOOL OsDate_cmp(const OsDate* a, const OsDate* b);

UBIG OsDate_getDaysInYearEx(const short y);
UBIG OsDate_getDaysInYear(const OsDate* self);
UBIG OsDate_getDaysInMonthEx(const short m, const short y);

UBIG OsDate_getDaysInMonth(const OsDate* self);

UBIG OsDate_getDaysFromYearZero(const OsDate* self);

UBIG OsDate_getSecondsFromYearZero(const OsDate* self);

BIG OsDate_differenceSeconds(const OsDate* curr, const OsDate* past);

int OsDate_getWeekDay(const OsDate* self);

void OsDate_addMonth(OsDate* self);
void OsDate_subMonth(OsDate* self);
void OsDate_addDay(OsDate* self);
void OsDate_subDay(OsDate* self);

void OsDate_getStringDateEU(const OsDate* self, char out[64]);
void OsDate_getStringDateUS(const OsDate* self, char out[64]);
void OsDate_getStringDateISO(const OsDate* self, char out[64]);
void OsDate_getStringDateText(const OsDate* self, char out[64], const char* month);
void OsDate_getStringTime2(const OsDate* self, char out[64]);
void OsDate_getStringTime3(const OsDate* self, char out[64]);

OsDate OsDate_initFromString(const UNI* str, OsDateTYPE type);

double OsDate_asNumber(const OsDate* self);
OsDate OsDate_initFromNumber(double number);
double OsDate_roundNumber(double number, OsDateTimeTYPE timeFormat);

char* OsDate_getStringDateTime(const OsDate* self, OsDateTYPE formatDate, OsDateTimeTYPE formatTime, const char* month, char time[64]);
UNI* OsDate_getStringDateTimeUNI(const OsDate* self, OsDateTYPE formatDate, OsDateTimeTYPE formatTime, const char* month);

typedef struct LimitTime_s
{
	double start;
	double dt;
}LimitTime;
void LimitTime_reset(LimitTime* self);
LimitTime LimitTime_initDt(double dt_sec);
LimitTime LimitTime_initFps(double fps);
double LimitTime_dt(const LimitTime* self);
void LimitTime_start(LimitTime* self);
BOOL LimitTime_isTimeout(LimitTime* self);

typedef struct LimitFps_s
{
	double m_time_start;
	double m_frames_dt[32];		//loop
	int m_act_frame;
	BOOL m_full_round;
}LimitFps;
LimitFps LimitFps_init(void);
void LimitFps_start(LimitFps* self);
double LimitFps_finish(LimitFps* self, const int FPS);
double LimitFps_getAvgDT(LimitFps* self);
double LimitFps_getAvgFPS(LimitFps* self);

typedef struct OsTimeStat_s OsTimeStat;
OsTimeStat OsTimeStat_init(void);
void OsTimeStat_addValue(OsTimeStat* self, double add);
double OsTimeStat_checkPointAndGet(OsTimeStat* self);
double OsTimeStat_getAll(const OsTimeStat* self);

typedef struct Image4_s Image4;
const char* OsCrypto_initGlobal(void);
void OsCrypto_freeGlobal(void);
BOOL OsCrypto_random(const int bytes, void* out);
double OsCrypto_randomDouble(void);
double OsCrypto_random01(void);

typedef struct OsCryptoSha2_s
{
	UCHAR m_key[32]; //256bits
} OsCryptoSha2;
OsCryptoSha2 OsCryptoSha2_init(void);
BOOL OsCryptoSha2_exe(const void* input, int input_size, OsCryptoSha2* hash);
void OsCryptoSha2_free(OsCryptoSha2* self);
BOOL OsCryptoSha2_cmp(const OsCryptoSha2* a, const OsCryptoSha2* b);

typedef struct OsCryptoKey32_s
{
	unsigned char m_bytes[32];	//256bits
} OsCryptoKey32;

typedef struct OsCryptoKey_s
{
	unsigned char m_bytes[32];	//256bits
	unsigned char m_iv[16];
} OsCryptoKey;
BOOL OsCryptoKey_cmp(const OsCryptoKey* a, const OsCryptoKey* b);
OsCryptoKey OsCryptoKey_initFromPassword(const UNI* password);
OsCryptoKey OsCryptoKey_initRandom(void);
OsCryptoKey OsCryptoKey_initZero(void);
void OsCryptoKey_free(OsCryptoKey* self);
void OsCryptoKey_exportString(OsCryptoKey* self, char out[65]);
void OsCryptoKey_aesEncrypt(OsCryptoKey* self, unsigned char* plain_text, unsigned char* cipher_text, int size);
void OsCryptoKey_aesDecrypt(OsCryptoKey* self, unsigned char* cipher_text, unsigned char* plain_text, int size);
void OsCryptoKey_aesEncryptIV(OsCryptoKey* self, unsigned char* plain_text, unsigned char* cipher_text, int size);
void OsCryptoKey_aesDecryptIV(OsCryptoKey* self, unsigned char* cipher_text, unsigned char* plain_text, int size);
void OsCryptoKey_aesEncryptDirect(const UCHAR* key, const UBIG block, unsigned char* plain_text, unsigned char* cipher_text, int size);
void OsCryptoKey_aesDecryptDirect(const UCHAR* key, const UBIG block, unsigned char* cipher_text, unsigned char* plain_text, int size);
void OsCryptoKey_moveIv(OsCryptoKey* self, unsigned char move[16]);
int OsCryptoKey_roundSizeBy16(int s);

//Public & Private keys can be generated from this one 32B key
typedef struct OsCryptoECDSAKey_s
{
	UCHAR m_bytes[32 * 3];
} OsCryptoECDSAKey;

typedef struct OsCryptoECDSAPublic_s
{
	UCHAR m_bytes[65];
} OsCryptoECDSAPublic;

typedef struct OsCryptoECDSAPrivate_s
{
	UCHAR m_bytes[279];
} OsCryptoECDSAPrivate;

typedef struct OsCryptoECDSASign_s
{
	UCHAR m_bytes[72];
} OsCryptoECDSASign;

BOOL OsCryptoECDSAKey_cmp(const OsCryptoECDSAKey* a, const OsCryptoECDSAKey* b);
BOOL OsCryptoECDSAPublic_cmp(const OsCryptoECDSAPublic* a, const OsCryptoECDSAPublic* b);
BOOL COsCryptoECDSAPrivate_cmp(const OsCryptoECDSAPrivate* a, const OsCryptoECDSAPrivate* b);
BOOL OsCryptoECDSASign_cmp(const OsCryptoECDSASign* a, const OsCryptoECDSASign* b);
void OsCryptoECDSAKey_free(OsCryptoECDSAKey* self);
void OsCryptoECDSAPublic_free(OsCryptoECDSAPublic* self);
void OsCryptoECDSAPrivate_free(OsCryptoECDSAPrivate* self);
void OsCryptoECDSASign_free(OsCryptoECDSASign* self);

typedef struct OsCryptoECDSA_s
{
	void* key;	//EC_KEY* key;
} OsCryptoECDSA;
BOOL OsCryptoECDSA_initRandom(OsCryptoECDSA* self);
BOOL OsCryptoECDSA_initFromKey(OsCryptoECDSA* self, const OsCryptoECDSAKey* key);
BOOL OsCryptoECDSA_initFromPublic(OsCryptoECDSA* self, const OsCryptoECDSAPublic* pub);
void OsCryptoECDSA_free(OsCryptoECDSA* self);
BOOL OsCryptoECDSA_exportKey(OsCryptoECDSA* self, OsCryptoECDSAKey* key);
BOOL OsCryptoECDSA_exportPublic(OsCryptoECDSA* self, OsCryptoECDSAPublic* pub);
BOOL OsCryptoECDSA_exportPrivate(OsCryptoECDSA* self, OsCryptoECDSAPrivate* pri);

int OsCryptoECDSA_sign(OsCryptoECDSA* self, const UCHAR* message, int message_size, OsCryptoECDSASign* sign);
BOOL OsCryptoECDSA_verify(OsCryptoECDSA* self, const UCHAR* message, int message_size, const OsCryptoECDSASign* sign, int sign_size);
BOOL OsCryptoECDSA_getSecret(const OsCryptoECDSA* self, const OsCryptoECDSA* selfPub, OsCryptoKey* secret);

void OsCryptoECDSA_test(void);
void OsCryptoSha2_test(void);

#ifdef _WIN32
typedef unsigned long(__stdcall* OsThread_loopFUNC)(void*);
#define THREAD_FUNC(NAME, PARAM) unsigned long NAME(void* PARAM)
#else
typedef void* (*OsThread_loopFUNC) (void*);
#define THREAD_FUNC(NAME, PARAM) void* NAME(void* PARAM)
#endif

typedef struct OsThread_s
{
#ifdef _WIN32
	void* m_thread;
#else
	unsigned long int m_thread;
#endif
	volatile BOOL m_running;
	OsThread_loopFUNC func;
	void* func_param;
} OsThread;

OsThread OsThread_initEmpty(void);
OsThread OsThread_initRoot(void);
const char* OsThread_init(OsThread* self, void* param, OsThread_loopFUNC func);
void OsThread_setGameOver(OsThread* self);
BOOL OsThread_waitUntilIsFinished(OsThread* self);
const char* OsThread_free(OsThread* self, BOOL gameOver);
BOOL OsThread_isRunning(const OsThread* self);
void OsThread_sleep(int ms);
UINT OsThread_getNumCPUCores(void);
UBIG OsThread_getID(void);
UBIG OsThread_isID(const OsThread* self, UBIG id);
UBIG OsThread_getMaxRam(void);
BOOL OsThread_tick(OsThread* self);

typedef struct OsLock_s
{
	void* m_mutex;
} OsLock;

BOOL OsLock_init(OsLock* self);
void OsLock_free(OsLock* self);
void OsLock_lock(OsLock* self);
void OsLock_unlock(OsLock* self);
BOOL OsLock_tryLock(OsLock* self);

typedef struct OsLockEvent_s
{
#ifdef _WIN32
	void* m_event;
#else
	UCHAR m_mutex[64];	//pthread_mutex_t
	UCHAR m_cond[64];	//pthread_cond_t
	volatile BOOL m_signalled;
#endif
}OsLockEvent;
BOOL OsLockEvent_init(OsLockEvent* self);
void OsLockEvent_free(OsLockEvent* self);
BOOL OsLockEvent_wait(OsLockEvent* self, int timeout_ms);
void OsLockEvent_trigger(OsLockEvent* self);

BOOL Os_getMAC(UCHAR mac[6]);
UBIG Os_getUID(void);

void OsWeb_openWebBrowser(const char* webAddress);
void OsWeb_openEmail(const char* dstEmail, const char* subject);

const char* OsNet_init();
void OsNet_free();

typedef struct OsNetSocket_s
{
	UBIG m_socket;
}OsNetSocket;
OsNetSocket OsNetSocket_initClosed(void);
void OsNetSocket_free(OsNetSocket* self);
void OsNetSocket_stop(OsNetSocket* self);
BOOL OsNetSocket_initUDP(OsNetSocket* self, USHORT port, BOOL blocking);
BOOL OsNetSocket_is(OsNetSocket* self);
//int OsNetSocket_recvDatagram(OsNetSocket* self, UCHAR* data, const int max_size, struct sockaddr_storage* clientInfo, UINT addrlen);
//int OsNetSocket_sendDatagramTo(OsNetSocket* self, const UCHAR* data, const int size, struct sockaddr_storage* clientInfo, UINT addrlen);

typedef struct OsNetLimit_s
{
	double reset_time;
	int max_bytes;
	int act_bytes;
} OsNetLimit;

OsNetLimit OsNetLimit_initBytes(const int max_bytes);
OsNetLimit OsNetLimit_initMegaBits(const float max_megabits);
OsNetLimit OsNetLimit_initInactive();
void OsNetLimit_free(OsNetLimit* self);
void OsNetLimit_reset(OsNetLimit* self);
BOOL OsNetLimit_isActive(OsNetLimit* self);
void OsNetLimit_addBytes(OsNetLimit* self, const int bytes);
void OsNetLimit_limit(OsNetLimit* self);

typedef struct OsHTTPS_s
{
	char* server_addr;
	void* m_curl;
} OsHTTPS;
OsHTTPS OsHTTPS_initEmpty(void);
void OsHTTPS_initGlobal(void);
void OsHTTPS_freeGlobal(void);
BOOL OsHTTPS_init(OsHTTPS* self, const char* server_adrr);
void OsHTTPS_free(OsHTTPS* self);
BIG OsHTTPS_get(OsHTTPS* self, const char* sub_addr, char** output);
BIG OsHTTPS_getSize(OsHTTPS* self, const char* sub_addr);
BIG OsHTTPS_downloadWithStatus(const char* path, volatile float* out_done, volatile BOOL* running, char** output);

typedef struct Vec2i_s Vec2i;
typedef struct Quad2i_s Quad2i;

void OsScreen_getMonitorResolution(Vec2i* out);
int OsScreen_getDPI(void);
void OsScreen_getMonitorCoord(Quad2i* out);
void OsScreen_getDefaultCoord(Quad2i* out);
float OsScreen_inchToMM(float inch);
void OsScreen_getMonitorMM(float diagonal_MM, Vec2i* out);

typedef enum
{
	Win_TOUCH_NONE,

	Win_TOUCH_DOWN_S, Win_TOUCH_DOWN_E,
	Win_TOUCH_FORCE_DOWN_S, Win_TOUCH_FORCE_DOWN_E, //Mouse(Middle | right click), Tablet(ForceTouch | Hold for 2sec)

	Win_TOUCH_WHEEL, //Tablet(two fingers move up or down)
} Win_TOUCH;

typedef enum
{
	Win_CURSOR_DEF = 0,
	Win_CURSOR_IBEAM,
	Win_CURSOR_WAIT,
	Win_CURSOR_HAND,
	Win_CURSOR_FLEUR,
	Win_CURSOR_COL_RESIZE,
	Win_CURSOR_ROW_RESIZE,
	Win_CURSOR_MOVE,
} Win_CURSOR;

#define Win_EXTRAKEY_NONE 0
#define Win_EXTRAKEY_CTRL (1 << 0)
#define Win_EXTRAKEY_SHIFT (1 << 1)
#define Win_EXTRAKEY_DELETE (1 << 2)
#define Win_EXTRAKEY_HOME (1 << 3)
#define Win_EXTRAKEY_END (1 << 4)
#define Win_EXTRAKEY_BACKSPACE (1 << 5)
#define Win_EXTRAKEY_TAB (1 << 6)
#define Win_EXTRAKEY_ENTER (1 << 7)
#define Win_EXTRAKEY_LEFT (1 << 8)
#define Win_EXTRAKEY_RIGHT (1 << 9)
#define Win_EXTRAKEY_UP (1 << 10)
#define Win_EXTRAKEY_DOWN (1 << 11)
#define Win_EXTRAKEY_INSERT (1 << 12)
#define Win_EXTRAKEY_ESCAPE (1 << 13)
#define Win_EXTRAKEY_PAGE_U (1 << 14)
#define Win_EXTRAKEY_PAGE_D (1 << 15)

#define Win_EXTRAKEY_ZOOM_0 (1 << 16)
#define Win_EXTRAKEY_ZOOM_A (1 << 17)
#define Win_EXTRAKEY_ZOOM_S (1 << 18)

#define Win_EXTRAKEY_SELECT_ALL (1 << 19)
#define Win_EXTRAKEY_COPY (1 << 20)
#define Win_EXTRAKEY_PASTE (1 << 21)
#define Win_EXTRAKEY_CUT (1 << 22)
#define Win_EXTRAKEY_DUPLICATE (1 << 23)

#define Win_EXTRAKEY_THEME (1 << 24)

#define Win_EXTRAKEY_NEW (1 << 25)
#define Win_EXTRAKEY_SAVE (1 << 26)
#define Win_EXTRAKEY_LOG (1 << 27)

#define Win_EXTRAKEY_FULLSCREEN (1 << 28)

#define Win_EXTRAKEY_BACK (1 << 29)
#define Win_EXTRAKEY_FORWARD (1 << 30)

#define Win_EXTRAKEY_BYPASS (1 << 31)
#define Win_EXTRAKEY_COMMENT (1ULL << 32)

//#define Win_EXTRAKEY_ALT (1ULL << 33)

#define Win_EXTRAKEY_LPANEL (1ULL << 34)

#define Win_EXTRAKEY_PRINTSCREEN (1ULL << 35)

#define Win_EXTRAKEY_SEARCH (1ULL << 36)
#define Win_EXTRAKEY_ADD_RECORD (1ULL << 37)

#define Win_EXTRAKEY_SELECT_ROW (1ULL << 38)
#define Win_EXTRAKEY_SELECT_COLUMN (1ULL << 39)

typedef struct Win_s Win;
typedef struct Quad2i_s Quad2i;
typedef struct Vec2i_s Vec2i;

BOOL Win_isFullscreen(Win* self);
void Win_setFullscreen(Win* self, BOOL fullscreen);
BOOL Win_isResize(Win* self);
void Win_pleaseResize(Win* self);
void Win_resetResize(Win* self);
void Win_getScreenRectEx(const Win* self, Quad2i* rect);
void Win_updateCursor(Win* self, Win_CURSOR cursor);
void Win_resetCursor(Win* self);
void Win_updateCursorReal(Win* self);
void Win_init(void);

Win* Win_new(Quad2i* abs_coord, BOOL(*tickFn)(void*, Quad2i* redrawRect), void* tickSelf);
void Win_delete(Win* self);
void Win_start(Win* self);
void Win_getWindowCoord(Win* self, Quad2i* out);
void Win_setTitle(Win* self, const char* title);

void Win_setFullscreen(Win* self, BOOL fullscreen);

UNI* Win_clipboard_getText(void);
void Win_clipboard_setText(const UNI* strUNI);

Image4 Win_getImage(Win* self);

UNI* Win_showFilePicker(Win* self, BOOL mode_open, BOOL mode_folder, BOOL mode_multiple, const UNI* langCancelUNI, const UNI* actionUNI, const UNI* exts);

void Win_savePrintscreen(Win* self);

typedef struct OsFont_s OsFont;

UINT OsWinIO_getTouch_action(void);
int OsWinIO_getTouch_wheel(void);
int OsWinIO_getDPI(void);
UNI OsWinIO_getKeyID(void);
UBIG OsWinIO_getKeyExtra(void);
OsFont* OsWinIO_getFontDefault(void);
BOOL OsWinIO_isTouch(void);
BOOL OsWinIO_isKey(void);
BOOL OsWinIO_isExit(void);
int OsWinIO_getTouchNum(void);
int OsWinIO_getTouchWheel(void);
Vec2i OsWinIO_getTouchPos(void);
Vec2i OsWinIO_getCursorRenderItemPos(void);
UNI* OsWinIO_getCursorRenderItemCache(void);
void* OsWinIO_getCursorRenderItem(void);
int OsWinIO_getCursorRenderItemDrawX(void);

void OsWinIO_setCursorRenderItemDrawX(int x);
void OsWinIO_setCursorRenderItemPosX(int x);
void OsWinIO_setCursorRenderItemPosY(int y);
void OsWinIO_setCursorRenderItemCache(UNI* str);
void OsWinIO_resetKeyID(void);
void OsWinIO_resetKeyEXTRA(void);
void OsWinIO_setTouchNum(int touch_num);
void OsWinIO_setTouch_action(UINT touch_action);
void OsWinIO_setDPI(UINT dpi);
UINT OsWinIO_cellSize(void);
UINT OsWinIO_lineSpace(void);
//int OsWinIO_rounded(void);
//int OsWinIO_shadows(void);
//float OsWinIO_shadowsAlpha(void);
BOOL OsWinIO_new(void);
void OsWinIO_delete(void);
void OsWinIO_setDrop(UNI* str, Vec2i* pos);
void OsWinIO_setDropFromInside(const UNI* str, Vec2i* pos);
void OsWinIO_resetDrop(void);
BOOL OsWinIO_isDrop(Quad2i* rect);
UNI* OsWinIO_getDropFile(int i);

void OsWinIO_setDrag(void* item);
void OsWinIO_resetDrag(void);
BOOL OsWinIO_isDragActive(void);

void OsWinIO_setActiveRenderItem(void* item);
void OsWinIO_resetActiveRenderItem(void);
BOOL OsWinIO_isActiveRenderItem(const void* item);
BOOL OsWinIO_existActiveRenderItem(void);
BOOL OsWinIO_canActiveRenderItem(const void* item);

BOOL OsWinIO_isCursorEmpty(void);
BOOL OsWinIO_isCursorGuiItem(void* item);
BOOL OsWinIO_isCursorGuiItemInTime(void* item);
void OsWinIO_updateCursorTimeout(void);
void OsWinIO_tryRemoveCursorGuiItem(void* item);
BOOL OsWinIO_isStartTouch(void);
BOOL OsWinIO_setCursorGuiItem(void* item);
void OsWinIO_setCursorText(const UNI* text);
void OsWinIO_resetCursorGuiItem(void);
double OsWinIO_getEditboxAnim(void* item);

void OsWinIO_pleaseExit(void);
void OsWinIO_pleaseTouch(void);
void OsWinIO_pleaseKey(void);
void OsWinIO_setTouch(Vec2i* pos, UINT action, BOOL move);
void OsWinIO_setTouchWheel(Vec2i* pos, int wheel);
void OsWinIO_resetTouch(void);
void OsWinIO_resetNumTouch(void);
void OsWinIO_resetKey(void);
void OsWinIO_tick(void);

BOOL OsWinIO_isAnotherClickOutOfTime(void);
BOOL OsWinIO_isRebuildAfterTouchNeeded(void);

typedef enum
{
	OsZlib_NO_COMPRESS = 0,
	OsZlib_SPEED_COMPRESS,
	OsZlib_BEST_COMPRESS,
}OsZlib_TYPE;

UBIG OsZlib_maxCompressSize(UBIG plain_n);
BIG OsZlib_compress(const UCHAR* plain, UCHAR* compress, UBIG plain_n, UBIG compress_n, const OsZlib_TYPE compressType);
BIG OsZlib_uncompress(const UCHAR* compress, UCHAR* plain, UBIG compress_n, UBIG plain_n);

void OsZlib_test(void);
void OsZlib_test2(void);
void OsZlib_test3(void);

typedef struct OsFontLetter_s
{
	UCHAR* img;
	int img_w;
	int img_h;

	int move_x;
	int move_y;

	int m_bytes_in_x;
	int m_len;
}OsFontLetter;

typedef struct OsFont_s OsFont;

void OsFont_clear(OsFont* self);
const char* OsFont_free(OsFont* self);

const char* OsFont_initFile(OsFont* self, const UNI* name, const char* path);
const char* OsFont_initMemory(OsFont* self, const UNI* name, const UCHAR* memory, int memory_bytes);

BOOL OsFont_is(const OsFont* self, const UNI* name);
OsFontLetter OsFont_get(OsFont* self, const UNI CH, const int Hpx);
Vec2i OsFont_getTextSize(OsFont* self, const UNI* text, const int Hpx, const int betweenLinePx, int* extra_down);
int OsFont_getTextSizeX(OsFont* self, const UNI* text, const int Hpx);
int OsFont_getCharPixelPos(OsFont* self, const int Hpx, const UNI* text, int cur_pos);
int OsFont_getCursorPos(OsFont* self, const int Hpx, const UNI* text, int pixel_x);

#define OsAudio_RECOMMAND_BLOCK_SIZE 4*1024 //not under 1ms!

typedef struct OsAudioSpeaker_s OsAudioSpeaker;

OsAudioSpeaker* OsAudioSpeaker_new(int buf_size, int samplesPerSec, int bitsPerSample, int channels);
void OsAudioSpeaker_delete(OsAudioSpeaker* a);

int OsAudioSpeaker_write(OsAudioSpeaker* a, short* data, int data_size, float volume);
void OsAudioSpeaker_pause(OsAudioSpeaker* a, BOOL pause);

typedef BIG OSMediaCallback_read(void* self, UCHAR* buff, int buff_size);	//returns numBytes
typedef BIG OSMediaCallback_seek(void* self, BIG offset);					//returns pos(or -1)

typedef struct OSMedia_s OSMedia;
void OSMedia_initGlobal(void);
void OSMedia_freeGlobal(void);

BOOL OSMedia_isAudio(const OSMedia* self);
BOOL OSMedia_isVideo(const OSMedia* self);
BOOL OSMedia_isSubtitles(const OSMedia* self);

OSMedia* OSMedia_newOpen(OSMediaCallback_read* funcRead, OSMediaCallback_seek* funcSeek, void* func_data, const char* fileName);	//note: fileName is the name, not path
UCHAR* OSMedia_newSave(const char* ext, Vec2i size, UCHAR* buff, UBIG bytes, UBIG* out_bytes);
const Vec2i* OSMedia_getOrigSize(const OSMedia* self);
void OSMedia_loadVideo(OSMedia* self, Image4* out);
void OSMedia_delete(OSMedia* self);

typedef struct Image1_s Image1;
void OSMedia_scale1(Image1* dst, const Image1* src);
void OSMedia_scale4(Image4* dst, const Image4* src);

typedef struct OsODBC_s OsODBC;
typedef struct OsODBCQuery_s OsODBCQuery;

typedef enum
{
	OsODBC_UNKNOWN,
	OsODBC_NUMBER,
	OsODBC_STRING,
	OsODBC_DATE,
}OsODBCType;

BOOL OsODBC_initGlobal(void);
void OsODBC_freeGlobal(void);
BIG OsODBC_getDriversList(UNI*** list);
BIG OsODBC_getDataSourcesList(UNI*** list);

OsODBC* OsODBC_new(const char* connectionName, const char* server, USHORT port, const char* userName, const char* password, const char* driver);
void OsODBC_delete(OsODBC* self);
BIG OsODBC_getTablesList(OsODBC* self, UNI*** list);
BIG OsODBC_getColumnsList(OsODBC* self, const UNI* tableName, UNI*** out_names, BIG** out_types);
BIG OsODBC_getPrimaryColumnList(OsODBC* self, const UNI* tableName, UNI*** out_names);
UNI* OsODBC_getPrimaryColumnName(OsODBC* self, const UNI* tableName);
BIG OsODBC_getForeignColumnList(OsODBC* self, const UNI* tableName, UNI*** out_srcColumnNames, UNI*** out_dstTableNames);
OsODBCQuery* OsODBC_createQuery(OsODBC* self, char* statement);

void OsODBCQuery_delete(OsODBCQuery* self);
BOOL OsODBCQuery_addVarDouble(OsODBCQuery* self, int index, double* value);
BOOL OsODBCQuery_addVarString(OsODBCQuery* self, int index, char* str, const int max_size);
BOOL OsODBCQuery_addColumnDouble(OsODBCQuery* self, int index, double* value);
BOOL OsODBCQuery_addColumnString(OsODBCQuery* self, int index, char* str, BIG max_len);
BOOL OsODBCQuery_execute(OsODBCQuery* self);
BOOL OsODBCQuery_fetch(OsODBCQuery* self);
UBIG OsODBCQuery_count(OsODBCQuery* self);

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

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#endif

#include <stdio.h>
#include <stdlib.h>

#ifdef _DEBUG
#include <crtdbg.h>
#endif

#include <string.h>
#include <math.h>
#include <float.h>

#include <zlib.h>
#include <ctype.h>
#include <locale.h>

#include <openssl/conf.h>
#include <openssl/rand.h>
#include <openssl/sha.h>
#include <openssl/aes.h>
#include <openssl/ecdsa.h>
#include <openssl/ecdh.h>
#include <openssl/obj_mac.h>

#include <ft2build.h>
#include <freetype.h>
#include <ftglyph.h>

#ifdef _WIN32
#include <direct.h>
#include <sys/stat.h>
#include <winsock2.h>
#include <windows.h>
#include <wininet.h>
#include <iphlpapi.h>
#include <ws2tcpip.h>
#include <mmsystem.h>
#include <limits.h>
#define SSIZE_MAX SIZE_MAX
#pragma comment(lib, "iphlpapi")
#pragma comment(lib,"WS2_32")
#pragma comment(lib, "Winmm")
#elif __linux__
#include <sys/time.h>
#include <sys/file.h>
#include <unistd.h>
#include <pthread.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/sysinfo.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <alsa/asoundlib.h>
#include <gtk/gtk.h>
#elif __APPLE__
//...
#endif

#include <libavformat/avformat.h>
#include <libswscale/swscale.h>

//#include <poppler/glib/poppler.h>	//pdf: libpoppler-glib-dev

#include <curl/curl.h>

//ODBC
#ifdef _WIN32
#include <sql.h>
#include <sqlext.h>
#elif __linux__
	//...
#elif __APPLE__
	//...
#endif

#ifdef _WIN32
#define FONT_DEFAULT_1 "C:\\Windows\\Fonts"
#define FONT_DEFAULT_2 "%WINDIR%\\Fonts"	//not working, try: char * pSysPath = getenv("WINDIR");
#elif __linux__
#define FONT_DEFAULT_1 "/usr/share/fonts"
#define FONT_DEFAULT_2 "/usr/local/share/fonts"
#elif __APPLE__
#define FONT_DEFAULT_1 "/System/Library/Fonts"
#define FONT_DEFAULT_2 "/Library/Fonts"
#endif

#ifdef __linux__
#define SOCKET_ERROR -1
#define SOCKET int
#define SOCKADDR_IN struct sockaddr_in
#endif

#ifdef _WIN32
	//maybe will not work ...
#define STD_DROP_FILE_HEADER _UNI32("file:///")
#define STD_DROP_FILE_HEADER_CHAR "file:///"
#elif __linux__
#define STD_DROP_FILE_HEADER _UNI32("file://")
#define STD_DROP_FILE_HEADER_CHAR "file://"
#elif __APPLE__

#endif

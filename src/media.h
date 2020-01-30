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

typedef struct MediaLibrary_s MediaLibrary;

BOOL MediaLibrary_new(void);
void MediaLibrary_delete(void);
BOOL MediaLibrary_add(FileRow fileId, Vec2i img_rectSize, BOOL* out_image, BOOL* out_audio, BOOL* out_text);
BOOL MediaLibrary_addImageBuffer(const char* url, const char* ext, UCHAR* buff, UBIG bytes, Vec2i rectSize);

BOOL MediaLibrary_hasImageBuffer(const char* url, const char* ext, Vec2i imgSize);

BOOL MediaLibrary_imageUpdate(FileRow fileId, Vec2i imgSize);
BOOL MediaLibrary_imageBufferUpdate(const char* url, const char* ext, Vec2i imgSize);

void MediaLibrary_imageDrawExt(FileRow fileId, Image4* img, Quad2i coord, int textH, Rgba cd);
void MediaLibrary_imageDrawInfo(FileRow fileId, Image4* img, Quad2i coord, int textH, Rgba cd);
BOOL MediaLibrary_imageDraw(FileRow fileId, Image4* img, Quad2i coord);
BOOL MediaLibrary_imageBufferDraw(const char* url, const char* ext, Image4* img, Quad2i coord);

void MediaLibrary_setVolume(float volume);
BOOL MediaLibrary_isPlayingSomething(void);
void MediaLibrary_stopAllPlay(void);

void MediaLibrary_play(FileRow fileId, BOOL play);
BOOL MediaLibrary_isPlay(FileRow fileId);
void MediaLibrary_setSeek(FileRow fileId, float t);
float MediaLibrary_getSeek(FileRow fileId);

void MediaNetwork_delete(void);
BOOL MediaNetwork_new(BOOL online);
BOOL MediaNetwork_is(void);
void MediaNetwork_run(BOOL run);
void MediaNetwork_addDelay(const char* url, double delay);
BIG MediaNetwork_download(const char* url, double priority, UCHAR** buff);
BOOL MediaNetwork_undownload(const char* url);
BOOL MediaNetwork_isDownloadActive(void);
double MediaNetwork_getBandwidth(void);
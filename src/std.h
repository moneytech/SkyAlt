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

#define StdArrCOPY void* (*)(void*)
#define StdArrFREE void (*)(void*)
#define StdArrFREE2 void (*)(void*, void*)

typedef struct StdArr_s
{
	void** ptrs;
	UBIG num;
}StdArr;
StdArr StdArr_init(void);
StdArr StdArr_initCopy(const StdArr* src);
StdArr StdArr_initCopyFn(const StdArr* src, void* (*copyFn)(void*));
void StdArr_freeBase(StdArr* self);
void StdArr_freeItems(StdArr* self, const int item_size);
void StdArr_freeFn(StdArr* self, void (*freeFn)(void*));
void StdArr_freeFnPrm(StdArr* self, void (*freeFn)(void*, void*), void* prm);
void StdArr_resize(StdArr* self, UBIG num);
void* StdArr_add(StdArr* self, void* ptr);
void StdArr_remove(StdArr* self, UBIG i);
void StdArr_insert(StdArr* self, UBIG i, void* ptr);
BIG StdArr_find(const StdArr* self, const void* ptr);
BOOL StdArr_removeFind(StdArr* self, const void* ptr);
BOOL StdArr_replace(StdArr* self, const void* old, void* nw);
void StdArr_swap(StdArr* self, UBIG iA, BIG iB);
void* StdArr_get(const StdArr* self, UBIG i);
void* StdArr_last(StdArr* self);
void StdArr_addArr(StdArr* self, StdArr* src);

typedef struct StdBigs_s
{
	BIG* ptrs;
	UBIG num;
	UBIG alloc;
}StdBigs;
StdBigs StdBigs_init(void);
StdBigs StdBigs_initCopy(const StdBigs* src);
void StdBigs_free(StdBigs* self);
void StdBigs_clear(StdBigs* self);
void StdBigs_setAlloc(StdBigs* self, const UBIG n);
void StdBigs_resize(StdBigs* self, UBIG num);
BIG StdBigs_add(StdBigs* self, BIG value);
void StdBigs_remove(StdBigs* self, UBIG i);
void StdBigs_insert(StdBigs* self, UBIG i, BIG value);
UBIG StdBigs_insertShort(StdBigs* self, BIG value);
BIG StdBigs_find(const StdBigs* self, const BIG value);
BOOL StdBigs_removeFind(StdBigs* self, const BIG value);
void StdBigs_swap(StdBigs* self, UBIG iA, BIG iB);
BIG StdBigs_get(StdBigs* self, BIG i);
BIG StdBigs_getNeg(StdBigs* self, BIG i);
BIG StdBigs_last(StdBigs* self);
void StdBigs_addArr(StdBigs* self, StdBigs* src);
void StdBigs_reversEx(StdBigs* self, const UBIG start, const UBIG end);
void StdBigs_revers(StdBigs* self);
void StdBigs_println(const StdBigs* self);
void StdBigs_rotate(StdBigs* self, BIG start, BIG end);
void StdBigs_qshortEx(StdBigs* self, const BIG start, const BIG end, const BOOL ascending);
void StdBigs_qshort(StdBigs* self, const BOOL ascending);
void StdBigs_copy(StdBigs* dst, const StdBigs* src);
void StdBigs_setAll(StdBigs* self, BIG value);

#define PII 3.14159265f

double Std_minDouble(void);
double Std_maxDouble(void);
float Std_fmin(float a, float b);
float Std_fmax(float a, float b);
float Std_fclamp(float v, float mi, float mx);
int Std_min(int a, int b);
int Std_max(int a, int b);
int Std_clamp(int v, int mi, int mx);
double Std_dmin(double a, double b);
double Std_dmax(double a, double b);
double Std_dclamp(double v, double mi, double mx);
BIG Std_bmin(BIG a, BIG b);
BIG Std_bmax(BIG a, BIG b);
BIG Std_bclamp(BIG v, BIG mi, BIG mx);
BOOL Std_isBetween(double v, double mi, double mx);
double Std_roundHalf(double v);
double Std_roundBy(double v, double start, double jump);
double Std_roundDown(double v);
double Std_roundUp(double v);
UBIG Std_round16(UBIG size);
int Std_abs(int a);
double Std_fabs(double a);
void Std_flip(int* a, int* b);
UBIG Std_ptrDistance(void* start, void* end);
UCHAR Std_numStartZeroBits(UBIG value);

UNI Std_getUNIsmall(UNI ch);
char Std_getCHARsmall(char ch);
UBIG Std_sizeUNI(const UNI* self);

UBIG Std_sizeCHAR(const char* self);
BOOL Std_isUNI(const UNI* self);
BOOL Std_isCHAR(const char* self);
UBIG Std_bytesUNI(const UNI* self);

BOOL Std_cmpUNI(const UNI* a, const UNI* b);
BOOL Std_cmpUNIsmall(const UNI* a, const UNI* b);
BOOL Std_cmpUNI_CHAR(const UNI* a, const char* b);
BOOL Std_cmpUNI_CHAR_small(const UNI* a, const char* b);
int Std_cmpUNIascending(const UNI* a, const UNI* b);
int Std_cmpCHARascending(const char* a, const char* b);
BOOL Std_cmpCHAR(const char* a, const char* b);

BOOL Std_cmpCHARsmall(const char* a, const char* b);
UNI* Std_newUNI_N(const UBIG N);
UNI* Std_newUNI_char(const char* src);
UNI* Std_newUNI_char_n(const char* src, UBIG N);
UNI* Std_newNumber(double value);
UNI* Std_newNumberPrecision(double value, UINT precision);

void Std_setHEX_char(char* self, const UCHAR* arr, const int N);
void Std_setHEX_uni(UNI* self, const UCHAR* arr, const int N);
void Std_getFromHEX(const char* str, const int N, UCHAR* out);
void Std_getFromHEX_uni(const UNI* str, const int N, UCHAR* out);
UNI* Std_newUNI(const UNI* src);
UNI* Std_newUNI_copy(const UNI* src, UBIG N);
UNI* Std_newUNI_copyEx(const UNI* src, UBIG N, UNI exclude);
char* Std_newCHAR_copyEx(const char* src, UBIG N, UNI exclude);

void Std_deleteCHAR(char* self);
void Std_deleteUNI(UNI* self);

void Std_replaceCHAR(char** dst, const char* src);
void Std_replaceUNI(UNI** dst, const UNI* src);
void Std_replaceInsideUNI(UNI** dst, const UNI* find, const UNI* replace);
void Std_replaceCharacters(UNI* self, UNI ch);
BIG Std_findCHAR(const char* self, char ch);
BIG Std_findUNI(const UNI* self, UNI ch);
BIG Std_findUNIex(const UNI* self, UNI ch, UNI exclude);
BIG Std_findUNI_last(const UNI* self, UNI ch);


void Std_print(UNI cp);
void Std_printUNI(const UNI* self);
void Std_printlnUNI(const UNI* self);
void Std_printlnUNI_sub(const UNI* self, const UNI* end);

UNI* Std_addUNI(const UNI* a, const UNI* b);
UNI* Std_addAfterUNI(UNI* dst, const UNI* src);
UNI* Std_addAfterUNI_char(UNI* dst, const char* src);

UNI* Std_insertChar(UNI* dst, const UNI ch, UBIG pos);
UNI* Std_insertUNI(UNI* dst, const UNI* src, UBIG pos);

UNI* Std_removeUNI(UNI* dst, int pos);
UNI* Std_removeChars(UNI* dst, int pos, int num);

BOOL Std_startWithCHAR(const char* self, const char* find);
BOOL Std_startWith(const UNI* self, const UNI* find);
BOOL Std_startWith_small(const UNI* self, const UNI* find);
BOOL Std_startWith_small_char(const UNI* self, const char* find);
BIG Std_subUNI(const UNI* self, const UNI* find);
BIG Std_subCHAR(const char* self, const char* find);
BIG Std_subUNI_small(const UNI* self, const UNI* find);
BIG Std_subUNI_small_char(const UNI* self, const char* find);
UNI* Std_copyUNI(UNI* dst, UBIG dstMax_N, const UNI* src);
UNI* Std_copyUNI_char(UNI* dst, UBIG dstMax_N, const char* src);
char* Std_copyCHAR_uni(char* dst, UBIG dstMax_N, const UNI* src);
char* Std_copyCHAR(char* dst, UBIG dstMax_N, const char* src);
void Std_convertUpUNI(UNI* self);

const char* Std_findSubCHAR(const char* self, const char* find);

BOOL Std_isDigit(char ch);
BOOL Std_isNotDigit(char ch);
const UNI* Std_goOver(const UNI* str, const UNI* find);
const UNI* Std_goOverIgnore(const UNI* str, const UNI* ignore);

UBIG Std_countDigitsInRowCHAR(const char* str);
UBIG Std_countDigitsInRowUNI(const UNI* str);
UBIG Std_countDigitsAndAlphabetInRow(const UNI* str, BOOL find);
double Std_getNumberFromUNI_n(const UNI* str, BIG max_n);
double Std_getNumberFromUNI(const UNI* str);

UBIG Std_separNumItemsCHAR(const char* self, const UNI separator);
char* Std_separGetItemCHAR(const char* self, UBIG value, const UNI separator);
UBIG Std_separNumItemsUNI(const UNI* self, const UNI separator);
UNI* Std_separGetItemUNI(const UNI* self, UBIG value, const UNI separator);
BOOL Std_SeparGetItemUNI_cmp(const UNI* self, UBIG value, const UNI separator, const UNI* src);
BIG Std_separFind(const UNI* self, const UNI* separators);

void Std_removeLetterUNI(UNI* self, UNI ch);
void Std_removeLetterCHAR(char* self, char ch);

void Std_array_print(UCHAR* data, UBIG data_size);
void Std_array_printDouble(double* data, UBIG data_size);

char* Std_newCHAR_N(const UBIG N);
char* Std_newCHAR(const char* src);
char* Std_newCHAR_uni(const UNI* src);
char* Std_newCHAR_uni_n(const UNI* src, const UBIG N);
char* Std_newCHAR_n(const char* src, const UBIG N);

void Std_rewriteCHAR(char** dst, const char* src);
char* Std_buildNumber(double value, int precision, char str[64]);
UNI* Std_buildNumberUNI(double value, int precision, UNI out[64]);
void Std_separNumberThousands(UNI* self, char separ);

void Std_setCHAR_uni(char* dst, const UNI* src, const int dst_max);
void Std_setUNI_char(UNI* dst, const char* src, const int dst_max);
char* Std_addCHAR(const char* a, const char* b);
char* Std_addAfterCHAR(char* dst, const char* src);
char* Std_addAfterCHAR_uni(char* dst, const UNI* src);

UNI* Std_newNumberSize(UBIG size);
UNI* Std_repeatUNI(const int N, const UNI ch);

char* Std_utf32_to_utf8(const UNI* in);

int Std_ishex(int x);
BIG Std_urlDecode(const char* s, char* dec);
char* Std_newCHAR_urlDecode(const char* url);
void Std_urlEncode(const UNI* s, char* out);
char* Std_newCHAR_urlEncode(const UNI* url);

typedef struct StdString_s
{
	UNI* str;
	UBIG n;
}StdString;
StdString StdString_init();
StdString StdString_initCopy(const StdString* src);
void StdString_freeIgnore(StdString* self);
void StdString_free(StdString* self);
UBIG StdString_size(const StdString* self);
void StdString_empty(StdString* self);
void StdString_resize(StdString* self, UBIG N);
void StdString_fit(StdString* self);
void StdString_setUNI(StdString* self, const UNI* str);
void StdString_setUNI_n(StdString* self, const UNI* str, UBIG n);
void StdString_addUNI(StdString* self, const UNI* str);
void StdString_setCHAR(StdString* self, const char* str);
void StdString_setCHAR_n(StdString* self, const char* str, UBIG n);
void StdString_addCHAR(StdString* self, const char* str);
void StdString_addNumber(StdString* self, int precision, double value);
BOOL StdString_cmp(const StdString* a, const StdString* b);
void StdString_print(StdString* self);

typedef struct Vec2i_s
{
	int x, y;
}Vec2i;
Vec2i Vec2i_init(void);
Vec2i Vec2i_init2(int x, int y);
UBIG Vec2i_num(const Vec2i self);
BOOL Vec2i_cmp(Vec2i a, Vec2i b);
int Vec2i_dot(Vec2i a, Vec2i b);
Vec2i Vec2i_abs(Vec2i a);
Vec2i Vec2i_add(Vec2i a, Vec2i b);
Vec2i Vec2i_sub(Vec2i a, Vec2i b);
Vec2i Vec2i_mulV(const Vec2i vec, float v);
Vec2i Vec2i_mul(Vec2i a, Vec2i b);
Vec2i Vec2i_div(Vec2i a, Vec2i b);
Vec2i Vec2i_divV(const Vec2i vec, float v);
Vec2i Vec2i_normalize(const Vec2i vec);
Vec2i Vec2i_getLen(const Vec2i vec, float len);
Vec2i Vec2i_aprox(Vec2i a, Vec2i b, float t);
BOOL Vec2i_inside(Vec2i start, Vec2i end, Vec2i test);
Vec2i Vec2i_min(Vec2i a, Vec2i b);
Vec2i Vec2i_max(Vec2i a, Vec2i b);
double Vec2i_len(Vec2i a);
double Vec2i_distance(Vec2i a, Vec2i b);
void Vec2i_print(Vec2i v, const char* name);
BOOL Vec2i_isZero(const Vec2i self);
BOOL Vec2i_is(const Vec2i self);
Vec2i Vec2i_subRatio(const Vec2i rect, const Vec2i orig);

typedef struct Vec2f_s
{
	double x, y;
}Vec2f;
Vec2f Vec2f_init(void);
Vec2f Vec2f_init2(double x, double y);
BOOL Vec2f_is(const Vec2f self);
BOOL Vec2f_cmp(Vec2f a, Vec2f b);
Vec2f Vec2f_add(Vec2f p, Vec2f q);
Vec2f Vec2f_sub(Vec2f p, Vec2f q);
Vec2f Vec2f_mul(Vec2f p, Vec2f q);
Vec2f Vec2f_mulV(Vec2f p, double t);
Vec2f Vec2f_divV(const Vec2f vec, float v);
Vec2f Vec2f_normalize(const Vec2f vec);
Vec2f Vec2f_min(Vec2f a, Vec2f b);
Vec2f Vec2f_max(Vec2f a, Vec2f b);
Vec2f Vec2f_bernstein(float u, Vec2f* p);
Vec2i Vec2f_to2i(Vec2f vi);
Vec2f Vec2i_to2f(Vec2i vi);
Vec2f Vec2f_aprox(Vec2f a, Vec2f b, float t);
double Vec2f_dot(Vec2f a, Vec2f b);
double Vec2f_len(Vec2f a);
double Vec2f_distance(Vec2f a, Vec2f b);
Vec2f Vec2f_perpendicularX(Vec2f a);
Vec2f Vec2f_perpendicularY(Vec2f a);

typedef struct Vec3f_s
{
	double x, y, z;
}Vec3f;
Vec3f Vec3f_init(void);
Vec3f Vec3f_init3(double x, double y, double z);
Vec3f Vec3f_add(Vec3f p, Vec3f q);
Vec3f Vec3f_mulV(Vec3f p, float t);
Vec3f Vec3f_aprox(Vec3f a, Vec3f b, float t);
BOOL Vec3f_cmp(Vec3f a, Vec3f b);

typedef struct Quad2i_s
{
	Vec2i start;
	Vec2i size;
}Quad2i;
Quad2i Quad2i_init(void);
Quad2i Quad2i_init2(const Vec2i start, const Vec2i size);
Quad2i Quad2i_init4(int px, int py, int sx, int sy);
Quad2i Quad2i_initEnd(const Vec2i start, const Vec2i end);
Quad2i Quad2i_initMid(Vec2i mid, Vec2i size);
Vec2i Quad2i_getSize(Quad2i* self);
Vec2i Quad2i_end(const Quad2i self);
BOOL Quad2i_cmp(const Quad2i a, const Quad2i b);
BOOL Quad2i_inside(const Quad2i self, Vec2i test);
BOOL Quad2i_isZero(const Quad2i self);
Quad2i Quad2i_extend(const Quad2i a, const Quad2i b);
Quad2i Quad2i_extend2(const Quad2i q, const Vec2i v);
BOOL Quad2i_hasCover(const Quad2i a, const Quad2i b);
BOOL Quad2i_hasCoverSoft(const Quad2i a, const Quad2i b);
Quad2i Quad2i_getIntersect(const Quad2i qA, const Quad2i qB);
Quad2i Quad2i_addSpaceX(const Quad2i q, int space);
Quad2i Quad2i_addSpaceY(const Quad2i q, int space);
Quad2i Quad2i_addSpace(const Quad2i q, int space);
Quad2i Quad2i_center(const Quad2i out, const Quad2i in);
Vec2i Quad2i_getMiddle(const Quad2i q);
Quad2i Quad2i_multV(const Quad2i q, float t);
void Quad2i_print(Quad2i q, const char* name);
Quad2i Quad2i_repair(Quad2i a, int add);
Quad2i Quad2i_initSE(const Vec2i start, const Vec2i end);
Vec2i Quad2i_clamp(Quad2i self, Vec2i v);
Vec2i Quad2i_getSubStart(Quad2i self, Vec2i size);
Quad2i Quad2i_getSub(Quad2i self, Vec2i size);

typedef struct Quad2f_s
{
	Vec2f start;
	Vec2f size;
}Quad2f;
Quad2f Quad2f_init(void);
Quad2f Quad2f_init2(const Vec2f start, const Vec2f size);
Quad2f Quad2f_init4(double px, double py, double sx, double sy);
Vec2f Quad2f_end(const Quad2f self);
double Quad2f_getArea(Quad2f q);
Quad2f Quad2f_extend(const Quad2f a, const Quad2f b);
Quad2f Quad2f_extend2(const Quad2f q, const Vec2f v);
BOOL Quad2f_hasCover(const Quad2f a, const Quad2f b);
Quad2f Quad2f_getIntersect(const Quad2f qA, const Quad2f qB);
Vec2f Quad2f_getMiddle(const Quad2f q);
//double Quad2f_getIntersectArea(Quad2f qA, Quad2f qB);
void Quad2f_print(Quad2f q, const char* name);

typedef struct Rgba_s
{
	UCHAR r, g, b, a;
}Rgba;
Rgba Rgba_init4(UCHAR r, UCHAR g, UCHAR b, UCHAR a);
UCHAR Rgba_r(Rgba self);
UCHAR Rgba_g(Rgba self);
UCHAR Rgba_b(Rgba self);
Rgba Rgba_initHSL(int H, float S, float L);
UINT Rgba_asNumber(const Rgba self);
Rgba Rgba_initFromNumber(UINT number);
void Rgba_getHSL(const Rgba* self, int* H, float* S, float* L);
Rgba Rgba_initRandom(int i, int N);
Rgba Rgba_initBlack(void);
Rgba Rgba_initWhite(void);
Rgba Rgba_initRed(void);
Rgba Rgba_initGreyLight(void);
Rgba Rgba_initGrey(void);
Rgba Rgba_initGreyDark(void);
BOOL Rgba_isBlack(const Rgba* self);
Rgba Rgba_multV(const Rgba self, float t);
Rgba Rgba_aprox(const Rgba s, const Rgba e, float t);
Rgba Rgba_aproxInt(const Rgba s, const Rgba e, unsigned int alpha);
Rgba Rgba_aproxQuad(Rgba st, Rgba et, Rgba sb, Rgba eb, float x, float y);
BOOL Rgba_cmp(Rgba a, Rgba b);
UINT Rgba_getUINT(Rgba self);
void Rgba_mulV(Rgba* self, float t);
void Rgba_print(Rgba s);
Vec3f Rgba_get3f(Rgba s);
void Rgba_switch(Rgba* a, Rgba* b);
Rgba Rgba_initFromHex(const char* hex);	//#RRGGBB
void Rgba_getHex(Rgba* self, char hex[8]);	//#RRGGBB
Rgba Rgba_getAproxHSL(Rgba mid, float t, float range360);
Rgba Rgba_getRandomHue(Rgba srcSL);
Rgba Rgba_getNextHue(Rgba* src);

typedef struct Image1_s
{
	Vec2i size;
	UCHAR* data;
}Image1;
UBIG Image1_num(const Image1* self);
UBIG Image1_bytes(const Image1* self);
Image1 Image1_init(void);
Image1 Image1_initSize(Vec2i size);
Image1 Image1_initCopy(const Image1* src);
void Image1_free(Image1* self);
BOOL Image1_is(const Image1* self);
void Image1_resize(Image1* self, Vec2i size);
Quad2i Image1_getSizeQuad(const Image1* self);
UCHAR* Image1_get(const Image1* self, int x, int y);
UCHAR* Image1_getV(const Image1* self, Vec2i pos);
void Image1_set(const Image1* self, Vec2i pos, const UCHAR cd);
int Image1_getPosSmoothRepeat(Image1* self, Vec2i p);

typedef struct Image4_s
{
	Rgba* data;
	Vec2i size;
	Quad2i rect;
}Image4;
UBIG Image4_num(const Image4* self);
UBIG Image4_bytes(const Image4* self);
Image4 Image4_init(void);
Image4 Image4_init2(Rgba* data, Vec2i size);
Image4 Image4_initSize(Vec2i size);
Image4 Image4_initCopy(const Image4* src);
void Image4_free(Image4* self);
void Image4_resize(Image4* self, Vec2i size);
Quad2i Image4_getSizeQuad(const Image4* self);
UBIG Image4_getPos(const Image4* self, int x, int y);
Rgba* Image4_get(const Image4* self, int x, int y);
Rgba* Image4_getV(const Image4* self, Vec2i p);
Rgba* Image4_getLast(Image4* self);
BOOL Image4_is(Image4* self, Vec2i p);
void Image4_setAlpha0(Image4* self);
Image1 Image4_convertToImage1(Image4* img4);
void Image4_repairRect(Image4* self);
Quad2i Image4_setRect(Image4* self, Quad2i rect);
void Image4_setPixel(Image4* self, Vec2i p, Rgba cd, float alpha);
void Image4_copyDirect(Image4* self, Quad2i rect, Image4* src);
BOOL Image4_saveBmp(Image4* self, const char* filename);
BOOL Image4_saveJpeg(Image4* self, const char* filename);

BOOL Image4_initBmp(Image4* self, const char* filename);
BOOL Image4_initFile(Image4* self, const char* path, const char* ext);	//ext = ".jpg"
BOOL Image4_initBuffer(Image4* self, const UCHAR* buffer, const UINT buffer_size, const char* ext);	//ext = ".jpg"

void Image1_drawNoise(Image1* self, Vec2i offset, const int zoom, const int octaves, const float p);
UCHAR Image1_getRectAvg(const Image1* img, Quad2f rect);
void Image1_scale(Image1* dst, const Image1* src);
void Image4_drawBoxStartEnd(Image4* self, Vec2i start, Vec2i end, Rgba cd);
void Image4_drawBoxQuad(Image4* self, Quad2i coord, Rgba cd);
void Image4_drawChessQuad(Image4* self, Quad2i coord, Vec2i cell, Rgba cd);
void Image4_drawBoxStartEndAlpha(Image4* self, Vec2i start, Vec2i end, Rgba cd);
void Image4_drawBoxQuadAlpha(Image4* self, Quad2i coord, Rgba cd);
void Image4_drawDot(Image4* self, Vec2i pos, int fat, Rgba cd);
void Image4_drawBorder(Image4* self, Quad2i coord, const int fat, Rgba cd);
void Image4_gausBlur(Image4* self);
Rgba Image4_getRectAvg(const Image4* img, Quad2f rect);
void Image4_scale(Image4* dst, const Image4* src);
//void Image4_blurScale(Image4* self);
//void Image4_blurFast(Image4* dst, const Image4* src, int res);
void Image4_mulV(Image4* self, unsigned int alpha);
void Image4_mulVSub(Image4* self, Vec2i start, Vec2i end, unsigned int alpha);
void Image4_mulVSubQ(Image4* self, Quad2i coord, unsigned int alpha);
void Image4_copyImage1(Image4* self, Vec2i start, Rgba cd, const Image1* src);
void Image4_copyImage4(Image4* self, Vec2i start, Image4* src);
void Image4_copyImage4_resize(Image4* self, Quad2i coord, Image4* src);
void Image4_drawCircleRect(Image4* self, Vec2i mid, int rad, Rgba cd, Quad2i q);
void Image4_drawCircle(Image4* self, Vec2i mid, int rad, Rgba cd);
void Image4_drawCircleLineRect(Image4* self, Vec2i mid, int rad, float fat, Rgba cd, Quad2i q);
void Image4_drawCircleLine(Image4* self, Vec2i mid, int rad, int fat, Rgba cd);
void Image4_drawCircleEx(Image4* self, Vec2i mid, int radIn, int radOut, Rgba cd, Quad2i q, double angleStart, double angleEnd);
Vec2i Image4_getCircleMid(Vec2i mid, double angleStart, double angleEnd, float t);
void Image4_drawRBox(Image4* self, Quad2i coord, int rad, Rgba cd);
void Image4_drawRBorder(Image4* self, Quad2i coord, int rad, int fat, Rgba cd);
void Image4_drawLine(Image4* self, Vec2i s, Vec2i e, int width, Rgba cd);
void Image4_drawArrow(Image4* self, Vec2i s, Vec2i e, int width, Rgba cd);
void Image4_drawBezier(Image4* self, Vec2f params[4], Rgba cd, int width);
void Image4_drawBezierArrow(Image4* self, Vec2f params[4], Rgba cd, int width);
void Image4_drawBezierArrowBack(Image4* self, Vec2f params[4], Rgba cd, int width);
void Image4_drawPolyLines(Image4* self, float* xy, const UBIG N, int width, Rgba cd);
void Image4_drawPolyFill(Image4* self, float* xy, const UBIG N, Rgba cd, const float alpha);
Quad2i Image4_getUnderline(Vec2i pos, BOOL centerText, Vec2i textSize);
Quad2i Image4_drawTextCoord(Vec2i s, BOOL center, OsFont* font, const UNI* text, const int Hpx, const int betweenLinePx);
void Image4_drawText(Image4* img, Vec2i s, BOOL center, OsFont* font, const UNI* text, const int Hpx, const int betweenLinePx, Rgba cd);
void Image4_drawTextBackground(Image4* img, Vec2i s, BOOL center, OsFont* font, const UNI* text, const int Hpx, const int betweenLinePx, Rgba text_cd, Rgba back_cd, int addSpace);
void Image4_drawTextAngle(Image4* img, Vec2i s, BOOL center, OsFont* font, const UNI* text, const int Hpx, const int betweenLinePx, Rgba cd, int angleDeg);
UBIG Image4_numTextLines(Quad2i coord, OsFont* font, const UNI* text, const int Hpx);
const UNI* Image4_skipTextLines(Quad2i coord, OsFont* font, const UNI* text, const int Hpx, const UBIG skipLines);
int Image4_drawTextMulti(Image4* img, Quad2i coord, BOOL center, OsFont* font, const UNI* text, const int Hpx, const int betweenLinePx, Rgba cd);

typedef struct StdBIndex_s StdBIndex;
StdBIndex* StdBIndex_new(void);
void StdBIndex_delete(StdBIndex* self);
void StdBIndex_clear(StdBIndex* self);
UBIG StdBIndex_size(const StdBIndex* self);
BIG StdBIndex_search(const StdBIndex* self, BIG hash);
void StdBIndex_add(StdBIndex* self, BIG hash, UBIG id);

double StdMap_getMetersPerPixel(double lat, double z);
Vec2f StdMap_getTile(double lon, double lat, double z);
Vec2f StdMap_getLonLat(double tileX, double tileY, double z);
Quad2f StdMap_lonLatToTileBbox(Vec2i res, const int tilePx, double lon, double lat, double z);

double Std_timeAprox2(double startTime, double jumpTime);
double Std_timeAprox3(double startTime, double jumpTime);

void StdProgress_initGlobal(void);
void StdProgress_freeGlobal(void);
float StdProgress_get(void);
void StdProgress_set(const char* trans, float done);
void StdProgress_setEx(const char* trans, double part, double maxx);
void StdProgress_setExx(const char* trans, double part, double maxx, double progressStart);
void StdProgress_run(BOOL run);
BOOL StdProgress_is(void);
const char* StdProgress_getTranslationID(void);

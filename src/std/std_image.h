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

Rgb Rgb_init3(UCHAR r, UCHAR g, UCHAR b)
{
	Rgb self;
	self.r = r;
	self.g = g;
	self.b = b;
	return self;
}
Rgb Rgb_multV(const Rgb self, float t)
{
	Rgb cd;
	cd.r = (UCHAR)(self.r * t);
	cd.g = (UCHAR)(self.g * t);
	cd.b = (UCHAR)(self.b * t);
	return cd;
}
Rgb Rgb_aprox(const Rgb s, const Rgb e, float t)
{
	Rgb self;
	self.r = (UCHAR)(s.r + (e.r - s.r) * t);
	self.g = (UCHAR)(s.g + (e.g - s.g) * t);
	self.b = (UCHAR)(s.b + (e.b - s.b) * t);
	return self;
}
BOOL Rgb_cmp(Rgb a, Rgb b)
{
	return a.r == b.r && a.g == b.g && a.b == b.b;;
}
UINT Rgb_getUINT(Rgb self)
{
	return *(UINT*)&self;
}

Rgba Rgba_init4(UCHAR r, UCHAR g, UCHAR b, UCHAR a)
{
	Rgba self;

#ifdef _WIN32
	self.r = b;
	self.b = r;
#else
	self.r = r;
	self.b = b;
#endif
	self.g = g;
	self.a = a;
	return self;
}

static float _HueToRGB(float v1, float v2, float vH)
{
	if (vH < 0)	vH += 1;
	if (vH > 1)	vH -= 1;

	if ((6 * vH) < 1)	return (v1 + (v2 - v1) * 6 * vH);
	if ((2 * vH) < 1)	return v2;
	if ((3 * vH) < 2)	return (v1 + (v2 - v1) * ((2.0f / 3) - vH) * 6);

	return v1;
}

//hue(0,360), saturation(0, 1), lightness(0, 1)
Rgba Rgba_initHSL(int H, float S, float L)
{
	Rgba self;

	if (S == 0)
	{
		self.r = self.g = self.b = (UCHAR)(L * 255);
	}
	else
	{
		float v1, v2;
		float hue = (float)H / 360;

		v2 = (L < 0.5) ? L * (1 + S) : (L + S) - (L * S);
		v1 = 2 * L - v2;

		self.r = (unsigned char)(255 * _HueToRGB(v1, v2, hue + (1.0f / 3)));
		self.g = (unsigned char)(255 * _HueToRGB(v1, v2, hue));
		self.b = (unsigned char)(255 * _HueToRGB(v1, v2, hue - (1.0f / 3)));
	}

	self.a = 255;

	return self;
}

UINT Rgba_asNumber(const Rgba self)
{
	return *(UINT*)&self;
}
Rgba Rgba_initFromNumber(UINT number)
{
	Rgba self = *(Rgba*)&number;
	return self;
}

void Rgba_getHSL(Rgba* self, int* H, float* S, float* L)
{
	float r = (self->r / 255.0f);
	float g = (self->g / 255.0f);
	float b = (self->b / 255.0f);

	float min = Std_fmin(Std_fmin(r, g), b);
	float max = Std_fmax(Std_fmax(r, g), b);
	float delta = max - min;

	*L = (max + min) / 2;

	if (delta == 0)
	{
		*H = 0;
		*S = 0.0f;
	}
	else
	{
		*S = (*L <= 0.5) ? (delta / (max + min)) : (delta / (2 - max - min));

		float hue;

		if (r == max)
		{
			hue = ((g - b) / 6) / delta;
		}
		else if (g == max)
		{
			hue = (1.0f / 3) + ((b - r) / 6) / delta;
		}
		else
		{
			hue = (2.0f / 3) + ((r - g) / 6) / delta;
		}

		if (hue < 0)
			hue += 1;
		if (hue > 1)
			hue -= 1;

		*H = (int)(hue * 360);
	}
}

Rgba Rgba_initRandom(int i, int N)
{
	i = (int)(i * (360.0f / N));

	UCHAR rnd1, rnd2;
	OsCrypto_random(1, &rnd1);
	OsCrypto_random(1, &rnd2);

	return Rgba_initHSL(i, 0.9f + (rnd1 / 255.0f) * 0.1f, 0.5f + (rnd1 / 255.0f) * 0.1f);
}

Rgba Rgba_initBlack(void)
{
	return Rgba_init4(0, 0, 0, 255);
}
Rgba Rgba_initWhite(void)
{
	return Rgba_init4(255, 255, 255, 255);
}

Rgba Rgba_initRed(void)
{
	return Rgba_init4(250, 50, 50, 255);
}

BOOL Rgba_isBlack(const Rgba* self)
{
	return self->r == 0 && self->g == 0 && self->b == 0;
}

Rgba Rgba_multV(const Rgba self, float t)
{
	Rgba cd;
	cd.r = (UCHAR)(self.r * t);
	cd.g = (UCHAR)(self.g * t);
	cd.b = (UCHAR)(self.b * t);
	cd.a = (UCHAR)(self.a * t);
	return cd;
}
Rgba Rgba_aprox(const Rgba s, const Rgba e, float t)
{
	Rgba self;
	self.r = (UCHAR)(s.r + (e.r - s.r) * t);
	self.g = (UCHAR)(s.g + (e.g - s.g) * t);
	self.b = (UCHAR)(s.b + (e.b - s.b) * t);
	self.a = (UCHAR)(s.a + (e.a - s.a) * t);
	return self;
}

Rgba Rgba_aproxQuad(Rgba st, Rgba et, Rgba sb, Rgba eb, float x, float y)
{
	Rgba top = Rgba_aprox(st, et, x);
	Rgba bottom = Rgba_aprox(sb, eb, x);
	return Rgba_aprox(top, bottom, y);
}

BOOL Rgba_cmp(Rgba a, Rgba b)
{
	return a.r == b.r && a.g == b.g && a.b == b.b && a.a == b.a;
}
UINT Rgba_getUINT(Rgba self)
{
	return *(UINT*)&self;
}

void Rgba_mulV(Rgba* self, float t)
{
	self->r = (UCHAR)(self->r * t);
	self->g = (UCHAR)(self->g * t);
	self->b = (UCHAR)(self->b * t);
	self->a = 255;
}

void Rgba_print(Rgba s)
{
	printf("cd: %d %d %d %d\n", (int)s.r, (int)s.g, (int)s.b, (int)s.a);
}
Vec3f Rgba_get3f(Rgba s)
{
	return Vec3f_init3(s.r, s.g, s.b);
}

void Rgba_switch(Rgba* a, Rgba* b)
{
	Rgba t = *a;
	*a = *b;
	*b = t;
}

Rgba Rgba_initFromHex(const char* hex)	//#RRGGBB
{
	UBIG n = Std_sizeCHAR(hex);

	if (n && hex[0] == '#')
		hex++;

	int r, g, b;
	r = g = b = 0;
	if (hex)
		sscanf(hex, "%02x%02x%02x", &r, &g, &b);

	return Rgba_init4(r, g, b, 255);
}

void Rgba_getHex(Rgba* self, char hex[8])	//#RRGGBB
{
	snprintf(hex, 8, "#%02x%02x%02x", self->r, self->g, self->b);
}

UBIG Image1_num(const Image1* self)
{
	return Vec2i_num(self->size);
}
UBIG Image1_bytes(const Image1* self)
{
	return Image1_num(self);
}

Image1 Image1_init(void)
{
	Image1 self;
	self.size = Vec2i_init();
	self.data = 0;
	return self;
}
Image1 Image1_initSize(Vec2i size)
{
	Image1 self;
	self.size = size;
	self.data = Os_calloc(size.x * size.y, 1);
	return self;
}

Image1 Image1_initCopy(const Image1* src)
{
	Image1 self = Image1_initSize(src->size);
	Os_memcpy(self.data, src->data, Image1_bytes(src));
	return self;
}

void Image1_free(Image1* self)
{
	Os_free(self->data, Image1_bytes(self));
}
void Image1_resize(Image1* self, Vec2i size)
{
	self->size = size;
	self->data = Os_realloc(self->data, Image1_bytes(self));
}

Quad2i Image1_getSizeQuad(const Image1* self)
{
	return Quad2i_init2(Vec2i_init(), self->size);
}
UCHAR* Image1_get(const Image1* self, int x, int y)
{
	return &self->data[y * self->size.x + x];
}
UCHAR* Image1_getV(const Image1* self, Vec2i pos)
{
	return Image1_get(self, pos.x, pos.y);
}
void Image1_set(const Image1* self, Vec2i pos, const UCHAR cd)
{
	*Image1_getV(self, pos) = cd;
}
int Image1_getPosSmoothRepeat(Image1* self, Vec2i p)
{
	//repeat
	int x = Std_abs(p.x % self->size.x);
	int y = Std_abs(p.y % self->size.y);

	//smooth - revers odd
	if ((p.x / self->size.x) % 2 != 0)	x = self->size.x - 1 - x;
	if ((p.y / self->size.y) % 2 != 0)	y = self->size.y - 1 - y;

	return *Image1_get(self, x, y);
}

UBIG Image4_num(const Image4* self)
{
	return Vec2i_num(self->size);
}
UBIG Image4_bytes(const Image4* self)
{
	return Image4_num(self) * sizeof(Rgba);
}

Image4 Image4_init(void)
{
	Image4 self;
	self.data = 0;
	self.size = Vec2i_init();
	self.rect = Quad2i_init();
	return self;
}
Image4 Image4_init2(Rgba* data, Vec2i size)
{
	Image4 self;
	self.data = data;
	self.size = size;
	self.rect = Quad2i_init2(Vec2i_init(), size);
	return self;
}
Image4 Image4_initSize(Vec2i size)
{
	return Image4_init2(Os_malloc(size.x * size.y * sizeof(Rgba)), size);
}

Image4 Image4_initCopy(const Image4* src)
{
	Image4 self = Image4_initSize(src->size);
	Os_memcpy(self.data, src->data, Image4_bytes(src));
	self.rect = src->rect;
	return self;
}
void Image4_free(Image4* self)
{
	Os_free(self->data, Image4_bytes(self));
	Os_memset(self, sizeof(Image4));
}

void Image4_resize(Image4* self, Vec2i size)
{
	self->size = size;
	self->data = Os_realloc(self->data, Image4_bytes(self));

	self->rect = Quad2i_init2(Vec2i_init(), self->size);
}

Quad2i Image4_getSizeQuad(const Image4* self)
{
	return Quad2i_init2(Vec2i_init(), self->size);
}

UBIG Image4_getPos(const Image4* self, int x, int y)
{
	return y * self->size.x + x;
}
Rgba* Image4_get(const Image4* self, int x, int y)
{
	return &self->data[Image4_getPos(self, x, y)];
}

Rgba* Image4_getV(const Image4* self, Vec2i p)
{
	return Image4_get(self, p.x, p.y);
}

Rgba* Image4_getLast(Image4* self)
{
	return Image4_get(self, 0, self->size.y);
}

BOOL Image4_is(Image4* self, Vec2i p)
{
	return Vec2i_inside(Vec2i_init(), self->size, p);
}

Image1 Image4_convertToImage1(Image4* img4)
{
	Image1 img1 = Image1_initSize(img4->size);

	Rgba* src = img4->data;
	UCHAR* dst = img1.data;
	UBIG i;
	for (i = 0; i < Image4_num(img4); i++)
	{
		*dst = Std_max(Std_max(Std_max(src->r, src->g), src->b), src->a);
		dst++;
		src++;
	}

	return img1;
}

void Image4_repairRect(Image4* self)
{
	self->rect = Quad2i_getIntersect(self->rect, Quad2i_init2(Vec2i_init(), self->size));
}

Quad2i Image4_setRect(Image4* self, Quad2i rect)
{
	self->rect = rect;
	Image4_repairRect(self);
	return self->rect;
}

void Image4_setPixel(Image4* self, Vec2i p, Rgba cd, float alpha)
{
	*Image4_getV(self, p) = Rgba_aprox(*Image4_getV(self, p), cd, alpha);
}

void Image4_copyDirect(Image4* self, Quad2i rect, Image4* src)
{
	rect = Quad2i_getIntersect(Image4_getSizeQuad(self), rect);
	rect = Quad2i_getIntersect(Image4_getSizeQuad(src), rect);

	Vec2i start = rect.start;
	Vec2i end = Quad2i_end(rect);

	Vec2i p;
	for (p.y = start.y; p.y < end.y; p.y++)
	{
		p.x = start.x;

		Rgba* sCd = Image4_getV(src, p);
		Rgba* dCd = Image4_getV(self, p);

		for (p.x = start.x; p.x < end.x; p.x++)
		{
			*dCd = *sCd;
			sCd++;
			dCd++;
		}
	}
}

BOOL Image4_saveBmp(Image4* self, const char* filename)
{
	OsFile f;
	int i, j;
	Vec2i S = self->size;

	UCHAR bmpfileheader[14] = { 'B','M', 0,0,0,0, 0,0, 0,0, 54,0,0,0 };
	UCHAR bmpinfoheader[40] = { 40,0,0,0, 0,0,0,0, 0,0,0,0, 1,0, 24,0 };
	UCHAR bmppad[3] = { 0,0,0 };

	int filesize = 54 + 3 * S.x * S.y;
	bmpfileheader[2] = (UCHAR)(filesize);
	bmpfileheader[3] = (UCHAR)(filesize >> 8);
	bmpfileheader[4] = (UCHAR)(filesize >> 16);
	bmpfileheader[5] = (UCHAR)(filesize >> 24);

	bmpinfoheader[4] = (UCHAR)(S.x);
	bmpinfoheader[5] = (UCHAR)(S.x >> 8);
	bmpinfoheader[6] = (UCHAR)(S.x >> 16);
	bmpinfoheader[7] = (UCHAR)(S.x >> 24);
	bmpinfoheader[8] = (UCHAR)(S.y);
	bmpinfoheader[9] = (UCHAR)(S.y >> 8);
	bmpinfoheader[10] = (UCHAR)(S.y >> 16);
	bmpinfoheader[11] = (UCHAR)(S.y >> 24);

	if (!OsFile_init(&f, filename, OsFile_W))
		return FALSE;

	OsFile_write(&f, bmpfileheader, 14);
	OsFile_write(&f, bmpinfoheader, 40);

	for (i = 0; i < S.y; i++)
	{
		for (j = 0; j < S.x; j++)	//S.y
		{
			Rgba* cd = Image4_get(self, j, S.y - i - 1);
			OsFile_write(&f, cd, 3);
		}
		OsFile_write(&f, bmppad, (4 - (S.x * 3) % 4) % 4);
	}

	OsFile_free(&f);
	return TRUE;
}

BOOL Image4_initBmp(Image4* self, const char* filename)
{
	OsFile f;
	int file_size, offset, dx, dy, compression, nb_colors, bpp, dx_bytes, align_bytes, buf_size;
	UCHAR header[64] = { 0 };
	UCHAR* buffer;
	int xoffset;
	UCHAR* ptrs;
	int x, y;

	*self = Image4_init();

	if (!OsFile_init(&f, filename, OsFile_R))
		return FALSE;
	OsFile_read(&f, header, 54);

	if (*header != 'B' || header[1] != 'M')
	{
		OsFile_free(&f);
		return FALSE;
	}
	//read header and pixel buffer
	file_size = header[0x02] + (header[0x03] << 8) + (header[0x04] << 16) + (header[0x05] << 24);
	offset = header[0x0A] + (header[0x0B] << 8) + (header[0x0C] << 16) + (header[0x0D] << 24);
	dx = header[0x12] + (header[0x13] << 8) + (header[0x14] << 16) + (header[0x15] << 24);
	dy = header[0x16] + (header[0x17] << 8) + (header[0x18] << 16) + (header[0x19] << 24);
	compression = header[0x1E] + (header[0x1F] << 8) + (header[0x20] << 16) + (header[0x21] << 24);
	nb_colors = header[0x2E] + (header[0x2F] << 8) + (header[0x30] << 16) + (header[0x31] << 24);
	bpp = header[0x1C] + (header[0x1D] << 8);

	if (compression)
	{
		OsFile_free(&f);
		return FALSE;
	}
	dx_bytes = (bpp == 1) ? (dx / 8 + (dx % 8 ? 1 : 0)) : ((bpp == 4) ? (dx / 2 + (dx % 2 ? 1 : 0)) : (dx * bpp / 8));
	align_bytes = (4 - dx_bytes % 4) % 4;
	buf_size = Std_min(Std_abs(dy) * (dx_bytes + align_bytes), file_size - offset);

	xoffset = offset - 54 - 4 * nb_colors;
	if (xoffset > 0)
		OsFile_seekRel(&f, xoffset);

	buffer = (UCHAR*)Os_malloc(buf_size);
	OsFile_read(&f, buffer, buf_size);
	ptrs = &buffer[0];

	dy = Std_abs(dy);

	if (bpp != 24)	//only 256 colors
		return FALSE;
	*self = Image4_init2(Os_calloc(dx * dy, 4), Vec2i_init2(dx, dy));

	for (y = 0; y < dy; ++y)
	{
		for (x = 0; x < dx; ++x)
		{
			Os_memcpy(Image4_get(self, x, dy - y - 1), ptrs, 3);
			ptrs += 3;
		}
		ptrs += align_bytes;
	}

	Os_free(buffer, buf_size);
	OsFile_free(&f);
	return TRUE;
}

typedef struct Image4File_s
{
	const UCHAR* data;
	UBIG bytes;

	UBIG offset;
}Image4File;

BIG _Image4File_read(void* selff, UCHAR* buff, int buff_size)
{
	Image4File* self = selff;

	const UBIG N = Std_min(buff_size, (self->bytes - self->offset));
	Os_memcpy(buff, self->data, N);

	self->offset += N;

	return N ? N : -1/*AVERROR_EOF*/;	//is -1 EOF?
}

BIG _Image4File_seek(void* selff, BIG offset)
{
	Image4File* self = selff;

	if (offset < 0)
		return self->bytes;

	self->offset = offset;
	return self->offset;
}

BOOL Image4_initFile(Image4* self, const char* path, const char* ext)	//ext = ".jpg"
{
	Image4File f;
	UCHAR* data = OsFile_initRead(path, &f.bytes, 0);
	f.data = data;
	f.offset = 0;

	OSMedia* media = OSMedia_new(&_Image4File_read, &_Image4File_seek, &f, ext);

	*self = Image4_initSize(*OSMedia_getOrigSize(media));
	OSMedia_loadVideo(media, self);

	Os_free(data, f.bytes);

	return TRUE;
}

BOOL Image4_initBuffer(Image4* self, const UCHAR* buffer, const UINT buffer_size, const char* ext)	//ext = ".jpg"
{
	Image4File f;
	f.data = buffer;
	f.bytes = buffer_size;
	f.offset = 0;

	OSMedia* media = OSMedia_new(&_Image4File_read, &_Image4File_seek, &f, ext);

	*self = Image4_initSize(*OSMedia_getOrigSize(media));
	OSMedia_loadVideo(media, self);

	return TRUE;
}

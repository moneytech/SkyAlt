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

double Std_maxDouble(void)
{
	return 3.402823466e+38F;	//FLT_MAX;	//DBL_MAX
}
double Std_minDouble(void)
{
	return -Std_maxDouble();
}

float Std_fmin(float a, float b)
{
	return a < b ? a : b;
}
float Std_fmax(float a, float b)
{
	return a > b ? a : b;
}
float Std_fclamp(float v, float mi, float mx)
{
	return Std_fmin(Std_fmax(v, mi), mx);
}

int Std_min(int a, int b)
{
	return a < b ? a : b;
}
int Std_max(int a, int b)
{
	return a > b ? a : b;
}
int Std_clamp(int v, int mi, int mx)
{
	return Std_min(Std_max(v, mi), mx);
}

double Std_dmin(double a, double b)
{
	return a < b ? a : b;
}
double Std_dmax(double a, double b)
{
	return a > b ? a : b;
}
double Std_dclamp(double v, double mi, double mx)
{
	return Std_dmin(Std_dmax(v, mi), mx);
}

BIG Std_bmin(BIG a, BIG b)
{
	return a < b ? a : b;
}
BIG Std_bmax(BIG a, BIG b)
{
	return a > b ? a : b;
}
BIG Std_bclamp(BIG v, BIG mi, BIG mx)
{
	return Std_bmin(Std_bmax(v, mi), mx);
}

BOOL Std_isBetween(double v, double mi, double mx)
{
	return (v >= mi && v <= mx) || (v <= mi && v >= mx);
}

double Std_roundHalf(double v)
{
	return (double)(BIG)(v + (v < 0 ? -0.5 : 0.5));
}

double Std_roundBy(double v, double start, double jump)
{
	double t = Std_roundHalf((v - start) / jump);
	return  start + t * jump;
}

double Std_roundDown(double v)
{
	return (double)(BIG)v;
}
double Std_roundUp(double v)
{
	return v > (BIG)v ? v + 1 : v;
}

UBIG Std_round16(UBIG size)
{
	UBIG v = size % 16;
	return (v > 0) ? (size - v + 16) : size;
}

int Std_abs(int a)
{
	return a < 0 ? -a : a;
}
double Std_fabs(double a)
{
	return a < 0 ? -a : a;
}
void Std_flip(int* a, int* b)
{
	int c = *a;
	*a = *b;
	*b = c;
}

UBIG Std_ptrDistance(void* start, void* end)
{
	return ((UBIG)end) - ((UBIG)start);
}

UCHAR Std_numStartZeroBits(UBIG value)
{
	UCHAR n = 0;
	int i;
	for (i = 63; i >= 0; i--)
	{
		if ((value & (1ULL << i)) == 0)
			n++;
		else
			break;
	}
	return n;
}

Vec2i Vec2i_init(void)
{
	Vec2i v;
	v.x = v.y = 0;
	return v;
}
Vec2i Vec2i_init2(int x, int y)
{
	Vec2i v;
	v.x = x;
	v.y = y;
	return v;
}
UBIG Vec2i_num(const Vec2i self)
{
	return self.x * self.y;
}
BOOL Vec2i_cmp(Vec2i a, Vec2i b)
{
	return a.x == b.x && a.y == b.y;
}
Vec2i Vec2i_abs(Vec2i a)
{
	return Vec2i_init2(Std_abs(a.x), Std_abs(a.y));
}
int Vec2i_dot(Vec2i a, Vec2i b)
{
	return a.x * b.x + a.y * b.y;
}
Vec2i Vec2i_add(Vec2i a, Vec2i b)
{
	return Vec2i_init2(a.x + b.x, a.y + b.y);
}
Vec2i Vec2i_sub(Vec2i a, Vec2i b)
{
	return Vec2i_init2(a.x - b.x, a.y - b.y);
}
Vec2i Vec2i_mulV(const Vec2i vec, float v)
{
	return Vec2i_init2((int)(vec.x * v), (int)(vec.y * v));
}
Vec2i Vec2i_mul(Vec2i a, Vec2i b)
{
	return Vec2i_init2(a.x * b.x, a.y * b.y);
}
Vec2i Vec2i_div(Vec2i a, Vec2i b)
{
	return Vec2i_init2(a.x / b.x, a.y / b.y);
}
Vec2i Vec2i_divV(const Vec2i vec, float v)
{
	return v ? Vec2i_init2((int)(vec.x / v), (int)(vec.y / v)) : Vec2i_init();
}
Vec2i Vec2i_normalize(const Vec2i vec)
{
	const float l = Vec2i_len(vec);
	return Vec2i_divV(vec, l);
}
Vec2i Vec2i_getLen(const Vec2i vec, float len)
{
	const float l = Vec2i_len(vec) / len;
	return Vec2i_divV(vec, l);
}
Vec2i Vec2i_aprox(Vec2i a, Vec2i b, float t)
{
	return Vec2i_init2(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t);
}
BOOL Vec2i_inside(Vec2i start, Vec2i end, Vec2i test)
{
	return	test.x >= start.x && test.y >= start.y &&
		test.x < end.x && test.y < end.y;
}
Vec2i Vec2i_min(Vec2i a, Vec2i b)
{
	return Vec2i_init2(Std_min(a.x, b.x), Std_min(a.y, b.y));
}
Vec2i Vec2i_max(Vec2i a, Vec2i b)
{
	return Vec2i_init2(Std_max(a.x, b.x), Std_max(a.y, b.y));
}

double Vec2i_len(Vec2i a)
{
	return Os_sqrt(a.x * a.x + a.y * a.y);
}
double Vec2i_distance(Vec2i a, Vec2i b)
{
	return Vec2i_len(Vec2i_sub(a, b));
}
void Vec2i_print(Vec2i v, const char* name)
{
	printf("%s: %d %d\n", name, v.x, v.y);
}
BOOL Vec2i_isZero(const Vec2i self)
{
	return self.x == 0 && self.y == 0;
}
BOOL Vec2i_is(const Vec2i self)
{
	return self.x != 0 && self.y != 0;
}

Vec2i Vec2i_subRatio(const Vec2i rect, const Vec2i orig)
{
	float rectRatio = rect.x / (float)rect.y;
	float origRatio = orig.x / (float)orig.y;
	float ratio = (origRatio > rectRatio) ? (rect.x / (float)orig.x) : (rect.y / (float)orig.y);
	return Vec2i_mulV(orig, ratio);
}

Vec2f Vec2f_init(void)
{
	Vec2f v;
	v.x = v.y = 0;
	return v;
}
Vec2f Vec2f_init2(double x, double y)
{
	Vec2f p;
	p.x = x;
	p.y = y;
	return p;
}
BOOL Vec2f_is(const Vec2f self)
{
	return self.x != 0 && self.y != 0;
}
BOOL Vec2f_cmp(Vec2f a, Vec2f b)
{
	return a.x == b.x && a.y == b.y;
}
Vec2f Vec2f_add(Vec2f p, Vec2f q)
{
	p.x += q.x;
	p.y += q.y;
	return p;
}
Vec2f Vec2f_sub(Vec2f p, Vec2f q)
{
	p.x -= q.x;
	p.y -= q.y;
	return p;
}
Vec2f Vec2f_mul(Vec2f p, Vec2f q)
{
	p.x *= q.x;
	p.y *= q.y;
	return p;
}
Vec2f Vec2f_mulV(Vec2f p, double t)
{
	p.x *= t;
	p.y *= t;
	return p;
}
Vec2f Vec2f_divV(const Vec2f vec, float v)
{
	return v ? Vec2f_init2(vec.x / v, vec.y / v) : Vec2f_init();
}
Vec2f Vec2f_normalize(const Vec2f vec)
{
	const float l = Vec2f_len(vec);
	return Vec2f_divV(vec, l);
}
Vec2f Vec2f_min(Vec2f a, Vec2f b)
{
	return Vec2f_init2(Std_dmin(a.x, b.x), Std_dmin(a.y, b.y));
}
Vec2f Vec2f_max(Vec2f a, Vec2f b)
{
	return Vec2f_init2(Std_dmax(a.x, b.x), Std_dmax(a.y, b.y));
}
Vec2f Vec2f_bernstein(float u, Vec2f* p)
{
	Vec2f a, b, c, d, r;	//temps
	a = Vec2f_mulV(p[0], Os_pow(u, 3));
	b = Vec2f_mulV(p[1], 3 * Os_pow(u, 2) * (1 - u));
	c = Vec2f_mulV(p[2], 3 * u * Os_pow((1 - u), 2));
	d = Vec2f_mulV(p[3], Os_pow((1 - u), 3));
	r = Vec2f_add(Vec2f_add(a, b), Vec2f_add(c, d));
	return r;
}

Vec2i Vec2f_to2i(Vec2f vi)
{
	return Vec2i_init2((int)vi.x, (int)vi.y);
}

Vec2f Vec2i_to2f(Vec2i vi)
{
	return Vec2f_init2(vi.x, vi.y);
}

Vec2f Vec2f_aprox(Vec2f a, Vec2f b, float t)
{
	return Vec2f_init2(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t);
}

double Vec2f_dot(Vec2f a, Vec2f b)
{
	return a.x * b.x + a.y * b.y;
}
double Vec2f_len(Vec2f a)
{
	return Os_sqrt(a.x * a.x + a.y * a.y);
}
double Vec2f_distance(Vec2f a, Vec2f b)
{
	return Vec2f_len(Vec2f_sub(a, b));
}
Vec2f Vec2f_perpendicularX(Vec2f a)
{
	return Vec2f_init2(-a.y, a.x);
}
Vec2f Vec2f_perpendicularY(Vec2f a)
{
	return Vec2f_init2(a.y, -a.x);
}


Vec3f Vec3f_init(void)
{
	Vec3f v;
	v.x = v.y = v.z = 0;
	return v;
}
Vec3f Vec3f_init3(double x, double y, double z)
{
	Vec3f p;
	p.x = x;
	p.y = y;
	p.z = z;
	return p;
}
Vec3f Vec3f_add(Vec3f p, Vec3f q)
{
	p.x += q.x;
	p.y += q.y;
	p.z += q.z;
	return p;
}
Vec3f Vec3f_mulV(Vec3f p, float t)
{
	p.x *= t;
	p.y *= t;
	p.z *= t;
	return p;
}
Vec3f Vec3f_aprox(Vec3f a, Vec3f b, float t)
{
	return Vec3f_init3(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, a.z + (b.z - a.z) * t);
}
BOOL Vec3f_cmp(Vec3f a, Vec3f b)
{
	return a.x == b.x && a.y == b.y && a.z == b.z;
}


Quad2i Quad2i_init(void)
{
	Quad2i q;
	q.start = Vec2i_init();
	q.size = Vec2i_init();
	return q;
}
Quad2i Quad2i_init2(const Vec2i start, const Vec2i size)
{
	Quad2i q;
	q.start = start;
	q.size = size;
	return q;
}
Quad2i Quad2i_init4(int px, int py, int sx, int sy)
{
	return Quad2i_init2(Vec2i_init2(px, py), Vec2i_init2(sx, sy));
}
Quad2i Quad2i_initEnd(const Vec2i start, const Vec2i end)
{
	Quad2i q;
	q.start = start;
	q.size = Vec2i_sub(end, start);
	return q;
}

Quad2i Quad2i_initMid(Vec2i mid, Vec2i size)
{
	return Quad2i_init4(mid.x - size.x / 2, mid.y - size.y / 2, size.x, size.y);
}

Vec2i Quad2i_getSize(Quad2i* self)
{
	return self->size;
}

Vec2i Quad2i_end(const Quad2i self)
{
	return Vec2i_add(self.start, self.size);
}
BOOL Quad2i_cmp(const Quad2i a, const Quad2i b)
{
	return Vec2i_cmp(a.start, b.start) && Vec2i_cmp(a.size, b.size);
}
BOOL Quad2i_inside(const Quad2i self, Vec2i test)
{
	return Vec2i_inside(self.start, Quad2i_end(self), test);
}
BOOL Quad2i_isZero(const Quad2i self)
{
	return Vec2i_isZero(self.size);
}

Quad2i Quad2i_extend(const Quad2i a, const Quad2i b)
{
	Vec2i start;
	start.x = Std_min(a.start.x, b.start.x);
	start.y = Std_min(a.start.y, b.start.y);

	Vec2i ae = Quad2i_end(a);
	Vec2i be = Quad2i_end(b);
	Vec2i end;
	end.x = Std_max(ae.x, be.x);
	end.y = Std_max(ae.y, be.y);

	return Quad2i_init2(start, Vec2i_sub(end, start));
}

Quad2i Quad2i_extend2(const Quad2i q, const Vec2i v)
{
	Vec2i start = q.start;
	start.x = Std_min(start.x, v.x);
	start.y = Std_min(start.y, v.y);

	Vec2i end = Quad2i_end(q);
	end.x = Std_max(end.x, v.x);
	end.y = Std_max(end.y, v.y);

	return Quad2i_init2(start, Vec2i_sub(end, start));
}

BOOL Quad2i_hasCover(const Quad2i a, const Quad2i b)
{
	Quad2i q = Quad2i_extend(a, b);

	return q.size.x < (a.size.x + b.size.x) && q.size.y < (a.size.y + b.size.y);
}

BOOL Quad2i_hasCoverSoft(const Quad2i a, const Quad2i b)
{
	Quad2i q = Quad2i_extend(a, b);

	return q.size.x < (a.size.x + b.size.x) && q.size.y < (a.size.y + b.size.y);
}

Quad2i Quad2i_getIntersect(const Quad2i qA, const Quad2i qB)
{
	if (Quad2i_hasCover(qA, qB))
	{
		Vec2i v_start = Vec2i_max(qA.start, qB.start);
		Vec2i v_end = Vec2i_min(Quad2i_end(qA), Quad2i_end(qB));
		return Quad2i_init2(v_start, Vec2i_sub(v_end, v_start));
	}
	return Quad2i_init();
}

Quad2i Quad2i_addSpaceX(const Quad2i q, int space)
{
	return Quad2i_init4(q.start.x + space,
		q.start.y,
		q.size.x - space * 2,
		q.size.y);
}

Quad2i Quad2i_addSpaceY(const Quad2i q, int space)
{
	return Quad2i_init4(q.start.x,
		q.start.y + space,
		q.size.x,
		q.size.y - space * 2);
}

Quad2i Quad2i_addSpace(const Quad2i q, int space)
{
	Quad2i r = Quad2i_addSpaceX(q, space);
	return Quad2i_addSpaceY(r, space);
}

Quad2i Quad2i_center(const Quad2i out, const Quad2i in)
{
	Quad2i r = in;
	if (out.size.x > in.size.x)	r.start.x += (out.size.x - in.size.x) / 2;
	if (out.size.y > in.size.y)	r.start.y += (out.size.y - in.size.y) / 2;
	return r;
}

Vec2i Quad2i_getMiddle(const Quad2i q)
{
	return Vec2i_add(q.start, Vec2i_mulV(q.size, 0.5f));
}

Quad2i Quad2i_multV(const Quad2i q, float t)
{
	return Quad2i_init2(Vec2i_mulV(q.start, t), Vec2i_mulV(q.size, t));
}
void Quad2i_print(Quad2i q, const char* name)
{
	printf("%s: %d %d %d %d\n", name, q.start.x, q.start.y, q.size.x, q.size.y);
}

Quad2i Quad2i_repair(Quad2i a, int add)
{
	if (a.size.x < 0)
	{
		a.start.x += a.size.x + add;
		a.size.x = -a.size.x;//+add;
	}

	if (a.size.y < 0)
	{
		a.start.y += a.size.y + add;
		a.size.y = -a.size.y;//+add;
	}

	return a;
}

Quad2i Quad2i_initSE(const Vec2i start, const Vec2i end)
{
	Quad2i q = Quad2i_init2(start, Vec2i_sub(end, start));
	q = Quad2i_repair(q, 0);
	return q;
}

Vec2i Quad2i_clamp(Quad2i self, Vec2i v)
{
	Vec2i end = Quad2i_end(self);
	return Vec2i_init2(Std_clamp(v.x, self.start.x, end.x), Std_clamp(v.y, self.start.y, end.y));
}

Vec2i Quad2i_getSubStart(Quad2i self, Vec2i size)
{
	return Vec2i_add(self.start, Vec2i_divV(Vec2i_sub(self.size, size), 2));
}
Quad2i Quad2i_getSub(Quad2i self, Vec2i size)
{
	return Quad2i_init2(Quad2i_getSubStart(self, size), size);
}

Quad2f Quad2f_init(void)
{
	Quad2f q;
	q.start = Vec2f_init();
	q.size = Vec2f_init();
	return q;
}
Quad2f Quad2f_init2(const Vec2f start, const Vec2f size)
{
	Quad2f q;
	q.start = start;
	q.size = size;
	return q;
}
Quad2f Quad2f_init4(double px, double py, double sx, double sy)
{
	return Quad2f_init2(Vec2f_init2(px, py), Vec2f_init2(sx, sy));
}

Vec2f Quad2f_end(const Quad2f self)
{
	return Vec2f_add(self.start, self.size);
}
double Quad2f_getArea(Quad2f q)
{
	Vec2f end = Quad2f_end(q);
	return (end.x - q.start.x) * (end.y - q.start.y);
}
Quad2f Quad2f_extend(const Quad2f a, const Quad2f b)
{
	Vec2f start;
	start.x = Std_min(a.start.x, b.start.x);
	start.y = Std_min(a.start.y, b.start.y);

	Vec2f ae = Quad2f_end(a);
	Vec2f be = Quad2f_end(b);
	Vec2f end;
	end.x = Std_max(ae.x, be.x);
	end.y = Std_max(ae.y, be.y);

	return Quad2f_init2(start, Vec2f_sub(end, start));
}

Quad2f Quad2f_extend2(const Quad2f q, const Vec2f v)
{
	Vec2f start = q.start;
	start.x = Std_min(start.x, v.x);
	start.y = Std_min(start.y, v.y);

	Vec2f end = Quad2f_end(q);
	end.x = Std_max(end.x, v.x);
	end.y = Std_max(end.y, v.y);

	return Quad2f_init2(start, Vec2f_sub(end, start));
}
BOOL Quad2f_hasCover(const Quad2f a, const Quad2f b)
{
	Quad2f q = Quad2f_extend(a, b);
	return q.size.x < (a.size.x + b.size.x) && q.size.y < (a.size.y + b.size.y);
}
Quad2f Quad2f_getIntersect(const Quad2f qA, const Quad2f qB)
{
	if (Quad2f_hasCover(qA, qB))
	{
		Vec2f v_start = Vec2f_max(qA.start, qB.start);
		Vec2f v_end = Vec2f_min(Quad2f_end(qA), Quad2f_end(qB));
		return Quad2f_init2(v_start, Vec2f_sub(v_end, v_start));
	}
	return Quad2f_init();
}

Vec2f Quad2f_getMiddle(const Quad2f q)
{
	return Vec2f_add(q.start, Vec2f_mulV(q.size, 0.5f));
}

void Quad2f_print(Quad2f q, const char* name)
{
	printf("%s: %f %f %f %f\n", name, q.start.x, q.start.y, q.size.x, q.size.y);
}

double StdMap_getMetersPerPixel(double lat, double z)
{
	//resolution = 156543.03 meters/pixel * cos(latitude) / (2 ^ zoomlevel)
	return 156543.034 * Os_cos(lat / 180 * M_PI) / Os_pow(2, z);
}

Vec2f StdMap_getTile(double lon, double lat, double z)
{
	Vec2f pos;
	pos.x = (lon + 180.0) / 360.0 * Os_pow(2.0, z);
	pos.y = (1.0 - Os_log(Os_tan(lat * M_PI / 180.0) + 1.0 / Os_cos(lat * M_PI / 180.0)) / M_PI) / 2.0 * Os_pow(2.0, z);
	return pos;
}

Vec2f StdMap_getLonLat(double tileX, double tileY, double z)
{
	Vec2f pos;
	pos.x = tileX / Os_pow(2.0, z) * 360.0 - 180;	//long

	double n = M_PI - 2.0 * M_PI * tileY / Os_pow(2.0, z);
	pos.y = 180.0 / M_PI * Os_atan(0.5 * (Os_exp(n) - Os_exp(-n)));	//lat

	return pos;
}

Quad2f StdMap_lonLatToTileBbox(Vec2i res, const int tilePx, double lon, double lat, double z)
{
	Vec2f tile = StdMap_getTile(lon, lat, z);

	int max_res = Os_pow(2, z);

	double xtile_s = Std_dclamp((tile.x * tilePx - res.x / 2) / tilePx, 0, max_res);
	double ytile_s = Std_dclamp((tile.y * tilePx - res.y / 2) / tilePx, 0, max_res);
	double xtile_e = Std_dclamp((tile.x * tilePx + res.x / 2) / tilePx, 0, max_res);
	double ytile_e = Std_dclamp((tile.y * tilePx + res.y / 2) / tilePx, 0, max_res);

	return Quad2f_init4(xtile_s, ytile_s, (xtile_e - xtile_s), (ytile_e - ytile_s));
}

double Std_timeAprox2(double startTime, double jumpTime)
{
	double dt = Os_time() - startTime;
	double t = dt / jumpTime;
	t -= (BIG)t;

	if (t < 0.5)
		return t * 2;	//up
	else
		return (1 - t) * 2;	//down
}

double Std_timeAprox3(double startTime, double jumpTime)
{
	double dt = Os_time() - startTime;
	double t = dt / jumpTime;
	t -= (BIG)t;

	if (t < 0.33)
		return t * 3;		//up
	else
		if (t < 0.67)
			return 1;			//stay
		else
			return (1 - t) * 3;	//down
}
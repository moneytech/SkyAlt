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

static float _Image1_findnoise(float x, float y)
{
	int n, nn;
	n = (int)x + (int)y * 57;
	n = (n << 13) ^ n;
	nn = (n * (n * n * 60493 + 19990303) + 1376312589) & 0x7fffffff;
	return 1.0f - ((float)nn / 1073741824.0f);
}
static float _Image1_interpolate(float a, float b, float x)
{
	float ft = x * 3.1415927f;
	float f = (1.0f - Os_cos(ft)) * 0.5f;
	return a * (1.0f - f) + b * f;
}
static float _Image1_noise(float x, float y)
{
	float i1, i2;
	float floorx = (float)((int)x);
	float floory = (float)((int)y);

	float s, t, u, v;

	s = _Image1_findnoise(floorx, floory);
	t = _Image1_findnoise(floorx + 1, floory);
	u = _Image1_findnoise(floorx, floory + 1);
	v = _Image1_findnoise(floorx + 1, floory + 1);

	i1 = _Image1_interpolate(s, t, x - floorx);
	i2 = _Image1_interpolate(u, v, x - floorx);
	return _Image1_interpolate(i1, i2, y - floory);
}
void Image1_drawNoise(Image1* self, Vec2i offset, const int zoom, const int octaves, const float p)
{
	int x, y, i;
	for (y = 0; y < self->size.y; y++)
	{
		for (x = 0; x < self->size.x; x++)
		{
			float getnoise = 0;
			for (i = 0; i < octaves - 1; i++)
			{
				float frequency = Os_pow(2.0f, i);
				float amplitude = Os_pow(p, i);
				getnoise += _Image1_noise(((float)x + offset.x) * frequency / zoom, ((float)y + offset.y) / zoom * frequency) * amplitude;
			}
			getnoise = (getnoise + 1) * 0.5f;
			if (getnoise < 0)	getnoise = 0;
			if (getnoise > 1)	getnoise = 1;
			*Image1_get(self, x, y) = (UCHAR)(255.0f * getnoise);
		}
	}
}

void Image1_scale(Image1* dst, const Image1* src)
{
	OSMedia_scale1(dst, src);
}

void Image4_drawBoxStartEnd(Image4* self, Vec2i start, Vec2i end, Rgba cd)
{
	//Image4_repairRect(self);
	start = Quad2i_clamp(self->rect, start);
	end = Quad2i_clamp(self->rect, end);

	for (; (start.y < end.y && start.y < self->size.y); start.y++)
	{
		int s = start.x;
		Rgba* pos = Image4_getV(self, start);

		for (; (start.x < end.x && start.x < self->size.x); start.x++)
		{
			*pos = cd;
			pos++;
		}
		start.x = s;
	}
}

void Image4_drawBoxQuad(Image4* self, Quad2i coord, Rgba cd)
{
	Image4_drawBoxStartEnd(self, coord.start, Quad2i_end(coord), cd);
}

void Image4_drawChessQuad(Image4* self, Quad2i coord, Vec2i cell, Rgba cd)
{
	BIG x, y;
	for (y = 0; y < coord.size.y / cell.y; y++)
		for (x = 0; x < coord.size.x / cell.x; x++)
			if (x % 2 == y % 2)
				Image4_drawBoxQuad(self, Quad2i_init2(Vec2i_add(coord.start, Vec2i_init2(x * cell.x, y * cell.y)), cell), cd);
}

void Image4_drawBoxStartEndAlpha(Image4* self, Vec2i start, Vec2i end, Rgba cd)
{
	float alpha = cd.a / 255.0f;

	//Image4_repairRect(self);
	start = Quad2i_clamp(self->rect, start);
	end = Quad2i_clamp(self->rect, end);

	for (; (start.y < end.y && start.y < self->size.y); start.y++)
	{
		int s = start.x;
		Rgba* dst = Image4_getV(self, start);
		for (; (start.x < end.x && start.x < self->size.x); start.x++)
		{
			*dst = Rgba_aprox(*dst, cd, alpha);
			dst++;
		}
		start.x = s;
	}
}

void Image4_drawBoxQuadAlpha(Image4* self, Quad2i coord, Rgba cd)
{
	Image4_drawBoxStartEndAlpha(self, coord.start, Quad2i_end(coord), cd);
}

void Image4_drawDot(Image4* self, Vec2i pos, int fat, Rgba cd)
{
	Vec2i f = Vec2i_init2(fat, fat);
	Image4_drawBoxStartEnd(self, Vec2i_sub(pos, f), Vec2i_add(pos, f), cd);
}

void Image4_drawBorder(Image4* self, Quad2i coord, const int fat, Rgba cd)
{
	if (coord.size.y >= fat)
	{
		Vec2i end = Quad2i_end(coord);

		Image4_drawBoxStartEnd(self, coord.start, Vec2i_init2(end.x, coord.start.y + fat), cd);		//top
		Image4_drawBoxStartEnd(self, Vec2i_init2(coord.start.x, end.y - fat), end, cd);				//bottom
		Image4_drawBoxStartEnd(self, coord.start, Vec2i_init2(coord.start.x + fat, end.y), cd);		//left
		Image4_drawBoxStartEnd(self, Vec2i_init2(end.x - fat, coord.start.y), end, cd);				//right
	}
}

/*const float gaus_filter[] =
{
0.00296902,    0.0133062,    0.0219382  ,  0.0133062  ,  0.00296902  ,
0.0133062 ,   0.0596343 ,   0.0983203  ,  0.0596343  ,  0.0133062    ,
0.0219382 ,   0.0983203 ,   0.162103   , 0.0983203   , 0.0219382    ,
0.0133062 ,   0.0596343  ,  0.0983203 ,   0.0596343 ,   0.0133062   ,
0.00296902  ,  0.0133062  ,  0.0219382  ,  0.0133062  ,  0.00296902,
};*/
void Image4_gausBlur(Image4* self)
{
	int x, y;
	const int RAD = 3;

	//compute matrix
	const int mat_x = (RAD * 2 + 1);
	const float max_len = Os_sqrt(RAD * RAD + RAD * RAD);
	BIG max_filter_bytes = mat_x * mat_x * sizeof(float);
	float* mat_filter = Os_malloc(max_filter_bytes);
	float mat_sum = 0;
	for (y = 0; y < mat_x; y++)
		for (x = 0; x < mat_x; x++)
			mat_sum += mat_filter[y * mat_x + x] = max_len - Os_sqrt((x - RAD) * (x - RAD) + (y - RAD) * (y - RAD));
	for (x = 0; x < mat_x * mat_x; x++)
		mat_filter[x] /= mat_sum;

	Image4 src = Image4_initCopy(self);

	for (y = 0; y < self->size.y; y++)
	{
		for (x = 0; x < self->size.x; x++)
		{
			int sum[4];
			sum[0] = sum[1] = sum[2] = sum[3] = 0;

			int xx, yy;
			for (yy = y - RAD; yy < y + RAD; yy++)
			{
				for (xx = x - RAD; xx < x + RAD; xx++)
				{
					if (yy >= 0 && yy < self->size.y && xx >= 0 && xx < self->size.x)
					{
						const float w = mat_filter[(yy - y + RAD) * mat_x + (xx - x + RAD)];

						Rgba* cd = Image4_get(&src, xx, yy);
						sum[0] += cd->r * w;
						sum[1] += cd->g * w;
						sum[2] += cd->b * w;
						sum[3] += cd->a * w;
					}
				}
			}

			Rgba* cd = Image4_get(self, x, y);
			cd->r = sum[0];
			cd->g = sum[1];
			cd->b = sum[2];
			cd->a = sum[3];
		}
	}

	Os_free(mat_filter, max_filter_bytes);
	Image4_free(&src);
}

void Image4_scale(Image4* dst, const Image4* src)
{
	OSMedia_scale4(dst, src);
}

void Image4_mulV(Image4* self, unsigned int alpha)
{
	Rgba* s = Image4_get(self, 0, 0);
	const Rgba* e = Image4_getLast(self);
	while (s < e)
	{
		Rgba_mulAlpha(s, alpha);
		s++;
	}
}

void Image4_mulVSub(Image4* self, Vec2i start, Vec2i end, unsigned int alpha)
{
	start.x = Std_max(start.x, 0);
	start.y = Std_max(start.y, 0);

	for (; (start.y < end.y && start.y < self->size.y); start.y++)
	{
		int s = start.x;
		for (; (start.x < end.x && start.x < self->size.x); start.x++)
			Rgba_mulAlpha(Image4_getV(self, start), alpha);
		start.x = s;
	}
}
void Image4_mulVSubQ(Image4* self, Quad2i coord, unsigned int alpha)
{
	Image4_mulVSub(self, coord.start, Quad2i_end(coord), alpha);
}

void Image4_copyImage1(Image4* self, Vec2i start, Rgba cd, const Image1* src)
{
	Quad2i qSrc = Quad2i_getIntersect(Quad2i_init2(start, src->size), self->rect);
	if (qSrc.size.x == 0 || qSrc.size.y == 0)
		return;

	//qSrc.start = Vec2i_sub(src->size, qSrc.size);
	qSrc.start = Vec2i_sub(self->rect.start, start);
	if (self->rect.start.x < start.x)	qSrc.start.x = 0;
	if (self->rect.start.y < start.y)	qSrc.start.y = 0;
	Vec2i qEnd = Quad2i_end(qSrc);

	Vec2i p;
	for (p.y = qSrc.start.y; p.y < qEnd.y; p.y++)
	{
		p.x = qSrc.start.x;
		UCHAR* s = Image1_getV(src, p);
		Rgba* d = Image4_getV(self, Vec2i_add(start, p));

		for (p.x = 0; p.x < qSrc.size.x; p.x++)
		{
			*d = Rgba_aproxInt(*d, cd, *s);
			d++;
			s++;
		}
	}
}

void Image4_copyImage4(Image4* self, Vec2i start, Image4* src)
{
	Quad2i qSrc = Quad2i_getIntersect(Quad2i_init2(start, src->size), self->rect);
	if (qSrc.size.x == 0 || qSrc.size.y == 0)
		return;

	//qSrc.start = Vec2i_sub(src->size, qSrc.size);
	qSrc.start = Vec2i_sub(self->rect.start, start);
	if (self->rect.start.x < start.x)	qSrc.start.x = 0;
	if (self->rect.start.y < start.y)	qSrc.start.y = 0;
	Vec2i qEnd = Quad2i_end(qSrc);

	Vec2i p;
	for (p.y = qSrc.start.y; p.y < qEnd.y; p.y++)
	{
		p.x = qSrc.start.x;
		Rgba* s = Image4_getV(src, p);
		Rgba* d = Image4_getV(self, Vec2i_add(start, p));
		Os_memcpy(d, s, qSrc.size.x * sizeof(Rgba));
	}
}

//optimalization:
	//1) Compute only 1/4 copy other(mirrors)
	//2) compute start/end points of line and rest fill with full color
void Image4_drawCircleRect(Image4* self, Vec2i mid, int rad, Rgba cd, Quad2i q)
{
	const Vec2i qend = Quad2i_end(q);

	//float rr = rad - 0.5f;
	const int anti = 4;
	const float maxHits = anti * anti;
	const float step = 1.0f / (anti + 1);

	Vec2i pos = q.start;
	for (; pos.y < qend.y; pos.y++)
	{
		pos.x = q.start.x;
		for (; pos.x < qend.x; pos.x++)
		{
			Vec2i sub = Vec2i_sub(pos, mid);
			float r = Vec2i_len(sub);

			if (r < rad - 1.5f)
				*Image4_getV(self, pos) = cd;
			else
				if (r > rad + 1.5f)
					continue;
				else
				{
					int hits = 0;
					int i, ii;
					for (i = 1; i <= anti; i++)
						for (ii = 1; ii <= anti; ii++)
							hits += (Vec2f_len(Vec2f_sub(Vec2f_init2(pos.x + i * step, pos.y + ii * step), Vec2i_to2f(mid))) < rad);

					*Image4_getV(self, pos) = Rgba_aprox(*Image4_getV(self, pos), cd, hits / maxHits);
				}
		}
	}
}
void Image4_drawCircle(Image4* self, Vec2i mid, int rad, Rgba cd)
{
	//Image4_repairRect(self);
	Quad2i q = Quad2i_init4(mid.x - rad, mid.y - rad, rad * 2, rad * 2);
	q = Quad2i_addSpace(q, -1);
	q = Quad2i_getIntersect(self->rect, q);

	Image4_drawCircleRect(self, mid, rad, cd, q);
}
void Image4_drawCircleLineRect(Image4* self, Vec2i mid, int rad, float fat, Rgba cd, Quad2i q)
{
	q = Quad2i_getIntersect(q, Image4_getSizeQuad(self));

	const Vec2i qend = Quad2i_end(q);

	const int anti = 4;
	const float maxHits = anti * anti;
	const float step = 1.0f / (anti + 1);

	fat /= 2;

	const float extraRad = 0.2f;

	Vec2i pos = q.start;
	for (; pos.y < qend.y; pos.y++)
	{
		pos.x = q.start.x;
		for (; pos.x < qend.x; pos.x++)
		{
			Vec2i sub = Vec2i_sub(pos, mid);
			float r = Vec2i_len(sub);

			if (r > rad - fat - 1.5f && r < rad + fat + 1.5f)
			{
				int hits = 0;
				int i, ii;
				for (i = 1; i <= anti; i++)
					for (ii = 1; ii <= anti; ii++)
					{
						r = Vec2f_len(Vec2f_sub(Vec2f_init2(pos.x + i * step, pos.y + ii * step), Vec2i_to2f(mid)));
						hits += (r >= rad - fat - extraRad && r <= rad + fat + extraRad);
					}

				*Image4_getV(self, pos) = Rgba_aprox(*Image4_getV(self, pos), cd, hits / maxHits);
			}
		}
	}
}
void Image4_drawCircleLine(Image4* self, Vec2i mid, int rad, int fat, Rgba cd)
{
	//Image4_repairRect(self);

	Quad2i q = Quad2i_init4(mid.x - rad, mid.y - rad, rad * 2, rad * 2);
	q = Quad2i_getIntersect(self->rect, q);
	q = Quad2i_addSpace(q, -1);

	Image4_drawCircleLineRect(self, mid, rad, fat, cd, q);
}

static double _Image4_getCircleAngle(Vec2i vec, float rad)
{
	//rotate so the start is up and rotation is clockwise
	vec = Vec2i_init2(-vec.y, vec.x);

	float angle = Os_acos(vec.x / Vec2i_len(vec));
	if (vec.y < 0)
		angle = 2 * M_PI - angle;

	return angle;
}
Vec2i Image4_getCircleMid(Vec2i mid, double angleStart, double angleEnd, float t)
{
	double angle = angleStart + (angleEnd - angleStart) / 2;

	Vec2f vec = Vec2f_init2(Os_cos(angle), Os_sin(angle));
	vec = Vec2f_init2(vec.y, -vec.x);

	vec = Vec2f_mulV(vec, t);

	return Vec2i_init2(mid.x + vec.x, mid.y + vec.y);
}

void Image4_drawCircleEx(Image4* self, Vec2i mid, int radIn, int radOut, Rgba cd, Quad2i q, double angleStart, double angleEnd)
{
	const Vec2i qend = Quad2i_end(q);

	const int anti = 4;
	const float maxHits = anti * anti;
	const float step = 1.0f / (anti + 1);

	Vec2i pos = q.start;
	for (; pos.y < qend.y; pos.y++)
	{
		pos.x = q.start.x;
		for (; pos.x < qend.x; pos.x++)
		{
			Vec2i sub = Vec2i_sub(pos, mid);

			//radIn ...

			double angle = _Image4_getCircleAngle(sub, radOut);
			if (angle >= angleStart && angle <= angleEnd)
			{
				float r = Vec2i_len(sub);
				if (r < radOut - 1.5f)
					*Image4_getV(self, pos) = cd;
				else
					if (r > radOut + 1.5f)
						continue;
					else
					{
						int hits = 0;
						int i, ii;
						for (i = 1; i <= anti; i++)
							for (ii = 1; ii <= anti; ii++)
								hits += (Vec2f_len(Vec2f_sub(Vec2f_init2(pos.x + i * step, pos.y + ii * step), Vec2i_to2f(mid))) < radOut);

						*Image4_getV(self, pos) = Rgba_aprox(*Image4_getV(self, pos), cd, hits / maxHits);
					}
			}
		}
	}
}

void Image4_drawRBox(Image4* self, Quad2i coord, int rad, Rgba cd)
{
	coord = Quad2i_addSpace(coord, rad);
	Vec2i s = coord.start;
	Vec2i e = Quad2i_end(coord);
	Vec2i v = Vec2i_sub(e, s);

	Image4_drawCircle(self, s, rad, cd);
	Image4_drawCircle(self, e, rad, cd);
	Image4_drawCircle(self, Vec2i_init2(e.x, s.y), rad, cd);
	Image4_drawCircle(self, Vec2i_init2(s.x, e.y), rad, cd);

	Image4_drawBoxQuad(self, Quad2i_init4(s.x, s.y - rad, v.x, v.y + rad + rad), cd);	//mid(top-botton)
	Image4_drawBoxQuad(self, Quad2i_init4(s.x - rad, s.y, rad, v.y), cd);	//left
	Image4_drawBoxQuad(self, Quad2i_init4(e.x, s.y, rad, v.y), cd);	//right
}

void Image4_drawRBorder(Image4* self, Quad2i coord, int rad, int fat, Rgba cd)
{
	//coord = Quad2i_addSpace(coord, rad);
	Vec2i s = coord.start;
	Vec2i e = Quad2i_end(coord);
	Vec2i v = Vec2i_sub(e, s);

	//border
	Image4_drawBoxQuad(self, Quad2i_init4(s.x + rad, s.y, v.x - rad - rad, fat), cd);	//top
	Image4_drawBoxQuad(self, Quad2i_init4(s.x + rad, e.y - fat, v.x - rad - rad, fat), cd);	//bottom
	Image4_drawBoxQuad(self, Quad2i_init4(s.x, s.y + rad, fat, v.y - rad - rad), cd);	//left
	Image4_drawBoxQuad(self, Quad2i_init4(e.x - fat, s.y + rad, fat, v.y - rad - rad), cd);	//right

	coord = Quad2i_addSpace(coord, rad);
	s = coord.start;
	e = Quad2i_end(coord);
	Image4_drawCircleLineRect(self, s, rad, fat, cd, Quad2i_init4(s.x - rad, s.y - rad, rad, rad));
	Image4_drawCircleLineRect(self, e, rad, fat, cd, Quad2i_init4(e.x, e.y, rad, rad));
	Image4_drawCircleLineRect(self, Vec2i_init2(e.x, s.y), rad, fat, cd, Quad2i_init4(e.x, s.y - rad, rad, rad));
	Image4_drawCircleLineRect(self, Vec2i_init2(s.x, e.y), rad, fat, cd, Quad2i_init4(s.x - rad, e.y, rad, rad));
}

//where on "sub" we are
static float Image4_getT(Vec2f pos, Vec2f start, Vec2f v)
{
	return ((pos.x - start.x) * v.x + (pos.y - start.y) * v.y) / Vec2f_dot(v, v);
}

/*static float Image4_getT2(Vec2f pos, Vec2f start, Vec2f end)
{
	//not "t", but distance ...

	Vec2f sub = Vec2f_sub(end, start);
	return (sub.y * pos.x - sub.x * pos.y + end.x * start.y - end.y * start.x) / Os_sqrt(sub.x * sub.x + sub.y * sub.y);
}*/

static float Image4_getDistance(Vec2f pos, Vec2f start, Vec2f end, float* t, BOOL round)
{
	Vec2f v = Vec2f_sub(end, start);
	*t = Image4_getT(pos, start, v);

	if (round)
		*t = Std_fclamp(*t, 0, 1);
	else
		if (*t < 0 || *t > 1)
			return 1000000;

	return Vec2f_distance(Vec2f_add(start, Vec2f_mulV(v, *t)), pos);
}

static void _Image4_vectorAlign(Vec2i* start, Vec2i* end)
{
	Vec2i vo = Vec2i_sub(*end, *start);
	Vec2i v = Vec2i_init2(Std_abs(vo.x), Std_abs(vo.y));

	//repair
	if (v.x - v.y == 1)
		v.y++;
	if (v.y - v.x == 1)
		v.x++;

	//return sign
	if (vo.x < 0)
		v.x *= -1;
	if (vo.y < 0)
		v.y *= -1;

	*end = Vec2i_add(*start, v);
}

static void _Image4_drawLineEx(Image4* self, Vec2i start, Vec2i end, int width, Rgba cd, BOOL arrow)
{
	/*_Image4_vectorAlign(&start, &end);

	Vec2i sub = Vec2i_sub(end, start);
	Vec2f subR = Vec2f_init2(sub.y, -sub.x);

	if (arrow)
	{
		float hWidth = 0.5f;

		float xy[6];
		xy[0] = end.x + subR.x * hWidth;
		xy[1] = end.y + subR.y * hWidth;

		xy[2] = start.x;
		xy[3] = start.y;

		xy[4] = end.x - subR.x * hWidth;
		xy[5] = end.y - subR.y * hWidth;

		Image4_drawPolyFill(self, xy, 3, cd, 1);

		//napsat vzl᚝ funkci arrow() ...
	}
	else
	{
		float hWidth = width * 0.5f;

		if (sub.x == 0)
			Image4_drawBoxQuad(self, Quad2i_init4(start.x - hWidth, start.y, width, sub.y), cd);	//vertical
		else
			if (sub.y == 0)
				Image4_drawBoxQuad(self, Quad2i_init4(start.x, start.y - hWidth, sub.x, width), cd);	//horizontal
			else
			{
				subR = Vec2f_normalize(subR);

				float xy[8];
				xy[0] = start.x + subR.x * hWidth;
				xy[1] = start.y + subR.y * hWidth;

				xy[2] = start.x - subR.x * hWidth;
				xy[3] = start.y - subR.y * hWidth;

				xy[4] = end.x - subR.x * hWidth;
				xy[5] = end.y - subR.y * hWidth;

				xy[6] = end.x + subR.x * hWidth;
				xy[7] = end.y + subR.y * hWidth;

				Image4_drawPolyFill(self, xy, 4, cd, 1);
			}
	}
	return;*/

	_Image4_vectorAlign(&start, &end);

	float hWidth = width * 0.5f;

	Quad2i q = Quad2i_initSE(start, end);

	if (!arrow && q.size.x == 0)
		Image4_drawBoxQuad(self, Quad2i_init4(q.start.x - hWidth, q.start.y, width, q.size.y), cd);	//vertical
	else
		if (!arrow && q.size.y == 0)
			Image4_drawBoxQuad(self, Quad2i_init4(q.start.x, q.start.y - hWidth, q.size.x, width), cd);	//horizontal
		else
		{
			q = Quad2i_addSpace(q, -width);

			Vec2i pos = Quad2i_clamp(self->rect, q.start);
			Vec2i qend = Quad2i_clamp(self->rect, Quad2i_end(q));

			for (; pos.y < qend.y; pos.y++)
			{
				pos.x = q.start.x;
				for (; pos.x < qend.x; pos.x++)
				{
					float t;
					const float dist = Image4_getDistance(Vec2i_to2f(pos), Vec2i_to2f(start), Vec2i_to2f(end), &t, !arrow);
					const float edge = hWidth * (arrow ? t : 1);// +0.5f;
					if (dist <= edge)
					{
						float a = Std_fmin(edge - dist, 1);
						if (a > 0)
							Image4_setPixel(self, pos, cd, a);
					}
				}
			}
		}
}

void Image4_drawLine(Image4* self, Vec2i s, Vec2i e, int width, Rgba cd)
{
	_Image4_drawLineEx(self, s, e, width, cd, FALSE);
}

void Image4_drawArrow(Image4* self, Vec2i s, Vec2i e, int width, Rgba cd)
{
	_Image4_drawLineEx(self, s, e, width, cd, TRUE);
}

void Image4_drawBezier(Image4* self, Vec2f params[4], Rgba cd, int width)
{
	Vec2f old = Vec2f_bernstein(0, params);

	float i;
	for (i = 0; i <= 1; i += 0.001f)
	{
		Vec2f pos = Vec2f_bernstein(i, params);
		if (Vec2f_distance(pos, old) >= 3)
		{
			Image4_drawLine(self, Vec2f_to2i(old), Vec2f_to2i(pos), width, cd);
			old = pos;
		}
	}
}

void Image4_drawBezierArrow(Image4* self, Vec2f params[4], Rgba cd, int width)
{
	Vec2f old = Vec2f_bernstein(1, params);

	float i;
	for (i = 1; i >= 0; i -= 0.001f)
	{
		Vec2f pos = Vec2f_bernstein(i, params);
		if (Vec2f_distance(pos, old) >= width)
		{
			//rounding
			if (old.x >= pos.x - 2 && old.x <= pos.x + 2)	old.x = pos.x;
			if (old.y >= pos.y - 2 && old.y <= pos.y + 2)	old.y = pos.y;

			//draw arrow
			Image4_drawArrow(self, Vec2f_to2i(old), Vec2f_to2i(pos), width, cd);
			break;	//only once
		}
	}
}

void Image4_drawBezierArrowBack(Image4* self, Vec2f params[4], Rgba cd, int width)
{
	Vec2f old = Vec2f_bernstein(0, params);

	float i;
	for (i = 0; i < 1; i += 0.001f)
	{
		Vec2f pos = Vec2f_bernstein(i, params);
		if (Vec2f_distance(pos, old) >= width)
		{
			//rounding
			if (old.x >= pos.x - 2 && old.x <= pos.x + 2)	old.x = pos.x;
			if (old.y >= pos.y - 2 && old.y <= pos.y + 2)	old.y = pos.y;

			//draw arrow
			Image4_drawArrow(self, Vec2f_to2i(old), Vec2f_to2i(pos), width, cd);
			break;	//only once
		}
	}
}

void Image4_drawPolyLines(Image4* self, float* xy, const UBIG N, int width, Rgba cd)
{
	if (N < 2)
		return;

	BIG i;
	for (i = 0; i < N - 1; i++)
	{
		Image4_drawLine(self, Vec2i_init2(xy[0], xy[1]), Vec2i_init2(xy[2], xy[3]), width, cd);
		xy += 2;
	}

	//last to first one
	Image4_drawLine(self, Vec2i_init2(xy[N - 2], xy[N - 1]), Vec2i_init2(xy[0], xy[1]), width, cd);
}

int Image4_drawPolyFill_antialias_start(Image4* self, int startX, int startY, Vec2f lineS, Vec2f lineE, Rgba cd, float alpha)
{
	//return startX;
	if (startX - 3 > self->rect.start.x)
	{
		Vec2f sub = Vec2f_sub(lineE, lineS);
		int x = startX + 1;
		int n = 1;
		//while(x > self->rect.start.x && x > lineE.x && n > 0)
		for (x = startX - 3; x <= startX + 1; x++)
		{
			n = 0;
			float xx, yy;
			//for (yy = -0.25f; yy < 0.6f; yy += 0.5f)
			//	for (xx = -0.25f; xx < 0.6f; xx += 0.5f)
			for (yy = -0.33f; yy < 0.4f; yy += 0.33f)
				for (xx = -0.33f; xx < 0.4f; xx += 0.33f)
					n += (sub.y * (x + xx) - sub.x * (startY + yy) + lineE.x * lineS.y - lineE.y * lineS.x) < 0;

			Rgba* pos = Image4_getV(self, Vec2i_init2(x, startY));
			*pos = Rgba_aprox(*pos, cd, alpha * n / 9);

			//	x--;
		}
		startX += 2;
	}
	return startX;
}

int Image4_drawPolyFill_antialias_end(Image4* self, int endX, int endY, Vec2f lineS, Vec2f lineE, Rgba cd, float alpha)
{
	//return endX;
	if (endX + 3 < self->rect.start.x + self->rect.size.x)
	{
		Vec2f sub = Vec2f_sub(lineE, lineS);
		int x = endX - 1;
		int n = 1;
		//while (x < self->rect.start.x + self->rect.size.x && (x < lineS.x) && n > 0)
		for (x = endX - 1; x < endX + 3; x++)
		{
			n = 0;
			float xx, yy;
			//for (yy = -0.25f; yy < 0.6f; yy += 0.5f)
			//	for (xx = -0.25f; xx < 0.6f; xx += 0.5f)
			for (yy = -0.33f; yy < 0.4f; yy += 0.33f)
				for (xx = -0.33f; xx < 0.4f; xx += 0.33f)
					n += (sub.y * (x + xx) - sub.x * (endY + yy) + lineE.x * lineS.y - lineE.y * lineS.x) < 0;

			Rgba* pos = Image4_getV(self, Vec2i_init2(x, endY));
			*pos = Rgba_aprox(*pos, cd, alpha * n / 9);

			//	x++;
		}
		endX -= 1;
	}
	return endX;
}

typedef struct Image4PolyItem_s
{
	int nodex;
	Vec2f start;
	Vec2f end;
}Image4PolyItem;
typedef struct Image4Poly_s
{
	Image4PolyItem* items;
	int num;
	int alloc;
}Image4Poly;

Image4Poly Image4Poly_init(void)
{
	Image4Poly self;
	self.items = 0;
	self.num = 0;
	self.alloc = 0;
	return self;
}
void Image4Poly_free(Image4Poly* self)
{
	Os_free(self->items, self->alloc * sizeof(Image4PolyItem));
}
void Image4Poly_empty(Image4Poly* self)
{
	self->num = 0;
}
void Image4Poly_add(Image4Poly* self, int nodex, Vec2f start, Vec2f end)
{
	int old = self->num;

	self->num++;
	if (self->num >= self->alloc)
	{
		self->alloc = self->num + 30;
		self->items = Os_realloc(self->items, self->alloc * sizeof(Image4PolyItem));
	}

	self->items[old].nodex = nodex;
	self->items[old].start = start;
	self->items[old].end = end;
}
void Image4Poly_switch(Image4Poly* self, int i, int j)
{
	Image4PolyItem swap = self->items[i];
	self->items[i] = self->items[i + 1];
	self->items[i + 1] = swap;
}
void Image4Poly_short(Image4Poly* self)
{
	BIG i = 0;
	while (i < self->num - 1)
	{
		if (self->items[i].nodex > self->items[i + 1].nodex)
		{
			Image4Poly_switch(self, i, i + 1);
			if (i)
				i--;
		}
		else
			i++;
	}
}

void Image4_drawPolyFill(Image4* self, float* xy, const UBIG N, Rgba cd, const float alpha)
{
	Image4Poly poly = Image4Poly_init();

	const Vec2i start = self->rect.start;
	const Vec2i end = Quad2i_end(self->rect);

	//iterate all rows
	int pixelY;
	for (pixelY = start.y; pixelY < end.y; pixelY++)
	{
		//create list
		Image4Poly_empty(&poly);

		BIG j = N - 1;
		BIG i;
		for (i = 0; i < N; i++)
		{
			Vec2f ipoly = Vec2f_init2(xy[i * 2 + 0], xy[i * 2 + 1]);
			Vec2f jpoly = Vec2f_init2(xy[j * 2 + 0], xy[j * 2 + 1]);

			if ((ipoly.y < pixelY && jpoly.y >= pixelY) || (jpoly.y < pixelY && ipoly.y >= pixelY))
				Image4Poly_add(&poly, (ipoly.x + (pixelY - ipoly.y) / (jpoly.y - ipoly.y) * (jpoly.x - ipoly.x)), ipoly, jpoly);

			j = i;
		}

		//sort list
		Image4Poly_short(&poly);

		//draw
		for (i = 0; i < poly.num; i += 2)
		{
			int ns = poly.items[i].nodex;
			int ne = poly.items[i + 1].nodex;

			if (ns >= end.x)
				break;

			if (ne > start.x)
			{
				//antialias
				ns = Image4_drawPolyFill_antialias_start(self, ns, pixelY, poly.items[i].start, poly.items[i].end, cd, alpha);
				ne = Image4_drawPolyFill_antialias_end(self, ne, pixelY, poly.items[i + 1].start, poly.items[i + 1].end, cd, alpha);

				if (ns < start.x)	ns = start.x;
				if (ne > end.x)		ne = end.x;

				//fill
				Rgba* pos = Image4_getV(self, Vec2i_init2(ns, pixelY));
				Rgba* posE = pos + (ne - ns);
				while (pos < posE)
				{
					*pos = Rgba_aprox(*pos, cd, alpha);
					pos++;
				}
			}
		}
	}

	Image4Poly_free(&poly);
}

Quad2i Image4_getUnderline(Vec2i pos, BOOL centerText, Vec2i textSize)
{
	const int EXTRA_SPACE_X = 2;
	const int DOWN_Y = 2;

	Quad2i q;
	q.size = Vec2i_init2(textSize.x + 2 * EXTRA_SPACE_X, 1);

	if (centerText)
		q.start = Vec2i_init2(pos.x - textSize.x / 2 - EXTRA_SPACE_X, pos.y + textSize.y / 2 + DOWN_Y);
	else
		q.start = Vec2i_init2(pos.x - EXTRA_SPACE_X, pos.y + textSize.y / 2 + DOWN_Y);

	return q;
}

Quad2i Image4_drawTextCoord(Vec2i s, BOOL center, OsFont* font, const UNI* text, const int Hpx, const int betweenLinePx)
{
	int extra_down;
	Vec2i size = OsFont_getTextSize(font, text, Hpx, betweenLinePx, &extra_down);

	Vec2i align;
	align.x = s.x - size.x / 2;
	align.y = s.y - size.y / 2;

	Vec2i pos;
	pos.x = center ? align.x : s.x;
	pos.y = align.y;

	return Quad2i_init4(pos.x, pos.y, size.x, size.y);
}

void Image4_drawText(Image4* img, Vec2i s, BOOL center, OsFont* font, const UNI* text, const int Hpx, const int betweenLinePx, Rgba cd)
{
	int extra_down;
	Vec2i size = OsFont_getTextSize(font, text, Hpx, betweenLinePx, &extra_down);

	Vec2i align;
	align.x = s.x - size.x / 2;
	align.y = s.y - size.y / 2;

	Vec2i pos;
	pos.x = center ? align.x : s.x;
	pos.y = align.y;

	while (text && *text)
	{
		if (*text == '\t')
		{
			pos.x += TEXT_TAB_SPACES * Hpx;
		}
		else
			if (*text != '\n')
			{
				OsFontLetter l = OsFont_get(font, *text, Hpx);

				Vec2i p = Vec2i_init2(pos.x + l.move_x, pos.y - l.move_y + (size.y - l.img_h) - extra_down);

				Image1 t;
				t.size = Vec2i_init2(l.img_w, l.img_h);
				t.data = l.img;
				Image4_copyImage1(img, p, cd, &t);

				pos.x += l.m_len;
			}
			else //next line
			{
				pos.y += Hpx + betweenLinePx;
				pos.x = center ? align.x : s.x;
			}

		text++;
	}
}

void Image4_drawTextBackground(Image4* img, Vec2i s, BOOL center, OsFont* font, const UNI* text, const int Hpx, const int betweenLinePx, Rgba text_cd, Rgba back_cd, int addSpace)
{
	Quad2i coord = Image4_drawTextCoord(s, center, font, text, Hpx, betweenLinePx);
	coord = Quad2i_addSpace(coord, -addSpace);
	Image4_drawBoxQuad(img, coord, back_cd);

	Image4_drawText(img, s, center, font, text, Hpx, betweenLinePx, text_cd);
}

static const BOOL _Image4_isEndLine(UNI ch)
{
	return(ch == 0 || ch == '\n');
}

static const BOOL _Image4_isEndWord(UNI ch)
{
	return(ch == ' ' || ch == '\t' || _Image4_isEndLine(ch));
}

static const UNI* _Image4_getTextLine(const int max_len, OsFont* font, const UNI* text, const int Hpx)
{
	int len = 0;

	const UNI* start = text;
	const UNI* end = text;

	BOOL breakIt = FALSE;

	while (len < max_len && text && !breakIt)
	{
		//get word
		while (!_Image4_isEndWord(*text))
		{
			OsFontLetter l = OsFont_get(font, *text, Hpx);
			len += l.m_len;
			text++;
		}

		breakIt = _Image4_isEndLine(*text);

		if (*text)
			text++;

		if (len < max_len || end == start) //add first
		{
			end = text;
		}

		if (!breakIt)
			len += OsFont_get(font, ' ', Hpx).m_len; //space to next word
	}

	return end;
}

UBIG Image4_numTextLines(Quad2i coord, OsFont* font, const UNI* text, const int Hpx)
{
	UBIG n = 0;
	while (text && *text)
	{
		text = _Image4_getTextLine(coord.size.x, font, text, Hpx);
		n++;
	}
	return n;
}

const UNI* Image4_skipTextLines(Quad2i coord, OsFont* font, const UNI* text, const int Hpx, const UBIG skipLines)
{
	UBIG n = 0;
	while (text && *text && n < skipLines)
	{
		text = _Image4_getTextLine(coord.size.x, font, text, Hpx);
		n++;
	}
	return text;
}

int Image4_drawTextMulti(Image4* img, Quad2i coord, BOOL center, OsFont* font, const UNI* text, const int Hpx, const int betweenLinePx, Rgba cd)
{
	Vec2i pos = coord.start;

	while (text && *text)
	{
		const UNI* end = _Image4_getTextLine(coord.size.x, font, text, Hpx);
		while (text < end && !_Image4_isEndLine(*text))
		{
			//do word
			if (*text == '\t')
			{
				pos.x += TEXT_TAB_SPACES * Hpx;
			}
			else
			{
				OsFontLetter l = OsFont_get(font, *text, Hpx);
				Vec2i p = Vec2i_init2(pos.x + l.move_x, pos.y - l.move_y + (Hpx - l.img_h));

				Image1 t;
				t.size = Vec2i_init2(l.img_w, l.img_h);
				t.data = l.img;
				Image4_copyImage1(img, p, cd, &t);
				pos.x += l.m_len;
			}
			text++;
		}

		//next line
		pos.y += Hpx + betweenLinePx;
		pos.x = coord.start.x;

		text = end;
	}

	return(pos.y + Hpx + betweenLinePx) - coord.start.x; //sizeY of text
}

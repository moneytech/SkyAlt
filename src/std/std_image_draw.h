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

UCHAR Image1_getRectAvg(const Image1* img, Quad2f rect)
{
	float sum = 0;
	Vec2f one = Vec2f_init2(1, 1);

	Vec2i start = Vec2i_init2((int)rect.start.x, (int)rect.start.y);
	Vec2i end = Vec2i_init2((int)Std_roundUp(Quad2f_end(rect).x), (int)Std_roundUp(Quad2f_end(rect).y));

	Quad2i r = Quad2i_getIntersect(Image1_getSizeQuad(img), Quad2i_initEnd(start, end));
	start = r.start;
	end = Quad2i_end(r);
	const float rect_area = rect.size.x * rect.size.y;

	{
		Vec2i pos = start;
		for (; pos.y < end.y; pos.y++)
		{
			pos.x = start.x;
			{
				UCHAR* data = Image1_getV(img, pos);
				for (; pos.x < end.x; pos.x++)
				{
					float a = Quad2f_getIntersectArea(Quad2f_init2(Vec2i_to2f(pos), one), rect) / rect_area;
					sum += ((int)*data) * a;
					data++;
				}
			}
		}
	}

	return (UCHAR)sum;
}

void Image1_scale(Image1* dst, const Image1* src)
{
	Vec2f scale = Vec2f_init2(((float)src->size.x) / dst->size.x,
		((float)src->size.y) / dst->size.y);

	Vec2f scaleSize = Vec2f_init2(scale.x < 1 ? 1 : scale.x, scale.y < 1 ? 1 : scale.y);

	Vec2i pos = Vec2i_init();
	for (; pos.y < dst->size.y; pos.y++)
	{
		pos.x = 0;
		{
			UCHAR* data = Image1_getV(dst, pos);
			for (; pos.x < dst->size.x; pos.x++)
			{
				*data = Image1_getRectAvg(src, Quad2f_init2(Vec2f_mul(Vec2i_to2f(pos), scale), scaleSize));
				data++;
			}
		}
	}
}

void Image4_drawBoxStartEnd(Image4* self, Vec2i start, Vec2i end, Rgba cd)
{
	Image4_repairRect(self);
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

void Image4_drawBoxStartEndAlpha(Image4* self, Vec2i start, Vec2i end, Rgba cd)
{
	float alpha = cd.a / 255.0f;

	Image4_repairRect(self);
	start = Quad2i_clamp(self->rect, start);
	end = Quad2i_clamp(self->rect, end);

	for (; (start.y < end.y && start.y < self->size.y); start.y++)
	{
		int s = start.x;
		for (; (start.x < end.x && start.x < self->size.x); start.x++)
			*Image4_getV(self, start) = Rgba_aprox(*Image4_getV(self, start), cd, alpha);
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

Rgba Image4_getRectAvg(const Image4* img, Quad2f rect)
{
	Vec3f sum = Vec3f_init();
	Vec2f one = Vec2f_init2(1, 1);

	Vec2i start = Vec2i_init2((int)rect.start.x, (int)rect.start.y);
	Vec2i end = Vec2i_init2((int)Std_roundUp(Quad2f_end(rect).x), (int)Std_roundUp(Quad2f_end(rect).y));

	Quad2i r = Quad2i_getIntersect(Image4_getSizeQuad(img), Quad2i_initEnd(start, end));
	start = r.start;
	end = Quad2i_end(r);
	const float rect_area = rect.size.x * rect.size.y;

	{
		Vec2i pos = start;
		for (; pos.y < end.y; pos.y++)
		{
			pos.x = start.x;
			{
				Rgba* data = Image4_getV(img, pos);
				for (; pos.x < end.x; pos.x++)
				{
					float a = Quad2f_getIntersectArea(Quad2f_init2(Vec2i_to2f(pos), one), rect) / rect_area;
					sum = Vec3f_add(sum, Vec3f_mulV(Rgba_get3f(*data), a));
					data++;
				}
			}
		}
	}

	Rgba cd;
	cd.r = (UCHAR)sum.x;
	cd.b = (UCHAR)sum.y;
	cd.b = (UCHAR)sum.z;
	cd.a = 255;
	return cd;
}

void Image4_scale(Image4* dst, const Image4* src)
{
	Vec2f scale = Vec2f_init2(((float)src->size.x) / dst->size.x,
		((float)src->size.y) / dst->size.y);

	Vec2f scaleSize = Vec2f_init2(scale.x < 1 ? 1 : scale.x, scale.y < 1 ? 1 : scale.y);

	Vec2i pos = Vec2i_init();
	for (; pos.y < dst->size.y; pos.y++)
	{
		pos.x = 0;
		{
			Rgba* data = Image4_getV(dst, pos);
			for (; pos.x < dst->size.x; pos.x++)
			{
				*data = Image4_getRectAvg(src, Quad2f_init2(Vec2f_mul(Vec2i_to2f(pos), scale), scaleSize));
				data++;
			}
		}
	}
}

void Image4_blurScale(Image4* self)
{
	const int RAD = 4;

	Image4 low = Image4_initSize(Vec2i_mulV(self->size, 1.0f / RAD));

	Image4_scale(&low, self);	//down
	Image4_scale(self, &low);	//up

	Image4_free(&low);
}

void Image4_blurFast(Image4* dst, const Image4* src, int res)
{
	const float jump = Std_max(src->size.x, src->size.y) / res;

	int sx = Std_roundUp(src->size.x / jump);
	int sy = Std_roundUp(src->size.y / jump);
	BIG points_bytes = sx * sy * sizeof(Rgba);
	Rgba* points = (Rgba*)Os_malloc(points_bytes);

	//compute points(from src)
	int x, y;
	for (y = 0; y < sy; y++)
		for (x = 0; x < sx; x++)
		{
			points[y * sx + x] = Image4_getRectAvg(src, Quad2f_init4(x * jump, y * jump, jump, jump));
		}

	//blur
	Quad2i quad = Image4_getSizeQuad(dst);
	for (y = 0; y < sy - 1; y++)
	{
		for (x = 0; x < sx - 1; x++)
		{
			//corners(s=start, e=end, t=top, b=bottom)
			Rgba st = points[(y + 0) * sx + (x + 0)];
			Rgba et = points[(y + 0) * sx + (x + 1)];
			Rgba sb = points[(y + 1) * sx + (x + 0)];
			Rgba eb = points[(y + 1) * sx + (x + 1)];

			float xx, yy;
			for (yy = 0; yy < jump; yy++)
			{
				for (xx = 0; xx < jump; xx++)
				{
					Vec2i p = Vec2i_init2(x * jump + xx, y * jump + yy);
					if (Quad2i_inside(quad, p))
					{
						Rgba* data = Image4_getV(dst, p);
						*data = Rgba_aproxQuad(st, et, sb, eb, xx / jump, yy / jump);
					}
				}
			}
		}
	}

	Os_free(points, points_bytes);
}

void Image4_mulV(Image4* self, float t)
{
	Rgba* s = Image4_get(self, 0, 0);
	const Rgba* e = Image4_getLast(self);
	while (s < e)
	{
		Rgba_mulV(s, t);
		s++;
	}
}

void Image4_mulVSub(Image4* self, Vec2i start, Vec2i end, float t)
{
	start.x = Std_max(start.x, 0);
	start.y = Std_max(start.y, 0);

	for (; (start.y < end.y && start.y < self->size.y); start.y++)
	{
		int s = start.x;
		for (; (start.x < end.x && start.x < self->size.x); start.x++)
			Rgba_mulV(Image4_getV(self, start), t);
		start.x = s;
	}
}
void Image4_mulVSubQ(Image4* self, Quad2i coord, float t)
{
	Image4_mulVSub(self, coord.start, Quad2i_end(coord), t);
}

void Image4_copyImage1(Image4* self, Vec2i start, Rgba cd, const Image1* src)
{
	Image4_repairRect(self);

	Vec2i p;
	for (p.y = 0; p.y < src->size.y; p.y++)
	{
		for (p.x = 0; p.x < src->size.x; p.x++)
		{
			Vec2i pp = Vec2i_add(start, p);
			if (Quad2i_inside(self->rect, pp))
			{
				float a = *Image1_getV(src, p) / 255.0f;
				Rgba* d = Image4_getV(self, pp);
				*d = Rgba_aprox(*d, cd, a);
			}
		}
	}
}

void Image4_copyImage4(Image4* self, Vec2i start, Image4* src)
{
	Image4_repairRect(self);

	Vec2i p;
	for (p.y = 0; p.y < src->size.y; p.y++)
	{
		for (p.x = 0; p.x < src->size.x; p.x++)
		{
			Vec2i pp = Vec2i_add(start, p);
			if (Quad2i_inside(self->rect, pp))
			{
				Rgba* s = Image4_getV(src, p);
				Rgba* d = Image4_getV(self, pp);
				*d = Rgba_aprox(*d, *s, (s->a / 255.0f));
			}
		}
	}
}

//optimalization:
	//1) Compute only 1/4 copy other(mirrors)
	//2) compy start/end line and rest fill with full color
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
	Image4_repairRect(self);

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
	Image4_repairRect(self);

	Quad2i q = Quad2i_init4(mid.x - rad, mid.y - rad, rad * 2, rad * 2);
	q = Quad2i_getIntersect(self->rect, q);
	q = Quad2i_addSpace(q, -1);

	Image4_drawCircleLineRect(self, mid, rad, fat, cd, q);
}

/*void Image4_drawCircleShadowRect(Image4* self, Vec2i mid, int radIn, int radOut, float alpha, Quad2i q)
{
	q = Quad2i_getIntersect(q, Image4_getSizeQuad(self));

	const Vec2i qend = Quad2i_end(q);
	const float radDiff = radOut - radIn;

	//int last_y = 0;
	int ri = 0;
	Vec2i pos = q.start;
	for(; pos.y < qend.y; pos.y++)
	{
		pos.x = q.start.x;
		for(; pos.x < qend.x; pos.x++)
		{
			Vec2i sub = Vec2i_sub(pos, mid);
			float r = Vec2i_len(sub);

			if(r < radIn)
				*Image4_getV(self, pos) = Rgba_aprox(*Image4_getV(self, pos), Rgba_initBlack(), alpha);
			else
			if(r >= radOut)
				continue;
			else
			{
				float t = (r - radIn) / radDiff;	//ramp
				t += g_winIO->randArray[(ri++) % OsWinIO_MAX_RAND] * t*0.15f;
				if(t > 1)	t = 1;
				*Image4_getV(self, pos) = Rgba_aprox(*Image4_getV(self, pos), Rgba_initBlack(), (1.0f-t)*alpha);

				//if(pos.y > last_y)
				//	last_y = pos.y;
			}
		}
	}
}*/

/*void Image4_drawCircleShadow(Image4* self, Vec2i mid, int radIn, int radOut, float alpha)
{
	Quad2i q = Quad2i_init4(mid.x-radOut, mid.y-radOut, radOut*2, radOut*2);
	q = Quad2i_getIntersect(self->rect, q);
	q = Quad2i_addSpace(q, -1);

	Image4_drawCircleShadowRect(self, mid, radIn, radOut, alpha, q);
}*/

/*
void Image4_drawRampShadow(Image4* self, Quad2i coord, float alpha, BOOL leftRight, BOOL oposite)
{
	coord = Quad2i_getIntersect(coord, Image4_getSizeQuad(self));

	const Vec2i s = coord.start;
	const Vec2i e = Quad2i_end(coord);
	const Vec2f sz = Vec2i_to2f(Vec2i_sub(e, s));

	int ri = 0;
	Vec2i pos;
	for(pos.y=s.y; pos.y < e.y; pos.y++)
	{
		pos.x = s.x;
		for(pos.x=s.x; pos.x < e.x; pos.x++)
		{
			float t = leftRight ? ((pos.x-s.x) / sz.x) : ((pos.y-s.y) / sz.y);
			t += g_winIO->randArray[(ri++) % OsWinIO_MAX_RAND] * t*0.15f;
			if(t > 1)	t = 1;
			*Image4_getV(self, pos) = Rgba_aprox(*Image4_getV(self, pos), Rgba_initBlack(), (oposite?(1-t):t)*alpha);
		}
	}
}*/

/*void Image4_drawRBoxShadow(Image4* self, Quad2i coord, int rad, float alpha, BOOL rounded)
{
	coord = Quad2i_addSpace(coord, rad);
	Vec2i s = coord.start;
	Vec2i e = Quad2i_end(coord);
	//rad*=2;

	Image4_drawCircleShadowRect(self, s, 0, rad, alpha, Quad2i_init4(s.x-rad, s.y-rad, rad, rad));
	Image4_drawCircleShadowRect(self, e, 0, rad, alpha, Quad2i_init4(e.x, e.y, rad, rad));
	Image4_drawCircleShadowRect(self, Vec2i_init2(e.x, s.y), 0, rad, alpha, Quad2i_init4(e.x, s.y-rad, rad, rad));
	Image4_drawCircleShadowRect(self, Vec2i_init2(s.x, e.y), 0, rad, alpha, Quad2i_init4(s.x-rad, e.y, rad, rad));

	if(rounded)
		Image4_drawBoxQuadAlpha(self, coord, Rgba_init4(0, 0, 0, 255*alpha));	//middle

	Image4_drawRampShadow(self, Quad2i_init4(s.x-rad, s.y, rad, e.y-s.y), alpha, TRUE, FALSE);	//left
	Image4_drawRampShadow(self, Quad2i_init4(e.x, s.y, rad, e.y-s.y), alpha, TRUE, TRUE);		//right
	Image4_drawRampShadow(self, Quad2i_init4(s.x, s.y-rad, e.x-s.x, rad), alpha, FALSE, FALSE);	//top
	Image4_drawRampShadow(self, Quad2i_init4(s.x, e.y, e.x-s.x, rad), alpha, FALSE, TRUE);		//bottom
}*/

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

static float Image4_getT(Vec2f pos, Vec2f start, Vec2f v)
{
	return ((pos.x - start.x) * v.x + (pos.y - start.y) * v.y) / Vec2f_dot(v, v);
}

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
	_Image4_vectorAlign(&start, &end);

	float half_fat = width * 0.5f;

	Quad2i q = Quad2i_initSE(start, end);
	q = Quad2i_addSpace(q, -width);
	Vec2i qend = Quad2i_end(q);

	Vec2i pos = q.start;
	for (; pos.y < qend.y; pos.y++)
	{
		pos.x = q.start.x;
		for (; pos.x < qend.x; pos.x++)
		{
			float t;
			const float dist = Image4_getDistance(Vec2i_to2f(pos), Vec2i_to2f(start), Vec2i_to2f(end), &t, !arrow);

			const float edge = half_fat * (arrow ? t : 1);// +0.5f;

			if (dist <= edge)
			{
				float r = edge - dist;
				float a = Std_fmin(r, 1);

				if (a > 0 && Quad2i_inside(self->rect, pos))
					Image4_setPixel(self, pos, cd, a);
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

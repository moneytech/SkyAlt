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

typedef struct GuiItemParticles_s
{
	GuiItem base;

	Vec2f* poses;
	Vec2f* vels;
	float* alphas;
	int num;
	double time;
	Image1 noiseX;
	Image1 noiseY;

	BIG num_draw;

	Quad2i old_rect;

	Image1 logoOrig;
	Image1 logo;

	double anim_max_time; //zero = deactivated
	double anim_act_time;

	BOOL showInfo;

	const UNI* textStatus;
	float oldDone;

	float restTimeStart;
	float restTimeResult;
} GuiItemParticles;

GuiItemParticles* GuiItemParticles_new(Quad2i grid, const Image1 logo, BOOL showInfo)
{
	GuiItemParticles* self = Os_malloc(sizeof(GuiItemParticles));
	self->base = GuiItem_init(GuiItem_PARTICLES, grid);

	self->logoOrig = Image1_initCopy(&logo);
	self->logo = Image1_init();

	self->noiseX = Image1_initSize(logo.size);
	self->noiseY = Image1_initSize(logo.size);

	self->poses = 0;
	self->vels = 0;
	self->alphas = 0;

	self->num_draw = 0;
	self->old_rect = Quad2i_init();

	self->anim_max_time = 0;
	self->anim_act_time = 0;
	self->time = 0;

	self->textStatus = 0;
	self->oldDone = 0;

	self->restTimeStart = 0;
	self->restTimeResult = 0;

	self->showInfo = showInfo;
	return self;
}

void GuiItemParticles_clear(GuiItemParticles* self)
{
	Os_free(self->poses, self->num * sizeof(Vec2f));
	Os_free(self->vels, self->num * sizeof(Vec2f));
	Os_free(self->alphas, self->num * sizeof(float));
	self->poses = 0;
	self->vels = 0;
	self->alphas = 0;
	self->num = 0;

	self->time = 0;
}

void GuiItemParticles_delete(GuiItemParticles* self)
{
	GuiItemParticles_clear(self);

	Image1_free(&self->logoOrig);
	Image1_free(&self->logo);

	Image1_free(&self->noiseX);
	Image1_free(&self->noiseY);

	GuiItem_free(&self->base);
	Os_free(self, sizeof(GuiItemParticles));
}

static void _GuiItemParticles_resizeLogo(GuiItemParticles* self, Vec2i size)
{
	if (size.y != self->logo.size.y)
	{
		size.x = self->logoOrig.size.x * (size.y / (float)self->logoOrig.size.y);

		Image1_resize(&self->logo, size);
		Image1_scale(&self->logo, &self->logoOrig);
	}
}

Vec2i _GuiItemParticles_getLogoSize(const GuiItemParticles* self)
{
	Vec2i size = self->base.coordScreen.size;

	if (self->showInfo)
		size.y -= OsWinIO_cellSize();

	return size;
}

void GuiItemParticles_emit(GuiItemParticles* self)
{
	GuiItemParticles_clear(self);
	GuiItem_setRedraw(&self->base, TRUE);

	int x, y, i, j;

	//resize
	_GuiItemParticles_resizeLogo(self, _GuiItemParticles_getLogoSize(self));

	//get num particles
	UBIG n = 0;
	for (y = 0; y < self->logo.size.y; y++)
		for (x = 0; x < self->logo.size.x; x++)
			if (*Image1_get(&self->logo, x, y))
				n++;

	const int SUBS = 3;
	n *= (SUBS * SUBS);

	//alloc
	self->poses = Os_malloc(n * sizeof(Vec2f));
	self->vels = Os_malloc(n * sizeof(Vec2f));
	self->alphas = Os_malloc(n * sizeof(float));
	self->num = n;
	self->time = 0;

	Vec2i st;
	UCHAR rnd;
	OsCrypto_random(sizeof(rnd), &rnd);
	st = Vec2i_init2(rnd, rnd);
	Image1_drawNoise(&self->noiseX, st, 30, 6, 0.5f);
	st = Vec2i_add(st, self->logo.size);
	Image1_drawNoise(&self->noiseY, st, 30, 6, 0.5f);

	//set data
	n = 0;
	for (y = 0; y < self->logo.size.y; y++)
		for (x = 0; x < self->logo.size.x; x++)
		{
			UCHAR* cd = Image1_get(&self->logo, x, y);
			if (*cd)
			{
				for (i = 0; i < SUBS; i++)
					for (j = 0; j < SUBS; j++)
					{
						self->poses[n] = Vec2f_init2(x + 1.0f / SUBS * i, y + 1.0f / SUBS * j);
						self->vels[n] = Vec2f_init();
						self->alphas[n] = *cd / -255.0f; //negative = not active
						n++;
					}
			}
		}
	self->num_draw = n;
	self->old_rect = Quad2i_init();
}

void GuiItemParticles_startAnim(GuiItemParticles* self, double time_sec)
{
	self->anim_max_time = time_sec;
	self->anim_act_time = 0;
	GuiItemParticles_emit(self); //, 1);
}

static float _GuiItemParticles_updateTime(GuiItemParticles* self)
{
	double t = Os_time();
	double dt = t - self->time;
	self->time = t;
	if (dt > 0.04f) //first tick
		dt = 0.04f; //at least 25fps
	return dt;
}

static Vec2i _GuiItemParticles_getMidStart(GuiItemParticles* self, Quad2i coord)
{
	return Vec2i_add(coord.start, Vec2i_mulV(Vec2i_sub(coord.size, self->logo.size), 0.5f));
}

static float _GuiItemParticles_getDone(GuiItemParticles* self, const float DT)
{
	if (self->anim_max_time)
	{
		self->anim_act_time += DT;
		StdProgress_setEx(0, self->anim_act_time, self->anim_max_time);
	}

	const UNI* oldStatus = self->textStatus;

	const char* trans = StdProgress_getTranslationID();
	self->textStatus = trans ? Lang_find(trans) : 0;

	float time = Os_time();

	const float done = StdProgress_get();
	if (oldStatus != self->textStatus || done < self->oldDone)
	{
		GuiItemParticles_emit(self);
		self->restTimeStart = time;
	}

	if (done)
		self->restTimeResult = (time - self->restTimeStart) / done * (1 - done);

	self->oldDone = done;
	return done;
}

void GuiItemParticles_draw(GuiItemParticles* self, Image4* img, Quad2i coord, Win* win)
{
	//redraw logo with text info
	//Quad2i q = coord;
	//q.size.y += OsWinIO_cellSize() * 2;	//text
	//img->rect = Quad2i_extend(img->rect, q);

	int i;
	Vec2i startDraw = _GuiItemParticles_getMidStart(self, coord);

	Rgba logoCd = self->num_draw == 0 ? self->base.front_cd : self->base.back_cd;
	Image4_copyImage1(img, startDraw, logoCd, &self->logo);

	if (self->num_draw == 0)
		return;

	//Text
	if (self->showInfo)
	{
		const int cell = OsWinIO_cellSize();
		int textH = cell / 2.5;
		Vec2i pos;
		pos.y = coord.size.y - cell / 2;
		pos.x = coord.size.x / 2;

		//Filtring(10% - 5sec)
		{
			char prc[32];
			snprintf(prc, 32, "(%.1f%% - %.1fsec)", StdProgress_get() * 100, self->restTimeResult);
			UNI str[64];
			Std_copyUNI(str, 64, self->textStatus);
			Std_copyUNI_char(str + Std_sizeUNI(str), 64 - Std_sizeUNI(str), prc);

			Image4_drawTextBackground(img, Vec2i_add(coord.start, pos), TRUE, OsWinIO_getFontDefault(), str, textH, 0, self->base.front_cd, logoCd, 2);
		}
	}

	const Quad2i winRect = Quad2i_init2(Vec2i_init(), Win_getImage(win).size);

	//draw noise
	/*int x, y;
	for(y=0; y < coord.size.y; y++)
		for(x=0; x < coord.size.x; x++)
		{
			int a = Image1_getPosSmoothRepeat(&self->noiseX, Vec2i_init2(x-100, y-100));
			*Image4_get(img, coord.start.x+x, coord.start.y+y) = Rgba_init4(a, a, 0, 255);
		}*/

	Rgba* last_data = 0;
	for (i = 0; i < self->num; i++)
	{
		//Vec2i p = Vec2i_add(startDraw, Vec2f_to2i(self->poses[i]));	//bug
		Vec2i p;
		p.x = startDraw.x + self->poses[i].x;
		p.y = startDraw.y + self->poses[i].y;

		float a = Std_fabs(self->alphas[i]);
		if (a > 0.01f && Quad2i_inside(winRect, p))
		{
			Rgba* data = Image4_get(img, p.x, p.y);
			if (data != last_data) //one particles per pixel(in row)
				*data = Rgba_aprox(*data, self->base.front_cd, a);
			last_data = data;
		}
	}
}

void GuiItemParticles_update(GuiItemParticles* self, Quad2i coord, Win* win)
{
	if (_GuiItemParticles_getLogoSize(self).y != self->logo.size.y)
		GuiItemParticles_emit(self);

	//note: not here because update() has low FPS
}

void GuiItemParticles_touch(GuiItemParticles* self, Quad2i coord, Win* win)
{
	int i;
	Rgba back_cd = Rgba_aprox(g_theme.main, g_theme.white, 0.5f);
	Rgba front_cd = g_theme.black;
	_GuiItem_updateFinalCd(&self->base, back_cd, front_cd, coord, win);

	//if (!self->base.touch)
	//	return;

	BOOL startTouch = (OsWinIO_getTouch_action() == Win_TOUCH_DOWN_S) || (OsWinIO_getTouch_action() == Win_TOUCH_FORCE_DOWN_S);
	BOOL active = OsWinIO_isActiveRenderItem(self);
	BOOL inside = Quad2i_inside(coord, OsWinIO_getTouchPos());
	BOOL touch = startTouch || active;

	if (inside && touch) //full touch
		GuiItem_callClick(&self->base);

	const float DT = _GuiItemParticles_updateTime(self);
	const float FADE = DT * 0.3f;

	float done = _GuiItemParticles_getDone(self, DT);
	const float edge = self->logo.size.x * (1.0f - done);

	if (edge == self->logo.size.x) //edge is on right side = nothing to simulate
		return;

	self->num_draw = 0;

	Vec2i startDraw = _GuiItemParticles_getMidStart(self, coord);
	Quad2i rect = Quad2i_init();// Quad2i_init2(Vec2i_init(), self->logo.size); //logo
	//rect.size.y += OsWinIO_cellSize();	//text

	for (i = 0; i < self->num; i++)
	{
		Vec2f* p = &self->poses[i];
		float* alp = &self->alphas[i];

		if (p->x > edge)
			*alp = Std_fabs(*alp); //activate

		if (*alp > 0)
		{
			Vec2f* v = &self->vels[i];

			Vec2i ppp = Vec2f_to2i(*p);
			Vec2f acc = Vec2f_init2(Image1_getPosSmoothRepeat(&self->noiseX, ppp) - 128,
				Image1_getPosSmoothRepeat(&self->noiseY, ppp) - 128);

			*v = Vec2f_add(*v, Vec2f_mulV(acc, DT)); //v += acc * dt;
			*p = Vec2f_add(*p, Vec2f_mulV(*v, DT)); //p += v * dt;

			*alp -= FADE;
			if (*alp < 0)
				*alp = 0;
		}

		if (*alp != 0)
		{
			rect = (self->num_draw == 0) ? Quad2i_init4(p->x, p->y, 1, 1) : Quad2i_extend2(rect, Vec2f_to2i(*p));
			self->num_draw++;
		}
	}

	Quad2i rectBackup = rect;
	rect = Quad2i_extend(rect, self->old_rect);
	self->old_rect = rectBackup;

	if (self->num_draw)
	{
		rect.start = Vec2i_add(rect.start, startDraw);
		rect = Quad2i_addSpace(rect, -1); //px=-0.1 is rounder to 0(not -1), so this fixes that

		//redraw logo with text info
		//Quad2i q = coord;
		//q.size.y += OsWinIO_cellSize();	//text
		//rect = Quad2i_extend(rect, q);

		rect = Quad2i_extend(rect, coord);

		GuiItemRoot_addBufferRect(rect);
	}

	if (self->num_draw == 0) //done
	{
		GuiItemParticles_clear(self);
		GuiItem_setRedraw(&self->base, TRUE);

		GuiItem_callClick(&self->base);
	}
}

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

typedef struct GuiScroller_s
{
	BIG wheel; //pixel move

	UBIG data_height;
	UBIG screen_height;
	UBIG item_height;

	BOOL clickFront;
	int clickRel;

	Rgba cd;

	BOOL redraw;

	//BOOL sendScrollDown;

	InterObject* value;

	double timeWheel;
} GuiScroller;

static UBIG GuiScroller_getWheel(const GuiScroller* self)
{
	return (self->data_height > self->screen_height) ? Std_bclamp(self->wheel, 0, (self->data_height - self->screen_height)) : 0;
	//return self->wheel;
}

UBIG GuiScroller_getWheelRow(const GuiScroller* self)
{
	return GuiScroller_getWheel(self) / self->item_height;
}

void GuiScroller_setWheelDirect(GuiScroller* self, BIG wheel)
{
	self->wheel = wheel;
	InterObject_setNumber(self->value, wheel);
}

void GuiScroller_setWheel(GuiScroller* self, BIG wheelPixel)
{
	UBIG oldWheel = self->wheel;

	self->wheel = (self->data_height > self->screen_height) ? Std_bclamp(wheelPixel, 0, (self->data_height - self->screen_height)) : 0;

	if (oldWheel != self->wheel)
	{
		//BOOL reset = !GuiItemRoot_hasChanges();
		InterObject_setNumber(self->value, self->wheel);
		//if (reset)
		//	GuiItemRoot_resetNumChanges();

		self->timeWheel = Os_time();
	}
}

void GuiScroller_setWheelRow(GuiScroller* self, BIG row)
{
	row *= self->item_height;
	row = Std_bclamp(row, 0, self->data_height);

	GuiScroller_setWheel(self, row);
}

void GuiScroller_set(GuiScroller* self, UBIG data_height, UBIG screen_height, UBIG item_height)
{
	self->data_height = data_height;
	self->screen_height = screen_height;
	self->item_height = item_height;

	if (self->item_height == 0)
		self->item_height = 1;

	//GuiScroller_setWheel(self, self->wheel);
}

void GuiScroller_setValue(GuiScroller* self, InterObject* value)
{
	self->value = value;
	self->wheel = InterObject_getNumber(self->value);
}

GuiScroller GuiScroller_init(void)
{
	GuiScroller self;
	self.clickFront = FALSE;
	self.clickRel = 0;
	self.redraw = TRUE;
	self.wheel = 0;
	self.value = 0;
	self.data_height = 1;
	self.screen_height = 1;
	self.item_height = 1;

	//self.sendScrollDown = TRUE;
	self.timeWheel = 0;

	self.cd = g_theme.main;

	return self;
}

GuiScroller GuiScroller_initCopy(GuiScroller* src)
{
	GuiScroller self = *src;
	return self;
}

void GuiScroller_free(GuiScroller* self)
{
	//DbValue_setNumber(&self->value, self->wheel);
	//DbValue_free(&self->value);
}

int GuiScroller_widthWin(Win* win)
{
	return OsWinIO_cellSize() / 2;
}


BOOL GuiScroller_is(const GuiScroller* self)
{
	return(self->data_height > self->screen_height);
}

BOOL GuiScroller_getRedrawAndReset(GuiScroller* self)
{
	BIG orig_wheel = GuiScroller_getWheel(self);
	self->wheel = InterObject_getNumber(self->value);
	self->redraw |= (orig_wheel != GuiScroller_getWheel(self));

	BOOL r = self->redraw;
	self->redraw = FALSE;
	return r;
}

static void _GuiScroller_updateV(const GuiScroller* self, Vec2i start, Win* win, Quad2i* outSlider)
{
	if (self->data_height <= self->screen_height)
	{
		outSlider->start = start;

		outSlider->size.x = GuiScroller_widthWin(win);
		outSlider->size.y = self->screen_height;
	}
	else
	{
		outSlider->start.x = start.x;
		outSlider->start.y = start.y + self->screen_height * (GuiScroller_getWheel(self) / (double)self->data_height);

		outSlider->size.x = GuiScroller_widthWin(win);
		outSlider->size.y = self->screen_height * (self->screen_height / (double)self->data_height);
	}
}

static void _GuiScroller_updateH(const GuiScroller* self, Vec2i start, Win* win, Quad2i* outSlider)
{
	if (self->data_height <= self->screen_height)
	{
		outSlider->start = start;

		outSlider->size.x = self->screen_height;
		outSlider->size.y = GuiScroller_widthWin(win);
	}
	else
	{
		outSlider->start.x = start.x + self->screen_height * (GuiScroller_getWheel(self) / (double)self->data_height);
		outSlider->start.y = start.y;

		outSlider->size.x = self->screen_height * (self->screen_height / (double)self->data_height);
		outSlider->size.y = GuiScroller_widthWin(win);
	}
}

static Rgba _GuiScroller_getSlideCd(const GuiScroller* self)
{
	Rgba cd_slide = self->cd;
	if (self->data_height <= self->screen_height)
		cd_slide = Rgba_aprox(g_theme.background, g_theme.main, 0.5f);	//disable

	return cd_slide;
}

void GuiScroller_drawV(GuiScroller* self, Vec2i start, Image4* img, Win* win)
{
	Quad2i slider;
	_GuiScroller_updateV(self, start, win, &slider);

	slider.start.x += 2;
	slider.size.x -= 4;

	//make scroll visible if there is a lot of records(items)
	if (slider.size.y == 0)
	{
		const int c = OsWinIO_cellSize() / 4;
		slider.start.y -= c / 2;
		slider.size.y += c;
	}

	Image4_drawBoxQuad(img, slider, _GuiScroller_getSlideCd(self));
}

void GuiScroller_drawH(GuiScroller* self, Vec2i start, Image4* img, Win* win)
{
	Quad2i slider;
	_GuiScroller_updateH(self, start, win, &slider);

	slider.start.y += 2;
	slider.size.y -= 4;

	//make scroll visible if there is a lot of records(items)
	if (slider.size.x == 0)
	{
		const int c = OsWinIO_cellSize() / 4;
		slider.start.x -= c / 2;
		slider.size.x += c;
	}

	Image4_drawBoxQuad(img, slider, _GuiScroller_getSlideCd(self));
}

static BIG _GuiScroller_getTempScroll(GuiScroller* self, const int srcl)
{
	BIG diff = (self->item_height == 1) ? OsWinIO_cellSize() : self->item_height;
	return diff * srcl;
}

void GuiScroller_touchV(GuiScroller* self, void* parent, Quad2i parentCoord, Vec2i start, Win* win)
{
	BIG orig_wheel = GuiScroller_getWheel(self);
	//self->wheel = DbValue_getNumber(&self->value);

	

	BOOL insideParent = Quad2i_inside(parentCoord, OsWinIO_getTouchPos());
	if (OsWinIO_getTouch_action() == Win_TOUCH_WHEEL && insideParent && !(OsWinIO_getKeyExtra() & Win_EXTRAKEY_SHIFT))
	{
		BIG scrll = OsWinIO_getTouch_wheel();
		//if (OsWinIO_getKeyEXTRA() & Win_EXTRAKEY_ALT)
		//	scrll *= GuiScroller_getMaxRows(self) / 10;
		//else
		//if (OsWinIO_getKeyEXTRA() & Win_EXTRAKEY_SHIFT)
		//	scrll *= GuiScroller_getMaxRows(self) / 100;

		GuiScroller_setWheel(self, GuiScroller_getWheel(self) + _GuiScroller_getTempScroll(self, scrll));
		if (orig_wheel != GuiScroller_getWheel(self)) //let parent scroll
			OsWinIO_resetTouch();

		//if (!self->sendScrollDown)
		//	OsWinIO_resetTouch();
	}

	if (!GuiScroller_is(self))
		return;

	Quad2i sliderFront;
	_GuiScroller_updateV(self, start, win, &sliderFront);
	const int midSlider = sliderFront.size.y / 2;

	BOOL startTouch = (OsWinIO_getTouch_action() == Win_TOUCH_DOWN_S) || (OsWinIO_getTouch_action() == Win_TOUCH_FORCE_DOWN_S);
	BOOL endTouch = (OsWinIO_getTouch_action() == Win_TOUCH_DOWN_E) || (OsWinIO_getTouch_action() == Win_TOUCH_FORCE_DOWN_E);

	BOOL inside = Quad2i_inside(sliderFront, OsWinIO_getTouchPos());
	BOOL touch = startTouch || OsWinIO_isActiveRenderItem(parent);

	if (startTouch)
	{
		self->clickFront = (inside && touch);
		self->clickRel = OsWinIO_getTouchPos().y - sliderFront.start.y - midSlider; //rel to middle of front slide
	}

	if (inside && touch) //full touch
		OsWinIO_setActiveRenderItem(parent);
	else
		if (inside && !touch) //mid color
		{
			//front_cd = Rgba_aprox(back_cd, front_cd, 0.8f);
		}

	if (endTouch)
		self->clickFront = FALSE;

	Quad2i sliderBack;
	sliderBack.start = start;
	sliderBack.size = Vec2i_init2(GuiScroller_widthWin(win), self->screen_height);

	if (self->clickFront) //click on slider
	{
		double mid = (OsWinIO_getTouchPos().y - start.y) - midSlider - self->clickRel;
		GuiScroller_setWheel(self, (mid / self->screen_height) * self->data_height);

		self->redraw = TRUE;
	}
	else
		if (startTouch && Quad2i_inside(sliderBack, OsWinIO_getTouchPos()) && !Quad2i_inside(sliderFront, OsWinIO_getTouchPos())) //click(once) on background
		{
			double mid = (OsWinIO_getTouchPos().y - start.y) - midSlider;
			GuiScroller_setWheel(self, (mid / self->screen_height) * self->data_height);

			//switch to 'click on slider'
			self->clickFront = TRUE;
			self->clickRel = 0;

			self->redraw = TRUE;
		}

	if (endTouch)
		OsWinIO_resetActiveRenderItem();

	self->redraw |= (orig_wheel != GuiScroller_getWheel(self));
}

void GuiScroller_touchH(GuiScroller* self, void* parent, Quad2i parentCoord, Vec2i start, Win* win, BOOL needShiftWheel)
{
	BIG orig_wheel = GuiScroller_getWheel(self);
	//self->wheel = DbValue_getNumber(&self->value);


	BOOL insideParent = Quad2i_inside(parentCoord, OsWinIO_getTouchPos());
	if (OsWinIO_getTouch_action() == Win_TOUCH_WHEEL && insideParent && (!needShiftWheel || OsWinIO_getKeyExtra() & Win_EXTRAKEY_SHIFT))
	{
		BIG scrll = OsWinIO_getTouch_wheel();

		GuiScroller_setWheel(self, GuiScroller_getWheel(self) + _GuiScroller_getTempScroll(self, scrll));
		if (orig_wheel != GuiScroller_getWheel(self)) //let parent scroll
			OsWinIO_resetTouch();

		//if (!self->sendScrollDown)
		//	OsWinIO_resetTouch();
	}

	if (!GuiScroller_is(self))
		return;

	Quad2i sliderFront;
	_GuiScroller_updateH(self, start, win, &sliderFront);
	const int midSlider = sliderFront.size.x / 2;

	BOOL startTouch = (OsWinIO_getTouch_action() == Win_TOUCH_DOWN_S) || (OsWinIO_getTouch_action() == Win_TOUCH_FORCE_DOWN_S);
	BOOL endTouch = (OsWinIO_getTouch_action() == Win_TOUCH_DOWN_E) || (OsWinIO_getTouch_action() == Win_TOUCH_FORCE_DOWN_E);

	BOOL inside = Quad2i_inside(sliderFront, OsWinIO_getTouchPos());
	BOOL touch = startTouch || OsWinIO_isActiveRenderItem(parent);

	if (startTouch)
	{
		self->clickFront = (inside && touch);
		self->clickRel = OsWinIO_getTouchPos().x - sliderFront.start.x - midSlider; //rel to middle of front slide
	}

	if (inside && touch) //full touch
		OsWinIO_setActiveRenderItem(parent);
	else
		if (inside && !touch) //mid color
		{
			//front_cd = Rgba_aprox(back_cd, front_cd, 0.8f);
		}

	if (endTouch)
		self->clickFront = FALSE;

	Quad2i sliderBack;
	sliderBack.start = start;
	sliderBack.size = Vec2i_init2(self->screen_height, GuiScroller_widthWin(win));

	if (self->clickFront) //click on slider
	{
		double mid = (OsWinIO_getTouchPos().x - start.x) - midSlider - self->clickRel;
		GuiScroller_setWheel(self, (mid / self->screen_height) * self->data_height);

		self->redraw = TRUE;
	}
	else
		if (startTouch && Quad2i_inside(sliderBack, OsWinIO_getTouchPos()) && !Quad2i_inside(sliderFront, OsWinIO_getTouchPos())) //click(once) on background
		{
			double mid = (OsWinIO_getTouchPos().x - start.x) - midSlider;
			GuiScroller_setWheel(self, (mid / self->screen_height) * self->data_height);

			//switch to 'click on slider'
			self->clickFront = TRUE;
			self->clickRel = 0;

			self->redraw = TRUE;
		}

	if (endTouch)
		OsWinIO_resetActiveRenderItem();

	self->redraw |= (orig_wheel != GuiScroller_getWheel(self));
}

BOOL GuiScroller_tryDragScroll(GuiScroller* self, const int fast_dt, const int sign)
{
	BIG wheelOld = GuiScroller_getWheel(self);

	const double dt = (1.0 / 2) / fast_dt;

	if (Os_time() - self->timeWheel > dt)
		GuiScroller_setWheel(self, GuiScroller_getWheel(self) + _GuiScroller_getTempScroll(self, sign));

	return GuiScroller_getWheel(self) != wheelOld;
}

BOOL GuiScroller_dragScrollV(GuiScroller* self, Quad2i coord)
{
	const int cell = OsWinIO_cellSize();
	Vec2i pos = OsWinIO_getTouchPos();

	//vertical top
	if (Quad2i_inside(Quad2i_init4(coord.start.x, coord.start.y, coord.size.x, cell), pos))
		return GuiScroller_tryDragScroll(self, (pos.y < coord.start.y + cell / 2) ? 3 : 1, -1);

	//vertical bottom
	if (Quad2i_inside(Quad2i_init4(coord.start.x, coord.start.y + coord.size.y - cell, coord.size.x, cell), pos))
		return GuiScroller_tryDragScroll(self, (pos.y > coord.start.y + coord.size.y - cell / 2) ? 3 : 1, +1);

	return FALSE;
}

BOOL GuiScroller_dragScrollH(GuiScroller* self, Quad2i coord)
{
	const int cell = OsWinIO_cellSize();
	Vec2i pos = OsWinIO_getTouchPos();

	//vertical top
	if (Quad2i_inside(Quad2i_init4(coord.start.x, coord.start.y, cell, coord.size.y), pos))
		return GuiScroller_tryDragScroll(self, (pos.x < coord.start.x + cell / 2) ? 3 : 1, -1);

	//vertical bottom
	if (Quad2i_inside(Quad2i_init4(coord.start.x + coord.size.x - cell, coord.start.y, cell, coord.size.y), pos))
		return GuiScroller_tryDragScroll(self, (pos.x > coord.start.x + coord.size.x - cell / 2) ? 3 : 1, +1);

	return FALSE;
}

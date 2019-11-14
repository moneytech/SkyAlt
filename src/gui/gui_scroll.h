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

typedef struct GuiScroll_s
{
	BIG wheel; //pixel move

	UBIG data_height;
	UBIG screen_height;
	UBIG item_height;

	BOOL clickFront;
	int clickRel;

	BOOL redraw;

	BOOL sendScrollDown;

	DbValue value;
} GuiScroll;

static UBIG GuiScroll_getWheel(const GuiScroll* self)
{
	return self->wheel;
}

UBIG GuiScroll_getWheelRow(const GuiScroll* self)
{
	return GuiScroll_getWheel(self) / self->item_height;
}

void GuiScroll_setWheelDirect(GuiScroll* self, BIG wheel)
{
	self->wheel = wheel;
}

void GuiScroll_setWheel(GuiScroll* self, BIG wheelPixel)
{
	UBIG oldWheel = self->wheel;
	self->wheel = (self->data_height > self->screen_height) ? Std_bclamp(wheelPixel, 0, (self->data_height - self->screen_height)) : 0;

	if (oldWheel != self->wheel)
	{
		BOOL reset = !GuiItemRoot_hasChanges();
		DbValue_setNumber(&self->value, self->wheel);
		if(reset)
			GuiItemRoot_resetNumChanges();
	}
}

void GuiScroll_setWheelRow(GuiScroll* self, BIG row)
{
	row *= self->item_height;
	row = Std_bclamp(row, 0, self->data_height);

	GuiScroll_setWheel(self, row);
}

void GuiScroll_set(GuiScroll* self, UBIG data_height, UBIG screen_height, UBIG item_height)
{
	self->data_height = data_height;
	self->screen_height = screen_height;
	self->item_height = item_height;

	if (self->item_height == 0)
		self->item_height = 1;

	GuiScroll_setWheel(self, self->wheel);
}

GuiScroll GuiScroll_init(DbValue value)
{
	GuiScroll self;
	self.clickFront = FALSE;
	self.clickRel = 0;
	self.redraw = TRUE;
	self.value = value;

	self.wheel = DbValue_getNumber(&value);
	self.data_height = 1;
	self.screen_height = 1;
	self.item_height = 1;

	self.sendScrollDown = TRUE;
	return self;
}

GuiScroll GuiScroll_initEmpty(void)
{
	return GuiScroll_init(DbValue_initNumber(0));
}

GuiScroll GuiScroll_initCopy(GuiScroll* src)
{
	GuiScroll self = *src;
	self.value = DbValue_initCopy(&src->value);
	return self;
}

void GuiScroll_free(GuiScroll* self)
{
	//DbValue_setNumber(&self->value, self->wheel);
	DbValue_free(&self->value);
}

int GuiScroll_widthWin(Win* win)
{
	return OsWinIO_cellSize() / 2;
}

void GuiScroll_setValue(GuiScroll* self, DbValue value)
{
	DbValue_free(&self->value);
	self->value = value;
	self->wheel = DbValue_getNumber(&self->value);
}

BOOL GuiScroll_is(const GuiScroll* self)
{
	return(self->data_height > self->screen_height);
}

BOOL GuiScroll_getRedrawAndReset(GuiScroll* self)
{
	BOOL r = self->redraw;
	//if(r)
		self->redraw = FALSE;
	return r;
}

static void _GuiScroll_updateV(const GuiScroll* self, Vec2i start, Win* win, Quad2i* outSlider)
{
	if (self->data_height <= self->screen_height)
	{
		outSlider->start = start;

		outSlider->size.x = GuiScroll_widthWin(win);
		outSlider->size.y = self->screen_height;
	}
	else
	{
		outSlider->start.x = start.x;
		outSlider->start.y = start.y + self->screen_height * (self->wheel / (double)self->data_height);

		outSlider->size.x = GuiScroll_widthWin(win);
		outSlider->size.y = self->screen_height * (self->screen_height / (double)self->data_height);
	}
}

static void _GuiScroll_updateH(const GuiScroll* self, Vec2i start, Win* win, Quad2i* outSlider)
{
	if (self->data_height <= self->screen_height)
	{
		outSlider->start = start;

		outSlider->size.x = self->screen_height;
		outSlider->size.y = GuiScroll_widthWin(win);
	}
	else
	{
		outSlider->start.x = start.x + self->screen_height * (self->wheel / (double)self->data_height);
		outSlider->start.y = start.y;

		outSlider->size.x = self->screen_height * (self->screen_height / (double)self->data_height);
		outSlider->size.y = GuiScroll_widthWin(win);
	}
}

static Rgba _GuiScroll_getSlideCd(const GuiScroll* self)
{
	Rgba cd_slide = g_theme.main;
	if (self->data_height <= self->screen_height)
		cd_slide = Rgba_aprox(g_theme.background, g_theme.main, 0.5f);	//disable

	return cd_slide;
}

void GuiScroll_drawV(GuiScroll* self, Vec2i start, Image4* img, Win* win)
{
	Quad2i slider;
	_GuiScroll_updateV(self, start, win, &slider);

	slider.start.x += 2;
	slider.size.x -= 4;

	//make scroll visible if there is a lot of records(items)
	if (slider.size.y == 0)
	{
		const int c = OsWinIO_cellSize()/4;
		slider.start.y -= c / 2;
		slider.size.y += c;
	}

	Image4_drawBoxQuad(img, slider, _GuiScroll_getSlideCd(self));
}

void GuiScroll_drawH(GuiScroll* self, Vec2i start, Image4* img, Win* win)
{
	Quad2i slider;
	_GuiScroll_updateH(self, start, win, &slider);

	slider.start.y += 2;
	slider.size.y -= 4;

	//make scroll visible if there is a lot of records(items)
	if (slider.size.x == 0)
	{
		const int c = OsWinIO_cellSize() / 4;
		slider.start.x -= c / 2;
		slider.size.x += c;
	}

	Image4_drawBoxQuad(img, slider, _GuiScroll_getSlideCd(self));
}

void GuiScroll_touchV(GuiScroll* self, void* parent, Quad2i parentCoord, Vec2i start, Win* win)
{
	BIG orig_wheel = self->wheel;

	BOOL insideParent = Quad2i_inside(parentCoord, OsWinIO_getTouchPos());
	if (OsWinIO_getTouch_action() == Win_TOUCH_WHEEL && insideParent && !(OsWinIO_getKeyExtra() & Win_EXTRAKEY_SHIFT))
	{
		BIG scrll = OsWinIO_getTouch_wheel();
		//if (OsWinIO_getKeyEXTRA() & Win_EXTRAKEY_ALT)
		//	scrll *= GuiScroll_getMaxRows(self) / 10;
		//else
		//if (OsWinIO_getKeyEXTRA() & Win_EXTRAKEY_SHIFT)
		//	scrll *= GuiScroll_getMaxRows(self) / 100;

		GuiScroll_setWheel(self, self->wheel + scrll * self->item_height);
		if (orig_wheel != self->wheel) //let parent scroll
			OsWinIO_resetTouch();

		if (!self->sendScrollDown)
			OsWinIO_resetTouch();
	}

	if (!GuiScroll_is(self))
		return;

	Quad2i sliderFront;
	_GuiScroll_updateV(self, start, win, &sliderFront);
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
	sliderBack.size = Vec2i_init2(GuiScroll_widthWin(win), self->screen_height);

	if (self->clickFront) //click on slider
	{
		double mid = (OsWinIO_getTouchPos().y - start.y) - midSlider - self->clickRel;
		GuiScroll_setWheel(self, (mid / self->screen_height) * self->data_height);

		self->redraw = TRUE;
	}
	else
		if (startTouch && Quad2i_inside(sliderBack, OsWinIO_getTouchPos()) && !Quad2i_inside(sliderFront, OsWinIO_getTouchPos())) //click(once) on background
		{
			double mid = (OsWinIO_getTouchPos().y - start.y) - midSlider;
			GuiScroll_setWheel(self, (mid / self->screen_height) * self->data_height);

			//switch to 'click on slider'
			self->clickFront = TRUE;
			self->clickRel = 0;

			self->redraw = TRUE;
		}

	if (endTouch)
		OsWinIO_resetActiveRenderItem();

	self->redraw |= (orig_wheel != self->wheel);
}

void GuiScroll_touchH(GuiScroll* self, void* parent, Quad2i parentCoord, Vec2i start, Win* win, BOOL needShiftWheel)
{
	BIG orig_wheel = self->wheel;

	BOOL insideParent = Quad2i_inside(parentCoord, OsWinIO_getTouchPos());
	if (OsWinIO_getTouch_action() == Win_TOUCH_WHEEL && insideParent && (!needShiftWheel || OsWinIO_getKeyExtra() & Win_EXTRAKEY_SHIFT))
	{
		BIG scrll = OsWinIO_getTouch_wheel();

		GuiScroll_setWheel(self, self->wheel + scrll * self->item_height);
		if (orig_wheel != self->wheel) //let parent scroll
			OsWinIO_resetTouch();

		if (!self->sendScrollDown)
			OsWinIO_resetTouch();
	}

	if (!GuiScroll_is(self))
		return;

	Quad2i sliderFront;
	_GuiScroll_updateH(self, start, win, &sliderFront);
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
	sliderBack.size = Vec2i_init2(self->screen_height, GuiScroll_widthWin(win));

	if (self->clickFront) //click on slider
	{
		double mid = (OsWinIO_getTouchPos().x - start.x) - midSlider - self->clickRel;
		GuiScroll_setWheel(self, (mid / self->screen_height) * self->data_height);

		self->redraw = TRUE;
	}
	else
		if (startTouch && Quad2i_inside(sliderBack, OsWinIO_getTouchPos()) && !Quad2i_inside(sliderFront, OsWinIO_getTouchPos())) //click(once) on background
		{
			double mid = (OsWinIO_getTouchPos().x - start.x) - midSlider;
			GuiScroll_setWheel(self, (mid / self->screen_height) * self->data_height);

			//switch to 'click on slider'
			self->clickFront = TRUE;
			self->clickRel = 0;

			self->redraw = TRUE;
		}

	if (endTouch)
		OsWinIO_resetActiveRenderItem();

	self->redraw |= (orig_wheel != self->wheel);
}

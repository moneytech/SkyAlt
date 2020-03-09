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

typedef struct GuiItemLevel_s
{
	GuiItem base;
	BOOL closeAfter;
	BOOL removeLater;

	Quad2i relativeCoord;

	StdArr origPath;	//<char*>
} GuiItemLevel;

GuiItem* GuiItemLevel_newCenter(BOOL closeAfter, GuiItem* sub)
{
	GuiItemLevel* self = Os_malloc(sizeof(GuiItemLevel));
	self->base = GuiItem_init(GuiItem_LEVEL, Quad2i_init4(0, 0, 1, 1));
	self->closeAfter = closeAfter;

	self->removeLater = FALSE;

	GuiItem_addSubName(&self->base, "level", sub);

	self->relativeCoord = Quad2i_init();

	self->origPath = StdArr_init();

	return(GuiItem*)self;
}

GuiItem* GuiItemLevel_newRelative(BOOL closeAfter, GuiItem* sub, Quad2i relativeCoord, StdArr origPath)
{
	GuiItemLevel* self = (GuiItemLevel*)GuiItemLevel_newCenter(closeAfter, sub);

	self->relativeCoord = relativeCoord;
	self->origPath = origPath;

	return(GuiItem*)self;
}

GuiItem* GuiItemLevel_newCopy(GuiItemLevel* src, BOOL copySub)
{
	GuiItemLevel* self = Os_malloc(sizeof(GuiItemLevel));
	*self = *src;
	GuiItem_initCopy(&self->base, &src->base, copySub);

	self->origPath = StdArr_initCopyFn(&src->origPath, (StdArrCOPY)&Std_newCHAR);

	GuiItemRoot_redrawAll();	//redraw screen(with fade)

	return(GuiItem*)self;
}

void GuiItemLevel_delete(GuiItemLevel* self)
{
	StdArr_freeFn(&self->origPath, (StdArrFREE)&Std_deleteCHAR);

	GuiItem_free(&self->base);
	Os_free(self, sizeof(GuiItemLevel));
}

/*void GuiItemLevel_printPath(const GuiItemLevel* self)
{
	int i;
	for (i = 0; i < self->origPath.num; i++)
		if(self->origPath.ptrs[i])
			printf("%s\n", (char*)self->origPath.ptrs[i]);
}*/

GuiItem* GuiItemLevel_getBackChain(const GuiItemLevel* self)
{
	return GuiItemRoot_findPath(&self->origPath);
}

BOOL GuiItemLevel_isBackChainValid(const GuiItemLevel* self)
{
	if (self->origPath.num == 0)	//center
		return TRUE;
	return GuiItemLevel_getBackChain(self) != 0;
}

void GuiItemLevel_tryCloseLater(GuiItemLevel* self)
{
	if (self && self->closeAfter)
		self->removeLater = TRUE;
}

void GuiItemLevel_close(GuiItemLevel* self)
{
	GuiItemEdit_saveCache();

	GuiItem_getBaseParent(&self->base)->remove = TRUE;
	//self->base.remove = TRUE;

	GuiItemRoot_redrawAll();
}

GuiItem* _GuiItemLevel_content(GuiItemLevel* self)
{
	return GuiItem_numSub(&self->base) ? GuiItem_getSub(&self->base, 0) : 0;
}

void GuiItemLevel_draw(GuiItemLevel* self, Image4* img, Quad2i coord, Win* win)
{
	Image4_mulVSubQ(img, img->rect, 127);
}

void GuiItemLevel_update(GuiItemLevel* self, Quad2i coord, Win* win)
{
}

void GuiItemLevel_touch(GuiItemLevel* self, Quad2i coord, Win* win)
{
	BOOL startTouch = (OsWinIO_getTouch_action() == Win_TOUCH_DOWN_S) || (OsWinIO_getTouch_action() == Win_TOUCH_FORCE_DOWN_S);
	//BOOL endTouch = (OsWinIO_getTouch_action() == Win_TOUCH_DOWN_E) || (OsWinIO_getTouch_action() == Win_TOUCH_FORCE_DOWN_E);

	if (self->base.touch && GuiItem_isEnable(&self->base) && OsWinIO_canActiveRenderItem(self))
	{
		GuiItem* content = _GuiItemLevel_content(self);
		if (content)
		{
			BOOL inside = Quad2i_inside(content->coordMoveCut, OsWinIO_getTouchPos());
			if (startTouch && !inside)
				GuiItemLevel_close(self);
		}
	}

	if (self->removeLater)
		GuiItemLevel_close(self);
}

void GuiItemLevel_key(GuiItemLevel* self, Quad2i coord, Win* win)
{
	if (/*!self->base.touch ||*/ !GuiItem_isEnable(&self->base))
		return;

	if (OsWinIO_getKeyExtra() & Win_EXTRAKEY_ESCAPE)
	{
		GuiItemLevel_close(self);
		OsWinIO_resetKeyEXTRA();
	}
}

void GuiItemLevel_resizeCenter(GuiItemLevel* self, Win* win)
{
	const Quad2i winRect = Quad2i_init2(Vec2i_init(), Win_getImage(win).size);

	GuiItem* content = _GuiItemLevel_content(self);
	if (content && content->type == GuiItem_LAYOUT)
	{
		//prepare for computing center
		GuiItem_setCoord(content, winRect);
		GuiItem_resize(content, (GuiItemLayout*)content, win);

		//center
		Quad2i q = GuiItem_getSubMaxCoord(content);
		q = Quad2i_getIntersect(q, winRect);

		q = Quad2i_center(winRect, q);

		GuiItem_setCoord(content, q);
		GuiItem_resize(content, (GuiItemLayout*)content, win);

		GuiItem_setCoord(&self->base, winRect);
	}
}

void GuiItemLevel_resizeToParent(GuiItemLevel* self, Win* win)
{
	const Quad2i winRect = Quad2i_init2(Vec2i_init(), Win_getImage(win).size);

	Quad2i source = self->relativeCoord;

	GuiItem* content = _GuiItemLevel_content(self);
	if (content && content->type == GuiItem_LAYOUT)
	{
		//position
		GuiItem_setCoord(content, winRect);
		GuiItem_resize(content, (GuiItemLayout*)content, win);

		//center
		Quad2i q = GuiItem_getSubMaxCoord(content);
		if (!Quad2i_isZero(source))
		{
			Vec2i srcStart = source.start;
			Vec2i srcSize = source.size;

			BOOL up = srcStart.y > (winRect.size.y - srcStart.y - srcSize.y);
			BOOL right = (srcStart.x + q.size.x < winRect.size.x);

			if (right)
			{
				q.start.x = srcStart.x;
				q.size.x = (q.start.x + q.size.x > winRect.size.x) ? (winRect.size.x - q.start.x) : q.size.x;
			}
			else
			{
				q.start.x = srcStart.x + srcSize.x - q.size.x;
				if (q.start.x < 0)
				{
					q.size.x = srcStart.x + srcSize.x;
					q.start.x = 0;
				}
			}

			if (up)
			{
				q.start.y = srcStart.y - q.size.y;
				if (q.start.y < 0)
				{
					q.size.y = srcStart.y;
					q.start.y = 0;
				}
			}
			else
			{
				q.start.y = srcStart.y + srcSize.y;
				q.size.y = (q.start.y + q.size.y > winRect.size.y) ? (winRect.size.y - q.start.y) : q.size.y;
			}
		}
		else
			q = Quad2i_getSub(winRect, Vec2i_divV(winRect.size, 1.5f));

		//set
		GuiItem_setCoord(content, q);
		GuiItem_resize(content, (GuiItemLayout*)content, win);

		//self->realCoord = q;
		GuiItem_setCoord(&self->base, winRect);
	}
}

GuiItemLayout* GuiItemLevel_resize(GuiItemLevel* self, GuiItemLayout* layout, Win* win)
{
	if (!self->base.resize)
		return layout;

	if (Quad2i_isZero(self->relativeCoord))
		GuiItemLevel_resizeCenter(self, win);
	else
		GuiItemLevel_resizeToParent(self, win);

	GuiItem_setResizeOff(&self->base);	//don't resize children

	return layout;
}

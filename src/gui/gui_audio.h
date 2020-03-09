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

typedef struct GuiItemAudio_s
{
	GuiItem base;
	DbValue info;
	FileRow fileRowBackup;

	Vec2i lastMousePos;
	double lastMouseTime;
	BOOL mouseActive;

	UNI* error;
} GuiItemAudio;

GuiItem* GuiItemAudio_new(Quad2i grid, DbValue info)
{
	GuiItemAudio* self = Os_malloc(sizeof(GuiItemAudio));
	self->base = GuiItem_init(GuiItem_AUDIO, grid);

	self->info = info;
	self->fileRowBackup = FileRow_initEmpty();

	self->lastMouseTime = 0;
	self->lastMousePos = Vec2i_init();
	self->mouseActive = FALSE;

	self->error = 0;

	return(GuiItem*)self;
}

GuiItem* GuiItemAudio_newCopy(GuiItemAudio* src, BOOL copySub)
{
	GuiItemAudio* self = Os_malloc(sizeof(GuiItemAudio));
	*self = *src;
	GuiItem_initCopy(&self->base, &src->base, copySub);

	self->info = DbValue_initCopy(&src->info);
	self->error = Std_newUNI(src->error);

	return(GuiItem*)self;
}

void GuiItemAudio_delete(GuiItemAudio* self)
{
	DbValue_free(&self->info);
	Std_deleteUNI(self->error);

	GuiItem_free(&self->base);
	Os_free(self, sizeof(GuiItemAudio));
}

static Quad2i _GuiItemAudio_getImgCoord(GuiItemAudio* self, Quad2i origCoord)
{
	if (!self->base.drawTable)
		origCoord = Quad2i_addSpace(origCoord, 3);
	return origCoord;
}

void GuiItemAudio_draw(GuiItemAudio* self, Image4* img, Quad2i coord, Win* win)
{
	coord = _GuiItemAudio_getImgCoord(self, coord);

	int textH = _GuiItem_textSize(1, coord.size.y);
	OsFont* font = OsWinIO_getFontDefault();

	int cell = OsWinIO_cellSize();

	Quad2i coordInfo = coord;
	coordInfo.size.y = cell;

	//background
	Image4_drawBoxQuad(img, coord, self->base.back_cd);

	if (self->error)
		Image4_drawText(img, Quad2i_getMiddle(coord), TRUE, font, self->error, textH, 0, self->base.front_cd);
	else
		if (FileRow_is(DbValue_getFileId(&self->info)))
		{
			if (self->mouseActive && coord.size.y > cell)
				MediaLibrary_imageDrawInfo(DbValue_getFileId(&self->info), img, coordInfo, textH, self->base.front_cd);

			//status
			const UNI* playText = MediaLibrary_isPlay(DbValue_getFileId(&self->info)) ? _UNI32("| |") : _UNI32("▷");
			Image4_drawText(img, Quad2i_getMiddle(coord), TRUE, font, playText, textH * 2, 0, self->base.front_cd);
		}

	if (self->base.drawTable)
	{
		Quad2i q = coord;
		q.size.x += 1;
		q.size.y += 1;
		Image4_drawBorder(img, q, 1, Rgba_aprox(self->base.back_cd, self->base.front_cd, 0.5f));
	}
	else
		Image4_drawBorder(img, coord, 1, self->base.front_cd);
}

UNI* GuiItemAudio_getError(DbValue* info)
{
	UNI* str = Std_newUNI(Lang_find("ERR_FILE_UNSUPPORTED_FORMAT"));
	str = Std_addAfterUNI(str, _UNI32(": "));

	UNI ext[8];
	DbValue_getFileExt(info, 0, ext);
	str = Std_addAfterUNI(str, ext);

	return str;
}

void GuiItemAudio_update(GuiItemAudio* self, Quad2i coord, Win* win)
{
	coord = _GuiItemAudio_getImgCoord(self, coord);

	FileRow fileRow = DbValue_getFileId(&self->info);
	if (!FileRow_cmp(self->fileRowBackup, fileRow))
	{
		if (FileRow_is(fileRow))
		{
			if (!MediaLibrary_addAudio(fileRow))
			{
				Std_deleteUNI(self->error);
				self->error = GuiItemAudio_getError(&self->info);
			}
		}

		self->fileRowBackup = fileRow;
		GuiItem_setRedraw(&self->base, TRUE);
	}

	GuiItemSlider* slider = GuiItem_findChildType(&self->base, GuiItem_SLIDER);
	if (slider)
	{
		GuiItem_setEnable(&slider->base, self->error == 0);

		//update seek
		GuiItemSlider_setNumber(slider, MediaLibrary_getSeek(fileRow));
	}

	BOOL oldMouseActive = self->mouseActive;
	self->mouseActive = Quad2i_inside(coord, OsWinIO_getTouchPos()) && (Os_time() - self->lastMouseTime) < 2;	//2 seconds

	GuiItem_setRedraw(&self->base, (DbValue_hasChanged(&self->info) || oldMouseActive != self->mouseActive));
}

void GuiItemAudio_touch(GuiItemAudio* self, Quad2i coord, Win* win)
{
	coord = _GuiItemAudio_getImgCoord(self, coord);

	Rgba back_cd = g_theme.white;
	Rgba front_cd = g_theme.black;

	if (self->error)
		back_cd = g_theme.warning;

	if (self->base.touch && GuiItem_isEnable(&self->base) && OsWinIO_canActiveRenderItem(self))
	{
		BOOL startTouch = (OsWinIO_getTouch_action() == Win_TOUCH_DOWN_S) || (OsWinIO_getTouch_action() == Win_TOUCH_FORCE_DOWN_S);
		BOOL endTouch = (OsWinIO_getTouch_action() == Win_TOUCH_DOWN_E) || (OsWinIO_getTouch_action() == Win_TOUCH_FORCE_DOWN_E);

		BOOL active = OsWinIO_isActiveRenderItem(self);
		BOOL inside = Quad2i_inside(coord, OsWinIO_getTouchPos());
		BOOL touch = startTouch || active;

		if (inside && touch) //full touch
		{
			back_cd = g_theme.white;
			front_cd = g_theme.black;
			OsWinIO_setActiveRenderItem(self);
		}
		else
			if ((inside && !touch) || (active && !inside)) //mid color
			{
				back_cd = Rgba_aprox(back_cd, front_cd, 0.2f);
			}

		if (inside && active && endTouch) //end
		{
			GuiItemEdit_saveCache();
			MediaLibrary_play(DbValue_getFileId(&self->info), !MediaLibrary_isPlay(DbValue_getFileId(&self->info)));
		}

		if (endTouch)
			OsWinIO_resetActiveRenderItem();

		//cursor
		if (inside)
			Win_updateCursor(win, Win_CURSOR_HAND);

		if (inside && !Vec2i_cmp(self->lastMousePos, OsWinIO_getTouchPos()))
		{
			self->lastMousePos = OsWinIO_getTouchPos();
			self->lastMouseTime = Os_time();
		}
	}

	_GuiItem_updateFinalCd(&self->base, back_cd, front_cd, coord, win);
}

void GuiItemAudio_clickSlider(GuiItem* item)
{
	GuiItemAudio* self = GuiItem_findParentType(item, GuiItem_AUDIO);
	GuiItemSlider* slider = GuiItem_findChildType(item, GuiItem_SLIDER);

	double t = GuiItemSlider_getValueT(slider);
	MediaLibrary_setSeek(DbValue_getFileId(&self->info), t);
}

GuiItemLayout* GuiItemAudio_resize(GuiItemAudio* self, GuiItemLayout* layout, Win* win)
{
	if (!self->base.resize)
		return (GuiItemLayout*)GuiItem_getSub(&self->base, 0);

	GuiItem_freeSubs(&self->base);

	//layout
	layout = GuiItemLayout_newCoord(&self->base, FALSE, FALSE, win);
	layout->drawBackground = FALSE;
	GuiItem_addSubName(&self->base, "layout_main", &layout->base);
	GuiItemLayout_addColumn(layout, 0, 100);
	GuiItemLayout_addRow(layout, 1, 100);

	//slider(seek)
	GuiItem_addSubName((GuiItem*)layout, "slider", GuiItemSlider_newEx(Quad2i_init4(2, 0, 1, 1), DbValue_initNumber(0), DbValue_initNumber(1), DbValue_initNumber(0.01), DbValue_initNumber(0), DbValue_initEmpty(), DbValue_initEmpty(), DbValue_initEmpty(), &GuiItemAudio_clickSlider));

	return layout;
}
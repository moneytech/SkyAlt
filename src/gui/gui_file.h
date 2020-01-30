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

typedef struct GuiItemFile_s
{
	GuiItem base;
	DbValue info;
	FileRow fileRowBackup;

	Vec2i imgSize;
	UNI* error;

	BOOL audio;
	BOOL image;
	BOOL text;

	BOOL showInfo;
	float timelinePosition;

	Vec2i lastMousePos;
	double lastMouseTime;
	BOOL mouseActive;
	BOOL isFullscreen;

	GuiScroll scroll;
	UNI* textStr;

	DbValue preview;
} GuiItemFile;

GuiItem* GuiItemFile_new(Quad2i grid, DbValue info, DbValue preview, BOOL isFullscreen)
{
	GuiItemFile* self = Os_malloc(sizeof(GuiItemFile));
	self->base = GuiItem_init(GuiItem_FILE, grid);

	self->info = info;
	self->fileRowBackup = FileRow_initEmpty();

	self->error = 0;

	self->audio = FALSE;
	self->image = FALSE;
	self->text = FALSE;

	self->showInfo = FALSE;

	self->timelinePosition = 0;

	self->scroll = GuiScroll_initEmpty();
	self->textStr = 0;

	self->imgSize = Vec2i_init();
	self->preview = preview;

	self->lastMouseTime = 0;
	self->lastMousePos = Vec2i_init();

	self->isFullscreen = isFullscreen;

	self->mouseActive = FALSE;

	return(GuiItem*)self;
}

GuiItem* GuiItemFile_newCopy(GuiItemFile* src, BOOL copySub)
{
	GuiItemFile* self = Os_malloc(sizeof(GuiItemFile));
	*self = *src;
	GuiItem_initCopy(&self->base, &src->base, copySub);

	self->info = DbValue_initCopy(&src->info);
	self->preview = DbValue_initCopy(&src->preview);

	self->scroll = GuiScroll_initCopy(&src->scroll);
	self->error = Std_newUNI(src->error);
	self->textStr = Std_newUNI(src->textStr);

	return(GuiItem*)self;
}

void GuiItemFile_delete(GuiItemFile* self)
{
	DbValue_free(&self->info);
	DbValue_free(&self->preview);

	Std_deleteUNI(self->error);

	GuiScroll_free(&self->scroll);
	Std_deleteUNI(self->textStr);

	GuiItem_free(&self->base);
	Os_free(self, sizeof(GuiItemFile));
}

BOOL GuiItemFile_hasContent(const GuiItemFile* self)
{
	return self->audio || self->image || self->text;
}

Quad2i GuiItemFile_getCloseRect(Quad2i coord)
{
	const int q = OsWinIO_cellSize() / 3;
	return Quad2i_init4(coord.start.x + coord.size.x - q * 2, coord.start.y + q, q, q);
}

Quad2i GuiItemFile_getTimelineQuad(Quad2i coord, Win* win)
{
	const int cell = OsWinIO_cellSize();
	Quad2i tl;
	tl.start.x = coord.start.x + cell + 5;
	tl.start.y = coord.start.y + coord.size.y - cell - 2;
	tl.size.x = coord.size.x - cell - 10;
	tl.size.y = cell - 4;
	return tl;
}

static Quad2i _GuiItemFile_getImgCoord(GuiItemFile* self, Quad2i origCoord)
{
	if (!self->base.drawTable)
		origCoord = Quad2i_addSpace(origCoord, 3);
	return origCoord;
}

void GuiItemFile_draw(GuiItemFile* self, Image4* img, Quad2i coord, Win* win)
{
	coord = _GuiItemFile_getImgCoord(self, coord);

	int textH = _GuiItem_textSize(1, coord.size.y);
	OsFont* font = OsWinIO_getFontDefault();

	int cell = OsWinIO_cellSize();

	Quad2i coordInfo = coord;
	coordInfo.size.y = cell;

	//background
	Image4_drawBoxQuad(img, coord, self->base.back_cd);

	if (FileRow_is(DbValue_getFileId(&self->info)))
	{
		const BOOL preview = DbValue_getNumber(&self->preview);

		if (!preview)
		{
			MediaLibrary_imageDrawInfo(DbValue_getFileId(&self->info), img, coord, textH, self->base.front_cd);
		}
		else
			if (self->audio)
			{
				if (self->showInfo && coord.size.y > cell)
					MediaLibrary_imageDrawInfo(DbValue_getFileId(&self->info), img, coordInfo, textH, self->base.front_cd);

				//status
				Vec2i tt;
				tt.x = coord.start.x + cell / 2;
				tt.y = coord.start.y + coord.size.y - cell / 3 * 2; //cell/3*2 no idea?
				const UNI* playText = MediaLibrary_isPlay(DbValue_getFileId(&self->info)) ? _UNI32("| |") : _UNI32("▷");
				Image4_drawText(img, tt, TRUE, font, playText, textH, 0, self->base.front_cd);

				if (self->mouseActive)
				{
					//timeline
					Quad2i tl = GuiItemFile_getTimelineQuad(coord, win);
					tl.start.y += cell / 2 - 1;
					tl.size.y = 2;
					Image4_drawBoxQuad(img, tl, self->base.front_cd);

					//timeline position
					tl.start.x += tl.size.x * self->timelinePosition;
					tl.start.y -= cell / 4;
					tl.size.x = 2;
					tl.size.y = cell / 2;
					Image4_drawBoxQuad(img, tl, self->base.front_cd);
				}
			}
			else
				if (self->image)
				{
					if (MediaLibrary_imageDraw(DbValue_getFileId(&self->info), img, coord))
					{
						if (self->showInfo && coord.size.y > cell)
							MediaLibrary_imageDrawInfo(DbValue_getFileId(&self->info), img, coordInfo, textH, self->base.front_cd);
					}
					else
						Image4_drawText(img, Quad2i_getMiddle(coord), TRUE, font, _UNI32("..."), textH, 0, self->base.front_cd);
				}
				else
					if (self->text)
					{
						GuiItemTextMulti_drawIt(img, coord, win, self->textStr, self->base.back_cd, self->base.front_cd, &self->scroll);
					}

		if (self->error)
		{
			Image4_drawText(img, Quad2i_getMiddle(coord), TRUE, font, self->error, textH, 0, self->base.front_cd);
		}
	}
	else
	{
		if (self->mouseActive)
		{
			//+
			Image4_drawText(img, Quad2i_getMiddle(coord), TRUE, font, _UNI32("+"), cell / 2, 0, self->base.front_cd);
		}
	}

	if (GuiItemFile_hasContent(self))
	{
		Quad2i close = GuiItemFile_getCloseRect(coord);
		if (self->mouseActive || Quad2i_inside(close, self->lastMousePos))
		{
			Vec2i end = Quad2i_end(close);
			Vec2i mid = Quad2i_getMiddle(close);
			const int SP = 2;
			const int FAT = 10;
			if (self->isFullscreen)
			{
				//close
				//Image4_drawLine(img, Vec2i_init2(close.start.x, close.start.y), Vec2i_init2(end.x, end.y), 2, self->base.front_cd);
				//Image4_drawLine(img, Vec2i_init2(end.x, close.start.y), Vec2i_init2(close.start.x, end.y), 2, self->base.front_cd);

				Image4_drawArrow(img, Vec2i_init2(mid.x - SP, mid.y + SP), Vec2i_init2(close.start.x, end.y), FAT, self->base.front_cd);
				Image4_drawArrow(img, Vec2i_init2(mid.x + SP, mid.y - SP), Vec2i_init2(end.x, close.start.y), FAT, self->base.front_cd);
			}
			else
			{
				Image4_drawArrow(img, Vec2i_init2(close.start.x, end.y), Vec2i_init2(mid.x - SP, mid.y + SP), FAT, self->base.front_cd);
				Image4_drawArrow(img, Vec2i_init2(end.x, close.start.y), Vec2i_init2(mid.x + SP, mid.y - SP), FAT, self->base.front_cd);
			}
		}
	}

	if (self->base.drawTable)
	{
		Quad2i q = coord;
		q.size.x += 1;
		q.size.y += 1;
		//img->rect = q;
		Image4_drawBorder(img, q, 1, Rgba_aprox(self->base.back_cd, self->base.front_cd, 0.5f));
	}
	else
		Image4_drawBorder(img, coord, 1, self->base.front_cd);
}

void GuiItemFile_update(GuiItemFile* self, Quad2i coord, Win* win)
{
	coord = _GuiItemFile_getImgCoord(self, coord);

	FileRow fileRow = DbValue_getFileId(&self->info);
	if (!FileRow_cmp(self->fileRowBackup, fileRow) || !Vec2i_cmp(self->imgSize, coord.size))
	{
		Std_deleteUNI(self->error);
		self->error = 0;

		if (FileRow_is(fileRow))
		{
			if (!MediaLibrary_add(fileRow, coord.size, &self->image, &self->audio, &self->text))
			{
				self->error = Std_newUNI(Lang_find("ERR_FILE_UNSUPPORTED_FORMAT"));
				self->error = Std_addAfterUNI(self->error, _UNI32(": "));

				UNI ext[8];
				DbValue_getFileExt(&self->info, 0, ext);
				self->error = Std_addAfterUNI(self->error, ext);
			}

			if (self->image)
			{
				//? ...
			}
			else
				if (self->audio)
				{
					//load info? ...
				}
				else
					if (self->text)
					{
						//alloc & load
						UBIG size = DbValue_getFileSize(&self->info, 0);
						UCHAR* buff = Os_malloc(size + 1);
						buff[size] = 0;

						DbValue_readFileCache(&self->info, 0, 0, size, buff);

						Std_deleteUNI(self->textStr);
						self->textStr = Std_newUNI_char((char*)buff);
						Os_free(buff, size + 1);
					}
		}

		self->fileRowBackup = fileRow;
		self->imgSize = coord.size;
		GuiItem_setRedraw(&self->base, TRUE);
	}

	if (self->image)
	{
		if (MediaLibrary_imageUpdate(fileRow, coord.size))	//update timer and returns true if just loaded
			GuiItem_setRedraw(&self->base, TRUE);
	}

	if (self->audio)
	{
		//update seek
		float pos = MediaLibrary_getSeek(fileRow);
		if (pos != self->timelinePosition)
			GuiItem_setRedraw(&self->base, TRUE);
		self->timelinePosition = pos;
	}

	BOOL oldMouseActive = self->mouseActive;
	{
		BOOL inside = Quad2i_inside(coord, OsWinIO_getTouchPos());
		self->mouseActive = inside && (Os_time() - self->lastMouseTime) < 2;	//2 seconds
	}

	//scroll
	GuiItem_setRedraw(&self->base, GuiScroll_getRedrawAndReset(&self->scroll) || DbValue_hasChanged(&self->preview) || (oldMouseActive != self->mouseActive));
}

void GuiItemFile_showFullscreen(GuiItem* parent, DbValue info)
{
	GuiItemLayout* layout = GuiItemLayout_new(Quad2i_init4(0, 0, 1, 1));
	GuiItemLayout_addColumn(layout, 0, 100);
	GuiItemLayout_addRow(layout, 0, 100);
	GuiItem_addSubName(&layout->base, "file_fullscreen", GuiItemFile_new(Quad2i_init4(0, 0, 1, 1), info, DbValue_initNumber(1), TRUE));

	GuiItemRoot_addDialogLayout(layout);
}

void GuiItemFile_switchFullscreen(GuiItemFile* self)
{
	if (self->isFullscreen)	//exist
		GuiItem_closeParentLevel(&self->base);
	else
		GuiItemFile_showFullscreen(&self->base, DbValue_initCopy(&self->info));
}

void GuiItemFile_touch(GuiItemFile* self, Quad2i coord, Win* win)
{
	coord = _GuiItemFile_getImgCoord(self, coord);

	Rgba back_cd = g_theme.white;
	Rgba front_cd = g_theme.black;

	if (self->base.touch && GuiItem_isEnable(&self->base) && OsWinIO_canActiveRenderItem(self))
	{
		BOOL startTouch = (OsWinIO_getTouch_action() == Win_TOUCH_DOWN_S) || (OsWinIO_getTouch_action() == Win_TOUCH_FORCE_DOWN_S);
		BOOL endTouch = (OsWinIO_getTouch_action() == Win_TOUCH_DOWN_E) || (OsWinIO_getTouch_action() == Win_TOUCH_FORCE_DOWN_E);

		BOOL active = OsWinIO_isActiveRenderItem(self);
		BOOL inside = Quad2i_inside(coord, OsWinIO_getTouchPos());
		BOOL touch = startTouch || active;

		self->showInfo = inside;

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

			if (!FileRow_is(DbValue_getFileId(&self->info)))
			{
				//+
				GuiItemRoot_addDialogRelLayout(GuiItemTags_dialogFileAdd(0), &self->base, coord, TRUE);
			}

			if (self->base.click)
			{
				GuiItem_callClick(&self->base);	//table preview
			}
			else
				if (self->audio)
				{
					Quad2i c = GuiItemFile_getTimelineQuad(coord, win);

					if (Quad2i_inside(c, OsWinIO_getTouchPos()))
					{
						float t;
						if (OsWinIO_getTouch_action() == Win_TOUCH_WHEEL) //not working ...
						{
							t = Std_fclamp(MediaLibrary_getSeek(DbValue_getFileId(&self->info)) + 0.1f, 0, 1);
							OsWinIO_resetTouch();
						}
						else
							t = (OsWinIO_getTouchPos().x - c.start.x) / (float)c.size.x;

						MediaLibrary_setSeek(DbValue_getFileId(&self->info), t);
					}
					else
						MediaLibrary_play(DbValue_getFileId(&self->info), !MediaLibrary_isPlay(DbValue_getFileId(&self->info)));
				}
		}

		if (endTouch)
			OsWinIO_resetActiveRenderItem();

		//cursor
		if (inside)
			Win_updateCursor(win, Win_CURSOR_HAND);

		if (self->text)
		{
			const int scroll_width = GuiScroll_widthWin(win);
			GuiScroll_touchV(&self->scroll, self, coord, Vec2i_init2(coord.start.x + coord.size.x - scroll_width, coord.start.y), win);
		}

		if (inside && !Vec2i_cmp(self->lastMousePos, OsWinIO_getTouchPos()))
		{
			self->lastMousePos = OsWinIO_getTouchPos();
			self->lastMouseTime = Os_time();
		}

		if (endTouch && Quad2i_inside(GuiItemFile_getCloseRect(coord), self->lastMousePos))
		{
			if (GuiItemFile_hasContent(self))
				GuiItemFile_switchFullscreen(self);
		}

		if (inside && endTouch && OsWinIO_getTouchNum() >= 2)
		{
			if (GuiItemFile_hasContent(self))
				GuiItemFile_switchFullscreen(self);
			//OsWinIO_setTouchNum(1);	//stupid ...
		}
	}

	_GuiItem_updateFinalCd(&self->base, back_cd, front_cd, coord, win);
}

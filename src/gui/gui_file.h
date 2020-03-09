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

typedef struct GuiItemFile_s
{
	GuiItem base;
	DbValue info;
	DbValue preview;
	FileRow fileRowBackup;

	UNI* error;

	BOOL audio;
	BOOL image;	
	BOOL text;
	BOOL map;

	BOOL isFullscreen;
} GuiItemFile;

GuiItem* GuiItemFile_new(Quad2i grid, DbValue info, DbValue preview, BOOL isFullscreen)
{
	GuiItemFile* self = Os_malloc(sizeof(GuiItemFile));
	self->base = GuiItem_init(GuiItem_FILE, grid);

	self->info = info;
	self->preview = preview;
	self->isFullscreen = isFullscreen;
	self->fileRowBackup = FileRow_initEmpty();

	self->error = 0;

	self->audio = FALSE;
	self->image = FALSE;
	self->text = FALSE;
	self->map = FALSE;

	return(GuiItem*)self;
}

GuiItem* GuiItemFile_newCopy(GuiItemFile* src, BOOL copySub)
{
	GuiItemFile* self = Os_malloc(sizeof(GuiItemFile));
	*self = *src;
	GuiItem_initCopy(&self->base, &src->base, copySub);

	self->info = DbValue_initCopy(&src->info);
	self->preview = DbValue_initCopy(&src->preview);

	self->error = Std_newUNI(src->error);

	return(GuiItem*)self;
}

void GuiItemFile_delete(GuiItemFile* self)
{
	DbValue_free(&self->info);
	DbValue_free(&self->preview);

	Std_deleteUNI(self->error);

	GuiItem_free(&self->base);
	Os_free(self, sizeof(GuiItemFile));
}

BOOL GuiItemFile_hasContent(const GuiItemFile* self)
{
	return self->audio || self->image || self->text || self->map;
}

Quad2i GuiItemFile_getCloseRect(Quad2i coord)
{
	const int q = OsWinIO_cellSize() / 3;
	return Quad2i_init4(coord.start.x + coord.size.x - q * 2, coord.start.y + q, q, q);
}


static Quad2i _GuiItemFile_getImgCoord(GuiItemFile* self, Quad2i origCoord)
{
	if (!self->base.drawTable)
		origCoord = Quad2i_addSpace(origCoord, 3);
	return origCoord;
}

void GuiItemFile_draw(GuiItemFile* self, Image4* img, Quad2i coord, Win* win)
{
}

void GuiItemFile_update(GuiItemFile* self, Quad2i coord, Win* win)
{
	coord = _GuiItemFile_getImgCoord(self, coord);

	FileRow fileRow = DbValue_getFileId(&self->info);
	if (!FileRow_cmp(self->fileRowBackup, fileRow))
	{
		Std_deleteUNI(self->error);
		self->error = 0;

		if (FileRow_is(fileRow))
		{
			if (!MediaLibrary_add(fileRow, coord.size, &self->image, &self->audio, &self->text, &self->map))
			{
				self->error = Std_newUNI(Lang_find("ERR_FILE_UNSUPPORTED_FORMAT"));
				self->error = Std_addAfterUNI(self->error, _UNI32(": "));

				UNI ext[8];
				DbValue_getFileExt(&self->info, 0, ext);
				self->error = Std_addAfterUNI(self->error, ext);
			}


			//multi ........................
					/*if (self->text)
					{
						//alloc & load
						UBIG size = DbValue_getFileSize(&self->info, 0);
						UCHAR* buff = Os_malloc(size + 1);
						buff[size] = 0;

						DbValue_readFileCache(&self->info, 0, 0, size, buff);

						Std_deleteUNI(self->textStr);
						self->textStr = Std_newUNI_char((char*)buff);
						Os_free(buff, size + 1);
					}*/

		}

		self->fileRowBackup = fileRow;
		GuiItem_setRedraw(&self->base, TRUE);
	}

	//scroll
	GuiItem_setRedraw(&self->base, DbValue_hasChanged(&self->preview));
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
				GuiItemRoot_addDialogRelLayout(GuiItemTags_dialogFileAdd(0, 0), &self->base, coord, TRUE);
			}

			if (self->base.click)
			{
				GuiItem_callClick(&self->base);	//table preview
			}
		}

		if (endTouch)
			OsWinIO_resetActiveRenderItem();

		//cursor
		if (inside)
			Win_updateCursor(win, Win_CURSOR_HAND);


		/*if (endTouch && Quad2i_inside(GuiItemFile_getCloseRect(coord), self->lastMousePos))
		{
			if (GuiItemFile_hasContent(self))
				GuiItemFile_switchFullscreen(self);
		}*/

		/*if (inside && endTouch && OsWinIO_getTouchNum() >= 2)
		{
			if (GuiItemFile_hasContent(self))
				GuiItemFile_switchFullscreen(self);
			//OsWinIO_setTouchNum(1);	//stupid ...
		}*/
	}

	_GuiItem_updateFinalCd(&self->base, back_cd, front_cd, coord, win);
}




GuiItemLayout* GuiItemFile_resize(GuiItemFile* self, GuiItemLayout* layout, Win* win)
{
	if (!self->base.resize)
		return (GuiItemLayout*)GuiItem_getSub(&self->base, 0);

	GuiItem_freeSubs(&self->base);

	//layout
	layout = GuiItemLayout_newCoord(&self->base, FALSE, FALSE, win);
	layout->drawBackground = FALSE;
	GuiItem_addSubName(&self->base, "layout_main", &layout->base);
	//GuiItemLayout_addColumn(layout, 0, 100);
	//GuiItemLayout_addRow(layout, 1, 100);


	//fullscreen ...
	//...


	return layout;
}
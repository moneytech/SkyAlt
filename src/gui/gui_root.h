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

typedef struct GuiItemRoot_s
{
	GuiItemLayout* particles;
	StdArr levels;	//<GuiItem*>

	OsThread thread;
	OsLockEvent syncEventStart;
	OsLockEvent syncEventEnd;
	volatile double run;

	BOOL doUpdate;
	Win* win;

	BOOL showPicker;
	BOOL pickerOpen;
	BOOL pickerFolder;
	BOOL pickerMultiple;
	Vec2i pickerPos;
	const UNI* pickerAction;
	const UNI* pickerExts;

	Quad2i drawRectOver;	//change column size, etc.
	Quad2i drawRectOver2;

	UBIG numChanges;
	Quad2i redrawRect;

	InterRoot* interpreter;
} GuiItemRoot;

GuiItemRoot* g_GuiItemRoot = 0;

THREAD_FUNC(GuiItemRoot_loop, param);

UBIG GuiItemRoot_numLevels(void)
{
	return g_GuiItemRoot->levels.num;
}
GuiItem* GuiItemRoot_getLevel(BIG i)
{
	return g_GuiItemRoot->levels.ptrs[i];
}
GuiItem* GuiItemRoot_getLevelTop(void)
{
	return g_GuiItemRoot->levels.ptrs[GuiItemRoot_numLevels() - 1];
}
static void _GuiItemRoot_deleteLevels(void)
{
	StdArr_freeFn(&g_GuiItemRoot->levels, (StdArrFREE)&GuiItem_delete);
}

void GuiItemRoot_delete(void)
{
	if (g_GuiItemRoot)
	{
		OsThread_setGameOver(&g_GuiItemRoot->thread);
		OsLockEvent_trigger(&g_GuiItemRoot->syncEventStart);
		//OsLockEvent_trigger(&g_GuiItemRoot->syncEventEnd);
		OsThread_free(&g_GuiItemRoot->thread, TRUE);

		OsLockEvent_free(&g_GuiItemRoot->syncEventStart);
		OsLockEvent_free(&g_GuiItemRoot->syncEventEnd);

		if (g_GuiItemRoot->particles)
			GuiItem_delete(&g_GuiItemRoot->particles->base);

		_GuiItemRoot_deleteLevels();

		InterRoot_delete(g_GuiItemRoot->interpreter);

		Os_free(g_GuiItemRoot, sizeof(GuiItemRoot));
		g_GuiItemRoot = 0;
	}
}

BOOL GuiItemRoot_new(void)
{
	if (g_GuiItemRoot)
		GuiItemRoot_delete();

	g_GuiItemRoot = Os_malloc(sizeof(GuiItemRoot));

	g_GuiItemRoot->particles = 0;
	g_GuiItemRoot->levels = StdArr_init();

	g_GuiItemRoot->showPicker = FALSE;
	g_GuiItemRoot->pickerOpen = FALSE;
	g_GuiItemRoot->pickerFolder = FALSE;
	g_GuiItemRoot->pickerMultiple = FALSE;
	g_GuiItemRoot->pickerPos = Vec2i_init();
	g_GuiItemRoot->pickerAction = 0;
	g_GuiItemRoot->pickerExts = 0;

	g_GuiItemRoot->run = 0;

	g_GuiItemRoot->drawRectOver = Quad2i_init();
	g_GuiItemRoot->drawRectOver2 = Quad2i_init();

	g_GuiItemRoot->numChanges = 0;

	g_GuiItemRoot->redrawRect = Quad2i_init();

	OsThread_init(&g_GuiItemRoot->thread, g_GuiItemRoot, &GuiItemRoot_loop);

	OsLockEvent_init(&g_GuiItemRoot->syncEventStart);
	OsLockEvent_init(&g_GuiItemRoot->syncEventEnd);

	g_GuiItemRoot->interpreter = InterRoot_new("code.txt");

	return TRUE;
}

void UiStartup_clickInterrupt(GuiItem* item)
{
	StdProgress_run(FALSE);
}

static GuiItemLayout* _GuiItemRoot_createParticles(Win* win)
{
	//Particles Logo
	Image1 logo = UiLogo_init();
	GuiItemParticles* particles = GuiItemParticles_new(Quad2i_init4(1, 1, 1, 1), logo, TRUE);
	Image1_free(&logo);

	//Layout
	GuiItemLayout* particlesLayout = GuiItemLayout_new(Quad2i_init4(0, 0, 1, 1));
	GuiItemLayout_addColumn(particlesLayout, 0, 100);
	GuiItemLayout_addColumn(particlesLayout, 1, 6);
	GuiItemLayout_addColumn(particlesLayout, 2, 100);
	GuiItemLayout_addRow(particlesLayout, 0, 100);
	GuiItemLayout_addRow(particlesLayout, 1, 4);
	GuiItemLayout_addRow(particlesLayout, 4, 100);

	Image4 back = Win_getImage(win);

	particlesLayout->imgBackground = Image4_initCopy(&back);
	Image4_mulVSubQ(&particlesLayout->imgBackground, Quad2i_init2(Vec2i_init(), Win_getImage(win).size), 180);	//0.6f

	GuiItem_addSubName((GuiItem*)particlesLayout, "particles", (GuiItem*)particles);
	GuiItem_addSubName((GuiItem*)particlesLayout, "interrupt", GuiItemButton_newBlackEx(Quad2i_init4(1, 3, 1, 1), DbValue_initLang("INTERUPT"), &UiStartup_clickInterrupt));

	return particlesLayout;
}

void GuiItemRoot_showPicker(Vec2i pickerPos, BOOL pickerOpen, BOOL pickerFolder, BOOL pickerMultiple, const UNI* pickerAction, const UNI* pickerExts)
{
	g_GuiItemRoot->showPicker = TRUE;
	g_GuiItemRoot->pickerPos = pickerPos;
	g_GuiItemRoot->pickerOpen = pickerOpen;
	g_GuiItemRoot->pickerFolder = pickerFolder;
	g_GuiItemRoot->pickerMultiple = pickerMultiple;
	g_GuiItemRoot->pickerAction = pickerAction;
	g_GuiItemRoot->pickerExts = pickerExts;
}

void GuiItemRoot_redrawAll(void)
{
	BIG i;
	for (i = 0; i < GuiItemRoot_numLevels(); i++)
		GuiItem_setRedraw(GuiItemRoot_getLevel(i), TRUE);
}

void GuiItemRoot_resizeAll(void)
{
	BIG i;
	for (i = 0; i < GuiItemRoot_numLevels(); i++)
		GuiItem_setResize(GuiItemRoot_getLevel(i), TRUE);
}

void GuiItemRoot_setContent(GuiItem* content)
{
	BIG i;
	for (i = 0; i < GuiItemRoot_numLevels(); i++)
		GuiItemRoot_getLevel(i)->remove = TRUE;
	StdArr_insert(&g_GuiItemRoot->levels, 0, content);
}
void GuiItemRoot_setContentLayout(GuiItemLayout* layout)
{
	GuiItemRoot_setContent(&layout->base);
}

GuiItem* GuiItemRoot_findPath(const StdArr* origPath)
{
	BIG i;
	for (i = 0; i < GuiItemRoot_numLevels(); i++)
	{
		GuiItem* it = GuiItem_findPath(GuiItemRoot_getLevel(i), origPath, 0);
		if (it)
			return it;
	}
	return 0;
}

void GuiItemRoot_addDialog(GuiItem* item)
{
	GuiItem* level = GuiItemLevel_newCenter(FALSE, item);

	GuiItemLayout* layout = GuiItemLayout_new(Quad2i_init4(0, 0, 1, 1));
	layout->drawBackground = FALSE;
	GuiItemLayout_addColumn(layout, 0, 100);
	GuiItemLayout_addRow(layout, 0, 100);
	GuiItem_addSubName((GuiItem*)layout, "level_center", level);

	StdArr_add(&g_GuiItemRoot->levels, &layout->base);

	GuiItem_setFirstCursor(&layout->base);
}
void GuiItemRoot_addDialogLayout(GuiItemLayout* layout)
{
	GuiItemRoot_addDialog(&layout->base);
}

void GuiItemRoot_addDialogRel(GuiItem* item, GuiItem* parent, Quad2i parentCoord, BOOL closeAfter)
{
	StdArr origPath = StdArr_init();
	GuiItem_createBackChain(parent, &origPath);
	GuiItem* level = GuiItemLevel_newRelative(closeAfter, item, parentCoord, origPath);

//GuiItemLevel_printPath((GuiItemLevel*)level);

	//if (parent)
	//	GuiItem_copyAttributes(level, parent);

	GuiItemLayout* layout = GuiItemLayout_new(Quad2i_init4(0, 0, 1, 1));
	layout->drawBackground = FALSE;
	GuiItemLayout_addColumn(layout, 0, 100);
	GuiItemLayout_addRow(layout, 0, 100);
	GuiItem_addSubName((GuiItem*)layout, "level_rel", level);

	StdArr_add(&g_GuiItemRoot->levels, &layout->base);

	GuiItem_setFirstCursor(&layout->base);
}
void GuiItemRoot_addDialogRelLayout(GuiItemLayout* layout, GuiItem* parent, Quad2i parentCoord, BOOL closeAfter)
{
	GuiItemRoot_addDialogRel(&layout->base, parent, parentCoord, closeAfter);
}

GuiItemLayout* GuiItemRoot_createDialogLayout(Vec2i maxGrid, DbValue name, GuiItem* content, GuiItemCallback* clickClose)
{
	if (!clickClose)
		clickClose = &GuiItem_closeParentLevel;

	//layout
	GuiItemLayout* layout = GuiItemLayout_new(Quad2i_init4(0, 0, 1, 1));
	GuiItemLayout_setDrawBorder(layout, TRUE);

	GuiItemLayout_addColumn(layout, 1, maxGrid.x);
	GuiItemLayout_addRow(layout, 2, maxGrid.y + 1);	//2=extra top+bottom rows
	GuiItemLayout_addRow(layout, 3, 1);

	GuiItem_addSubName((GuiItem*)layout, "name", GuiItemText_new(Quad2i_init4(0, 0, 2, 1), TRUE, name, DbValue_initEmpty()));								//title
	GuiItem* t = GuiItem_addSubName((GuiItem*)layout, "x", GuiItemButton_newAlphaEx(Quad2i_init4(2, 0, 1, 1), DbValue_initStaticCopy(_UNI32("X")), clickClose));	//close
	GuiItem_setShortcutKey(t, FALSE, Win_EXTRAKEY_ESCAPE, 0, clickClose);

	GuiItem_setGrid(content, Quad2i_init4(1, 2, 1, 1));
	GuiItem_addSubName((GuiItem*)layout, "content", content);

	return layout;
}

void GuiItemRoot_addBufferRect(Quad2i rect)
{
	if (Quad2i_isZero(g_GuiItemRoot->redrawRect))
		g_GuiItemRoot->redrawRect = rect;
	else
		g_GuiItemRoot->redrawRect = Quad2i_extend(g_GuiItemRoot->redrawRect, rect);
}

void GuiItemRoot_setDrawRectOver(Quad2i drawRectOver)
{
	g_GuiItemRoot->drawRectOver = drawRectOver;
}
void GuiItemRoot_setDrawRectOver2(Quad2i drawRectOver)
{
	g_GuiItemRoot->drawRectOver2 = drawRectOver;
}

void GuiItemRoot_closeLevels(void)
{
	BIG i;
	for (i = 1; i < GuiItemRoot_numLevels(); i++)
		GuiItemRoot_getLevel(i)->remove = TRUE;
}

void GuiItemRoot_closeLevelTop(void)
{
	UBIG N = GuiItemRoot_numLevels();
	if(N > 1)
		GuiItemRoot_getLevel(N-1)->remove = TRUE;
}

void GuiItemRoot_key(Win* win)
{
	if (OsWinIO_getKeyExtra() & Win_EXTRAKEY_ESCAPE)
	{
		StdProgress_run(FALSE);

		if (OsWinIO_getKeyExtra() & Win_EXTRAKEY_SHIFT)	//SHIFT + ESC
			GuiItemRoot_closeLevels();
	}

	if (OsWinIO_getKeyExtra() & Win_EXTRAKEY_TAB)
	{
		if (GuiItemRoot_numLevels())
			GuiItem_setNextCursor(GuiItemRoot_getLevelTop());
	}
}

BOOL GuiItemRoot_hasChanges(void)
{
	return (g_GuiItemRoot->numChanges != DbRoot_numInfoChanges());
}
void GuiItemRoot_resetNumChanges(void)
{
	g_GuiItemRoot->numChanges = DbRoot_numInfoChanges();
}

static Quad2i _GuiItemRoot_getScreenRect(Win* win)
{
	Quad2i screenRect;
	Win_getScreenRectEx(win, &screenRect);
	return screenRect;
}

static void _GuiItemRoot_callResize(GuiItem* content, Win* win)
{
	//GuiItem_tryRemove(content);
	GuiItem_setTouch(content, (content == GuiItemRoot_getLevelTop() || content == &g_GuiItemRoot->particles->base));
	GuiItem_setCoord(content, Quad2i_init2(Vec2i_init(), Win_getImage(win).size));

	if (content->type == GuiItem_LAYOUT)
		GuiItem_resize(content, (GuiItemLayout*)content, win);

	GuiItem_updateCoord(content, Vec2i_init(), _GuiItemRoot_getScreenRect(win), win);
}

static void _GuiItemRoot_guiTouchUpdate(GuiItem* content, BOOL doUpdate, Win* win)
{
	if (!content)
		return;

	//touch & key
	GuiItem_touchPrior(content, win);
	GuiItem_touch(content, win);

	BOOL endTouch = (OsWinIO_getTouch_action() == Win_TOUCH_DOWN_E) || (OsWinIO_getTouch_action() == Win_TOUCH_FORCE_DOWN_E);
	if (endTouch)
		OsWinIO_resetActiveRenderItem();

	GuiItem_shortcut(content);

	GuiItem_updateCoord(content, Vec2i_init(), _GuiItemRoot_getScreenRect(win), win);

	if (doUpdate)
		GuiItem_update(content, win);

	_GuiItemRoot_callResize(content, win);	//ex.: prepare for drawing(clean, etc.)
}

static void _GuiItemRoot_guiTouchUpdateLevels(BOOL doUpdate, Win* win)
{
	BIG i;

	Win_resetCursor(win);

	//remove old
	for (i = 0; i < GuiItemRoot_numLevels(); i++)
	{
		_GuiItemRoot_callResize(GuiItemRoot_getLevel(i), win);	//ex.: prepare for first touch/key(coord, etc.)

		GuiItem* it = GuiItemRoot_getLevel(i);

		GuiItemLevel* level = (GuiItemLevel*)GuiItem_findChildType(it, GuiItem_LEVEL);

//if (level && i == 1)
//	GuiItemLevel_printPath(level);

		if (level && !GuiItemLevel_isBackChainValid(level))	//don't have parent
			it->remove = TRUE;

		if (it->remove)
		{
			GuiItem_delete(it);
			StdArr_remove(&g_GuiItemRoot->levels, i);
			GuiItemRoot_redrawAll();
			i--;
		}
	}

	//update
	for (i = GuiItemRoot_numLevels() - 1; i >= 0; i--)
	{
		_GuiItemRoot_guiTouchUpdate(GuiItemRoot_getLevel(i), doUpdate, win);
		OsWinIO_resetTouch();	//don't click through level

		if (GuiItemRoot_hasChanges())	//for eg.: Removed Table is in active panel
			break;
	}
}

static void _GuiItemRoot_guiDraw(GuiItem* content, Win* win)
{
	if (!content)
		return;

	Image4 img = Win_getImage(win);
	Image4_setRect(&img, g_GuiItemRoot->redrawRect);
	GuiItem_draw(content, win, &img);
}

static void _GuiItemRoot_guiDrawRect(Quad2i rect, Win* win)
{
	Image4 img = Win_getImage(win);

	Image4_setRect(&img, rect);
	Rgba cd = g_theme.edit;
	cd.a = 150;
	Image4_drawBoxQuadAlpha(&img, img.rect, cd);
	GuiItemRoot_addBufferRect(img.rect);
}

Quad2i _GuiItemRoot_guiDrawLayers(Win* win)
{
	BIG i;

	//computes rect
	for (i = 0; i < GuiItemRoot_numLevels(); i++)
		GuiItem_addRedraw(GuiItemRoot_getLevel(i), win);
	g_GuiItemRoot->redrawRect = Quad2i_getIntersect(g_GuiItemRoot->redrawRect, _GuiItemRoot_getScreenRect(win));

	//draw
	for (i = 0; i < GuiItemRoot_numLevels(); i++)
		_GuiItemRoot_guiDraw(GuiItemRoot_getLevel(i), win);

	//extra draw(resize, change order, etc.)
	if (!Quad2i_isZero(g_GuiItemRoot->drawRectOver))
	{
		_GuiItemRoot_guiDrawRect(g_GuiItemRoot->drawRectOver, win);
		g_GuiItemRoot->drawRectOver = Quad2i_init();
		GuiItemRoot_redrawAll();
	}
	if (!Quad2i_isZero(g_GuiItemRoot->drawRectOver2))
	{
		_GuiItemRoot_guiDrawRect(g_GuiItemRoot->drawRectOver2, win);
		g_GuiItemRoot->drawRectOver2 = Quad2i_init();
		GuiItemRoot_redrawAll();
	}

	//resets rect
	Quad2i rect = g_GuiItemRoot->redrawRect;
	g_GuiItemRoot->redrawRect = Quad2i_init();

	if (!OsWinIO_existActiveRenderItem() && DbRoot_is()) 	//not during mouse pressed moved
	{
		if (GuiItemRoot_numLevels())
		{
			if (DbRoot_tryRefreshRemote() || GuiItemRoot_hasChanges())
			{
				DbRoot_updateTables();
				GuiItemRoot_resizeAll();
				g_GuiItemRoot->numChanges = DbRoot_numInfoChanges();
			}

			DbRoot_refresh();
		}
	}

	


Image4 img = Win_getImage(g_GuiItemRoot->win);
InterRoot_render(g_GuiItemRoot->interpreter, &img, win);
rect = img.rect;
//note: povoli _GuiItemRoot_guiTouchUpdateLevels()

	Win_updateCursorReal(win);
	return rect;
}

void GuiItemRoot_tick(BOOL doUpdate, BOOL doDraw, Win* win, Quad2i* redrawRect)
{
	if (OsWinIO_isExit() && g_GuiItemRoot->particles)	//exit
		StdProgress_run(FALSE);

	if (g_GuiItemRoot->showPicker)
	{
		UNI* path = Win_showFilePicker(win, g_GuiItemRoot->pickerOpen, g_GuiItemRoot->pickerFolder, g_GuiItemRoot->pickerMultiple, Lang_find("CANCEL"), g_GuiItemRoot->pickerAction, g_GuiItemRoot->pickerExts);
		if (path && Std_sizeUNI(path))
		{
			OsWinIO_setDrop(path, &g_GuiItemRoot->pickerPos);
			OsWinIO_setTouch(&g_GuiItemRoot->pickerPos, Win_TOUCH_DOWN_E, FALSE);
		}

		g_GuiItemRoot->showPicker = FALSE;
	}

	if (g_GuiItemRoot->run < 0)
	{
		*redrawRect = _GuiItemRoot_guiDrawLayers(win);

		g_GuiItemRoot->run = 0;	//try it
		OsLockEvent_wait(&g_GuiItemRoot->syncEventEnd, 0);	//!
	}

	if (g_GuiItemRoot->run == 0)
	{
		if (g_GuiItemRoot->particles)
		{
			GuiItem_delete(&g_GuiItemRoot->particles->base);
			g_GuiItemRoot->particles = 0;
			GuiItemRoot_redrawAll();	//back to content
		}

		//run it in 2nd thread
		g_GuiItemRoot->doUpdate = doUpdate;
		g_GuiItemRoot->win = win;
		g_GuiItemRoot->run = Os_time();
		OsLockEvent_trigger(&g_GuiItemRoot->syncEventStart);

		//wait on 2nd thread with timeout
		if (OsLockEvent_wait(&g_GuiItemRoot->syncEventEnd, MAX_COMPUTING_WAIT))   //0, MAX_COMPUTING_WAIT
		{
			*redrawRect = _GuiItemRoot_guiDrawLayers(win);
			g_GuiItemRoot->run = 0;	//try it
		}
	}
	else
		if (g_GuiItemRoot->run > 0)
		{
			if (!g_GuiItemRoot->particles)
			{
				g_GuiItemRoot->particles = _GuiItemRoot_createParticles(win);
				OsWinIO_resetActiveRenderItem();
			}

			_GuiItemRoot_guiTouchUpdate(&g_GuiItemRoot->particles->base, doUpdate, win);

			GuiItem_addRedraw(&g_GuiItemRoot->particles->base, win);
			*redrawRect = g_GuiItemRoot->redrawRect;
			_GuiItemRoot_guiDraw(&g_GuiItemRoot->particles->base, win);
			g_GuiItemRoot->redrawRect = Quad2i_init();
		}
}

THREAD_FUNC(GuiItemRoot_loop, param)
{
	while (OsThread_tick(&g_GuiItemRoot->thread))
	{
		OsLockEvent_wait(&g_GuiItemRoot->syncEventStart, 0);

InterRoot_update(g_GuiItemRoot->interpreter, Win_getImage(g_GuiItemRoot->win).size, g_GuiItemRoot->win);

//		_GuiItemRoot_guiTouchUpdateLevels(g_GuiItemRoot->doUpdate, g_GuiItemRoot->win);
		StdProgress_run(TRUE);

		//OsThread_sleep(2000);
		//printf("done---\n");

		g_GuiItemRoot->run = -1; //deactivate

		OsLockEvent_trigger(&g_GuiItemRoot->syncEventEnd);
	}
	return 0;
}

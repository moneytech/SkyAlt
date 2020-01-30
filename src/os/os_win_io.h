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

typedef struct OsWinIO_s
{
	OsFont m_font_default;
	int dpi;

	void* active_render_item;
	void* cursor_render_item;

	UNI* cursor_render_item_cache;
	Vec2i cursor_render_item_pos; //cursor pos
	int cursor_render_draw_x;
	double cursor_live_time;

	double cursor_stime;
	void* dragItem;

	Vec2i m_touch_pos;

	double m_touchDown_time;
	Vec2i m_touchDown_pos;
	int m_touch_num;

	UINT m_touch_action;
	int m_touch_wheel;

	volatile UNI m_key_id;
	volatile UBIG m_key_extra;
	int m_key_buffer_pos;

	BOOL m_pleaseExit;
	BOOL m_pleaseTouch;
	BOOL m_pleaseKey;

	float* randArray;

	UNI* dropText;
	Vec2i dropPos;
}OsWinIO;

OsWinIO* g_winIO = 0;

UINT OsWinIO_getTouch_action(void)
{
	return g_winIO->m_touch_action;
}
int OsWinIO_getTouch_wheel(void)
{
	return g_winIO->m_touch_wheel;
}
int OsWinIO_getDPI(void)
{
	return g_winIO->dpi;
}

UNI OsWinIO_getKeyID(void)
{
	return g_winIO->m_key_id;
}
UBIG OsWinIO_getKeyExtra(void)
{
	return g_winIO->m_key_extra;
}

OsFont* OsWinIO_getFontDefault(void)
{
	return &g_winIO->m_font_default;
}

BOOL OsWinIO_isTouch(void)
{
	return g_winIO->m_pleaseTouch;
}
BOOL OsWinIO_isKey(void)
{
	return g_winIO->m_pleaseKey;
}
BOOL OsWinIO_isExit(void)
{
	return g_winIO->m_pleaseExit;
}

int OsWinIO_getTouchNum(void)
{
	return g_winIO->m_touch_num;
}
int OsWinIO_getTouchWheel(void)
{
	return g_winIO->m_touch_wheel;
}

Vec2i OsWinIO_getTouchPos(void)
{
	return g_winIO->m_touch_pos;
}

Vec2i OsWinIO_getCursorRenderItemPos(void)
{
	return g_winIO->cursor_render_item_pos;
}
UNI* OsWinIO_getCursorRenderItemCache(void)
{
	return g_winIO->cursor_render_item_cache;
}
void* OsWinIO_getCursorRenderItem(void)
{
	return g_winIO->cursor_render_item;
}
int OsWinIO_getCursorRenderItemDrawX(void)
{
	return g_winIO->cursor_render_draw_x;
}

void OsWinIO_setCursorRenderItemDrawX(int x)
{
	g_winIO->cursor_render_draw_x = x;
}

void OsWinIO_setCursorRenderItemPosX(int x)
{
	g_winIO->cursor_render_item_pos.x = x;
}
void OsWinIO_setCursorRenderItemPosY(int y)
{
	g_winIO->cursor_render_item_pos.y = y;
}
void OsWinIO_setCursorRenderItemCache(UNI* str)
{
	g_winIO->cursor_render_item_cache = str;
}

void OsWinIO_resetKeyID(void)
{
	g_winIO->m_key_id = 0;
}
void OsWinIO_resetKeyEXTRA(void)
{
	g_winIO->m_key_extra = Win_EXTRAKEY_NONE;
}

void OsWinIO_setTouchNum(int touch_num)
{
	g_winIO->m_touch_num = touch_num;
}
void OsWinIO_setTouch_action(UINT touch_action)
{
	g_winIO->m_touch_action = touch_action;
}

#define OsWinIO_MAX_RAND 256

static BOOL _OsWinIO_loadFont(const char* path, const char* font)
{
	char* pathF = malloc(Std_sizeCHAR(path) + 1 + Std_sizeCHAR(font) + 1);
	strcpy(pathF, path);
	strcat(pathF, "/");
	strcat(pathF, font);
	const char* err = OsFont_initFile(OsWinIO_getFontDefault(), _UNI32("Default"), pathF);
	Std_deleteCHAR(pathF);
	BOOL ok = !err;
	if (!ok)
	{
		int i;
		char** folders = 0;
		int num_folders = OsFileDir_getFileList(path, FALSE, TRUE, TRUE, &folders);
		for (i = 0; i < num_folders; i++)
		{
			if ((ok = _OsWinIO_loadFont(folders[i], font)))
				break;
		}
		for (i = 0; i < num_folders; i++)
			Std_deleteCHAR(folders[i]);
		free(folders);
	}
	return ok;
}

void OsWinIO_setDPI(UINT dpi)
{
	g_winIO->dpi = Std_clamp(dpi, 30, 5000);
}

UINT OsWinIO_cellSize(void)
{
	return g_winIO->dpi / 2.9f;
}
Vec2i OsWinIO_cellSize2(void)
{
	return Vec2i_init2(OsWinIO_cellSize(), OsWinIO_cellSize());
}

UINT OsWinIO_lineSpace(void)
{
	return OsWinIO_cellSize() / 4;
}

BOOL OsWinIO_new(void)
{
	g_winIO = malloc(sizeof(OsWinIO));
	g_winIO->active_render_item = 0;
	g_winIO->cursor_render_item = 0;

	g_winIO->cursor_render_item_cache = 0;
	g_winIO->cursor_render_item_pos = Vec2i_init();
	g_winIO->cursor_render_draw_x = 0;

	g_winIO->dragItem = 0;

	g_winIO->m_touch_pos = Vec2i_init2(-1, -1);
	g_winIO->m_touch_action = Win_TOUCH_NONE;
	g_winIO->m_touch_wheel = 0;
	g_winIO->m_touch_num = 0;
	g_winIO->m_touchDown_time = 0;
	g_winIO->m_touchDown_pos = Vec2i_init2(-1, -1);

	OsWinIO_resetKeyID();
	OsWinIO_resetKeyEXTRA();

	g_winIO->cursor_stime = Os_time();

	g_winIO->m_pleaseExit = FALSE;

	g_winIO->m_pleaseTouch = FALSE;
	g_winIO->m_pleaseKey = FALSE;

	//font
	{
		BOOL fontLoaded = FALSE;

		//	fontLoaded = !fontLoaded ? (OsFont_initFile(OsWinIO_getFontDefault(), _UNI32("Default"), "freeSans") == 0) : fontLoaded;
		fontLoaded = !fontLoaded ? (OsFont_initFile(OsWinIO_getFontDefault(), _UNI32("Default"), "arial") == 0) : fontLoaded;

		char* currPath = OsFileDir_currentDir();
		printf("%s\n", currPath);
		fontLoaded = !fontLoaded ? _OsWinIO_loadFont(currPath, "font.ttf") : fontLoaded;
		//fontLoaded = !fontLoaded ? Win_loadFont(self, FONT_DEFAULT_1, "LiberationMono-BoldItalic.ttf") : fontLoaded;
		//fontLoaded = !fontLoaded ? Win_loadFont(self, FONT_DEFAULT_2, "LiberationMono-BoldItalic.ttf") : fontLoaded;
		fontLoaded = !fontLoaded ? _OsWinIO_loadFont(FONT_DEFAULT_1, "FreeSans.ttf") : fontLoaded;
		fontLoaded = !fontLoaded ? _OsWinIO_loadFont(FONT_DEFAULT_2, "FreeSans.ttf") : fontLoaded;

		fontLoaded = !fontLoaded ? _OsWinIO_loadFont(FONT_DEFAULT_1, "micross.ttf") : fontLoaded;
		fontLoaded = !fontLoaded ? _OsWinIO_loadFont(FONT_DEFAULT_2, "micross.ttf") : fontLoaded;

		fontLoaded = !fontLoaded ? _OsWinIO_loadFont(FONT_DEFAULT_1, "Arial.ttf") : fontLoaded;
		fontLoaded = !fontLoaded ? _OsWinIO_loadFont(FONT_DEFAULT_2, "Arial.ttf") : fontLoaded;

		Std_deleteCHAR(currPath);
		if (!fontLoaded)
		{
			//printf("Error: Font not found\n");
			return FALSE;
		}
	}

	g_winIO->dpi = OsScreen_getDPI();

	g_winIO->cursor_live_time = 0;

	g_winIO->randArray = malloc(OsWinIO_MAX_RAND * sizeof(float));
	int i;
	for (i = 0; i < OsWinIO_MAX_RAND; i++)
		g_winIO->randArray[i] = OsCrypto_random01();

	g_winIO->dropText = 0;
	g_winIO->dropPos = Vec2i_init2(-1, -1);

	return TRUE;
}

void OsWinIO_delete(void)
{
	if (!g_winIO)
		return;

	Std_deleteUNI(g_winIO->dropText);

	free(g_winIO->randArray);
	Std_deleteUNI(g_winIO->cursor_render_item_cache);
	OsFont_free(&g_winIO->m_font_default);

	Os_free(g_winIO, sizeof(OsWinIO));
	g_winIO = 0;
}

void OsWinIO_setDrop(UNI* str, Vec2i* pos)
{
	Std_deleteUNI(g_winIO->dropText);
	g_winIO->dropText = str;
	g_winIO->dropPos = *pos;
}

void OsWinIO_setDropFromInside(const UNI* str, Vec2i* pos)
{
	OsWinIO_setDrop(Std_addUNI(STD_DROP_FILE_HEADER, str), pos);
}

void OsWinIO_resetDrop(void)
{
	Vec2i t = Vec2i_init2(-1, -1);
	OsWinIO_setDrop(0, &t);
}
BOOL OsWinIO_isDrop(Quad2i* rect)
{
	return Quad2i_inside(*rect, g_winIO->dropPos) && g_winIO->dropText;
}

UNI* OsWinIO_getDropFile(int i)
{
	UNI* str = g_winIO->dropText;
	if (!str)
		return 0;

	int pos_second = Std_sizeUNI(str);
	int pos_first = pos_second + 1;	//higher than second

	int pos = 0;
	while (i >= 0 && pos <= pos_second)// && pos_first==-1)
	{
		int pp = Std_subUNI(&str[pos], STD_DROP_FILE_HEADER);
		if (pp >= 0)
		{
			pos += pp + Std_sizeUNI(STD_DROP_FILE_HEADER);
			if (i == 0)
				pos_first = pos;
		}
		i--;
	}

	//extract
	UNI* ret = (pos_second > pos_first) ? Std_newUNI_copy(g_winIO->dropText + pos_first, pos_second - pos_first) : 0;
	if (ret)
	{
		//get rid of 'enter' characters
		int p = Std_findUNI(ret, '\r');
		if (p >= 0)	ret[p] = 0;

		p = Std_findUNI(ret, '\n');
		if (p >= 0)	ret[p] = 0;
	}

	return ret;
}

void OsWinIO_setDrag(void* item)
{
	g_winIO->dragItem = item;
}

void OsWinIO_resetDrag(void)
{
	OsWinIO_setDrag(0);
}

BOOL OsWinIO_isDragActive(void)
{
	return g_winIO->dragItem != 0;
}

void OsWinIO_setActiveRenderItem(void* item)
{
	g_winIO->active_render_item = item;
}

void OsWinIO_resetActiveRenderItem(void)
{
	OsWinIO_setActiveRenderItem(0);
}

BOOL OsWinIO_isActiveRenderItem(const void* item)
{
	return(g_winIO->active_render_item == item);
}

BOOL OsWinIO_existActiveRenderItem(void)
{
	return(g_winIO->active_render_item != 0);
}

BOOL OsWinIO_canActiveRenderItem(const void* item)
{
	return(g_winIO->active_render_item == 0) || OsWinIO_isActiveRenderItem(item);
}

BOOL OsWinIO_isCursorEmpty(void)
{
	return g_winIO->cursor_render_item == 0;
}

BOOL OsWinIO_isCursorGuiItem(void* item)
{
	return (g_winIO->cursor_render_item == item);
}

BOOL OsWinIO_isCursorGuiItemInTime(void* item)
{
	return OsWinIO_isCursorGuiItem(item) && g_winIO->cursor_render_item_pos.x == g_winIO->cursor_render_item_pos.y && (Os_time() - g_winIO->cursor_live_time) < 6;
}

void OsWinIO_updateCursorTimeout(void)
{
	g_winIO->cursor_live_time = Os_time();
}

double OsWinIO_getEditboxAnim(void* item)
{
	return OsWinIO_isCursorGuiItemInTime(item) ? Std_timeAprox3(g_winIO->cursor_stime, 2) : 1.0;
}

void OsWinIO_tryRemoveCursorGuiItem(void* item)
{
	if (g_winIO->cursor_render_item == item)
		g_winIO->cursor_render_item = 0;
	if (g_winIO->active_render_item == item)
		g_winIO->active_render_item = 0;
}

BOOL OsWinIO_isStartTouch(void)
{
	return g_winIO->m_touch_action == Win_TOUCH_DOWN_S || g_winIO->m_touch_action == Win_TOUCH_FORCE_DOWN_S;
}

BOOL OsWinIO_setCursorGuiItem(void* item)
{
	BOOL same = OsWinIO_isCursorGuiItem(item);

	g_winIO->cursor_stime = Os_time();
	g_winIO->cursor_render_draw_x = 0;
	g_winIO->cursor_render_item = item;

	return same;
}

void OsWinIO_setCursorText(const UNI* text)
{
	Std_deleteUNI(g_winIO->cursor_render_item_cache);

	g_winIO->cursor_render_item_cache = Std_newUNI(text);
}
void OsWinIO_resetCursorGuiItem(void)
{
	OsWinIO_setCursorText(0);
	g_winIO->cursor_render_item = 0;
	g_winIO->cursor_render_item_pos = Vec2i_init();
	OsWinIO_setCursorGuiItem(0);
}

void OsWinIO_pleaseExit(void)
{
	g_winIO->m_pleaseExit = TRUE;
}

void OsWinIO_pleaseTouch(void)
{
	g_winIO->m_pleaseTouch = TRUE;
}

void OsWinIO_pleaseKey(void)
{
	g_winIO->m_pleaseKey = TRUE;
}

void OsWinIO_setKey(UNI key)
{
	g_winIO->m_key_id = key;
	OsWinIO_pleaseKey();
}

void OsWinIO_setKeyExtra(UBIG extra_key)
{
	g_winIO->m_key_extra = extra_key;
	OsWinIO_pleaseKey();
}

BOOL OsWinIO_isAnotherClickOutOfTime(void)
{
	return (Os_time() - g_winIO->m_touchDown_time) > 0.5f;
}

void OsWinIO_setTouch(Vec2i* pos, UINT action, BOOL move)
{
	g_winIO->m_touch_pos = *pos;

	if (g_winIO->m_touch_num > 0 && (action == Win_TOUCH_DOWN_S || action == Win_TOUCH_FORCE_DOWN_S))
		if (OsWinIO_isAnotherClickOutOfTime() || Vec2i_distance(g_winIO->m_touchDown_pos, g_winIO->m_touch_pos) > 5)
			g_winIO->m_touch_num = 0;

	//get doubleClick
	if (action == Win_TOUCH_DOWN_E || action == Win_TOUCH_FORCE_DOWN_E)
	{
		g_winIO->m_touchDown_time = Os_time();
		g_winIO->m_touchDown_pos = *pos;
	}
	else
		if (action == Win_TOUCH_DOWN_S || action == Win_TOUCH_FORCE_DOWN_S)
		{
			g_winIO->m_touch_num++;

			if (!move)
				g_winIO->m_touchDown_pos = *pos;
		}

	if (action)
		g_winIO->m_touch_action = action;
	OsWinIO_pleaseTouch();
}

void OsWinIO_setTouchWheel(Vec2i* pos, int wheel)
{
	OsWinIO_setTouch(pos, Win_TOUCH_WHEEL, FALSE);
	g_winIO->m_touch_wheel = wheel;
}

void OsWinIO_resetTouch(void)
{
	g_winIO->m_touch_action = Win_TOUCH_NONE;
	//g_winIO->m_touch_pos = Vec2i_init2(-1, -1);
	g_winIO->m_touch_wheel = 0;
	//g_winIO->m_touch_num = 0;
}

void OsWinIO_resetNumTouch(void)
{
	g_winIO->m_touch_num = 0;
}

void OsWinIO_resetKey(void)
{
	g_winIO->m_key_id = 0;
	g_winIO->m_key_extra = Win_EXTRAKEY_NONE;
}

void OsWinIO_tick(void)
{
	if (g_winIO->m_pleaseTouch || g_winIO->m_pleaseKey)
		OsWinIO_updateCursorTimeout();

	g_winIO->m_pleaseExit = FALSE;
	g_winIO->m_pleaseTouch = FALSE;
	g_winIO->m_pleaseKey = FALSE;

	g_winIO->m_touch_action = Win_TOUCH_NONE;
	g_winIO->m_touch_wheel = 0;

	OsWinIO_resetKey();

	if (((g_winIO->m_touch_action == Win_TOUCH_DOWN_S) || (g_winIO->m_touch_action == Win_TOUCH_FORCE_DOWN_S)) && !g_winIO->active_render_item)
	{
		g_winIO->active_render_item = (void*)1; //If I clicked into empty area I don't allow highlighting other items until click is realeased
	}

	if (g_winIO->m_touch_action == Win_TOUCH_DOWN_E || g_winIO->m_touch_action == Win_TOUCH_FORCE_DOWN_E || g_winIO->m_touch_action == Win_TOUCH_WHEEL)
	{
		OsWinIO_resetActiveRenderItem();
		g_winIO->m_touch_action = Win_TOUCH_NONE;
	}
}
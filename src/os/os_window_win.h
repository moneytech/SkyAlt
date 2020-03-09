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

typedef struct Win_s
{
	BOOL m_fullscreen;
	Vec2i canvas_size;
	BOOL pleaseResize;
	Quad2i backup_nofullscreen;

	Image4 img;

	HWND m_hWnd;
	WNDCLASSW m_wc;
	HINSTANCE m_hInstance;
	BITMAPINFO m_bi_src;

	HCURSOR actual_cursor;

	HCURSOR cursor_def;
	HCURSOR cursor_ibeam;
	HCURSOR cursor_wait;
	HCURSOR cursor_hand;
	HCURSOR cursor_fleur;
	HCURSOR cursor_col_resize;
	HCURSOR cursor_row_resize;
	HCURSOR cursor_move;

	BOOL(*tickFn)(void*, Quad2i* redrawRect);
	void* tickSelf;

	char* title;
	BOOL title_update;
} Win;

BOOL Win_isResize(Win* self)
{
	return self->pleaseResize;
}
void Win_pleaseResize(Win* self)
{
	self->pleaseResize = TRUE;
}
void Win_resetResize(Win* self)
{
	self->pleaseResize = FALSE;
}

BOOL Win_isFullscreen(Win* self)
{
	return self->m_fullscreen;
}

static void _Win_resize(Win* self, Vec2i size)
{
	if (!Vec2i_cmp(size, self->canvas_size))
	{
		Image4_resize(&self->img, size);

		ZeroMemory(&self->m_bi_src, sizeof(self->m_bi_src));
		self->m_bi_src.bmiHeader.biSize = sizeof(self->m_bi_src.bmiHeader);
		self->m_bi_src.bmiHeader.biWidth = size.x;
		self->m_bi_src.bmiHeader.biHeight = -size.y;	//minus!
		self->m_bi_src.bmiHeader.biPlanes = 1;
		self->m_bi_src.bmiHeader.biBitCount = 32;	//24 was working fine
		self->m_bi_src.bmiHeader.biCompression = BI_RGB;

		self->canvas_size = size;
		self->pleaseResize = TRUE;
	}
}

Quad2i Win_getScreenRect(const Win* self)
{
	return Quad2i_init2(Vec2i_init(), self->canvas_size);
}
void Win_getScreenRectEx(const Win* self, Quad2i* rect)
{
	*rect = Win_getScreenRect(self);
}

Image4 Win_getImage(Win* self)
{
	return self->img;
}

void Win_savePrintscreen(Win* self)
{
	char path[256];

	UBIG i = 0;
	do
	{
		snprintf(path, 255, "image_%lld.jpg", i++);
	} while (OsFile_existFile(path) && i < 1000000);

	Image4 img = Win_getImage(self);
	Image4_saveJpeg(&img, path);
}

void Win_updateCursor(Win* self, Win_CURSOR cursor)
{
	if (self->actual_cursor != self->cursor_def)	//let it set only once
		return;

	switch (cursor)
	{
		case Win_CURSOR_DEF: self->actual_cursor = self->cursor_def;
			break;
		case Win_CURSOR_IBEAM: self->actual_cursor = self->cursor_ibeam;
			break;
		case Win_CURSOR_WAIT: self->actual_cursor = self->cursor_wait;
			break;
		case Win_CURSOR_HAND: self->actual_cursor = self->cursor_hand;
			break;
		case Win_CURSOR_FLEUR: self->actual_cursor = self->cursor_fleur;
			break;
		case Win_CURSOR_COL_RESIZE: self->actual_cursor = self->cursor_col_resize;
			break;
		case Win_CURSOR_ROW_RESIZE: self->actual_cursor = self->cursor_row_resize;
			break;
		case Win_CURSOR_MOVE:	self->actual_cursor = self->cursor_move;
			break;
	}
}
void Win_resetCursor(Win* self)
{
	self->actual_cursor = self->cursor_def;
}

void Win_updateCursorReal(Win* self)
{
	if (GetCursor() != self->actual_cursor)
	{
		HCURSOR h = GetCursor();
		if(	h == self->cursor_def)	//be sure that window resize cursor shows corectly
			SetCursor(self->actual_cursor);
	}
}

static void _Win_setKeyExtra(UNI key)
{
	UBIG key_extra = Win_EXTRAKEY_NONE;

	//if (GetAsyncKeyState(VK_MENU) & 0x8000) //Left or Right ALT
	//	key_extra |= Win_EXTRAKEY_ALT;

	if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
		key_extra |= Win_EXTRAKEY_CTRL;

	if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
	{
		key_extra |= Win_EXTRAKEY_SHIFT;

		if (GetAsyncKeyState(VK_RETURN))
			key_extra |= Win_EXTRAKEY_SELECT_ROW;
	}

	if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
	{
		if (GetAsyncKeyState(VK_ADD) & 0x8000) key_extra |= Win_EXTRAKEY_ZOOM_A; //+
		if (GetAsyncKeyState(VK_SUBTRACT) & 0x8000) key_extra |= Win_EXTRAKEY_ZOOM_S; //-
		if (GetAsyncKeyState(VK_NUMPAD0) & 0x8000)	key_extra |= Win_EXTRAKEY_ZOOM_0; //0

		if (key == 'a' || key == 'A') key_extra |= Win_EXTRAKEY_SELECT_ALL;
		if (key == 'x' || key == 'X') key_extra |= Win_EXTRAKEY_CUT;
		if (key == 'c' || key == 'C') key_extra |= Win_EXTRAKEY_COPY;
		if (key == 'v' || key == 'V') key_extra |= Win_EXTRAKEY_PASTE;
		if (key == 'd' || key == 'D') key_extra |= Win_EXTRAKEY_DUPLICATE;

		if (key == 'f' || key == 'F') key_extra |= Win_EXTRAKEY_SEARCH;
		if (key == 't' || key == 'T') key_extra |= Win_EXTRAKEY_THEME;

		if (key == 'n' || key == 'N') key_extra |= Win_EXTRAKEY_NEW;
		if (key == 's' || key == 'S') key_extra |= Win_EXTRAKEY_SAVE;

		if (key == 'l' || key == 'L') key_extra |= Win_EXTRAKEY_LOG;

		if (key == 'z' || key == 'Z') key_extra |= Win_EXTRAKEY_BACK;
		if (key == 'y' || key == 'Y') key_extra |= Win_EXTRAKEY_FORWARD;

		if (key == 'b' || key == 'B') key_extra |= Win_EXTRAKEY_BYPASS;
		if (key == ';') key_extra |= Win_EXTRAKEY_COMMENT; //;

		if (GetAsyncKeyState(VK_RETURN)) key_extra |= Win_EXTRAKEY_SELECT_COLUMN;

		if (key == 'r' || key == 'R') key_extra |= Win_EXTRAKEY_ADD_RECORD;

		if (key == 'g' || key == 'G') key_extra |= Win_EXTRAKEY_GOTO;	
	}
	else
	{
		if (GetAsyncKeyState(VK_DELETE) & 0x8000) key_extra |= Win_EXTRAKEY_DELETE;
		if (GetAsyncKeyState(VK_HOME) & 0x8000) key_extra |= Win_EXTRAKEY_HOME;
		if (GetAsyncKeyState(VK_END) & 0x8000) key_extra |= Win_EXTRAKEY_END;
		if (GetAsyncKeyState(VK_BACK) & 0x8000) key_extra |= Win_EXTRAKEY_BACKSPACE;
		if (GetAsyncKeyState(VK_INSERT) & 0x8000) key_extra |= Win_EXTRAKEY_INSERT;
		if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) key_extra |= Win_EXTRAKEY_ESCAPE;
		if (GetAsyncKeyState(VK_PRIOR) & 0x8000) key_extra |= Win_EXTRAKEY_PAGE_U;
		if (GetAsyncKeyState(VK_NEXT) & 0x8000) key_extra |= Win_EXTRAKEY_PAGE_D;

		if (GetAsyncKeyState(VK_TAB) & 0x8000) key_extra |= Win_EXTRAKEY_TAB;
		if (GetAsyncKeyState(VK_RETURN) & 0x8000) key_extra |= Win_EXTRAKEY_ENTER;

		if (GetAsyncKeyState(VK_LEFT) & 0x8000) key_extra |= Win_EXTRAKEY_LEFT;
		if (GetAsyncKeyState(VK_RIGHT) & 0x8000) key_extra |= Win_EXTRAKEY_RIGHT;
		if (GetAsyncKeyState(VK_UP) & 0x8000) key_extra |= Win_EXTRAKEY_UP;
		if (GetAsyncKeyState(VK_DOWN) & 0x8000) key_extra |= Win_EXTRAKEY_DOWN;

		if (GetAsyncKeyState(VK_F5) & 0x8000) key_extra |= Win_EXTRAKEY_PRINTSCREEN;
		if (GetAsyncKeyState(VK_F11) & 0x8000) key_extra |= Win_EXTRAKEY_FULLSCREEN;
		if (GetAsyncKeyState(VK_F2) & 0x8000) key_extra |= Win_EXTRAKEY_LPANEL;
	}

	if (key_extra)
		OsWinIO_setKeyExtra(key_extra);
}

static UNI* _Win_clipboard_convertToUNI(char* text)
{
	int N = MultiByteToWideChar(CP_UTF8, 0, text, -1, NULL, 0);
	USHORT* wstr = calloc(N + 1, sizeof(USHORT));
	MultiByteToWideChar(CP_UTF8, 0, text, -1, wstr, N);

	//copy
	UNI* str = Std_newUNI_N(N);
	int i;
	for (i = 0; i < N; i++)
		str[i] = wstr[i];

	free(wstr);

	return str;
}

Win* s_win = 0;
LRESULT CALLBACK Win_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	Win* self = s_win;
	if (self && self->m_hWnd == hWnd)
	{
		Vec2i v = Vec2i_init2(LOWORD(lParam), HIWORD(lParam));
		switch (uMsg)
		{
			case WM_ACTIVATE:
			{
				//win->readyToDraw = !HIWORD(wParam);
				if (!HIWORD(wParam))
					Win_pleaseResize(self);
				return 0;
			}

			case WM_SIZE:
			{
				//Win_pleaseResize(win, v);
				return 0;
			}

			case WM_PAINT:
			{
				//Win_pleaseResize(win);
				//Win_pleaseDraw(win);
				break;	//return 0;
			}

			case WM_CLOSE:
			{
				PostMessage(hWnd, WM_QUIT, 0, 0);
				return 0;
			}

			case WM_LBUTTONDOWN:
			{
				SetCapture(hWnd); _Win_setKeyExtra(0);
				OsWinIO_setTouch(&v, Win_TOUCH_DOWN_S, FALSE);		InvalidateRect(hWnd, NULL, FALSE);	break;
			}
			case WM_RBUTTONDOWN: { SetCapture(hWnd); _Win_setKeyExtra(0); OsWinIO_setTouch(&v, Win_TOUCH_FORCE_DOWN_S, FALSE);	InvalidateRect(hWnd, NULL, FALSE); break; }
			case WM_MBUTTONDOWN: { SetCapture(hWnd); _Win_setKeyExtra(0); OsWinIO_setTouch(&v, Win_TOUCH_FORCE_DOWN_S, FALSE);	InvalidateRect(hWnd, NULL, FALSE); break; }

			case WM_LBUTTONUP:
			{
				_Win_setKeyExtra(0);
				OsWinIO_setTouch(&v, Win_TOUCH_DOWN_E, FALSE);		ReleaseCapture(); break;
			}
			case WM_RBUTTONUP: { _Win_setKeyExtra(0); OsWinIO_setTouch(&v, Win_TOUCH_FORCE_DOWN_E, FALSE);	ReleaseCapture(); break; }
			case WM_MBUTTONUP: { _Win_setKeyExtra(0); OsWinIO_setTouch(&v, Win_TOUCH_FORCE_DOWN_E, FALSE);	ReleaseCapture(); break; }

							 //case WM_LBUTTONDBLCLK: { OsWinIO_setTouch(&v, Win_TOUCH_DOWN_S, FALSE); OsWinIO_setTouch(&v, Win_TOUCH_DOWN_S, FALSE);	break; }

			case WM_MOUSEWHEEL:
			{
				//convert
				POINT pt;	pt.x = v.x;		pt.y = v.y;
				ScreenToClient(hWnd, &pt);
				v.x = pt.x;
				v.y = pt.y;

				_Win_setKeyExtra(0);
				OsWinIO_setTouchWheel(&v, GET_WHEEL_DELTA_WPARAM(wParam) / -WHEEL_DELTA);
				break;
			}
			case WM_MOUSEMOVE:
			{
				OsWinIO_setTouch(&v, OsWinIO_getTouch_action(), TRUE);
				InvalidateRect(hWnd, NULL, FALSE);
				break;
			}

			case WM_KEYDOWN:
			{
				_Win_setKeyExtra(wParam);
				break;
			}

			case WM_KEYUP:
			{
				_Win_setKeyExtra(0);
				break;
			}

			case WM_CHAR:
			{
				if (wParam >= 32)	//ignore extra keys
					OsWinIO_setKey((unsigned int)wParam);
				break;
			}

			case WM_SETCURSOR:
			{
				//Win_updateCursorReal(self);

				//if (Quad2i_inside(Quad2i_addSpace(Quad2i_init2(Vec2i_init(), self->canvas_size), 10), g_winIO->m_touch_pos))
				//	return TRUE;

				break;	//default cursor(window resize)
			}

			case WM_DROPFILES:
			{
				HDROP hDrop = (HDROP)wParam;

				UNI* dec = 0;
				const int N = DragQueryFile(hDrop, 0xFFFFFFFF, 0, 0);
				int i;
				for (i = 0; i < N; i++)
				{
					char lpszFile[MAX_PATH] = { 0 };

					if (DragQueryFile(hDrop, i, lpszFile, MAX_PATH))
					{
						UNI* file = _Win_clipboard_convertToUNI(lpszFile);
						dec = Std_addAfterUNI_char(dec, STD_DROP_FILE_HEADER_CHAR);
						dec = Std_addAfterUNI(dec, file);
						dec = Std_addAfterUNI_char(dec, "\r\n");
						free(file);
					}
				}

				POINT ppt;
				DragQueryPoint(hDrop, &ppt);
				Vec2i pos = Vec2i_init2(ppt.x, ppt.y);

				OsWinIO_setDrop(dec, &pos);
				OsWinIO_setTouch(&pos, Win_TOUCH_DOWN_E, FALSE);

				DragFinish(hDrop);
				break;
			}
		}
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void Win_init(void)
{
	//on Windows(Release mode only, Debug is ok), gtk_init() fucked something(probably '.' -> ',') so atof(), snprintf(), etc. returns invalid values, so this will return things to default
	setlocale(LC_ALL, "C");
}

const DWORD g_dwStyle = WS_OVERLAPPEDWINDOW/*Show controls*/ | WS_POPUP | WS_MINIMIZEBOX | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
const DWORD g_dwExStyle = 0;

Win* Win_new(Quad2i* abs_coord, BOOL(*tickFn)(void*, Quad2i* redrawRect), void* tickSelf)
{
	Quad2i res;
	OsScreen_getMonitorCoord(&res);
	if (Quad2i_cmp(*abs_coord, res))
		OsScreen_getDefaultCoord(abs_coord); //never start in fullscreen

	Win* self = malloc(sizeof(Win));

	s_win = self;

	self->title = 0;
	self->title_update = FALSE;
	self->tickFn = tickFn;
	self->tickSelf = tickSelf;
	self->m_fullscreen = FALSE;

	self->canvas_size = Vec2i_init();
	self->img = Image4_init();
	self->pleaseResize = FALSE;

	self->m_hWnd = 0;
	self->m_hInstance = GetModuleHandle(NULL);

	self->m_wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;// | CS_DBLCLKS;
	self->m_wc.lpfnWndProc = Win_WndProc;
	self->m_wc.cbClsExtra = 0;
	self->m_wc.cbWndExtra = 0;
	self->m_wc.hInstance = self->m_hInstance;
	self->m_wc.hIcon = LoadIcon(NULL, MAKEINTRESOURCE("icon.ico"));
	self->m_wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	self->m_wc.hbrBackground = NULL;
	self->m_wc.lpszMenuName = NULL;
	self->m_wc.lpszClassName = L"SKYALT";

	if (!RegisterClassW(&self->m_wc))
	{
		free(self);
		return 0;
	}

	DWORD dwExStyle = 0;//WS_EX_APPWINDOW;

	if (!(self->m_hWnd = CreateWindowExW(g_dwExStyle,
		L"SKYALT",
		L"SkyAlt",
		g_dwStyle,
		abs_coord->start.x, abs_coord->start.y,
		abs_coord->size.x, abs_coord->size.y,
		NULL,
		NULL,
		self->m_hInstance,
		NULL)))
	{
		free(self);
		return FALSE;
	}

	DragAcceptFiles(self->m_hWnd, TRUE);

	self->cursor_def = LoadCursor(0, IDC_ARROW);
	self->cursor_ibeam = LoadCursor(0, IDC_IBEAM);
	self->cursor_wait = LoadCursor(0, IDC_WAIT);
	self->cursor_hand = LoadCursor(0, IDC_HAND);
	self->cursor_fleur = LoadCursor(0, IDC_SIZENWSE);	//eg.: extend cells ... IDC_CROSS

	self->cursor_col_resize = LoadCursor(0, IDC_SIZEWE);
	self->cursor_row_resize = LoadCursor(0, IDC_SIZENS);
	self->cursor_move = LoadCursor(0, IDC_SIZEALL);

	Win_resetCursor(self);

	_Win_resize(self, abs_coord->size);

	return self;
}

void Win_delete(Win* self)
{
	Std_deleteCHAR(self->title);

	memset(self, 0, sizeof(Win));
	free(self);
}

void Win_getWindowCoord(Win* self, Quad2i* out)
{
	RECT rect;
	GetWindowRect(self->m_hWnd, &rect);

	out->start.x = rect.left;
	out->start.y = rect.top;
	out->size.x = rect.right - rect.left;
	out->size.y = rect.bottom - rect.top;
}
void Win_getWindowInnerCoord(Win* self, Quad2i* out)
{
	RECT rect;
	GetClientRect(self->m_hWnd, &rect);

	out->start.x = rect.left;
	out->start.y = rect.top;
	out->size.x = rect.right - rect.left;
	out->size.y = rect.bottom - rect.top;
}

static void _Win_display(Win* self, Quad2i rect)
{
	RECT r;
	r.left = rect.start.x;
	r.top = rect.start.y;
	r.right = Quad2i_end(rect).x;
	r.bottom = Quad2i_end(rect).y;
	InvalidateRect(self->m_hWnd, &r, FALSE);

	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(self->m_hWnd, &ps);

	/*	SetDIBitsToDevice(hdc,
							rect.start.x, self->canvas_size.y - (rect.start.y+rect.size.y),	//dest start
							rect.size.x, rect.size.y,										//src size
							rect.start.x, rect.start.y,										//src start
							0, self->canvas_size.y,
							self->img.data, &self->m_bi_src, DIB_RGB_COLORS);*/

	SetStretchBltMode(hdc, HALFTONE);	//!
	int n = StretchDIBits(hdc,
		rect.start.x, rect.start.y,//self->canvas_size.y - (rect.start.y+rect.size.y),
		rect.size.x, rect.size.y,

		rect.start.x, self->canvas_size.y - (rect.start.y + rect.size.y),
		rect.size.x, rect.size.y,
		self->img.data, &self->m_bi_src, DIB_RGB_COLORS, SRCCOPY);

	EndPaint(self->m_hWnd, &ps);
}

BOOL _Win_tick(Win* self)
{
	Quad2i rectImg;
	Win_getWindowInnerCoord(self, &rectImg);
	_Win_resize(self, rectImg.size);	//accurate

	if (self->title_update)
	{
		SetWindowTextW(self->m_hWnd, (LPCWSTR)self->title);
		self->title_update = FALSE;
	}

	MSG msg;
	if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))	//without "m_hWnd" => faster response! Also must by if() not while(), because otherwise it can do clickDown and clickUp in once and Gui will never know
	{
		if (msg.message == WM_QUIT)
			OsWinIO_pleaseExit();
		else
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	//if (OsWinIO_getKeyExtra())
	//	OsWinIO_resetKeyID();

	if (OsWinIO_getKeyExtra() & Win_EXTRAKEY_PRINTSCREEN)
		Win_savePrintscreen(self);

	Quad2i redrawRect = Quad2i_init();
	if (!self->tickFn(self->tickSelf, &redrawRect))	//call project "logic"
		return FALSE;

	if (!Quad2i_isZero(redrawRect))
	{
		Quad2i rect = Quad2i_getIntersect(redrawRect, Win_getScreenRect(self));
		//Quad2i_print(redrawRect, "win");
		//Win_savePrintscreen(self);
		_Win_display(self, rect);
	}

	return TRUE;
}

void Win_start(Win* self)
{
	while (_Win_tick(self))
		OsThread_sleep(1);
}

void Win_setTitle(Win* self, const char* title)
{
	if (!Std_cmpCHAR(self->title, title))
	{
		Std_deleteCHAR(self->title);
		self->title = Std_newCHAR(title);
		self->title_update = TRUE;
	}
}

void Win_setFullscreen(Win* self, BOOL fullscreen)
{
	if (fullscreen)
	{
		if (!self->m_fullscreen)
		{
			Win_getWindowCoord(self, &self->backup_nofullscreen);
			Vec2i mon_res;
			OsScreen_getMonitorResolution(&mon_res);
			SetWindowLongPtr(self->m_hWnd, GWL_STYLE, WS_SYSMENU | WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE);
			MoveWindow(self->m_hWnd, 0, 0, mon_res.x, mon_res.y, TRUE);
		}
	}
	else
	{
		if (self->m_fullscreen)
		{
			SetWindowLongPtr(self->m_hWnd, GWL_STYLE, g_dwStyle);

			Quad2i coord = self->backup_nofullscreen;
			SetWindowPos(self->m_hWnd, 0, coord.start.x, coord.start.y, coord.size.x, coord.size.y, SWP_NOZORDER | SWP_NOACTIVATE);
		}
	}

	self->m_fullscreen = fullscreen;
	return;
}

UNI* Win_clipboard_getText(void)
{
	UNI* str = 0;
	if (IsClipboardFormatAvailable(CF_UNICODETEXT) && OpenClipboard(s_win->m_hWnd))
	{
		void* data;
		if (data = GetClipboardData(CF_UNICODETEXT))
		{
			USHORT* text = GlobalLock(data);
			if (text)
			{
				//copy
				UBIG n = 0;	while (text[n])n++;	//get size
				str = Std_newUNI_N(n);
				int i;
				for (i = 0; i < n; i++)
					str[i] = text[i];

				GlobalUnlock(text);
			}
		}
		CloseClipboard();
	}
	return str;
}

void Win_clipboard_setText(const UNI* strUNI)
{
	BOOL ok = FALSE;
	if (OpenClipboard(s_win->m_hWnd))
	{
		const UBIG N = Std_sizeUNI(strUNI);
		void* data;

		EmptyClipboard();

		if (data = GlobalAlloc(GMEM_MOVEABLE, (N + 1) * sizeof(USHORT)))
		{
			USHORT* text = GlobalLock(data);
			if (text)
			{
				//convert
				int i;
				for (i = 0; i < N; i++)
					text[i] = strUNI[i];
				text[N] = 0;

				GlobalUnlock(text);
				ok = TRUE;
			}
			SetClipboardData(CF_UNICODETEXT, data);
		}
		CloseClipboard();
	}
}

UNI* Win_showFilePicker(Win* self, BOOL mode_open, BOOL mode_folder, BOOL mode_multiple, const UNI* langCancelUNI, const UNI* actionUNI, const UNI* exts)
{
	UNI* filename = 0;

	char szFile[420];
	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = self->m_hWnd;
	ofn.lpstrFile = szFile;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = "All\0*.*\0Text\0*.TXT\0";	//generate from 'exts' ...
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;	//OFN_NOCHANGEDIR => no SetCurrentDirectory()
	if (mode_multiple)
		ofn.Flags |= OFN_ALLOWMULTISELECT | OFN_EXPLORER;

	BOOL ok;
	if (mode_open)	ok = GetOpenFileName(&ofn);
	else			ok = GetSaveFileName(&ofn);

	if (ok)
	{
		char* nextname = &ofn.lpstrFile[ofn.nFileOffset];

		if (!mode_open || nextname[strlen(nextname) + 1] == 0)
		{
			//single
			UNI* str = _Win_clipboard_convertToUNI(ofn.lpstrFile);
			filename = Std_addAfterUNI_char(filename, STD_DROP_FILE_HEADER_CHAR);
			filename = Std_addAfterUNI(filename, str);
			filename = Std_addAfterUNI_char(filename, "\r\n");
			Std_deleteUNI(str);
		}
		else
		{
			//multiple
			UNI* strBase = _Win_clipboard_convertToUNI(ofn.lpstrFile);
			while (*nextname)
			{
				UNI* strFile = _Win_clipboard_convertToUNI(nextname);

				filename = Std_addAfterUNI_char(filename, STD_DROP_FILE_HEADER_CHAR);
				filename = Std_addAfterUNI(filename, strBase);
				filename = Std_addAfterUNI_char(filename, "\\");
				filename = Std_addAfterUNI(filename, strFile);
				filename = Std_addAfterUNI_char(filename, "\r\n");

				Std_deleteUNI(strFile);

				nextname += strlen(nextname) + 1;
			}

			Std_deleteUNI(strBase);
		}
	}

	return filename;
}

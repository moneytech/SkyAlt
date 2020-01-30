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

typedef struct Win_s
{
	BOOL m_fullscreen;
	Vec2i canvas_size;
	BOOL pleaseResize;

	GtkWidget* window;
	GtkWidget* canvas;
	GdkPixbuf* pixbuf;
	GdkCursor* actual_cursor;

	GdkCursor* cursor_def;
	GdkCursor* cursor_ibeam;
	GdkCursor* cursor_wait;
	GdkCursor* cursor_hand;
	GdkCursor* cursor_fleur;
	GdkCursor* cursor_col_resize;
	GdkCursor* cursor_row_resize;
	GdkCursor* cursor_move;

	BOOL(*tickFn)(void*, Quad2i* redrawRect);
	void* tickSelf;

	UBIG tickCounter;
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
		if (self->pixbuf)
			g_object_unref(self->pixbuf); //free old one
		self->pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, size.x, size.y); //TRUE = alpha

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
	Image4 img;
	img.data = (Rgba*)gdk_pixbuf_get_pixels(self->pixbuf);
	img.size = self->canvas_size;
	Image4_setRect(&img, Win_getScreenRect(self));
	return img;
}

void Win_savePrintscreen(Win* self)
{
	char path[256];

	UBIG i = 0;
	do
	{
		snprintf(path, 255, "image_%lld.bmp", i++);
	} while (OsFile_existFile(path) && i < 1000000);

	Image4 img = Win_getImage(self);
	Image4_saveBmp(&img, path);
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
	GdkWindow* window = gtk_widget_get_window(self->window);

	if (gdk_window_get_cursor(window) != self->actual_cursor)
		gdk_window_set_cursor(window, self->actual_cursor);
}

static void target_drop_data_received(GtkWidget* widget, GdkDragContext* context, gint x, gint y, GtkSelectionData* data, guint info, guint time, Win* self)
{
	//BOOL v1 = gtk_selection_data_targets_include_image (data, FALSE);
	//BOOL v2 = gtk_selection_data_targets_include_text (data);
	//BOOL v3 = gtk_selection_data_targets_include_uri (data);
	//BOOL v4 = gtk_selection_data_targets_include_rich_text (data);
	//const char* text = gtk_selection_data_get_data(data); //file:///home/milan/Desktop/123\r\nfile:///home/milan/Desktop/xyz\r\n
	//printf("Got: %s\n", text);

	Vec2i pos = Vec2i_init2(x, y);

	char* dec = Std_newCHAR_urlDecode((char*)gtk_selection_data_get_data(data));
	OsWinIO_setDrop(Std_newUNI_char(dec), &pos);	//utf8_to_utf32(dec)
	free(dec);

	OsWinIO_setTouch(&pos, Win_TOUCH_DOWN_E, FALSE);

	gtk_window_present(GTK_WINDOW(self->window));	//activate my window after drop
	gtk_drag_finish(context, FALSE, FALSE, time);
}

static void _Win_setKeyExtra(GdkModifierType state, UNI key)
{
	UBIG key_extra = Win_EXTRAKEY_NONE;

	//if (state & GDK_MOD1_MASK || state & GDK_MOD5_MASK) //Left or Right ALT
	//	key_extra |= Win_EXTRAKEY_ALT;

	if (state & GDK_CONTROL_MASK)
	{
		key_extra |= Win_EXTRAKEY_CTRL;
	}

	if (state & GDK_SHIFT_MASK)
	{
		key_extra |= Win_EXTRAKEY_SHIFT;

		if (key == GDK_KEY_space) key_extra |= Win_EXTRAKEY_SELECT_ROW;
	}

	if (state & GDK_CONTROL_MASK)
	{
		if (key == GDK_KEY_KP_Add) key_extra |= Win_EXTRAKEY_ZOOM_A; //+
		if (key == GDK_KEY_KP_Subtract) key_extra |= Win_EXTRAKEY_ZOOM_S; //-
		if (key == GDK_KEY_KP_0) key_extra |= Win_EXTRAKEY_ZOOM_0; //0

		if (key == GDK_KEY_a || key == GDK_KEY_A) key_extra |= Win_EXTRAKEY_SELECT_ALL;
		if (key == GDK_KEY_x || key == GDK_KEY_X) key_extra |= Win_EXTRAKEY_CUT;
		if (key == GDK_KEY_c || key == GDK_KEY_C) key_extra |= Win_EXTRAKEY_COPY;
		if (key == GDK_KEY_v || key == GDK_KEY_V) key_extra |= Win_EXTRAKEY_PASTE;
		if (key == GDK_KEY_d || key == GDK_KEY_D) key_extra |= Win_EXTRAKEY_DUPLICATE;

		if (key == GDK_KEY_f || key == GDK_KEY_F) key_extra |= Win_EXTRAKEY_SEARCH;
		if (key == GDK_KEY_t || key == GDK_KEY_T) key_extra |= Win_EXTRAKEY_THEME;

		if (key == GDK_KEY_n || key == GDK_KEY_N) key_extra |= Win_EXTRAKEY_NEW;
		if (key == GDK_KEY_s || key == GDK_KEY_S) key_extra |= Win_EXTRAKEY_SAVE;

		if (key == GDK_KEY_l || key == GDK_KEY_L) key_extra |= Win_EXTRAKEY_LOG;

		if (key == GDK_KEY_z || key == GDK_KEY_Z) key_extra |= Win_EXTRAKEY_BACK;
		if (key == GDK_KEY_y || key == GDK_KEY_Y) key_extra |= Win_EXTRAKEY_FORWARD;

		if (key == GDK_KEY_b || key == GDK_KEY_B) key_extra |= Win_EXTRAKEY_BYPASS; //B
		if (key == GDK_KEY_semicolon) key_extra |= Win_EXTRAKEY_COMMENT; //;

		if (key == GDK_KEY_F12) key_extra |= Win_EXTRAKEY_PRINTSCREEN;

		if (key == GDK_KEY_space) key_extra |= Win_EXTRAKEY_SELECT_COLUMN;

		if (key == GDK_KEY_r || key == GDK_KEY_R) key_extra |= Win_EXTRAKEY_ADD_RECORD;
	}
	else
	{
		if (key == GDK_KEY_Delete) key_extra |= Win_EXTRAKEY_DELETE;
		if (key == GDK_KEY_Home) key_extra |= Win_EXTRAKEY_HOME;
		if (key == GDK_KEY_End) key_extra |= Win_EXTRAKEY_END;
		if (key == GDK_KEY_BackSpace) key_extra |= Win_EXTRAKEY_BACKSPACE;
		if (key == GDK_KEY_Insert) key_extra |= Win_EXTRAKEY_INSERT;
		if (key == GDK_KEY_Escape) key_extra |= Win_EXTRAKEY_ESCAPE;
		if (key == GDK_KEY_Page_Up) key_extra |= Win_EXTRAKEY_PAGE_U;
		if (key == GDK_KEY_Page_Down) key_extra |= Win_EXTRAKEY_PAGE_D;

		if (key == GDK_KEY_Tab) key_extra |= Win_EXTRAKEY_TAB;
		if (key == 65293 || key == 65421) key_extra |= Win_EXTRAKEY_ENTER;

		if (key == GDK_KEY_Left) key_extra |= Win_EXTRAKEY_LEFT;
		if (key == GDK_KEY_Right) key_extra |= Win_EXTRAKEY_RIGHT;
		if (key == GDK_KEY_Up) key_extra |= Win_EXTRAKEY_UP;
		if (key == GDK_KEY_Down) key_extra |= Win_EXTRAKEY_DOWN;

		if (key == GDK_KEY_F11) key_extra |= Win_EXTRAKEY_FULLSCREEN;
		if (key == GDK_KEY_F2) key_extra |= Win_EXTRAKEY_LPANEL;
	}

	if (key_extra)
		OsWinIO_setKeyExtra(key_extra);
}

static BOOL _Win_tick(void* selff);

static BOOL _Win_key_press(GtkWidget* widget, GdkEventKey* event, Win* self)
{
	//if (event->length)
	OsWinIO_setKey(gdk_keyval_to_unicode(event->keyval));
	_Win_setKeyExtra(event->state, event->keyval);

	//if (event->length)
	//	_Win_tick(self);	//run right now!

	//printf("key: %d, %d\n", event->length, gdk_keyval_to_unicode(event->keyval));
	return TRUE;
}

static BOOL _Win_mouse_motion(GtkWidget* widget, GdkEventMotion* event, Win* self)
{
	Vec2i pos = Vec2i_init2(event->x, event->y);
	OsWinIO_setTouch(&pos, OsWinIO_getTouch_action(), TRUE);

	//wheel
	double delta_x, delta_y;
	gdk_event_get_scroll_deltas((GdkEvent*)event, &delta_x, &delta_y);
	if (delta_y != 0)
		OsWinIO_setTouchWheel(&pos, delta_y);

	// gtk_widget_queue_draw(self->canvas);
	// gtk_widget_queue_draw (widget);

	return TRUE;
}

static BOOL _Win_mouse_scroll(GtkWidget* widget, GdkEventScroll* event, Win* self)
{
	Vec2i pos = Vec2i_init2(event->x, event->y);

	if (event->delta_y)
		OsWinIO_setTouchWheel(&pos, event->delta_y);
	else
		if (event->direction == GDK_SCROLL_UP)
			OsWinIO_setTouchWheel(&pos, -1);
	if (event->direction == GDK_SCROLL_DOWN)
		OsWinIO_setTouchWheel(&pos, 1);

	_Win_setKeyExtra(event->state, 0);

	//_Win_tick(self);	//run right now!
	return TRUE;
}

static BOOL _Win_mouse_press(GtkWidget* widget, GdkEventButton* event, Win* self)
{
	Vec2i pos = Vec2i_init2(event->x, event->y);

	if (event->button == 1)
		OsWinIO_setTouch(&pos, Win_TOUCH_DOWN_S, FALSE);
	else
		OsWinIO_setTouch(&pos, Win_TOUCH_FORCE_DOWN_S, FALSE);

	_Win_setKeyExtra(event->state, 0);

	//printf("button_down: %d\n", event->button);
	//_Win_tick(self);	//run right now!
	return TRUE;
}

static BOOL _Win_mouse_release(GtkWidget* widget, GdkEventButton* event, Win* self)
{
	Vec2i pos = Vec2i_init2(event->x, event->y);

	if (event->button == 1)
		OsWinIO_setTouch(&pos, Win_TOUCH_DOWN_E, FALSE);
	else
		OsWinIO_setTouch(&pos, Win_TOUCH_FORCE_DOWN_E, FALSE);

	_Win_setKeyExtra(event->state, 0);

	// printf("buttonup: %d\n", event->button);
 //	_Win_tick(self);	//run right now!
	return TRUE;
}

static gboolean _Win_redraw(GtkWidget* widget, cairo_t* cr, Win* self)
{
	//maybe self->img is not needed - If redraw is not from 'app' I can copy from screen(it's double-buffered, so it could have original data?) ...
	gdk_cairo_set_source_pixbuf(cr, self->pixbuf, 0, 0); //rect.x, rect.y
	cairo_paint(cr);

	//printf("draw:\n");

	return FALSE;
}

static BOOL _Win_tick(void* selff)
{
	Win* self = selff;
	//GdkWindow* window = gtk_widget_get_window(self->window);

		//every second call has to be quick, so main thread has extra time for calling draw() callback
	if ((self->tickCounter++) % 2 == 0)
		return G_SOURCE_CONTINUE;

	_Win_resize(self, Vec2i_init2(gtk_widget_get_allocated_width(self->canvas), gtk_widget_get_allocated_height(self->canvas)));	//accurate

	if (OsWinIO_getKeyExtra() & Win_EXTRAKEY_PRINTSCREEN)
		Win_savePrintscreen(self);

	if (self->tickFn && self->tickSelf)
	{
		Quad2i redrawRect = Quad2i_init();
		if (!self->tickFn(self->tickSelf, &redrawRect))	//call project "logic"
			gtk_main_quit();

		//redrawRect = Win_getScreenRect(self);

		if (!Quad2i_isZero(redrawRect))
		{
			Quad2i rect = Quad2i_getIntersect(redrawRect, Win_getScreenRect(self));
			//Quad2i_print(redrawRect, "redrawRect");

			gtk_widget_queue_draw_area(self->canvas, rect.start.x, rect.start.y, rect.size.x, rect.size.y);

			//mo\9En\E1 toto nefunguje kdy\9E se vol\E1 p\F8\EDmo z key() nebo mousePress() ...
			//printf("redraw_ask: %d, %d, %d, %d\n", rect.start.x, rect.start.y, rect.size.x, rect.size.y);
		}
	}

	return G_SOURCE_CONTINUE; //G_SOURCE_REMOVE when you want to stop
}

static BOOL _Win_quit(GtkWidget* widget, void* data)
{
	OsWinIO_pleaseExit();
	return TRUE;
}

void Win_init(void)
{
	gtk_init(0, 0);

	//on Windows(Release mode only, Debug is ok), gtk_init() fucked something(probably '.' -> ',') so atof(), snprintf(), etc. returns invalid values, so this will return things to default
	setlocale(LC_ALL, "C");
}

Win* Win_new(Quad2i* abs_coord, BOOL(*tickFn)(void*, Quad2i* redrawRect), void* tickSelf)
{
	Quad2i res;
	OsScreen_getMonitorCoord(&res);
	if (Quad2i_cmp(*abs_coord, res))
		OsScreen_getDefaultCoord(abs_coord); //never start in fullscreen

	Win* self = malloc(sizeof(Win));

	self->tickCounter = 0;
	self->tickFn = tickFn;
	self->tickSelf = tickSelf;
	self->m_fullscreen = FALSE;

	self->canvas_size = Vec2i_init();
	self->pixbuf = 0;
	self->pleaseResize = FALSE;

	self->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	gtk_window_set_title(GTK_WINDOW(self->window), "SkyAlt");

	gtk_window_move(GTK_WINDOW(self->window), abs_coord->start.x, abs_coord->start.y);
	gtk_window_set_default_size(GTK_WINDOW(self->window), abs_coord->size.x, abs_coord->size.y);	//gtk_window_resize

	self->cursor_def = gdk_cursor_new_from_name(gdk_display_get_default(), "default");
	self->cursor_ibeam = gdk_cursor_new_from_name(gdk_display_get_default(), "text");
	self->cursor_wait = gdk_cursor_new_from_name(gdk_display_get_default(), "wait");
	self->cursor_hand = gdk_cursor_new_from_name(gdk_display_get_default(), "pointer");
	self->cursor_fleur = gdk_cursor_new_from_name(gdk_display_get_default(), "nwse-resize");	//eg.: extend cells

	self->cursor_col_resize = gdk_cursor_new_from_name(gdk_display_get_default(), "col-resize");
	self->cursor_row_resize = gdk_cursor_new_from_name(gdk_display_get_default(), "row-resize");
	self->cursor_move = gdk_cursor_new_from_name(gdk_display_get_default(), "grabbing");

	Win_resetCursor(self);

	self->canvas = gtk_drawing_area_new();
	gtk_container_add(GTK_CONTAINER(self->window), self->canvas);
	g_signal_connect(self->canvas, "draw", G_CALLBACK(_Win_redraw), self);
	gtk_widget_set_app_paintable(self->canvas, TRUE);

	//g_signal_connect(self->window, "destroy", G_CALLBACK(_Win_quit), self);
	g_signal_connect(self->window, "delete_event", G_CALLBACK(_Win_quit), self);
	g_signal_connect(self->window, "key_press_event", G_CALLBACK(_Win_key_press), self);

	g_signal_connect(self->canvas, "motion_notify_event", G_CALLBACK(_Win_mouse_motion), self);
	g_signal_connect(self->canvas, "button_press_event", G_CALLBACK(_Win_mouse_press), self);
	g_signal_connect(self->canvas, "button_release_event", G_CALLBACK(_Win_mouse_release), self);
	g_signal_connect(self->canvas, "scroll_event", G_CALLBACK(_Win_mouse_scroll), self);

	enum
	{
		TARGET_STRING,
		TARGET_URL
	};
	static GtkTargetEntry targetentries[] = {
		{ "STRING", 0, TARGET_STRING},
		{ "text/plain", 0, TARGET_STRING},
		{ "text/uri-list", 0, TARGET_URL},
	};
	gtk_drag_dest_set(self->window, GTK_DEST_DEFAULT_ALL, targetentries, 3, GDK_ACTION_COPY | GDK_ACTION_MOVE | GDK_ACTION_LINK);
	g_signal_connect(self->window, "drag_data_received", G_CALLBACK(target_drop_data_received), self);

	gtk_widget_set_events(self->canvas,
		//GDK_POINTER_MOTION_MASK |
	   // GDK_EXPOSURE_MASK |
		GDK_BUTTON_PRESS_MASK |
		GDK_BUTTON_RELEASE_MASK |
		GDK_KEY_PRESS_MASK |
		GDK_SCROLL_MASK |
		//GDK_KEY_RELEASE_MASK |
		GDK_POINTER_MOTION_MASK
		//| GDK_POINTER_MOTION_HINT_MASK
	);

	gtk_widget_show_all(self->window);

	_Win_resize(self, abs_coord->size);

	gtk_widget_show_all(self->window);

	return self;
}

void Win_delete(Win* self)
{
	memset(self, 0, sizeof(Win));
	free(self);
}

void Win_start(Win* self)
{
	g_timeout_add(5, _Win_tick, self); //200fps
	gtk_main();
}

void Win_getWindowCoord(Win* self, Quad2i* out)
{
	int x, y, w, h;
	gtk_window_get_position(GTK_WINDOW(self->window), &x, &y);
	gtk_window_get_size(GTK_WINDOW(self->window), &w, &h);
	*out = Quad2i_init4(x, y, w, h);
}

void Win_setTitle(Win* self, const char* title)
{
	if (Std_sizeCHAR(title))
		gtk_window_set_title(GTK_WINDOW(self->window), title);
}

void Win_setFullscreen(Win* self, BOOL fullscreen)
{
	if (fullscreen)
	{
		if (!self->m_fullscreen)
			gtk_window_fullscreen(GTK_WINDOW(self->window));
	}
	else
	{
		if (self->m_fullscreen)
			gtk_window_unfullscreen(GTK_WINDOW(self->window));
	}

	self->m_fullscreen = fullscreen;
	return;
}

static void _Win_clipboard_getText_callback(GtkClipboard* clipboard, const gchar* text, void* data)
{
	char** t = data;
	*t = Std_newCHAR(text);
}

UNI* Win_clipboard_getText(void)
{
	GtkClipboard* clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);

	char* res = 0;
	gtk_clipboard_request_text(clipboard, _Win_clipboard_getText_callback, &res);
	gtk_clipboard_wait_for_text(clipboard);

	UNI* uniResult = Std_newUNI_char(res);
	free(res);

	return uniResult;
}

void Win_clipboard_setText(const UNI* strUNI)
{
	char* str = Std_newCHAR_uni(strUNI);

	GtkClipboard* clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
	gtk_clipboard_set_text(clipboard, str, -1);

	free(str);
}

UNI* Win_showFilePicker(Win* self, BOOL mode_open, BOOL mode_folder, BOOL mode_multiple, const UNI* langCancelUNI, const UNI* actionUNI, const UNI* exts)
{
	UNI* filename = 0;

	gchar* cancel = Std_utf32_to_utf8(langCancelUNI);
	gchar* action = Std_utf32_to_utf8(actionUNI);

	GtkFileChooserAction mode;
	if (mode_folder)
		mode = mode_open ? GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER : GTK_FILE_CHOOSER_ACTION_CREATE_FOLDER;
	else
		mode = mode_open ? GTK_FILE_CHOOSER_ACTION_OPEN : GTK_FILE_CHOOSER_ACTION_SAVE;

	GtkWidget* dialog = gtk_file_chooser_dialog_new(action,
		GTK_WINDOW(self->window),
		mode,
		cancel,
		GTK_RESPONSE_CANCEL,
		action,
		GTK_RESPONSE_ACCEPT,
		NULL);
	gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);
	gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(dialog), mode_multiple);
	if (mode != GTK_FILE_CHOOSER_ACTION_OPEN)
		gtk_file_chooser_set_create_folders(GTK_FILE_CHOOSER(dialog), TRUE);

	//exts
	const UBIG nSepars = Std_separNumItemsUNI(exts, '/');
	if (nSepars)
	{
		GtkFileFilter* filter = gtk_file_filter_new();

		BIG i;
		for (i = 0; i < nSepars; i++)
		{
			UNI* it = Std_separGetItemUNI(exts, i, '/');
			gchar* itg = Std_utf32_to_utf8(it);

			gtk_file_filter_add_pattern(filter, itg);

			Std_deleteUNI(it);
			free(itg);
		}

		gchar* allg = Std_utf32_to_utf8(exts);
		gtk_file_filter_set_name(filter, allg);
		gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
		free(allg);
	}

	//run
	gint res = gtk_dialog_run(GTK_DIALOG(dialog));
	if (res == GTK_RESPONSE_ACCEPT)
	{
		char* p = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		filename = Std_addAfterUNI_char(filename, STD_DROP_FILE_HEADER_CHAR);
		filename = Std_addAfterUNI_char(filename, p);
		Std_deleteCHAR(p);
	}

	//free
	gtk_widget_destroy(dialog);
	free(action);
	free(cancel);

	return filename;
}

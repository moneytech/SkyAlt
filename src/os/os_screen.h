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

void OsScreen_getMonitorResolution(Vec2i* out)
{
#ifdef _WIN32
	HDC screen = GetDC(NULL);
	*out = Vec2i_init2(GetDeviceCaps(screen, HORZRES), GetDeviceCaps(screen, VERTRES));
#else
	GdkRectangle workarea = { 0 };
	gdk_monitor_get_workarea(gdk_display_get_primary_monitor(gdk_display_get_default()), &workarea);
	*out = Vec2i_init2(workarea.width, workarea.height);
#endif
}

int OsScreen_getDPI(void)
{
#ifdef _WIN32
	HDC screen = GetDC(0);
	double x = (double)GetDeviceCaps(screen, LOGPIXELSX);
	double y = (double)GetDeviceCaps(screen, LOGPIXELSY);
	ReleaseDC(0, screen);
#else
	Vec2i res;
	OsScreen_getMonitorResolution(&res);

	int w_mm = gdk_monitor_get_width_mm(gdk_display_get_primary_monitor(gdk_display_get_default()));
	int h_mm = gdk_monitor_get_height_mm(gdk_display_get_primary_monitor(gdk_display_get_default()));

	double x = ((((double)res.x) * 25.4) / ((double)w_mm)) + 0.5f;
	double y = ((((double)res.y) * 25.4) / ((double)h_mm)) + 0.5f;
#endif

	return Std_max(x, y);
}

void OsScreen_getMonitorCoord(Quad2i* out)
{
	out->start = Vec2i_init();
	OsScreen_getMonitorResolution(&out->size);
}

void OsScreen_getDefaultCoord(Quad2i* out)
{
	Vec2i res;
	OsScreen_getMonitorResolution(&res);

	const int around = res.y / 8; //8
	out->size.x = res.x - around * 2;
	out->size.y = res.y - around * 2;
	out->start.x = around;
	out->start.y = around;
}

float OsScreen_inchToMM(float inch)
{
	return inch / 0.039f;
}

void OsScreen_getMonitorMM(float diagonal_MM, Vec2i* out)
{
	OsScreen_getMonitorResolution(out);
	float t = diagonal_MM / (double)Vec2i_len(*out);
	*out = Vec2i_mulV(*out, t);
}

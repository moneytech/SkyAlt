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

void UiDialogLicense_clickAutoUpdate(GuiItem* item)
{
	UiIniSettings_setUpdate(GuiItemCheck_isActive(GuiItem_findName(item, "update")));
}

void UiDialogLicense_clickCheck(GuiItem* item)
{
	UiAutoUpdate_run();

	BIG ver = UiAutoUpdate_getVersionUNI();
	GuiItem_setEnable(GuiItem_findName(item, "button"), UiAutoUpdate_hasUpdate());
	GuiItemText* latest = GuiItem_findName(item, "latestVer");
	if (ver < 0)
		GuiItemText_setText(latest, Lang_find("CHECKING"));
	else
		GuiItemText_setNumber(latest, ver);
}

void UiDialogLicense_clickUpdate(GuiItem* item)
{
	if (UiAutoUpdate_hasUpdate())
	{
		if (UiAutoUpdate_updateFile())
			Logs_addInfo("INF_UPDATES_NOTE");
		else
			Logs_addError("ERR_UPDATE_FAIL");
	}

	GuiItem_closeParentLevel(item);	//hide
}

GuiItemLayout* UiDialogLicense_new(void)
{
	//layout
	GuiItemLayout* layout = GuiItemLayout_new(Quad2i_init4(0, 0, 1, 1));
	GuiItemLayout_addColumn(layout, 0, 99);

	//info
	if (!License_exist())
		GuiItem_addSubName((GuiItem*)layout, "expiration", GuiItemText_new(Quad2i_init4(0, 0, 1, 2), TRUE, DbValue_initLang("NON_COMMERCIAL_USE_ONLY"), DbValue_initEmpty()));
	else
	{
		OsDate date = License_getExpiration();
		char month[32];
		Std_copyCHAR_uni(month, 32, Lang_find_month(date.m_month));
		char time[64];
		OsDate_getStringDateTime(&date, UiIniSettings_getDateFormat(), OsDate_NONE, month, time);

		GuiItem_addSubName((GuiItem*)layout, "company", GuiItemText_new(Quad2i_init4(0, 0, 1, 2), TRUE, DbValue_initStaticCopy(License_getCompany()), DbValue_initLang("LICENSE_COMPANY")));
		GuiItem_addSubName((GuiItem*)layout, "count", GuiItemText_new(Quad2i_init4(0, 3, 1, 2), TRUE, DbValue_initNumber(License_getCount()), DbValue_initLang("LICENSE_COUNT")));
		GuiItem_addSubName((GuiItem*)layout, "expiration", GuiItemText_new(Quad2i_init4(0, 6, 1, 2), TRUE, DbValue_initStaticCopyCHAR(time), DbValue_initLang("LICENSE_EXPIRATION")));

		if (!License_isTimeValid())
		{
			//error
			GuiItemText* err = (GuiItemText*)GuiItem_addSubName((GuiItem*)layout, "expiration", GuiItemText_new(Quad2i_init4(0, 9, 1, 1), TRUE, DbValue_initLang("COMMERCIAL_EXPIRED"), DbValue_initEmpty()));
			GuiItemText_setColorBorder(err, Rgba_initRed());
		}
	}

	return GuiItemRoot_createDialogLayout(Vec2i_init2(11, 10), DbValue_initLang("LICENSE"), (GuiItem*)layout, 0);
}

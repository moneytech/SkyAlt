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

void UiRootRemote_clickConnect(GuiItem* item)
{
	BIG row = GuiItem_findAttribute(item, "row");

	OsODBC* odbc = DbRoot_connectRemote(row, TRUE);
	if (odbc)
		OsODBC_delete(odbc);
}

void UiRootRemote_clickRefresh(GuiItem* item)
{
	BIG row = GuiItem_findAttribute(item, "row");
	DbRoot_refreshRemoteNow(row);
}

void UiRootRemote_clickChooseConnection(GuiItem* item)
{
	GuiItemEdit* connection = GuiItem_findName(item, "connection");

	UBIG i = GuiItemComboStatic_getValue((GuiItemComboStatic*)item);
	if (i > 0)
	{
		const UNI* str = GuiItemComboStatic_getValueName((GuiItemComboStatic*)item, i);
		GuiItemEdit_setText(connection, str);

		GuiItemComboStatic_setValue((GuiItemComboStatic*)item, 0);
	}
}

void UiRootRemote_clickChooseDriver(GuiItem* item)
{
	GuiItemEdit* driver = GuiItem_findName(item, "driver");

	UBIG i = GuiItemComboStatic_getValue((GuiItemComboStatic*)item);
	if (i > 0)
	{
		const UNI* str = GuiItemComboStatic_getValueName((GuiItemComboStatic*)item, i);
		GuiItemEdit_setText(driver, str);

		GuiItemComboStatic_setValue((GuiItemComboStatic*)item, 0);
	}
}

static GuiItem* UiRootRemote_build(GuiItemLayout* layout, UBIG row)
{
	GuiItemLayout_addColumn(layout, 0, 99);
	GuiItemLayout_addColumn(layout, 1, 20);
	GuiItemLayout_addColumn(layout, 2, 99);
	GuiItemLayout_addRow(layout, 1, 99);

	GuiItem_setAttribute((GuiItem*)layout, "row", row);

	//header
	{
		GuiItemLayout* layoutMenu = GuiItemLayout_new(Quad2i_init4(0, 0, 3, 1));
		GuiItemLayout_setDrawBackground(layoutMenu, FALSE);
		GuiItemLayout_addColumn(layoutMenu, 0, 6);	//name
		GuiItemLayout_addColumn(layoutMenu, 2, 4);
		GuiItemLayout_addColumn(layoutMenu, 4, 4);
		GuiItemLayout_addColumn(layoutMenu, 6, 4);
		GuiItem_addSubName((GuiItem*)layout, "header_top", (GuiItem*)layoutMenu);

		//name
		GuiItem_addSubName((GuiItem*)layoutMenu, "header", UiRoot_createMenuNameHeader(Quad2i_init4(0, 0, 1, 1), row));
	}

	//login
	{
		GuiItemLayout* layoutLogin = GuiItemLayout_newTitle(Quad2i_init4(1, 1, 1, 1), DbValue_initLang("REMOTE_TITLE"));
		GuiItemLayout_setDrawBackground(layoutLogin, FALSE);
		GuiItemLayout_addColumn(layoutLogin, 0, 99);
		GuiItemLayout_addColumn(layoutLogin, 1, 99);
		GuiItem_addSubName((GuiItem*)layout, "header_login", (GuiItem*)layoutLogin);

		//GuiItem_addSubName((GuiItem*)layoutLogin, "title", GuiItemText_new(Quad2i_init4(0, 0, 1, 1), TRUE, DbValue_initLang("REMOTE_TITLE"), DbValue_initEmpty()));

		//combo
		GuiItem_addSubName((GuiItem*)layoutLogin, "connection", GuiItemEdit_newEx(Quad2i_init4(0, 1, 1, 2), DbValue_initOption(row, "connection", 0), DbValue_initLang("REMOTE_CONNECTION"), 0));
		{
			GuiItemComboStatic* conCombo = GuiItem_addSubName((GuiItem*)layoutLogin, "connectionCombo", GuiItemComboStatic_newEx(Quad2i_init4(1, 1, 1, 2), DbValue_initNumber(0), 0, DbValue_initLang("EXISTING"), &UiRootRemote_clickChooseConnection));

			GuiItemComboStatic_addItem(conCombo, DbValue_initStaticCopyCHAR("---"));

			StdArr arr;
			arr.num = OsODBC_getDataSourcesList((UNI***)&arr.ptrs);
			BIG i;
			for (i = 0; i < arr.num; i++)
				GuiItemComboStatic_addItem(conCombo, DbValue_initStaticCopy(arr.ptrs[i]));
			StdArr_freeFn(&arr, (StdArrFREE)&Std_deleteUNI);
		}

		GuiItem_addSubName((GuiItem*)layoutLogin, "server", GuiItemEdit_new(Quad2i_init4(0, 4, 1, 2), DbValue_initOption(row, "server", _UNI32("127.0.0.1")), DbValue_initLang("REMOTE_SERVER")));
		GuiItem_addSubName((GuiItem*)layoutLogin, "port", GuiItemEdit_new(Quad2i_init4(1, 4, 1, 2), DbValue_initOption(row, "port", _UNI32("3306")), DbValue_initLang("REMOTE_PORT")));

		GuiItem_addSubName((GuiItem*)layoutLogin, "user", GuiItemEdit_new(Quad2i_init4(0, 7, 1, 2), DbValue_initOption(row, "user", _UNI32("root")), DbValue_initLang("REMOTE_USER")));
		GuiItemEdit* password = (GuiItemEdit*)GuiItem_addSubName((GuiItem*)layoutLogin, "password", GuiItemEdit_new(Quad2i_init4(1, 7, 1, 2), DbValue_initOption(row, "password", 0), DbValue_initLang("REMOTE_PASSWORD")));
		GuiItemEdit_setPasswordStars(password, TRUE);

		//combo
		GuiItem_addSubName((GuiItem*)layoutLogin, "driver", GuiItemEdit_new(Quad2i_init4(0, 10, 1, 2), DbValue_initOption(row, "driver", 0), DbValue_initLang("REMOTE_DRIVER")));
		{
			GuiItemComboStatic* conCombo = GuiItem_addSubName((GuiItem*)layoutLogin, "driverCombo", GuiItemComboStatic_newEx(Quad2i_init4(1, 10, 1, 2), DbValue_initNumber(0), 0, DbValue_initLang("EXISTING"), &UiRootRemote_clickChooseDriver));

			GuiItemComboStatic_addItem(conCombo, DbValue_initStaticCopyCHAR("---"));

			StdArr arr;
			arr.num = OsODBC_getDriversList((UNI***)&arr.ptrs);
			BIG i;
			for (i = 0; i < arr.num; i++)
				GuiItemComboStatic_addItem(conCombo, DbValue_initStaticCopy(arr.ptrs[i]));
			StdArr_freeFn(&arr, (StdArrFREE)&Std_deleteUNI);
		}

		//refresh every
		GuiItem_addSubName((GuiItem*)layoutLogin, "refresh", GuiItemEdit_new(Quad2i_init4(0, 13, 1, 2), DbValue_initOption(row, "refresh", _UNI32("15")), DbValue_initLang("REMOTE_REFRESH_EVERY")));
		GuiItemComboStatic* refreshCombo = GuiItem_addSubName((GuiItem*)layoutLogin, "refreshCombo", GuiItemComboStatic_new(Quad2i_init4(1, 13, 1, 2), DbValue_initOption(row, "refresh_type", 0), 0, DbValue_initEmpty()));
		GuiItemComboStatic_addItem(refreshCombo, DbValue_initLang("MINUTES"));
		GuiItemComboStatic_addItem(refreshCombo, DbValue_initLang("HOURS"));

		//refresh info
		{
			OsDate next = DbRoot_getRemoteRefreshNextTime(row);
			//OsDate now = OsDate_initActual();
			//BIG remainSeconds = OsDate_differenceSeconds(&now, &next);

			char month[32];
			Std_copyCHAR_uni(month, 32, Lang_find_month(next.m_month));
			char time[64];
			OsDate_getStringDateTime(&next, UiIniSettings_getDateFormat(), OsDate_HM, month, time);

			GuiItem_addSubName((GuiItem*)layoutLogin, "refresh_info", GuiItemText_new(Quad2i_init4(0, 15, 1, 2), TRUE, DbValue_initStaticCopyCHAR(time), DbValue_initLang("REMOTE_REFRESH_NEXT")));
			GuiItem_addSubName((GuiItem*)layoutLogin, "refresh_now", GuiItemButton_newClassicEx(Quad2i_init4(1, 15, 1, 2), DbValue_initLang("REMOTE_REFRESH_NOW"), &UiRootRemote_clickRefresh));
		}

		GuiItem_addSubName((GuiItem*)layoutLogin, "save_file", GuiItemCheck_new(Quad2i_init4(0, 18, 2, 1), DbValue_initOption(row, "save_file", 0), DbValue_initLang("REMOTE_SAVE_FILE")));

		//button
		GuiItem_addSubName((GuiItem*)layoutLogin, "execute", GuiItemButton_newClassicEx(Quad2i_init4(0, 20, 2, 2), DbValue_initLang("REMOTE_CONNECT"), &UiRootRemote_clickConnect));
	}

	return (GuiItem*)layout;
}

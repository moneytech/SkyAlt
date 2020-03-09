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

BOOL UiRootMapPanel_isShowPanel(BIG row)
{
	return DbValue_getOptionNumber(row, "panel_settings_enable", 1);
}

void UiRootMapPanel_setShowPanel(BIG row, BOOL show)
{
	DbValue_setOptionNumber(row, "panel_settings_enable", show);
}

void UiRootMapPanel_clickShowPanel(GuiItem* item)
{
	BIG row = GuiItem_findAttribute(item, "row");
	UiRootMapPanel_setShowPanel(row, !UiRootMapPanel_isShowPanel(row));

	//QFilter disable
	UiRootQuickFilter_setShowPanel(row, FALSE);
}

BOOL GuiItem_enableColor(GuiItem* item)
{
	BIG row = GuiItem_findAttribute(item, "row");

	//DbRows rws = DbRows_initRefLink(row, "color");
	//DbColumn* columnCd = DbRoot_findColumn(DbRows_getRow(&rws, 0));
	//DbRows_free(&rws);
	DbColumn* columnCd = DbRoot_ref_column(DbRows_findOrCreateSubType(row, "color"));

	return columnCd != 0;
}

static double UiRootMapPanel_getWidth(UBIG row)
{
	return DbValue_getOptionNumber(row, "panel_settings_width", 10);
}

GuiItem* UiRootMapPanel_buildButton(Quad2i grid, UBIG row)
{
	GuiItemButton* button = (GuiItemButton*)GuiItemButton_newAlphaEx(grid, DbValue_initLang("PROPERTIES"), &UiRootMapPanel_clickShowPanel);
	GuiItemButton_setPressed(button, UiRootMapPanel_isShowPanel(row));
	GuiItem_setAttribute((GuiItem*)button, "row", row);

	return (GuiItem*)button;
}

GuiItemLayout* UiRootMapPanel_build(Quad2i grid, UBIG row, DbRows* filter, DbValue scroll)
{
	GuiItemLayout* layout = 0;

	DbTable* table = DbRoot_findParentTable(row);

	//BIG rowXRow = DbRows_findOrCreateSubType(row, "row_x");
	//BIG titleXRow = DbRows_findOrCreateSubType(row, "title_x");

	//selection panel
	if (UiRootMapPanel_isShowPanel(row))
	{
		layout = GuiItemLayout_new(grid);
		GuiItemLayout_setScrollV(layout, scroll);
		//GuiItemLayout_setDrawBackground(layout, TRUE);
		GuiItemLayout_setBackgroundWhite(layout, TRUE);
		//GuiItemLayout_setDrawBorder(layout, TRUE);
		GuiItemLayout_addColumn(layout, 0, 99);
		//GuiItemLayout_addRow(layout, 2, 99);
		GuiItem_setChangeSize((GuiItem*)layout, TRUE, DbValue_initOption(row, "panel_settings_width", 0), TRUE, TRUE, GuiItemTheme_getWhite());
		GuiItem_setAttribute((GuiItem*)layout, "row", row);

		//type
		{
			GuiItemLayout* layoutType = GuiItemLayout_newTitle(Quad2i_init4(0, 0, 1, 3), DbValue_initLang("MAP_TYPE"));
			GuiItemLayout_addColumn(layoutType, 0, 20);
			GuiItemLayout_addColumn(layoutType, 1, 20);

			GuiItemComboStatic* render = (GuiItemComboStatic*)GuiItem_addSubName((GuiItem*)layoutType, "render", GuiItemComboStatic_new(Quad2i_init4(0, 1, 1, 2), DbValue_initOption(row, "map_render", 0), 0, DbValue_initLang("MAP_RENDER")));
			GuiItemComboStatic_addItemIcon(render, GuiImage_new1(UiIcons_init_map_icon()), DbValue_initLang("MAP_RENDER_ICON"));
			GuiItemComboStatic_addItemIcon(render, GuiImage_new1(UiIcons_init_map_circle()), DbValue_initLang("MAP_RENDER_CIRCLE"));
			GuiItemComboStatic_addItemIcon(render, GuiImage_new1(UiIcons_init_map_area()), DbValue_initLang("MAP_RENDER_AREA"));

			GuiItemComboStatic* type = (GuiItemComboStatic*)GuiItem_addSubName((GuiItem*)layoutType, "type", GuiItemComboStatic_new(Quad2i_init4(1, 1, 1, 2), DbValue_initOption(row, "map_type", 0), 0, DbValue_initLang("MAP_TILE")));
			GuiItemComboStatic_addItem(type, DbValue_initLang("MAP_TILE_STANDARD"));

			GuiItem_addSubName((GuiItem*)layout, "layoutType", (GuiItem*)layoutType);
		}

		//address column
		{
			GuiItemLayout* layoutAddr = GuiItemLayout_newTitle(Quad2i_init4(0, 4, 1, 2), DbValue_initLang("MAP_LOCATION"));
			GuiItemLayout_addColumn(layoutAddr, 0, 20);

			GuiItem_addSubName((GuiItem*)layoutAddr, "location", GuiItemComboDynamic_new(Quad2i_init4(0, 1, 1, 1), TRUE, DbRows_initRefLink(row, "location"), DbValue_initOption(-1, "name", 0), DbRows_initSubs(table, "columns", FALSE), DbValue_initEmpty()));

			GuiItem_addSubName((GuiItem*)layout, "layoutAddr", (GuiItem*)layoutAddr);
		}

		//labels
		{
			GuiItemLayout* layoutLabels = GuiItemLayout_newTitle(Quad2i_init4(0, 7, 1, 7), DbValue_initLang("MAP_LABELS"));
			GuiItemLayout_addColumn(layoutLabels, 0, 20);

			//GuiItem_addSubName((GuiItem*)layoutLabels, "label_location", GuiItemCheck_new(Quad2i_init4(0, 1, 1, 1), DbValue_initOption(row, "label_location", _UNI32("1")), DbValue_initLang("MAP_LABEL_LOCATION")));

			GuiItemLayout* colLayout = UiRootTable_buildShortingList(row, "label_columns", "LABELS_ENABLE", FALSE, FALSE, DbRows_initEmpty(), FALSE);
			GuiItem_setGrid((GuiItem*)colLayout, Quad2i_init4(0, 1, 1, 5));
			GuiItem_addSubName((GuiItem*)layoutLabels, "columnsLayout", (GuiItem*)colLayout);

			GuiItem_addSubName((GuiItem*)layoutLabels, "label_center", GuiItemCheck_new(Quad2i_init4(0, 6, 1, 1), DbValue_initOption(row, "label_center", _UNI32("1")), DbValue_initLang("MAP_LABEL_CENTER")));

			GuiItem_addSubName((GuiItem*)layout, "layoutLabels", (GuiItem*)layoutLabels);
		}

		//colors
		{
			GuiItemLayout* layoutColor = GuiItemLayout_newTitle(Quad2i_init4(0, 15, 1, 3), DbValue_initLang("MAP_COLORS"));
			GuiItemLayout_addColumn(layoutColor, 0, 20);
			GuiItemLayout_addColumn(layoutColor, 2, 20);

			GuiItem_addSubName((GuiItem*)layoutColor, "cd1", (GuiItem*)GuiItemColor_new(Quad2i_init4(0, 1, 1, 2), DbRows_getSubOption(row, "color", "cd1", 0), FALSE));

			GuiItem_addSubName((GuiItem*)layoutColor, "cd_arrow", (GuiItem*)GuiItemText_new(Quad2i_init4(1, 1, 1, 2), TRUE, DbValue_initStaticCopyCHAR("->"), DbValue_initEmpty()));

			GuiItem_addSubName((GuiItem*)layoutColor, "column", GuiItemComboDynamic_new(Quad2i_init4(2, 1, 1, 1), FALSE, DbRows_initRefLink(row, "color"), DbValue_initOption(-1, "name", 0), DbRows_initSubsEx(table, "columns", FALSE, TRUE, FALSE, FALSE), DbValue_initEmpty()));
			GuiItem* cd2 = GuiItem_addSubName((GuiItem*)layoutColor, "cd2", (GuiItem*)GuiItemColor_new(Quad2i_init4(2, 2, 1, 1), DbRows_getSubOption(row, "color", "cd2", 0), FALSE));
			GuiItem_setEnableCallback(cd2, &GuiItem_enableColor);

			GuiItem_addSubName((GuiItem*)layout, "layoutColor", (GuiItem*)layoutColor);
		}

		//radius
		{
			GuiItemLayout* layoutRadius = GuiItemLayout_newTitle(Quad2i_init4(0, 19, 1, 3), DbValue_initLang("MAP_RADIUS"));
			GuiItemLayout_addColumn(layoutRadius, 0, 20);
			GuiItemLayout_addColumn(layoutRadius, 1, 20);

			DbValue multV = DbRows_getSubOption(row, "radius", "multiplier", 0);
			multV.staticPost = Std_newUNI_char("x");
			GuiItem_addSubName((GuiItem*)layoutRadius, "radius_mult", GuiItemEdit_new(Quad2i_init4(0, 1, 1, 2), multV, DbValue_initLang("MAP_RADIUS_MULT")));
			GuiItem_addSubName((GuiItem*)layoutRadius, "radius", GuiItemComboDynamic_new(Quad2i_init4(1, 1, 1, 2), FALSE, DbRows_initRefLink(row, "radius"), DbValue_initOption(-1, "name", 0), DbRows_initSubs(table, "columns", FALSE), DbValue_initLang("COLUMN")));

			GuiItem_addSubName((GuiItem*)layout, "layoutRadius", (GuiItem*)layoutRadius);
		}
	}

	return layout;
}

/*void UiRootMap_clickFocusItems(GuiItem* item)
{
	GuiItemMap* map = GuiItem_findName(item, "map");
	if (map)
		GuiItemMap_focusItems(map);
}*/
void UiRootMap_clickFocusSearch(GuiItem* item)
{
	GuiItemMap* map = GuiItem_findName(item, "map");
	if (map)
		GuiItemMap_focusSearch(map);
}

static GuiItem* UiRootMap_build(GuiItemLayout* layout, UBIG row, DbValue cam_lat, DbValue cam_long, DbValue cam_zoom, DbValue search, DbValue qfilterScroll, DbValue propertiesScroll)
{
	BIG thisRow = row;
	//if (DbRoot_isTypeViewReference(row))
	//	row = DbRoot_getOrigReference(row);

	if (UiRootQuickFilter_isShowPanel(row))
		UiRootMapPanel_setShowPanel(row, FALSE);

	const int width = UiRootQuickFilter_isShowPanel(row) ? UiRootQuickFilter_getWidth(row) : UiRootMapPanel_getWidth(row);

	GuiItemLayout_addColumn(layout, 0, Std_max(2, width));
	GuiItemLayout_addColumn(layout, 1, 99);
	GuiItemLayout_addRow(layout, 1, 99);

	//top header
	{
		GuiItemLayout* layoutMenu = GuiItemLayout_new(Quad2i_init4(0, 0, 2, 1));
		GuiItemLayout_setDrawBackground(layoutMenu, FALSE);
		GuiItemLayout_addColumn(layoutMenu, 0, 6);
		GuiItemLayout_addColumn(layoutMenu, 2, 4);
		GuiItemLayout_addColumn(layoutMenu, 4, 4);
		GuiItemLayout_addColumn(layoutMenu, 5, 99);
		GuiItemLayout_addColumn(layoutMenu, 6, 6);
		GuiItem_addSubName((GuiItem*)layout, "header_top", (GuiItem*)layoutMenu);

		//name
		GuiItem_addSubName((GuiItem*)layoutMenu, "header", UiRoot_createMenuNameHeader(Quad2i_init4(0, 0, 1, 1), thisRow));

		if (!DbRoot_isReference(row))
		{
			//Panel Settings activate
			GuiItem_addSubName((GuiItem*)layoutMenu, "panel_settings", UiRootMapPanel_buildButton(Quad2i_init4(2, 0, 1, 1), row));

			//Quick Filter activate
			GuiItem_addSubName((GuiItem*)layoutMenu, "quick_filter", UiRootQuickFilter_buildButton(Quad2i_init4(4, 0, 1, 1), row));

			//search
			GuiItemEdit* searchEdit = GuiItem_addSubName((GuiItem*)layoutMenu, "search", GuiItemEdit_newEx(Quad2i_init4(6, 0, 1, 1), search, DbValue_initLang("SEARCH"), &UiRootMap_clickFocusSearch));
			GuiItem_setIcon((GuiItem*)searchEdit, GuiImage_new1(UiIcons_init_search()));
			GuiItem_setShortcutKey((GuiItem*)searchEdit, FALSE, Win_EXTRAKEY_CTRL | Win_EXTRAKEY_SEARCH, 0, &GuiItemEdit_clickActivate);
		}
	}

	//bottom header
	/*if (!DbRoot_isReference(row))
	{
		GuiItemLayout* layoutMenu = GuiItemLayout_new(Quad2i_init4(0, 2, 2, 1));
		GuiItemLayout_setDrawBackground(layoutMenu, FALSE);
		GuiItemLayout_addColumn(layoutMenu, 0, 2);
		GuiItemLayout_addColumn(layoutMenu, 1, 4);
		GuiItemLayout_addColumn(layoutMenu, 2, 2);
		GuiItemLayout_addColumn(layoutMenu, 3, 4);
		GuiItemLayout_addColumn(layoutMenu, 4, 2);
		GuiItemLayout_addColumn(layoutMenu, 5, 2);
		GuiItemLayout_addColumn(layoutMenu, 6, 6);
		GuiItemLayout_addColumn(layoutMenu, 7, 6);
		GuiItem_addSubName((GuiItem*)layout, "header_bottom", (GuiItem*)layoutMenu);

		GuiItem_addSubName((GuiItem*)layoutMenu, "cam_longL", GuiItemText_new(Quad2i_init4(0, 0, 1, 1), FALSE, DbValue_initLang("MAP_CAM_LONG"), DbValue_initEmpty()));
		GuiItem_addSubName((GuiItem*)layoutMenu, "cam_long", GuiItemEdit_new(Quad2i_init4(1, 0, 1, 1), DbValue_initCopy(&cam_long), DbValue_initLang("MAP_CAM_LONG")));

		GuiItem_addSubName((GuiItem*)layoutMenu, "cam_latL", GuiItemText_new(Quad2i_init4(2, 0, 1, 1), FALSE, DbValue_initLang("MAP_CAM_LAT"), DbValue_initEmpty()));
		GuiItem_addSubName((GuiItem*)layoutMenu, "cam_lat", GuiItemEdit_new(Quad2i_init4(3, 0, 1, 1), DbValue_initCopy(&cam_lat), DbValue_initLang("MAP_CAM_LAT")));

		GuiItem_addSubName((GuiItem*)layoutMenu, "cam_zoomL", GuiItemText_new(Quad2i_init4(4, 0, 1, 1), FALSE, DbValue_initLang("MAP_CAM_ZOOM"), DbValue_initEmpty()));
		GuiItem_addSubName((GuiItem*)layoutMenu, "cam_zoom", GuiItemEdit_new(Quad2i_init4(5, 0, 1, 1), DbValue_initCopy(&cam_zoom), DbValue_initLang("MAP_CAM_ZOOM")));

		GuiItem_addSubName((GuiItem*)layoutMenu, "refocus", GuiItemButton_newClassicEx(Quad2i_init4(7, 0, 1, 1), DbValue_initLang("MAP_REFOCUS"), &UiRootMap_clickFocusItems));
	}*/

	BOOL hasPanel = FALSE;

	//Quick Filter
	DbRows filter = DbRows_initFilter(thisRow);
	//DbRows_forceEmptyFilter(&filter);

	if (!DbRoot_isReference(row))
	{
		GuiItemLayout* layoutQuick = UiRootQuickFilter_buildPanel(Quad2i_init4(0, 1, 1, 1), row, &filter, qfilterScroll);
		if (layoutQuick)
		{
			GuiItem_addSubName((GuiItem*)layout, "layout_quick_filter", (GuiItem*)layoutQuick);
			hasPanel = TRUE;
		}

		GuiItemLayout* layoutSettings = UiRootMapPanel_build(Quad2i_init4(0, 1, 1, 1), row, &filter, propertiesScroll);
		if (layoutSettings)
		{
			GuiItem_addSubName((GuiItem*)layout, "layout_panel_settings", (GuiItem*)layoutSettings);
			hasPanel = TRUE;
		}
	}

	//Map
	GuiItem_addSubName((GuiItem*)layout, "map", GuiItemMap_new(Quad2i_init4(hasPanel ? 1 : 0, 1, hasPanel ? 1 : 2, 1), row, filter, cam_lat, cam_long, cam_zoom, search));

	return (GuiItem*)layout;
}

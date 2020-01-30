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

BOOL UiRootChartPanel_isShowPanel(BIG row)
{
	return DbValue_getOptionNumber(row, "panel_settings_enable", 1);
}

void UiRootChartPanel_setShowPanel(BIG row, BOOL show)
{
	DbValue_setOptionNumber(row, "panel_settings_enable", show);
}

void UiRootChartPanel_clickShowPanel(GuiItem* item)
{
	BIG row = GuiItem_findAttribute(item, "row");
	UiRootChartPanel_setShowPanel(row, !UiRootChartPanel_isShowPanel(row));

	//QFilter disable
	UiRootQuickFilter_setShowPanel(row, FALSE);
}

BOOL UiRootChartPanel_enableRowX(GuiItem* item)
{
	BIG row = GuiItem_findAttribute(item, "row");
	return GuiItemChart_isTypeXY_public(DbValue_getOptionNumber(row, "chart_type", 0));
}
BOOL UiRootChartPanel_enableTitleX(GuiItem* item)
{
	BIG row = GuiItem_findAttribute(item, "row");
	return !GuiItemChart_isTypePIE_public(DbValue_getOptionNumber(row, "chart_type", 0));
}

static double UiRootChartPanel_getWidth(UBIG row)
{
	return DbValue_getOptionNumber(row, "panel_settings_width", 10);
}

GuiItem* UiRootChartPanel_buildButton(Quad2i grid, UBIG row)
{
	GuiItemButton* button = (GuiItemButton*)GuiItemButton_newAlphaEx(grid, DbValue_initLang("PROPERTIES"), &UiRootChartPanel_clickShowPanel);
	GuiItemButton_setPressed(button, UiRootChartPanel_isShowPanel(row));
	GuiItem_setAttribute((GuiItem*)button, "row", row);

	return (GuiItem*)button;
}

GuiItemLayout* UiRootChartPanel_build(Quad2i grid, UBIG row, DbRows* filter, DbValue scroll)
{
	GuiItemLayout* layout = 0;

	DbTable* table = DbRoot_findParentTable(row);

	//BIG rowXRow = DbRows_findOrCreateSubType(row, "row_x");
	//BIG titleXRow = DbRows_findOrCreateSubType(row, "title_x");

	//selection panel
	if (UiRootChartPanel_isShowPanel(row))
	{
		layout = GuiItemLayout_new(grid);
		GuiItemLayout_setScrollV(layout, scroll);
		//GuiItemLayout_setDrawBackground(layout, TRUE);
		GuiItemLayout_setBackgroundWhite(layout, TRUE);

		GuiItemLayout_addColumn(layout, 0, 99);
		//GuiItemLayout_addRow(layout, 2, 99);
		GuiItem_setChangeSize((GuiItem*)layout, TRUE, DbValue_initOption(row, "panel_settings_width", 0), TRUE);
		GuiItem_setAttribute((GuiItem*)layout, "row", row);

		//type
		{
			GuiItemLayout* layoutType = GuiItemLayout_newTitle(Quad2i_init4(0, 0, 1, 2), DbValue_initLang("CHART_TYPE"));
			//GuiItemLayout_setTitle(layoutType, DbValue_initLang("CHART_TYPE"));
			GuiItemLayout_addColumn(layoutType, 0, 20);
			GuiItemLayout_addColumn(layoutType, 1, 20);

			GuiItemComboStatic* type = (GuiItemComboStatic*)GuiItem_addSubName((GuiItem*)layoutType, "type", GuiItemComboStatic_new(Quad2i_init4(0, 1, 2, 1), DbValue_initOption(row, "chart_type", 0), 0, DbValue_initEmpty()));
			GuiItemComboStatic_addItemIcon(type, GuiImage_new1(UiIcons_init_chart_column_normal()), DbValue_initLang("CHART_TYPE_COLUMN_NORMAL"));
			GuiItemComboStatic_addItemIcon(type, GuiImage_new1(UiIcons_init_chart_column_stack()), DbValue_initLang("CHART_TYPE_COLUMN_STACK"));
			GuiItemComboStatic_addItemIcon(type, GuiImage_new1(UiIcons_init_chart_column_stack_proc()), DbValue_initLang("CHART_TYPE_COLUMN_STACK_PROC"));

			GuiItemComboStatic_addItemIcon(type, GuiImage_new1(UiIcons_init_chart_pie()), DbValue_initLang("CHART_TYPE_PIE"));
			//GuiItemComboStatic_addItemIcon(type, GuiImage_new1(UiIcons_init_chart_pie()), DbValue_initLang("CHART_TYPE_PIE_STACK"));

			GuiItemComboStatic_addItemIcon(type, GuiImage_new1(UiIcons_init_chart_point()), DbValue_initLang("CHART_TYPE_POINTS"));
			GuiItemComboStatic_addItemIcon(type, GuiImage_new1(UiIcons_init_chart_point_xy()), DbValue_initLang("CHART_TYPE_XY_POINTS"));
			//GuiItemComboStatic_addItemIcon(type, GuiImage_new1(UiIcons_init_chart_point_xy_ordered()), DbValue_initLang("CHART_TYPE_XY_POINTS_ORDERED"));

			GuiItemComboStatic_addItemIcon(type, GuiImage_new1(UiIcons_init_chart_line()), DbValue_initLang("CHART_TYPE_LINE"));
			GuiItemComboStatic_addItemIcon(type, GuiImage_new1(UiIcons_init_chart_line_xy()), DbValue_initLang("CHART_TYPE_XY_LINE"));
			GuiItemComboStatic_addItemIcon(type, GuiImage_new1(UiIcons_init_chart_line_xy_ordered()), DbValue_initLang("CHART_TYPE_XY_LINE_ORDERED"));

			//values color
			//GuiItem_addSubName((GuiItem*)layoutType, "cd", (GuiItem*)GuiItemColor_new(Quad2i_init4(1, 4, 1, 1), DbValue_initOption(row, "text_cd", 0), FALSE));	//white: _UNI32("3439329279")

			GuiItem_addSubName((GuiItem*)layout, "type", (GuiItem*)layoutType);
		}
		//columns
		{
			GuiItemLayout* columnsLayout = GuiItemLayout_newTitle(Quad2i_init4(0, 3, 1, 6), DbValue_initLang("CHART_COLUMNS_X"));
			GuiItemLayout_addColumn(columnsLayout, 0, 20);
			{
				GuiItemLayout* colLayout = UiRootTable_buildShortingList(row, "chart_columns", 0, FALSE, TRUE, DbRows_initSubsEx(table, "columns", FALSE, TRUE, FALSE, FALSE), TRUE);
				GuiItem_setGrid((GuiItem*)colLayout, Quad2i_init4(0, 1, 1, 4));
				GuiItem_addSubName((GuiItem*)columnsLayout, "colLayout", (GuiItem*)colLayout);
			}
			//show values
			GuiItem_addSubName((GuiItem*)columnsLayout, "show_values", GuiItemCheck_new(Quad2i_init4(0, 5, 1, 1), DbValue_initOption(row, "show_columns_values", _UNI32("1")), DbValue_initLang("CHART_SHOW_VALUES")));
			GuiItem_addSubName((GuiItem*)layout, "columnss", (GuiItem*)columnsLayout);
		}

		//rows
		{
			GuiItemLayout* rowsLayout = GuiItemLayout_newTitle(Quad2i_init4(0, 10, 1, 6), DbValue_initLang("CHART_ROW_X"));
			GuiItemLayout_addColumn(rowsLayout, 0, 20);
			{
				BOOL isTypeXY = GuiItemChart_isTypeXY_public(DbValue_getOptionNumber(row, "chart_type", 0));

				GuiItemLayout* roLayout = UiRootTable_buildShortingList(row, "chart_rows", 0, FALSE, FALSE, DbRows_initSubsEx(table, "columns", FALSE, TRUE, TRUE, FALSE), isTypeXY);
				GuiItem_setGrid((GuiItem*)roLayout, Quad2i_init4(0, 1, 1, 4));
				GuiItem_addSubName((GuiItem*)rowsLayout, "rowLayout", (GuiItem*)roLayout);
			}
			GuiItem_addSubName((GuiItem*)rowsLayout, "show_values", GuiItemCheck_new(Quad2i_init4(0, 5, 1, 1), DbValue_initOption(row, "show_rows_values", _UNI32("1")), DbValue_initLang("CHART_SHOW_AXIS")));
			//GuiItem_setEnableCallback((GuiItem*)rowsLayout, &UiRootChartPanel_enableRowX);
			GuiItem_addSubName((GuiItem*)layout, "rowss", (GuiItem*)rowsLayout);
		}

		//groups
		{
			GuiItemLayout* groupLayout = GuiItemLayout_newTitle(Quad2i_init4(0, 17, 1, 7), DbValue_initLang("GROUP"));
			GuiItemLayout_addColumn(groupLayout, 0, 20);
			{
				GuiItemLayout* grLayout = UiRootTable_buildShortingList(row, "chart_groups", "GROUP_ENABLE", TRUE, FALSE, DbRows_initEmpty(), FALSE);
				GuiItem_setGrid((GuiItem*)grLayout, Quad2i_init4(0, 1, 1, 5));
				GuiItem_addSubName((GuiItem*)groupLayout, "grLayout", (GuiItem*)grLayout);
			}

			GuiItem* groupCount = GuiItem_addSubName((GuiItem*)groupLayout, "groupCount", GuiItemCheck_new(Quad2i_init4(0, 6, 1, 1), DbRows_getSubOption(row, "chart_groups", "group_count", 0), DbValue_initLang("CHART_COUNT")));

			GuiItem_setAttribute(groupCount, "row", DbRows_findOrCreateSubType(row, "chart_groups"));
			GuiItem_setEnableCallback((GuiItem*)groupCount, &GuiItem_enableEnableAttr);

			GuiItem_addSubName((GuiItem*)layout, "groups", (GuiItem*)groupLayout);
		}

		//advanced
		{
			GuiItemLayout* layoutAdv = GuiItemLayout_newTitle(Quad2i_init4(0, 25, 1, 3), DbValue_initLang("OTHERS"));
			GuiItemLayout_addColumn(layoutAdv, 0, 20);
			GuiItemLayout_addColumn(layoutAdv, 1, 20);

			//precision
			GuiItem_addSubName((GuiItem*)layoutAdv, "precision", GuiItemEdit_new(Quad2i_init4(0, 1, 2, 2), DbValue_initOption(row, "precision", 0), DbValue_initLang("PRECISION")));

			//GuiItem_setGrid((GuiItem*)layoutAdv, Quad2i_init4(0, 26, 1, 7));
			GuiItem_addSubName((GuiItem*)layout, "data_description", (GuiItem*)layoutAdv);
		}
	}

	return layout;
}

static GuiItem* UiRootChart_build(GuiItemLayout* layout, UBIG row, DbValue scrollH, DbValue qfilterScroll, DbValue propertiesScroll)
{
	BIG thisRow = row;

	if (UiRootQuickFilter_isShowPanel(row))
		UiRootChartPanel_setShowPanel(row, FALSE);

	const int width = UiRootQuickFilter_isShowPanel(row) ? UiRootQuickFilter_getWidth(row) : UiRootChartPanel_getWidth(row);

	GuiItemLayout_addColumn(layout, 0, Std_max(2, width));
	GuiItemLayout_addColumn(layout, 1, 99);
	GuiItemLayout_addRow(layout, 1, 99);
	//GuiItemLayout_setDrawBackground(layout, FALSE);

	//header
	{
		GuiItemLayout* layoutMenu = GuiItemLayout_new(Quad2i_init4(0, 0, 2, 1));
		GuiItemLayout_setDrawBackground(layoutMenu, FALSE);
		GuiItemLayout_addColumn(layoutMenu, 0, 6);	//name
		GuiItemLayout_addColumn(layoutMenu, 2, 4);
		GuiItemLayout_addColumn(layoutMenu, 4, 4);
		GuiItemLayout_addColumn(layoutMenu, 6, 4);
		GuiItem_addSubName((GuiItem*)layout, "header_top", (GuiItem*)layoutMenu);

		//name
		GuiItem_addSubName((GuiItem*)layoutMenu, "header", UiRoot_createMenuNameHeader(Quad2i_init4(0, 0, 1, 1), thisRow));

		if (!DbRoot_isReference(row))
		{
			//Panel Settings activate
			GuiItem_addSubName((GuiItem*)layoutMenu, "panel_settings", UiRootChartPanel_buildButton(Quad2i_init4(2, 0, 1, 1), row));

			//Quick Filter activate
			GuiItem_addSubName((GuiItem*)layoutMenu, "quick_filter", UiRootQuickFilter_buildButton(Quad2i_init4(4, 0, 1, 1), row));

			//width
			GuiItemComboStatic* rowH = GuiItem_addSubName((GuiItem*)layoutMenu, "width", GuiItemComboStatic_new(Quad2i_init4(6, 0, 1, 1), DbValue_initOption(row, "chart_width", _UNI32("1")), 0, DbValue_initEmpty()));
			GuiItemComboStatic_addItem(rowH, DbValue_initStaticCopyCHAR("50%"));
			GuiItemComboStatic_addItem(rowH, DbValue_initStaticCopyCHAR("100%"));
			GuiItemComboStatic_addItem(rowH, DbValue_initStaticCopyCHAR("150%"));
			GuiItemComboStatic_addItem(rowH, DbValue_initStaticCopyCHAR("200%"));
			GuiItemComboStatic_addItem(rowH, DbValue_initStaticCopyCHAR("300%"));
			GuiItem_setIcon((GuiItem*)rowH, GuiImage_new1(UiIcons_init_table_column_height()));
		}
	}

	BOOL hasPanel = FALSE;

	//Quick Filter
	DbRows filter = DbRows_initFilter(thisRow);

	if (!DbRoot_isReference(row))
	{
		GuiItemLayout* layoutQuick = UiRootQuickFilter_buildPanel(Quad2i_init4(0, 1, 1, 1), row, &filter, qfilterScroll);
		if (layoutQuick)
		{
			GuiItem_addSubName((GuiItem*)layout, "layout_quick_filter", (GuiItem*)layoutQuick);
			hasPanel = TRUE;
		}

		GuiItemLayout* layoutSettings = UiRootChartPanel_build(Quad2i_init4(0, 1, 1, 1), row, &filter, propertiesScroll);
		if (layoutSettings)
		{
			GuiItem_addSubName((GuiItem*)layout, "layout_panel_settings", (GuiItem*)layoutSettings);
			hasPanel = TRUE;
		}
	}

	//Chart
	GuiItem_addSubName((GuiItem*)layout, "chart", GuiItemChart_new(Quad2i_init4(hasPanel ? 1 : 0, 1, hasPanel ? 1 : 2, 1), row, filter, scrollH));

	return (GuiItem*)layout;
}

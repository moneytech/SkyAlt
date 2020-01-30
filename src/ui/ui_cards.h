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

static GuiItem* UiRootCard_build(GuiItemLayout* layout, UBIG row, DbValue scroll, DbValue qfilterScroll)
{
	BIG thisRow = row;
	//if (DbRoot_isTypeViewReference(row))
	//	row = DbRoot_getOrigReference(row);

	GuiItemLayout_setScrollV(layout, scroll);
	GuiItemLayout_addColumn(layout, 0, Std_max(2, UiRootQuickFilter_getWidth(row)));
	GuiItemLayout_addColumn(layout, 1, 99);
	GuiItemLayout_addRow(layout, 1, 99);

	BIG cardWidthV = DbValue_getOptionNumber(row, "card_width", 0);

	//header
	{
		GuiItemLayout* layoutMenu = GuiItemLayout_new(Quad2i_init4(0, 0, 2, 1));
		GuiItemLayout_setDrawBackground(layoutMenu, FALSE);
		GuiItemLayout_addColumn(layoutMenu, 0, 6);	//name
		GuiItemLayout_addColumn(layoutMenu, 2, 3);
		GuiItemLayout_addColumn(layoutMenu, 4, 4);
		GuiItemLayout_addColumn(layoutMenu, 6, 4);

		GuiItem_addSubName((GuiItem*)layout, "menu", (GuiItem*)layoutMenu);

		//name
		GuiItem_addSubName((GuiItem*)layoutMenu, "header", UiRoot_createMenuNameHeader(Quad2i_init4(0, 0, 1, 1), thisRow));

		if (!DbRoot_isReference(row))
		{
			//Hidden
			GuiItemMenu* columns = (GuiItemMenu*)GuiItem_addSubName((GuiItem*)layoutMenu, "columns", GuiItemMenu_new(Quad2i_init4(2, 0, 2, 1), DbValue_initLang("COLUMNS"), FALSE));
			GuiItemMenu_setContext(columns, GuiItemTable_buildShowedList(row));
			GuiItemMenu_setHighligthBackground(columns, DbRows_hasColumnsSubDeactive(row, "columns"));
			GuiItemMenu_setTransparent(columns, FALSE);
			GuiItem_setIcon((GuiItem*)columns, GuiImage_new1(UiIcons_init_table_hide()));
			GuiItemMenu_setCenter(columns, FALSE);

			//Item width
			DbValue cardWidth = DbValue_initOption(row, "card_width", 0);
			GuiItem* rowH = GuiItem_addSubName((GuiItem*)layoutMenu, "card_width", GuiItemComboStatic_new(Quad2i_init4(4, 0, 1, 1), cardWidth, Lang_find("CARD_WIDTH_OPTIONS"), DbValue_initEmpty()));
			GuiItem_setIcon(rowH, GuiImage_new1(UiIcons_init_table_column_height()));

			//Quick Filter activate
			GuiItem_addSubName((GuiItem*)layoutMenu, "quick_filter", UiRootQuickFilter_buildButton(Quad2i_init4(6, 0, 1, 1), row));
		}
	}

	//Quick Filter
	DbRows filter = DbRows_initFilter(thisRow);
	BOOL hasPanel = FALSE;
	if (!DbRoot_isReference(row))
	{
		GuiItemLayout* layoutQuick = UiRootQuickFilter_buildPanel(Quad2i_init4(0, 1, 1, 1), row, &filter, qfilterScroll);
		if (layoutQuick)
		{
			GuiItem_addSubName((GuiItem*)layout, "layout_quick_filter", (GuiItem*)layoutQuick);
			hasPanel = TRUE;
		}
	}

	//List
	GuiItemLayout* card = GuiItemTable_buildPage(row, -1, TRUE, FALSE);// , !DbValue_getOptionNumber(row, "edit_mode", 1));
	GuiItemLayout_setDrawBorder(card, TRUE);
	GuiItemList* list = (GuiItemList*)GuiItem_addSubName((GuiItem*)layout, "list", GuiItemList_new(Quad2i_init4(hasPanel ? 1 : 0, 1, hasPanel ? 1 : 2, 1), filter, (GuiItem*)card, DbValue_initEmpty()));
	GuiItemList_setShowBorder(list, FALSE);
	GuiItemList_setAsGrid(list, (cardWidthV + 1) * 5);
	GuiItemList_setShowRemove(list, DbValue_getOptionNumber(row, "show_remove", 0));

	return (GuiItem*)layout;
}
/*
 * Copyright (c) 2018 Milan Suk
 *
 * Use of this software is governed by the Business Source License included
 * in the LICENSE file and at www.mariadb.com/bsl11.
 *
 * Change Date: 2024-11-01
 *
 * On the date above, in accordance with the Business Source License, use
 * of this software will be governed by version 2 or later of the General
 * Public License.
 */

static GuiItem* UiRootCard_build(GuiItemLayout* layout, UBIG row)
{
	GuiItemLayout_addColumn(layout, 0, 99);
	GuiItemLayout_addRow(layout, 1, 99);

	//header
	GuiItemLayout* layoutMenu = GuiItemLayout_new(Quad2i_init4(0, 0, 1, 1));
	GuiItemLayout_setDrawBackground(layoutMenu, FALSE);
	GuiItemLayout_addColumn(layoutMenu, 0, 6);	//name
	GuiItemLayout_addColumn(layoutMenu, 2, 3);
	GuiItemLayout_addColumn(layoutMenu, 4, 4);
	GuiItemLayout_addColumn(layoutMenu, 6, 4);
	GuiItemLayout_addColumn(layoutMenu, 8, 4);

	GuiItem_addSubName((GuiItem*)layout, "menu", (GuiItem*)layoutMenu);

	//name
	GuiItem* name = GuiItem_addSubName((GuiItem*)layoutMenu, "name", GuiItemEdit_newEx(Quad2i_init4(0, 0, 1, 1), DbValue_initOption(row, "name", 0), DbValue_initLang("NAME"), 0));
	GuiItem_setIcon(name, GuiImage_new1(UiIcons_init_name()));

	//Hidden
	GuiItemMenu* hidden = (GuiItemMenu*)GuiItem_addSubName((GuiItem*)layoutMenu, "hidden", GuiItemMenu_new(Quad2i_init4(2, 0, 2, 1), DbValue_initLang("HIDDEN"), FALSE));
	GuiItemMenu_setContext(hidden, UiRootTable_buildShowedList(row, TRUE));
	GuiItemMenu_setHighligthBackground(hidden, DbRows_hasColumnsSubDeactive(row, "columns"));
	GuiItem_setIcon((GuiItem*)hidden, GuiImage_new1(UiIcons_init_table_hide()));
	GuiItemMenu_setCenter(hidden, FALSE);

	//Item width
	DbValue cardWidth = DbValue_initOption(row, "card_width", 0);
	BIG cardWidthV = DbValue_getNumber(&cardWidth);
	GuiItem* rowH = GuiItem_addSubName((GuiItem*)layoutMenu, "card_width", GuiItemComboStatic_new(Quad2i_init4(4, 0, 1, 1), cardWidth, Lang_find("CARD_WIDTH_OPTIONS"), DbValue_initEmpty()));
	GuiItem_setIcon(rowH, GuiImage_new1(UiIcons_init_table_column_height()));

	//edit move
	GuiItem_addSubName((GuiItem*)layoutMenu, "edit_mode", GuiItemCheck_newEx(Quad2i_init4(6, 0, 1, 1), DbValue_initOption(row, "edit_mode", 0), DbValue_initLang("CARD_EDIT_MOVE"), 0));

	//show remove
	GuiItem_addSubName((GuiItem*)layoutMenu, "remove", GuiItemCheck_newEx(Quad2i_init4(8, 0, 1, 1), DbValue_initOption(row, "show_remove", 0), DbValue_initLang("SHOW_REMOVE"), 0));

	//List
	GuiItemLayout* card = GuiItemTable_buildPage(row, TRUE, !DbValue_getOptionNumber(row, "edit_mode", 1));
	GuiItemLayout_setDrawBorder(card, TRUE);
	DbRows filter = DbRows_initFilter(row);
	GuiItemList* list = (GuiItemList*)GuiItem_addSubName((GuiItem*)layout, "list", GuiItemList_new(Quad2i_init4(0, 1, 1, 1), filter, (GuiItem*)card, DbValue_initEmpty()));
	GuiItemList_setShowBorder(list, FALSE);
	GuiItemList_setAsGrid(list, (cardWidthV + 1) * 5);
	GuiItemList_setShowRemove(list, DbValue_getOptionNumber(row, "show_remove", 0));

	return (GuiItem*)layout;
}
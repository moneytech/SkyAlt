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

static GuiItem* UiRootGroup_build(GuiItemLayout* layout, UBIG row, DbValue scrollH)
{
	GuiItemLayout_addColumn(layout, 0, 99);
	GuiItemLayout_addRow(layout, 1, 99);

	//header
	{
		GuiItemLayout* layoutMenu = GuiItemLayout_new(Quad2i_init4(0, 0, 1, 1));
		GuiItemLayout_setDrawBackground(layoutMenu, FALSE);
		GuiItemLayout_addColumn(layoutMenu, 0, 6);	//name
		GuiItemLayout_addColumn(layoutMenu, 2, 6);
		GuiItemLayout_addColumn(layoutMenu, 4, 6);

		GuiItem_addSubName((GuiItem*)layout, "menu", (GuiItem*)layoutMenu);

		//name
		GuiItem* name = GuiItem_addSubName((GuiItem*)layoutMenu, "name", GuiItemEdit_newEx(Quad2i_init4(0, 0, 1, 1), DbValue_initOption(row, "name", 0), DbValue_initLang("NAME"), 0));
		GuiItem_setIcon(name, GuiImage_new1(UiIcons_init_name()));
	}

	//Group
	DbRows filter = DbRows_initFilter(row);
	DbRows_forceEmptyFilter(&filter);
	GuiItem_addSubName((GuiItem*)layout, "group", GuiItemGroup_new(Quad2i_init4(0, 1, 1, 1), row, filter, scrollH));

	return (GuiItem*)layout;
}

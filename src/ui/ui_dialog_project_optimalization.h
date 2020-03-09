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

void UiDialogProjectOptimalization_clickDeleteMapTiles(GuiItem* item)
{
	Map_deleteTilesFile();
}
void UiDialogProjectOptimalization_clickDeleteMapGeolocation(GuiItem* item)
{
	DbRoot_mapDeleteGeolocation();
}

GuiItemLayout* UiDialogProjectOptimalization_new(void)
{
	//layout
	GuiItemLayout* layout = GuiItemLayout_new(Quad2i_init4(0, 0, 1, 1));
	GuiItemLayout_addColumn(layout, 0, 10);
	GuiItemLayout_addColumn(layout, 2, 10);

	//Show/disable Audio ...
	//Show/disable Video ...
	//Show/disable PDF ...
	//Show/disable Map ...

	//Show/disable Web ...
	//Show/disable Email ...

	//maps
	{
		GuiItem_addSubName((GuiItem*)layout, "map_tiles_bytes", GuiItemText_new(Quad2i_init4(0, 0, 1, 2), TRUE, DbValue_initStatic(Std_newNumberSize(Map_bytes())), DbValue_initLang("MAP_TILES")));
		GuiItem_addSubName((GuiItem*)layout, "map_geolocation_bytes", GuiItemText_new(Quad2i_init4(2, 0, 1, 2), TRUE, DbValue_initStatic(Std_newNumberSize(DbRoot_mapBytes())), DbValue_initLang("MAP_LOCATION_CACHE")));

		GuiItem_addSubName((GuiItem*)layout, "map_delete_tiles", GuiItemButton_newClassicEx(Quad2i_init4(0, 2, 1, 1), DbValue_initLang("DELETE"), &UiDialogProjectOptimalization_clickDeleteMapTiles));
		GuiItem_addSubName((GuiItem*)layout, "map_delete_geolocation", GuiItemButton_newClassicEx(Quad2i_init4(2, 2, 1, 1), DbValue_initLang("DELETE"), &UiDialogProjectOptimalization_clickDeleteMapGeolocation));
	}

	return GuiItemRoot_createDialogLayout(Vec2i_init2(15, 10), DbValue_initLang("PROJECT_OPTIMALIZATION"), (GuiItem*)layout, 0);
}

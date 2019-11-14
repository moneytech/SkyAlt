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

GuiItemLayout* UiDialogAbout_new(void)
{
	//layout
	GuiItemLayout* layout = GuiItemLayout_new(Quad2i_init());
	GuiItemLayout_addColumn(layout, 0, 100);
	GuiItemLayout_addRow(layout, 0, 100);

	//logo
	GuiItem_addSubName((GuiItem*)layout, "logo", GuiItemBox_newImage(Quad2i_init4(0, 0, 1, 2), GuiImage_new1(UiLogo_init())));

	//Version
	{
		UNI* version = UiScreen_getNameVersionUNI();
		GuiItem_addSubName((GuiItem*)layout, "version", GuiItemText_new(Quad2i_init4(0, 3, 1, 1), TRUE, DbValue_initStatic(version), DbValue_initEmpty()));
	}

	//UID
	{
		UNI* uid = FileProject_getUID_string();
		UNI* s = Std_newUNI(_UNI32("User identifier(UID): "));
		s = Std_addAfterUNI(s, uid);
		GuiItem_addSubName((GuiItem*)layout, "uid", GuiItemText_new(Quad2i_init4(0, 4, 1, 1), TRUE, DbValue_initStatic(s), DbValue_initEmpty()));
		Std_deleteUNI(uid);
	}

	//website
	GuiItem_addSubName((GuiItem*)layout, "website", GuiItemText_newUnderline(Quad2i_init4(0, 6, 1, 1), TRUE, DbValue_initStaticCopy(STD_WEBSITE), DbValue_initEmpty(), TRUE, FALSE));

	//copyright
	GuiItem_addSubName((GuiItem*)layout, "copyright_skyalt", GuiItemText_new(Quad2i_init4(0, 7, 1, 1), TRUE, DbValue_initLang("COPYRIGHT_SKYALT"), DbValue_initEmpty()));

	//warranty
	GuiItem_addSubName((GuiItem*)layout, "warranty", GuiItemText_new(Quad2i_init4(0, 8, 1, 1), TRUE, DbValue_initLang("WARRANTY"), DbValue_initEmpty()));

	//icons
	GuiItem_addSubName((GuiItem*)layout, "copyright_skyalt", GuiItemText_newUnderline(Quad2i_init4(0, 10, 1, 2), TRUE, DbValue_initStaticCopy(_UNI32("https://icons8.com/")), DbValue_initLang("ICONS_LICENSE"), TRUE, FALSE));

	//OpenStreetMap
	GuiItem_addSubName((GuiItem*)layout, "copyright_osm", GuiItemText_newUnderline(Quad2i_init4(0, 13, 1, 2), TRUE, DbValue_initStaticCopy(_UNI32("https://www.openstreetmap.org/copyright")), DbValue_initLang("OPENSTREETMAP_LICENSE"), TRUE, FALSE));

	return GuiItemRoot_createDialogLayout(Vec2i_init2(15, 17), DbValue_initLang("ABOUT"), (GuiItem*)layout, 0);
}

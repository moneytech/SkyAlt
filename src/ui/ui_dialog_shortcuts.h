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

GuiItemLayout* UiDialogShortcuts_new(void)
{
	//layout
	GuiItemLayout* layout = GuiItemLayout_new(Quad2i_init4(0, 0, 1, 1));
	GuiItemLayout_addColumn(layout, 0, 5);
	GuiItemLayout_addColumn(layout, 1, 2);
	GuiItemLayout_addColumn(layout, 2, 5);

	//Column A
	GuiItem_addSubName((GuiItem*)layout, "1", GuiItemText_new(Quad2i_init4(0, 0, 1, 2), TRUE, DbValue_initStaticCopy(_UNI32("Ctrl + n")), DbValue_initLang("NEW")));
	GuiItem_addSubName((GuiItem*)layout, "2", GuiItemText_new(Quad2i_init4(0, 3, 1, 2), TRUE, DbValue_initStaticCopy(_UNI32("Ctrl + s")), DbValue_initLang("SAVE")));
	//...

	//Column B
	GuiItem_addSubName((GuiItem*)layout, "3", GuiItemText_new(Quad2i_init4(2, 0, 1, 2), TRUE, DbValue_initStaticCopy(_UNI32("Ctrl + '+'")), DbValue_initLang("ZOOM_IN")));
	GuiItem_addSubName((GuiItem*)layout, "4", GuiItemText_new(Quad2i_init4(2, 3, 1, 2), TRUE, DbValue_initStaticCopy(_UNI32("Ctrl + '-'")), DbValue_initLang("ZOOM_OUT")));
	GuiItem_addSubName((GuiItem*)layout, "5", GuiItemText_new(Quad2i_init4(2, 6, 1, 2), TRUE, DbValue_initStaticCopy(_UNI32("Ctrl + 0")), DbValue_initLang("ZOOM_DEFAULT")));
	GuiItem_addSubName((GuiItem*)layout, "6", GuiItemText_new(Quad2i_init4(2, 9, 1, 2), TRUE, DbValue_initStaticCopy(_UNI32("F11")), DbValue_initLang("FULLSCREEN_MODE")));
	//...

	return GuiItemRoot_createDialogLayout(Vec2i_init2(15, 15), DbValue_initLang("SHORTCUTS"), (GuiItem*)layout, 0);
}

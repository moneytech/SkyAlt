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

 //note: this will not be translated
const char* g_gui_changelog = ""
"Build.1 (1/Nov/2019)\n"
"\tFirst public release\n"
"\n";

GuiItemLayout* UiDialogChangelog_new(void)
{
	//layout
	GuiItemLayout* layout = GuiItemLayout_new(Quad2i_init4(0, 0, 1, 1));
	GuiItemLayout_addColumn(layout, 0, 20);
	GuiItemLayout_addRow(layout, 0, 20);

	//info
	GuiItem_addSubName((GuiItem*)layout, "text", GuiItemTextMulti_new(Quad2i_init4(0, 0, 1, 1), DbValue_initStaticCopyCHAR(g_gui_changelog)));

	return GuiItemRoot_createDialogLayout(Vec2i_init2(20, 20), DbValue_initLang("CHANGELOG"), (GuiItem*)layout, 0);
}

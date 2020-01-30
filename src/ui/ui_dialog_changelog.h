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

 //note: this will not be translated
const char* g_gui_changelog = ""
"Build.1 (14/Nov/2019)\n"
"\tFirst public release\n"
"\n"
"Build.2 (30/Jan/2020)\n"
"\tFixed: Tons\n"

"\tChanged:\n"
"\t\tFew icons\n"
"\t\tQuickShort() for shorting\n"

"\tAdded:\n"
"\t\tMenu - Report Problem\n"
"\t\tExpand/Hide items in side panel Tree view\n"
"\t\tRemove Filter view(keep content)\n"
"\t\tGenerate Filter view from Group view\n"
"\t\tDate filter: less/greate than Today() + x(years/months/etc.)\n"
"\t\tDrag&Drop scrolled list/layout if you are close to edge\n"
"\t\tWaiting progress bar shows Description, %Done and Time estimation\n"
"\t\tUnits(Thousands, Millions, Billions) for Column format Number and Currency\n"
"\t\tProject reload dialog, If other device made changes to project\n"
"\t\tCommercial license file support\n"
"\t\tColor picker\n"
"\t\tView as reference\n"

"\t\tInternet manager + GUI status/bandwidth\n"
"\t\tGeolocation\n"

"\t\tMap view\n"
"\t\tMap disk cache\n"

"\t\tChart view\n"

"\t\tSummary column\n"
"\t\tSummary view\n"

"\t\tQuick Filter for Cards/Map/Chart\n"

"\t\tLink - Mirrored column\n"
"\t\tLink - Jointed column\n"
"\t\tLink - Filtered column\n"

"\t\tRemote data sources(ODBC)\n"

"\tPorted: Linux\n"

//"\tExamples-added: ...\n"

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

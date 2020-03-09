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
    "\t\tIMDB example project\n"
"\tPorted: Linux\n"
"\n"
"Build.2.5 (10/Mar/2020)\n"
"\tFixed:\n"
    "\t\tMap bbox limit\n"
    "\t\tFast double-click on a cell\n"
    "\t\tGroup view lanes remember scroll positions\n"
    "\t\tMany other small bugs\n"

"\tChanged:\n"
    "\t\tMap controls inside layout\n"
    "\t\tTable view has fixed 'id' column\n"

"\tAdded:\n"
    "\t\tColor background option for Tags\n"
    "\t\tColor background option for Menu\n"
    "\t\tColor background option for Checkbox\n"
    "\t\tMap zoom +/-\n"
    "\t\tMap smooth zoom\n"

    //"\t\tRender .gpx on Map\n"

    "\t\t'Add column to Filter' in Column setting\n"
    "\t\t'Add column to Short' in Column setting\n"
    "\t\tAdd column Description edit-box\n"

    "\t\tTable 'id' column can contain values from other column(s)\n"
    "\t\tTable bottom insights\n"

    "\t\tCtrl+'g' for jumping on specific table row\n"

    "\t\tCtrl+click on Combo will choose next option\n"

    "\t\tFind in Table(+highlight)\n"
    "\t\tFind & replace in Table\n"
"\n";

GuiItemLayout* UiDialogChangelog_new(void)
{
	//layout
	GuiItemLayout* layout = GuiItemLayout_new(Quad2i_init4(0, 0, 1, 1));
	GuiItemLayout_addColumn(layout, 0, 20);
	GuiItemLayout_addRow(layout, 0, 20);

	//info
    GuiItemTextMulti* tx = GuiItem_addSubName((GuiItem*)layout, "text", GuiItemTextMulti_new(Quad2i_init4(0, 0, 1, 1), DbValue_initStaticCopyCHAR(g_gui_changelog)));
    GuiItemTextMulti_scroll(tx, 10000000);

	return GuiItemRoot_createDialogLayout(Vec2i_init2(20, 20), DbValue_initLang("CHANGELOG"), (GuiItem*)layout, 0);
}

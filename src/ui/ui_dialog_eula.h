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

void UiDialogEula_clickChooseLanguage(GuiItem* item)
{
	Lang_setPos(Std_max(0, GuiItemComboStatic_getValue(GuiItem_findName(item, "language"))));
}

void UiDialogEula_clickYes(GuiItem* item)
{
	UiIniSettings_setLicenseAccept();
	UiScreen_setStartup();
}
void UiDialogEula_clickNo(GuiItem* item)
{
	BOOL onlyText = GuiItem_findAttribute(item, "onlyText");

	if (onlyText)
		GuiItem_closeParentLevel(item);
	else
	{
		printf("License declined\n");
		UiIniSettings_setLicenseVersion(-1);
	}
}

GuiItemLayout* UiDialogEula_new(BOOL onlyText)
{
	//layout
	GuiItemLayout* layout = GuiItemLayout_new(Quad2i_init4(0, 0, 1, 1));
	GuiItemLayout_addColumn(layout, 0, onlyText ? 1 : 100);
	GuiItemLayout_addColumn(layout, 1, 7);
	GuiItemLayout_addColumn(layout, 3, 7);
	GuiItemLayout_addColumn(layout, 5, 7);
	GuiItemLayout_addColumn(layout, 6, onlyText ? 1 : 100);
	GuiItemLayout_addRow(layout, 0, 100);
	GuiItem_setAttribute((GuiItem*)layout, "onlyText", onlyText);

	GuiItem_addSubName((GuiItem*)layout, "license", GuiItemTextMulti_new(Quad2i_init4(1, 0, 5, 5), DbValue_initLang("EULA_TEXT")));

	if (!onlyText)
	{
		//Laguage
		GuiItem_addSubName((GuiItem*)layout, "language", GuiItemComboStatic_newEx(Quad2i_init4(1, 6, 1, 1), DbValue_initNumber(Lang_getPos()), Lang_find("LANGUAGE_LIST"), DbValue_initLang("LANGUAGE"), &UiDialogEula_clickChooseLanguage));

		GuiItem_addSubName((GuiItem*)layout, "accept", GuiItemButton_newClassicEx(Quad2i_init4(3, 6, 1, 1), DbValue_initLang("ACCEPT"), &UiDialogEula_clickYes));
		GuiItem_addSubName((GuiItem*)layout, "decline", GuiItemButton_newClassicEx(Quad2i_init4(5, 6, 1, 1), DbValue_initLang("DECLINE"), &UiDialogEula_clickNo));
	}

	return layout;
}

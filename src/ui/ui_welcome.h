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

void UiWelcome_clickCreateDialog(GuiItem* item)
{
	GuiItemRoot_addDialogLayout(UiDialogCreate_new());
}

void UiWelcome_clickOpenDialog(GuiItem* item)
{
	GuiItemRoot_addDialogLayout(UiDialogOpen_new(0));
}

void UiWelcome_clickOpenDrop(GuiItem* item)
{
	UNI* pathUNI = OsWinIO_getDropFile(0);
	if (pathUNI)
	{
		if(FileProject_isExist(pathUNI))
		{
			GuiItemLayout* dialog = UiDialogOpen_new(pathUNI);
			GuiItemRoot_addDialogLayout(dialog);

			if (!FileProject_hasPassword(pathUNI))
				UiDialogOpen_clickOpen((GuiItem*)dialog);	//execute
		}
		//clean
		Std_deleteUNI(pathUNI);
		OsWinIO_resetDrop();
	}
}

void UiWelcome_clickChooseLanguage(GuiItem* item)
{
	Lang_setPos(Std_max(0, GuiItemComboStatic_getValue(GuiItem_findName(item, "language"))));
}

GuiItemLayout* UiWelcome_new(void)
{
	GuiItemLayout* layout = GuiItemLayout_new(Quad2i_init4(0, 0, 1, 1));
	GuiItemLayout_addColumn(layout, 0, 100);
	GuiItemLayout_addColumn(layout, 1, 8);
	GuiItemLayout_addColumn(layout, 2, 8);
	GuiItemLayout_addColumn(layout, 3, 100);

	GuiItemLayout_addRow(layout, 0, 4);		//space
	GuiItemLayout_addRow(layout, 1, 3);
	GuiItemLayout_addRow(layout, 2, 100);	//space
	GuiItemLayout_addRow(layout, 3, 2);
	GuiItemLayout_addRow(layout, 4, 2);		//space
	GuiItemLayout_addRow(layout, 5, 2);
	GuiItemLayout_addRow(layout, 6, 100);	//space
	GuiItemLayout_addRow(layout, 7, 1);
	GuiItemLayout_addRow(layout, 8, 2);

	GuiItemLayout_setDrop(layout, &UiWelcome_clickOpenDrop);

	//Logo
	GuiItem_addSubName((GuiItem*)layout, "logo", GuiItemBox_newImage(Quad2i_init4(1, 1, 2, 1), GuiImage_new1(UiLogo_init())));

	//Create
	GuiItem_addSubName((GuiItem*)layout, "create", GuiItemButton_newClassicEx(Quad2i_init4(1, 3, 2, 1), DbValue_initLang("NEW"), &UiWelcome_clickCreateDialog));

	//Open
	GuiItem_addSubName((GuiItem*)layout, "open", GuiItemButton_newClassicEx(Quad2i_init4(1, 5, 2, 1), DbValue_initLang("OPEN"), UiWelcome_clickOpenDialog));

	//Version
	GuiItem_addSubName((GuiItem*)layout, "version", GuiItemText_new(Quad2i_init4(1, 7, 1, 1), TRUE, DbValue_initStatic(UiScreen_getNameVersionUNI()), DbValue_initEmpty()));

	//Laguage
	GuiItem_addSubName((GuiItem*)layout, "language", GuiItemComboStatic_newEx(Quad2i_init4(2, 7, 1, 1), DbValue_initNumber(Lang_getPos()), Lang_find("LANGUAGE_LIST"), DbValue_initLang("LANGUAGE"), &UiWelcome_clickChooseLanguage));

	GuiItem_setShortcutKey((GuiItem*)layout, FALSE, Win_EXTRAKEY_CTRL | Win_EXTRAKEY_NEW, 0, &UiWelcome_clickCreateDialog);

	return layout;
}

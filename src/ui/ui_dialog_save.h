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

typedef enum
{
	UiDialogSave_NONE,
	UiDialogSave_NEW,
	UiDialogSave_OPEN,
	UiDialogSave_CLOSE,
}UiDialogSaveTYPE;

void UiDialogSave_clickNotSave(GuiItem* item)
{
	GuiItem_closeParentLevel(item);	//hide

	UiDialogSaveTYPE type = GuiItem_findAttribute(item, "type");
	switch (type)
	{
		case UiDialogSave_NONE:
		break;
		case UiDialogSave_NEW:
		GuiItemRoot_addDialogLayout(UiDialogCreate_new());
		break;
		case UiDialogSave_OPEN:
		GuiItemRoot_addDialogLayout(UiDialogOpen_new(0));
		break;
		case UiDialogSave_CLOSE:
		UiScreen_closeHard();
		break;
	}
}

void UiDialogSave_clickSave(GuiItem* item)
{
	DbRoot_save();
	UiDialogSave_clickNotSave(item);
}

GuiItemLayout* UiDialogSave_new(UiDialogSaveTYPE type)
{
	//layout
	GuiItemLayout* layout = GuiItemLayout_new(Quad2i_init4(0, 0, 1, 1));
	GuiItemLayout_addColumn(layout, 0, 5);
	GuiItemLayout_addColumn(layout, 2, 5);
	GuiItem_setAttribute((GuiItem*)layout, "type", type);

	//Buttons
	GuiItem_addSubName((GuiItem*)layout, "not_save", GuiItemButton_newClassicEx(Quad2i_init4(0, 0, 1, 1), DbValue_initLang("NOT_SAVE"), &UiDialogSave_clickNotSave));

	GuiItem* b = GuiItem_addSubName((GuiItem*)layout, "save", GuiItemButton_newClassicEx(Quad2i_init4(2, 0, 3, 1), DbValue_initLang("SAVE"), &UiDialogSave_clickSave));
	GuiItem_setShortcutKey(b, FALSE, Win_EXTRAKEY_ENTER, 0, &UiDialogSave_clickSave);

	return GuiItemRoot_createDialogLayout(Vec2i_init2(10, 1), DbValue_initLang("SAVE"), (GuiItem*)layout, 0);
}

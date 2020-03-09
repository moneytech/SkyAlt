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

void UiDialogLog_clickRefresh(GuiItem* item)
{
	GuiItemLayout* list = GuiItem_findName(item, "list");
	if (list)
	{
		GuiItem_freeSubs((GuiItem*)list);

		UBIG i;
		for (i = 0; i < Logs_num(); i++)
		{
			GuiItem_addSubName((GuiItem*)list, "type", GuiItemText_new(Quad2i_init4(0, i, 1, 1), FALSE, DbValue_initLang(Logs_type(i)), DbValue_initEmpty()));
			GuiItem_addSubName((GuiItem*)list, "date", GuiItemText_new(Quad2i_init4(1, i, 1, 1), FALSE, DbValue_initStatic(Logs_dateStr(i)), DbValue_initEmpty()));
			GuiItem_addSubName((GuiItem*)list, "string", GuiItemText_new(Quad2i_init4(2, i, 1, 1), FALSE, DbValue_initLang(Logs_id(i)), DbValue_initEmpty()));
		}
	}
}

void UiDialogLog_clickClear(GuiItem* item)
{
	Logs_clear();
	UiDialogLog_clickRefresh(item);
}

GuiItemLayout* UiDialogLog_new(void)
{
	GuiItemLayout* layout = GuiItemLayout_new(Quad2i_init4(0, 0, 1, 1));
	GuiItemLayout_addColumn(layout, 0, 20);
	GuiItemLayout_addColumn(layout, 1, 20);
	GuiItemLayout_addRow(layout, 1, 10);

	//Skin for List
	GuiItemLayout* list = (GuiItemLayout*)GuiItem_addSubName((GuiItem*)layout, "list", (GuiItem*)GuiItemLayout_new(Quad2i_init4(0, 0, 2, 5)));
	GuiItemLayout_addColumn(list, 0, 3);
	GuiItemLayout_addColumn(list, 1, 5);
	GuiItemLayout_addColumn(list, 2, 20);
	UiDialogLog_clickRefresh((GuiItem*)list);	//fill it

	GuiItem_addSubName((GuiItem*)layout, "refresh", GuiItemButton_newClassicEx(Quad2i_init4(0, 6, 1, 1), DbValue_initLang("REFRESH"), &UiDialogLog_clickRefresh));
	GuiItem_addSubName((GuiItem*)layout, "clear", GuiItemButton_newClassicEx(Quad2i_init4(1, 6, 1, 1), DbValue_initLang("CLEAR"), &UiDialogLog_clickClear));

	return GuiItemRoot_createDialogLayout(Vec2i_init2(30, 10), DbValue_initLang("LOGS"), (GuiItem*)layout, 0);
}

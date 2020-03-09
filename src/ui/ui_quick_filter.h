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

void UiRootQuickFilter_clickSelection(GuiItem* item)
{
	GuiItemRoot_resizeAll();
}

void UiRootQuickFilter_clickSelectionPosAll(GuiItem* item)
{
	BIG row = GuiItem_findAttribute(item, "row");
	DbValue_setOptionNumber(row, "select", -1);
}
void UiRootQuickFilter_clickSelectionPos(GuiItem* item)
{
	BIG row = GuiItem_findAttribute(item, "row");

	GuiItemList* list = GuiItem_findParentTypeLIST(item);
	if (list)
	{
		BIG pos = GuiItemList_getClickPos(list, GuiItem_getRow(item));

		DbValue_setOptionNumber(row, "select", pos);
	}
}

BOOL UiRootQuickFilter_isShowPanel(BIG row)
{
	BIG groupRow = DbRows_findOrCreateSubType(row, "group");
	return DbRows_isEnable(groupRow);
}
void UiRootQuickFilter_setShowPanel(BIG row, BOOL show)
{
	BIG groupRow = DbRows_findOrCreateSubType(row, "group");
	DbRows_setEnable(groupRow, show);
}
void UiRootQuickFilter_clickShowPanel(GuiItem* item)
{
	BIG row = GuiItem_findAttribute(item, "row");
	UiRootQuickFilter_setShowPanel(row, !UiRootQuickFilter_isShowPanel(row));
}

static double UiRootQuickFilter_getWidth(UBIG row)
{
	BIG selectionRow = DbRows_findOrCreateSubType(row, "group");
	return DbValue_getOptionNumber(selectionRow, "width", 10);
}

GuiItem* UiRootQuickFilter_buildButton(Quad2i grid, UBIG row)
{
	BIG groupRow = DbRows_findOrCreateSubType(row, "group");
	BOOL showPanel = DbRows_isEnable(groupRow);

	GuiItemButton* button = (GuiItemButton*)GuiItemButton_newAlphaEx(grid, DbValue_initLang("QUICK_FILTER"), &UiRootQuickFilter_clickShowPanel);
	GuiItemButton_setPressed(button, showPanel);
	GuiItem_setAttribute((GuiItem*)button, "row", row);

	return (GuiItem*)button;
}

GuiItemLayout* UiRootQuickFilter_buildPanel(Quad2i grid, UBIG row, DbRows* filter, DbValue scroll)
{
	GuiItemLayout* layout = 0;

	DbTable* table = DbRoot_findParentTable(row);

	BIG groupRow = DbRows_findOrCreateSubType(row, "group");
	BOOL showPanel = DbRows_isEnable(groupRow);
	BIG groupColumnRow = DbRoot_subs_row(groupRow);
	DbColumn* groupColumn = DbRoot_ref_column(groupColumnRow);

	//selection panel
	if (showPanel)
	{
		layout = GuiItemLayout_new(grid);
		//GuiItemLayout_setScrollV(layout, scroll);
		GuiItemLayout_setDrawBackground(layout, FALSE);
		GuiItemLayout_setDrawBorder(layout, TRUE);
		GuiItemLayout_addColumn(layout, 0, 99);
		GuiItemLayout_addRow(layout, 2, 99);
		GuiItem_setChangeSize((GuiItem*)layout, TRUE, DbValue_initOption(groupRow, "width", 0), TRUE, TRUE, GuiItemTheme_getWhite());
		GuiItem_setAttribute((GuiItem*)layout, "row", row);

		//column
		GuiItem_addSubName((GuiItem*)layout, "selection", GuiItemComboDynamic_newEx(Quad2i_init4(0, 0, 1, 1), FALSE, DbRows_initLink1(DbRoot_ref(), groupColumnRow), DbValue_initOption(-1, "name", 0), DbRows_initSubs(table, "columns", FALSE), DbValue_initEmpty(), &UiRootQuickFilter_clickSelection));

		//close
		GuiItem_addSubName((GuiItem*)layout, "X", GuiItemButton_newAlphaEx(Quad2i_init4(1, 0, 1, 1), DbValue_initStaticCopyCHAR("X"), &UiRootQuickFilter_clickShowPanel));

		if (groupColumn)
		{
			BIG selectPos = DbValue_getOptionNumber(row, "select", -1);

			//select "all"
			GuiItemText* all = (GuiItemText*)GuiItem_addSubName((GuiItem*)layout, "selection_all", GuiItemText_new(Quad2i_init4(0, 1, 2, 1), FALSE, DbValue_initLang("ALL"), DbValue_initEmpty()));
			GuiItemText_setPressed(all, selectPos < 0);
			GuiItem_setCallClick((GuiItem*)all, &UiRootQuickFilter_clickSelectionPosAll);

			DbRows_hasChanged(filter);	//execute

			if (DbRows_getSize(filter) > 0)
			{
				BIG selectRow = 0;
				GuiItem_addSubName((GuiItem*)layout, "list", (GuiItem*)GuiItemGroup_getLaneList(Quad2i_init4(0, 2, 2, 1), groupColumn, filter->filter, &selectRow, selectPos, scroll, &UiRootQuickFilter_clickSelectionPos, 0));

				if (selectRow >= 0 && selectPos >= 0)
				{
					//StdBigs mapRows = DbColumnN_copyActiveBigs(DbFilter_getColumnGroupRows(filter->filter), selectRow);
					StdBigs mapRows = StdBigs_init();
					DbColumnN_getArrayPoses(DbFilter_getColumnGroupRows(filter->filter), selectRow, &mapRows);

					DbRows finalFilter = DbRows_initArray(table, mapRows);

					DbRows_free(filter);
					*filter = finalFilter;
				}
			}
		}
	}

	return layout;
}

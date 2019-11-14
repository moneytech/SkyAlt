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

typedef struct GuiItemGroup_s
{
	GuiItem base;
	BIG viewRow;
	DbRows filter;

	DbValue scrollH;
}GuiItemGroup;

GuiItem* GuiItemGroup_new(Quad2i grid, UBIG viewRow, DbRows filter, DbValue scrollH)
{
	GuiItemGroup* self = Os_malloc(sizeof(GuiItemGroup));
	self->base = GuiItem_init(GuiItem_GROUP, grid);

	self->viewRow = viewRow;
	self->filter = filter;
	self->scrollH = scrollH;

	return (GuiItem*)self;
}

GuiItem* GuiItemGroup_newCopy(GuiItemGroup* src, BOOL copySub)
{
	GuiItemGroup* self = Os_malloc(sizeof(GuiItemGroup));
	*self = *src;

	GuiItem_initCopy(&self->base, &src->base, copySub);
	self->filter = DbRows_initCopy(&src->filter);
	self->scrollH = DbValue_initCopy(&src->scrollH);

	return (GuiItem*)self;
}

void GuiItemGroup_delete(GuiItemGroup* self)
{
	DbValue_free(&self->scrollH);
	DbRows_free(&self->filter);
	GuiItem_free(&self->base);
	Os_free(self, sizeof(GuiItemGroup));
}

BIG GuiItemGroup_getBaseRow(GuiItemGroup* self)
{
	return DbRows_getBaseRow(&self->filter);
}

void GuiItemGroup_setBaseRow(GuiItemGroup* self, BIG row)
{
	DbRows_setBaseRow(&self->filter, row);
}

UBIG GuiItemGroup_numRows(GuiItemGroup* self)
{
	return DbRows_getSize(&self->filter);
}
DbTable* GuiItemGroup_getTable(const GuiItemGroup* self)
{
	return DbRoot_findParentTable(self->viewRow);
}
UBIG GuiItemGroup_numLanes(const GuiItemGroup* self)
{
	return DbRows_getSubsNum(self->viewRow, "group", FALSE);
}
DbColumn* GuiItemGroup_getLane(const GuiItemGroup* self, UBIG i)
{
	return DbRows_getSubsColumn(self->viewRow, "group", FALSE, i);
}
BIG GuiItemGroup_getLaneRow(const GuiItemGroup* self, UBIG i)
{
	return DbRows_getSubsRow(self->viewRow, "group", FALSE, i);
}

UINT GuiItemGroup_getLaneWidth(const GuiItemGroup* self, UBIG c)
{
	DbValue v = DbRows_getSubsOption(self->viewRow, "group", FALSE, c, "width", FALSE);
	int width = DbValue_getNumber(&v);
	DbValue_free(&v);
	return Std_bmax(1, width);
}

BOOL GuiItemGroup_getLaneAscending(const GuiItemGroup* self, UBIG c)
{
	DbValue v = DbRows_getSubsOption(self->viewRow, "group", FALSE, c, "ascending", FALSE);
	BOOL asc = DbValue_getNumber(&v);
	DbValue_free(&v);
	return asc;
}

UINT GuiItemGroup_getLaneSelect(const GuiItemGroup* self, UBIG c)
{
	DbValue v = DbRows_getSubsOption(self->viewRow, "group", FALSE, c, "select", FALSE);
	int select = DbValue_getNumber(&v);
	DbValue_free(&v);
	return Std_bmax(0, select);
}

void GuiItemGroup_setLaneSelect(GuiItemGroup* self, int c, UINT pos)
{
	DbValue v = DbRows_getSubsOption(self->viewRow, "group", FALSE, c, "select", FALSE);
	DbValue_setNumber(&v, pos);
	DbValue_free(&v);
}

void GuiItemGroup_draw(GuiItemGroup* self, Image4* img, Quad2i coord, Win* win)
{
}
void GuiItemGroup_update(GuiItemGroup* self, Quad2i coord, Win* win)
{
	if (DbRows_hasChanged(&self->filter))
		GuiItem_setResize(&self->base, TRUE);
}
void GuiItemGroup_touch(GuiItemGroup* self, Quad2i coord, Win* win)
{
}

void GuiItemGroup_clickRebuild(GuiItem* self)
{
	GuiItemGroup* group = GuiItem_findParentType(self, GuiItem_GROUP);
	if (group)
		GuiItem_setResize(&group->base, TRUE);
}

void GuiItemGroup_clickAddColumn(GuiItem* self)
{
	GuiItemGroup* group = GuiItem_findParentType(self, GuiItem_GROUP);
	if (group)
	{
		DbRows_getAddSubsLine(group->viewRow, "group");
		GuiItem_setResize(&group->base, TRUE);
	}
}

void GuiItemGroup_clickColumnRemove(GuiItem* self)
{
	GuiItemGroup* group = GuiItem_findParentType(self, GuiItem_GROUP);
	if (group && self->type == GuiItem_BUTTON)
	{
		BIG c = GuiItem_findAttribute(self, "c");
		BIG row = GuiItemGroup_getLaneRow(group, c);
		DbRoot_removeRow(row);
	}
}

void GuiItemGroup_clickShowRecords(GuiItem* self)
{
	GuiItemGroup* group = GuiItem_findParentType(self, GuiItem_GROUP);
	if (group)
	{
		BIG guiRow = DbTable_getRow(GuiItemGroup_getTable(group));
		BIG srcRow = GuiItem_getRow(self);

		//layout
		GuiItemLayout* layout = GuiItemLayout_new(Quad2i_init4(0, 0, 1, 1));
		GuiItemLayout_addColumn(layout, 0, 100);
		GuiItemLayout_addRow(layout, 0, 100);

		//Table
		GuiItem_addSubName((GuiItem*)layout, "table", (GuiItem*)GuiItemTable_new(Quad2i_init4(0, 0, 1, 1), guiRow, DbRows_initLink(DbFilter_getColumnGroupRows(group->filter.filter), srcRow), TRUE, FALSE, DbValue_initEmpty(), DbValue_initEmpty(), DbValue_initStaticCopyCHAR("0 0 0 0")));

		GuiItemRoot_addDialogLayout(GuiItemRoot_createDialogLayout(Vec2i_init2(50, 20), DbValue_initLang("GROUP"), (GuiItem*)layout, 0));
	}
}

void GuiItemGroup_clickSelectRow(GuiItem* self)
{
	GuiItemGroup* group = GuiItem_findParentType(self, GuiItem_GROUP);
	GuiItemList* list = GuiItem_findParentType(self, GuiItem_LIST);
	if (group && list)
	{
		int c = ((GuiItemLayout*)self)->extra_big;
		BIG pos = Std_max(0, DbRows_findLinkPos(&list->filter, GuiItem_getRow(self)));

		//save
		GuiItemGroup_setLaneSelect(group, c, pos);

		//reset columns on RIGHT to 0
		const int N_COLS = GuiItemGroup_numLanes(group);
		for (c = c + 1; c < N_COLS; c++)
			GuiItemGroup_setLaneSelect(group, c, 0);

		GuiItem_setResize(&group->base, TRUE);
	}
}

GuiItemLayout* GuiItemGroup_resize(GuiItemGroup* self, GuiItemLayout* layout, Win* win)
{
	if (!self->base.resize)
		return (GuiItemLayout*)GuiItem_getSub(&self->base, 0);

	GuiItem_freeSubs(&self->base);

	//layout
	layout = GuiItemLayout_newCoord(&self->base, TRUE, TRUE, win);
	GuiItemLayout_setDrawBackground(layout, FALSE);
	GuiItem_addSubName(&self->base, "layout_main", &layout->base);
	GuiItemLayout_addRow(layout, 1, 99);
	GuiItemLayout_setScrollH(layout, DbValue_initCopy(&self->scrollH));

	//cells will not be under scroll
	layout->base.coordScreen.size.x -= GuiScroll_widthWin(win);	//vertical

	int c;
	DbTable* table = GuiItemGroup_getTable(self);
	const int N_COLS = GuiItemGroup_numLanes(self);

	//update
	DbRows_hasChanged(&self->filter);

	BIG selectRow = table ? (GuiItemGroup_numRows(self) > 0 ? 0 : -1) : -1;
	int width_sum = 0;
	for (c = 0; c < N_COLS; c++)
	{
		BIG crow = GuiItemGroup_getLaneRow(self, c);
		DbColumn* column = GuiItemGroup_getLane(self, c);

		char nameId[64];
		Std_buildNumber(crow, 0, nameId);

		const int width = GuiItemGroup_getLaneWidth(self, c);
		const int selectPos = GuiItemGroup_getLaneSelect(self, c);

		GuiItemLayout* layColumn = GuiItemLayout_new(Quad2i_init4(width_sum, 0, width, 2));
		layColumn->drawBorder = TRUE;
		GuiItemLayout_setDrawBackground(layColumn, FALSE);
		GuiItemLayout_addColumn(layColumn, 0, 99);
		GuiItemLayout_addRow(layColumn, 1, 99);
		GuiItem_addSubName((GuiItem*)layout, nameId, (GuiItem*)layColumn);

		GuiItem_setAttribute((GuiItem*)layColumn, "c", c);

		//header
		{
			GuiItemLayout* layHeader = GuiItemLayout_new(Quad2i_init4(0, 0, 1, 1));
			GuiItemLayout_setBackgroundWhite(layHeader, TRUE);
			GuiItemLayout_addColumn(layHeader, 0, 99);
			GuiItemLayout_addColumn(layHeader, 1, 99);
			GuiItem_addSubName((GuiItem*)layColumn, "layout_header", (GuiItem*)layHeader);

			GuiItem_setChangeSize(&layHeader->base, TRUE, DbValue_initOption(crow, "width", 0), FALSE);
			GuiItem_setDrop(&layHeader->base, "group", "group", TRUE, DbRows_getSubsArray(self->viewRow, "group"), crow);

			GuiItem_setIcon((GuiItem*)layHeader, GuiImage_new1(GuiItem_getColumnIcon(column ? DbColumnFormat_findColumn(column) : -1)));

			GuiItem_addSubName((GuiItem*)layHeader, "column", GuiItemComboDynamic_new(Quad2i_init4(0, 0, 1, 1), FALSE, DbRows_initLink(DbRoot_getColumnSubs(), crow), DbValue_initOption(-1, "name", 0), DbRows_initSubs(table, "columns", FALSE), DbValue_initEmpty()));

			GuiItem_addSubName((GuiItem*)layHeader, "ascending", GuiItemComboStatic_new(Quad2i_init4(1, 0, 1, 1), DbValue_initOption(crow, "ascending", 0), Lang_find("LANGUAGE_ORDER"), DbValue_initEmpty()));

			GuiItem_addSubName((GuiItem*)layHeader, "x", GuiItemButton_newClassicEx(Quad2i_init4(2, 0, 1, 1), DbValue_initStaticCopyCHAR("X"), &GuiItemGroup_clickColumnRemove));
		}

		if (column)
		{
			//list
			DbRows ids = DbRows_initLink(DbFilter_getColumnGroupSubs(self->filter.filter), selectRow);
			selectRow = DbRows_getRow(&ids, selectPos);	//get next select row

			//skin
			GuiItemLayout* skin = GuiItemLayout_new(Quad2i_init4(0, 0, 1, 1));
			GuiItemLayout_setDrawBackground(skin, FALSE);
			GuiItemLayout_addColumn(skin, 0, 99);
			GuiItemLayout_addColumn(skin, 1, 2);
			GuiItemLayout_addColumn(skin, 2, 2);

			skin->extra_big = c;
			skin->highlightRow = selectRow;	//last doesn't have highlight
			skin->enableClickLayout = TRUE;
			GuiItem_setCallClick(&skin->base, &GuiItemGroup_clickSelectRow);

			//value
			{
				GuiItemLayout* skin2 = GuiItemLayout_new(Quad2i_init4(0, 0, 1, 1));
				GuiItemLayout_addColumn(skin2, 0, 99);
				skin2->drawBackground = FALSE;
				GuiItemText* name = (GuiItemText*)GuiItem_addSubName((GuiItem*)skin2, "value", GuiItemText_new(Quad2i_init4(0, 0, 1, 1), FALSE, DbValue_initGET(GuiItemGroup_getLane(self, c), -1), DbValue_initEmpty()));
				name->drawBackground = FALSE;

				DbRows ids2 = DbRows_initLink(DbFilter_getColumnGroupRows(self->filter.filter), -1);
				GuiItemList* list2 = (GuiItemList*)GuiItem_addSubName((GuiItem*)skin, "value_list", GuiItemList_new(Quad2i_init4(0, 0, 1, 1), ids2, (GuiItem*)skin2, DbValue_initEmpty()));
				list2->showScroll = FALSE;
				GuiItemList_setShowBorder(list2, FALSE);
			}

			//count
			{
				DbValue cv = DbValue_initGET((DbColumn*)DbFilter_getColumnGroupCount(self->filter.filter), -1);
				cv.staticPost = Std_newUNI_char("x");
				GuiItem_addSubName((GuiItem*)skin, "count", GuiItemText_new(Quad2i_init4(1, 0, 1, 1), FALSE, cv, DbValue_initEmpty()));
			}

			//show
			GuiItem_addSubName((GuiItem*)skin, "show", GuiItemButton_newClassicEx(Quad2i_init4(2, 0, 1, 1), DbValue_initLang("SHOW"), &GuiItemGroup_clickShowRecords));

			//list
			GuiItemList* list = (GuiItemList*)GuiItem_addSubName((GuiItem*)layColumn, "list", GuiItemList_new(Quad2i_init4(0, 1, 1, 1), ids, (GuiItem*)skin, DbValue_initEmpty()));
			GuiItemList_setShowBorder(list, FALSE);
		}

		width_sum += width;
		width_sum++;	//add space beween groups
	}

	//add new column
	GuiItem_addSubName((GuiItem*)layout, "+", GuiItemButton_newClassicEx(Quad2i_init4(width_sum, 0, 2, 1), DbValue_initStaticCopyCHAR("+"), &GuiItemGroup_clickAddColumn));

	return layout;
}
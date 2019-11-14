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

typedef struct GuiItemTable_s
{
	GuiItem base;
	BIG viewRow;
	DbRows filter;

	BIG oldNumRows;

	UBIG num_rows_max;		//visible
	UBIG num_rows_real;
	UBIG subs_start_rowOptionOrder;
	UBIG subs_start_rowOptionCards;
	UBIG subs_start_rowOption;
	UBIG subs_end_rowCell;
	UBIG subs_start_add;

	GuiScroll scroll;

	BIG selectedRow;

	BOOL showChangeOrder;
	BOOL showHeader;
	BOOL showAddRecord;

	BOOL drawBorder;

	BOOL selectActive;
	Vec2i selectFirstTouch;
	Vec2i selectLastTouch;
	DbValue selectGrid;	//[firstX firstY, lastX, lastY]

	Quad2i cellExtendRect;
	BOOL cellExtendActive;

	Rgba cdSelect;
	UBIG subs_start_rowCell;

	DbValue rowSize;

	DbRows group;

	DbColumn* choosenSourceColumn;

	BOOL showAddButton;
	BOOL showRemoveButton;

	char* changeOrderNameDst;
	char* changeOrderNameSrc;

	DbValue scrollH;
}GuiItemTable;

DbTable* GuiItemTable_getTable(const GuiItemTable* self)
{
	return DbRoot_findParentTable(self->viewRow);
}

UBIG GuiItemTable_numColumns(const GuiItemTable* self)
{
	return DbRows_getSubsNum(self->viewRow, "columns", TRUE);
}
DbColumn* GuiItemTable_getColumn(const GuiItemTable* self, UBIG i)
{
	return DbRows_getSubsColumn(self->viewRow, "columns", TRUE, i);
}
BIG GuiItemTable_getColumnRow(const GuiItemTable* self, UBIG i)
{
	return DbRows_getSubsRow(self->viewRow, "columns", TRUE, i);
}

UBIG GuiItemTable_numRows(GuiItemTable* self)
{
	return DbRows_getSize(&self->filter);
}

BOOL GuiItemTable_repairSelect(GuiItemTable* self)
{
	const BIG NUM_COLS = GuiItemTable_numColumns(self);
	const BIG NUM_ROWS = GuiItemTable_numRows(self);

	BOOL ok = (NUM_COLS > 0 && NUM_ROWS > 0);

	self->selectFirstTouch.x = Std_clamp(self->selectFirstTouch.x, 0, NUM_COLS - 1);
	self->selectLastTouch.x = Std_clamp(self->selectLastTouch.x, 0, NUM_COLS - 1);

	self->selectFirstTouch.y = Std_clamp(self->selectFirstTouch.y, 0, NUM_ROWS - 1);
	self->selectLastTouch.y = Std_clamp(self->selectLastTouch.y, 0, NUM_ROWS - 1);

	return ok;
}

GuiItemTable* GuiItemTable_new(Quad2i grid, BIG tableRow, DbRows filter, BOOL showHeader, BOOL showAddRecord, DbValue scrollV, DbValue scrollH, DbValue selectGrid)
{
	GuiItemTable* self = Os_malloc(sizeof(GuiItemTable));
	self->base = GuiItem_init(GuiItem_TABLE, grid);

	self->viewRow = tableRow;
	self->filter = filter;

	self->oldNumRows = -1;

	self->num_rows_max = 0;
	self->num_rows_real = 0;

	self->subs_start_rowOptionOrder = 0;
	self->subs_start_rowOptionCards = 0;
	self->subs_start_rowOption = 0;
	self->subs_start_rowCell = 0;
	self->subs_start_add = 0;

	self->scroll = GuiScroll_init(scrollV);
	self->scrollH = scrollH;

	self->showChangeOrder = FALSE;
	self->showHeader = showHeader;
	self->showAddRecord = showAddRecord;

	self->drawBorder = FALSE;

	self->selectedRow = -1;
	self->selectActive = FALSE;

	self->cellExtendRect = Quad2i_init();
	self->cellExtendActive = FALSE;

	self->cdSelect = Rgba_initWhite();

	self->subs_start_rowCell = 0;

	self->rowSize = DbValue_initOption(tableRow, "height", 0);

	self->choosenSourceColumn = 0;

	self->group = DbRows_initEmpty();

	self->showAddButton = FALSE;
	self->showRemoveButton = FALSE;

	self->changeOrderNameDst = 0;
	self->changeOrderNameSrc = 0;

	self->selectGrid = selectGrid;
	DbValue_hasChanged(&self->selectGrid);

	char gridC[64];
	Std_copyCHAR_uni(gridC, 64, self->selectGrid.result.str);
	gridC[63] = 0;

	self->selectFirstTouch = Vec2i_init2(0, 0);
	self->selectLastTouch = Vec2i_init2(0, 0);
	sscanf(gridC, "%d %d %d %d", &self->selectFirstTouch.x, &self->selectFirstTouch.y, &self->selectLastTouch.x, &self->selectLastTouch.y);
	GuiItemTable_repairSelect(self);

	return self;
}

GuiItem* GuiItemTable_newCopy(GuiItemTable* src, BOOL copySub)
{
	GuiItemTable* self = Os_malloc(sizeof(GuiItemTable));
	*self = *src;
	GuiItem_initCopy(&self->base, &src->base, copySub);

	self->filter = DbRows_initCopy(&src->filter);
	self->group = DbRows_initCopy(&src->group);

	self->oldNumRows = -1;
	self->scroll = GuiScroll_initCopy(&src->scroll);
	self->scrollH = DbValue_initCopy(&src->scrollH);

	self->selectGrid = DbValue_initCopy(&src->selectGrid);

	self->changeOrderNameDst = Std_newCHAR(src->changeOrderNameDst);
	self->changeOrderNameSrc = Std_newCHAR(src->changeOrderNameSrc);

	return (GuiItem*)self;
}

BOOL GuiItemTable_isColumnLocked(const GuiItemTable* self, UBIG c)
{
	DbValue v = DbRows_getSubsOption(self->viewRow, "columns", TRUE, c, "lock", FALSE);
	BOOL locked = DbValue_getNumber(&v);
	DbValue_free(&v);
	return locked;
}

int GuiItemTable_getColumnIdsWidth(const GuiItemTable* self)
{
	DbValue v = DbValue_initOption(self->viewRow, "width_ids", _UNI32("3"));
	int width = DbValue_getNumber(&v);
	DbValue_free(&v);
	return Std_bmax(1, width);
}

int GuiItemTable_getColumnWidth(const GuiItemTable* self, UBIG c)
{
	DbValue v = DbRows_getSubsOption(self->viewRow, "columns", TRUE, c, "width", FALSE);
	int width = DbValue_getNumber(&v);
	DbValue_free(&v);
	return Std_bmax(1, width);
}

DbFormatTYPE GuiItemTable_getColumnType(GuiItemTable* self, UBIG c)
{
	return DbColumnFormat_findColumn(GuiItemTable_getColumn(self, c));
}

void GuiItemTable_delete(GuiItemTable* self)
{
	DbRows_free(&self->filter);
	DbRows_free(&self->group);

	Std_deleteCHAR(self->changeOrderNameDst);
	Std_deleteCHAR(self->changeOrderNameSrc);

	DbValue_free(&self->scrollH);
	GuiScroll_free(&self->scroll);

	GuiItem_free(&self->base);
	Os_free(self, sizeof(GuiItemTable));
}

void GuiItemTable_disableScrollSend(GuiItemTable* self)
{
	self->scroll.sendScrollDown = FALSE;
}

DbColumn* GuiItemTable_getBaseColumn(const GuiItemTable* self)
{
	return self->filter.column;
}

BIG GuiItemTable_getBaseRow(GuiItemTable* self)
{
	return DbRows_getBaseRow(&self->filter);
}

void GuiItemTable_setBaseRow(GuiItemTable* self, BIG row)
{
	DbRows_setBaseRow(&self->filter, row);
}

int GuiItemTable_getRowSize(GuiItemTable* self)
{
	int rs = DbValue_getNumber(&self->rowSize);
	rs += 1;

	if (rs >= 2)
		rs *= 2;
	return rs;
}

Quad2i GuiItemTable_getSelectRect(const GuiItemTable* self)
{
	Quad2i q;
	q.start = self->selectFirstTouch;
	q.size = Vec2i_sub(self->selectLastTouch, self->selectFirstTouch);

	q = Quad2i_repair(q, 0);
	q.size.x++;
	q.size.y++;

	return q;
}

static int GuiItemTable_scrollExtra(const GuiItemTable* self, Win* win)
{
	return self->showHeader * OsWinIO_cellSize();
}

void GuiItemTable_clickNewColumnLinkSET_1(GuiItem* self)
{
	GuiItemTable* table = GuiItem_findParentType(self->parent, GuiItem_TABLE);
	if (table && self->type == GuiItem_BUTTON)
	{
		DbTable* btable = DbRoot_findParentTable(((GuiItemButton*)self)->text.row);
		if (btable)
			DbTable_createColumnFormat(GuiItemTable_getTable(table), DbFormat_LINK_1, 0, btable);
	}

	GuiItem_closeParentLevel(self);
}

void GuiItemTable_clickNewColumnLinkSET_N(GuiItem* self)
{
	GuiItemTable* table = GuiItem_findParentType(self->parent, GuiItem_TABLE);
	if (table && self->type == GuiItem_BUTTON)
	{
		DbTable* btable = DbRoot_findParentTable(((GuiItemButton*)self)->text.row);
		if (btable)
			DbTable_createColumnFormat(GuiItemTable_getTable(table), DbFormat_LINK_N, 0, btable);
	}

	GuiItem_closeParentLevel(self);
}

void GuiItemTable_clickAddColumn(GuiItem* self)
{
	BOOL close = TRUE;

	GuiItemTable* table = GuiItem_findParentType(self->parent, GuiItem_TABLE);
	if (table)
	{
		DbTable* tab = GuiItemTable_getTable(table);
		DbFormatTYPE format = GuiItem_findAttribute(self, "type");

		if (format == DbFormat_LINK_1 || format == DbFormat_LINK_N)
		{
			GuiItemLayout* layout = GuiStruct_create(GuiStruct_SHOW_TABLES, DbTable_getRow(tab), DbValue_initLang("TABLES"), (format == DbFormat_LINK_1 ? &GuiItemTable_clickNewColumnLinkSET_1 : &GuiItemTable_clickNewColumnLinkSET_N), -1, -1, -1);
			GuiItemRoot_addDialogRelLayout(layout, self, self->coordMove, TRUE);
			close = FALSE;
		}
		else
		{
			DbTable_createColumnFormat(tab, format, 0, 0);
		}
	}

	if (close)
		GuiItem_closeParentLevel(self);
}

BIG _GuiItemTable_addOptionLine(BIG parentRow)
{
	DbRows rows = DbRows_initLink(DbRoot_getColumnSubs(), parentRow);
	BIG row = DbRows_addNewRow(&rows);
	DbRows_free(&rows);
	return row;
}
void GuiItemTable_clickAddOptionLine(GuiItem* self)
{
	GuiItemTable* table = GuiItem_findParentType(self->parent, GuiItem_TABLE);
	if (table)
	{
		BIG c = GuiItem_findAttribute(self, "c");
		BIG crowT = DbColumn_getRow(GuiItemTable_getColumn(table, c));

		BIG optionsRow = DbRows_findSubType(crowT, "options");

		_GuiItemTable_addOptionLine(optionsRow);
	}
}

void GuiItemTable_clickRebuild(GuiItem* self)
{
	GuiItemTable* table = GuiItem_findParentType(self, GuiItem_TABLE);
	if (table)
		GuiItem_setResize(&table->base, TRUE);
}

void GuiItemTable_clickMoveLink(GuiItem* self)
{
	GuiItemTable* table = GuiItem_findParentType(self->parent, GuiItem_TABLE);
	if (table && self->type == GuiItem_BUTTON)
	{
		BIG c = GuiItem_findAttribute(self, "c");
		DbColumn* columnSrc = GuiItemTable_getColumn(table, c);
		if (columnSrc)
			DbColumn_moveToTable(columnSrc);

		GuiItem_closeParentLevel(self);
	}
}

void GuiItemTable_clickSendLinksSET(GuiItem* self)
{
	GuiItemTable* table = GuiItem_findParentType(self->parent, GuiItem_TABLE);
	if (table && self->type == GuiItem_BUTTON)
	{
		BIG c = GuiItem_findAttribute(self, "c");
		DbColumn* columnSrc = GuiItemTable_getColumn(table, c);

		BIG dstColumnRow = ((GuiItemButton*)self)->text.row;
		DbColumn* columnDst = DbRoot_findColumn(dstColumnRow);

		if (columnSrc && columnDst)
		{
			DbColumn_addLinksToColumn(columnDst, columnSrc, &table->filter);
		}
	}
}
void GuiItemTable_clickSendLinks(GuiItem* self)
{
	GuiItemTable* table = GuiItem_findParentType(self->parent, GuiItem_TABLE);
	if (table && self->type == GuiItem_BUTTON)
	{
		BIG c = GuiItem_findAttribute(self, "c");
		DbColumn* columnSrc = GuiItemTable_getColumn(table, c);
		if (columnSrc)
		{
			DbTable* btable = DbColumn_getBTable(columnSrc);

			if (btable)
			{
				BIG columnSrcRow = DbColumn_getRow(columnSrc);

				GuiItemLayout* layout = GuiStruct_create(GuiStruct_SHOW_COLUMNS, columnSrcRow, DbValue_initLang("COLUMNS"), &GuiItemTable_clickSendLinksSET, DbTable_getRow(GuiItemTable_getTable(table)), DbTable_getRow(btable), columnSrcRow);
				GuiItemRoot_addDialogRelLayout(layout, self, self->coordMove, TRUE);
			}
		}
	}
}

void GuiItemTable_clickColumnConvertLINK(GuiItem* self)
{
	GuiItemTable* table = GuiItem_findParentType(self->parent, GuiItem_TABLE);
	if (table && self->type == GuiItem_BUTTON)
	{
		BIG c = GuiItem_findAttribute(self, "c");
		BIG co = GuiItem_findAttribute(self, "dstType");

		DbFormatTYPE type = GuiItemTable_getColumnType(table, c);

		BIG columnRow = ((GuiItemButton*)self)->text.row;

		const DbColumnConvert* cc = DbColumnConvert_get(type, co);
		DbColumn* col = DbRoot_findColumn(columnRow);
		if (col && cc)
		{
			DbColumnConvertPrm prm;
			prm.srcColumn = GuiItemTable_getColumn(table, c);
			prm.bTableColumn = col;
			DbColumnConvert_convert(cc, prm);
		}

		GuiItem_closeParentLevel(self);
	}
}

void GuiItemTable_clickColumnConvertDATE(GuiItem* self)
{
	GuiItemTable* table = GuiItem_findParentType(self->parent, GuiItem_TABLE);
	GuiItemComboStatic* timeFormat = (GuiItemComboStatic*)GuiItem_findNameType(self, "timeFormat", GuiItem_COMBO_STATIC);
	if (table && timeFormat && self->type == GuiItem_BUTTON)
	{
		BIG c = GuiItem_findAttribute(self, "c");
		BIG co = GuiItem_findAttribute(self, "dstType");

		DbFormatTYPE type = GuiItemTable_getColumnType(table, c);
		const DbColumnConvert* cc = DbColumnConvert_get(type, co);
		if (cc)
		{
			DbColumnConvertPrm prm;
			prm.srcColumn = GuiItemTable_getColumn(table, c);
			prm.dateType = GuiItemComboStatic_getValue(timeFormat);
			DbColumnConvert_convert(cc, prm);
		}

		GuiItem_closeParentLevel(self);
	}
}

void GuiItemTable_clickColumnConvert(GuiItem* self)
{
	GuiItemTable* table = GuiItem_findParentType(self->parent, GuiItem_TABLE);

	if (table && self->type == GuiItem_COMBO_STATIC)
	{
		BIG c = GuiItem_findAttribute(self, "c");
		DbFormatTYPE type = GuiItemTable_getColumnType(table, c);

		int co = GuiItemComboStatic_getValue((GuiItemComboStatic*)self) - 1;	//-1: first line is current format
		const DbColumnConvert* cc = DbColumnConvert_get(type, co);
		if (cc)
		{
			DbColumn* bTableColumn = 0;
			if (cc->dstType == DbFormat_LINK_1 || cc->dstType == DbFormat_LINK_N)
			{
				if (cc->srcType != DbFormat_LINK_1 && cc->srcType != DbFormat_LINK_N)
				{
					GuiItem_setAttribute(self, "dstType", co);
					GuiItemLayout* layout = GuiStruct_create(GuiStruct_SHOW_COLUMNS, DbColumn_getRow(GuiItemTable_getColumn(table, c)), DbValue_initLang("COLUMNS"), &GuiItemTable_clickColumnConvertLINK, -1, -1, -1);
					GuiItemRoot_addDialogRelLayout(layout, self, self->coordMove, TRUE);
					return;
				}
			}
			else
				if (cc->dstType == DbFormat_DATE)
				{
					GuiItem_setAttribute(self, "dstType", co);

					//create layout
					GuiItemLayout* layout = GuiItemLayout_new(Quad2i_init4(0, 0, 1, 1));
					GuiItemLayout_addColumn(layout, 0, 10);

					GuiItem_addSubName((GuiItem*)layout, "timeFormat", GuiItemComboStatic_newEx(Quad2i_init4(0, 0, 2, 1), DbValue_initNumber(UiIniSettings_getDateFormat()), Lang_find("CALENDAR_FORMAT_DATE"), DbValue_initLang("TIME_FORMAT"), 0));
					GuiItem_addSubName((GuiItem*)layout, "convert", GuiItemButton_newClassicEx(Quad2i_init4(0, 1, 2, 1), DbValue_initLang("CONVERT"), &GuiItemTable_clickColumnConvertDATE));

					GuiItemRoot_addDialogRelLayout(layout, self, self->coordMove, FALSE);
					return;
				}

			DbColumnConvertPrm prm;
			prm.srcColumn = GuiItemTable_getColumn(table, c);
			prm.bTableColumn = bTableColumn;

			DbColumnConvert_convert(cc, prm);
		}

		GuiItem_closeParentLevel(self);
	}
}

void GuiItemTable_clickHideColumn(GuiItem* self)
{
	GuiItemTable* table = GuiItem_findParentType(self->parent, GuiItem_TABLE);
	if (table && self->type == GuiItem_BUTTON)
	{
		BIG c = GuiItem_findAttribute(self, "c");

		DbValue v = DbRows_getSubsOption(table->viewRow, "columns", TRUE, c, "enable", FALSE);
		DbValue_setNumber(&v, !DbValue_getNumber(&v));
		DbValue_free(&v);
	}
}

void GuiItemTable_clickRemoveColumn(GuiItem* self)
{
	GuiItemTable* table = GuiItem_findParentType(self->parent, GuiItem_TABLE);
	if (table && self->type == GuiItem_BUTTON)
	{
		BIG c = GuiItem_findAttribute(self, "c");

		DbColumn* column = GuiItemTable_getColumn(table, c);
		BIG row = DbColumn_getRow(column);

		DbRoot_removeRow(row);
	}
}

void GuiItemTable_clickDuplicateColumn(GuiItem* self)
{
	GuiItemTable* table = GuiItem_findParentType(self->parent, GuiItem_TABLE);
	if (table && self->type == GuiItem_BUTTON)
	{
		BIG c = GuiItem_findAttribute(self, "c");

		DbColumn* column = GuiItemTable_getColumn(table, c);
		BIG row = DbColumn_getRow(column);

		DbRoot_duplicateRow(row, DbRoot_findParent(row));
	}
}

void GuiItemTable_clickAddRecord(GuiItem* self)
{
	GuiItemTable* table = GuiItem_findParentType(self->parent, GuiItem_TABLE);
	if (table && self->type == GuiItem_BUTTON)
	{
		DbRows_addNewRow(&table->filter);
	}
}

void GuiItemTable_clickRemoveRow(GuiItem* self)
{
	GuiItemTable* table = GuiItem_findParentType(self->parent, GuiItem_TABLE);
	if (table && self->type == GuiItem_BUTTON)
	{
		DbRows_removeRow(&table->filter, GuiItem_getRow(self));
	}
}

void GuiItemTable_clickRemoveRows(GuiItem* self)
{
	GuiItemTable* table = GuiItem_findParentType(self->parent, GuiItem_TABLE);
	if (table)
	{
		Quad2i selectRect = GuiItemTable_getSelectRect(table);
		Vec2i st = selectRect.start;
		Vec2i e = Quad2i_end(selectRect);

		BIG y;
		for (y = e.y - 1; y >= st.y; y--)
			DbRows_removeRow(&table->filter, DbRows_getRow(&table->filter, y));
	}
}

void GuiItemTable_clickSelectCalendar(GuiItem* self)
{
	if (self->type == GuiItem_BUTTON)
	{
		DbValue column = DbValue_initCopy(&((GuiItemButton*)self)->text);
		//BIG columnRow = DbColumn_getRow(column.column);
		OsDateTimeTYPE timeFormat = GuiItem_findAttribute(self, "timeFormat");

		GuiItemLayout* layout = GuiItemLayout_new(Quad2i_init());
		GuiItemCalendarSmall* calc = GuiItem_addSubName((GuiItem*)layout, "calendar", GuiItemCalendarSmall_new(Quad2i_init4(0, 0, 1, 1), column, timeFormat, 0));
		GuiItemLayout_addColumn(layout, 0, 10);
		GuiItemLayout_addRow(layout, 0, GuiItemCalendarSmall_getSizeY(calc));

		GuiItemRoot_addDialogRel((GuiItem*)layout, self, self->coordMove, FALSE);
		//GuiItem_addSubName(GuiItem_getMostDeep(self), GuiItemLevel_newCoord(FALSE, (GuiItem*)layout, self->coordMove));
	}
}

void GuiItemTable_clickImportFile(GuiItem* self)
{
	GuiItemTags* tags = (GuiItemTags*)GuiItem_findParentType(self->parent, GuiItem_TAGS);
	if (tags)
	{
		char* paths = Std_newCHAR_uni(GuiItemEdit_getText((GuiItemEdit*)GuiItem_findName(self, "pathFile")));

		BIG row = GuiItemTags_getRow(tags);
		DbColumn* column = GuiItemTags_getColumn(tags);
		UBIG N = GuiItemTags_numItems(tags);
		DbValue col = DbValue_initGET(column, row);

		int NP = Std_separNumItemsCHAR(paths, ';');
		int i;
		for (i = 0; i < NP; i++)
		{
			char* path = Std_separGetItemCHAR(paths, i, ';');
			IOFiles_readSingle(path, &col, N++, DbRoot_getProgress());
			Std_deleteCHAR(path);
		}

		Std_deleteCHAR(paths);
		DbValue_free(&col);
	}
}
void GuiItemTable_clickImportWeb(GuiItem* self)
{
	GuiItemTags* tags = (GuiItemTags*)GuiItem_findParentType(self->parent, GuiItem_TAGS);
	GuiItemFile* file = (GuiItemFile*)GuiItem_findParentType(self->parent, GuiItem_FILE);

	if (tags || file)
	{
		char* paths = Std_newCHAR_uni(GuiItemEdit_getText((GuiItemEdit*)GuiItem_findName(self, "pathWeb")));

		DbValue col;
		UBIG N = 1;
		if (tags)
		{
			N = GuiItemTags_numItems(tags);

			BIG row = GuiItemTags_getRow(tags);
			DbColumn* column = GuiItemTags_getColumn(tags);
			col = DbValue_initGET(column, row);
		}
		else
		{
			col = DbValue_initCopy(&file->info);
		}

		int NP = Std_separNumItemsCHAR(paths, ';');
		int i;
		for (i = 0; i < NP; i++)
		{
			char* path = Std_separGetItemCHAR(paths, i, ';');
			IOFiles_readNet(path, &col, N++, DbRoot_getProgress());
			Std_deleteCHAR(path);
		}

		DbValue_free(&col);
		Std_deleteCHAR(paths);
	}
}

GuiItem* GuiItemTable_getCardSkinItem(Quad2i grid, DbValue column, BOOL showDescription, BOOL onlyRead)
{
	UNI name[64];
	DbFormatTYPE type = DbValue_getFormat(&column);

	BIG crowT = DbColumn_getRow(column.column);

	switch (type)
	{
		case DbFormat_NUMBER_1:
		case DbFormat_NUMBER_N:
		case DbFormat_CURRENCY:
		case DbFormat_PERCENTAGE:
		{
			GuiItem* edit;
			if (onlyRead)
			{
				edit = GuiItemText_new(grid, TRUE, column, showDescription ? DbValue_initStaticCopy(DbColumn_getName(column.column, name, 64)) : DbValue_initEmpty());
				GuiItemText_setWhiteBack((GuiItemText*)edit, TRUE);
			}
			else
				edit = GuiItemEdit_new(grid, column, showDescription ? DbValue_initStaticCopy(DbColumn_getName(column.column, name, 64)) : DbValue_initEmpty());
			return edit;
		}

		case DbFormat_RATING:
		{
			GuiItemRating* rating = (GuiItemRating*)GuiItemRating_new(grid, column, DbValue_initOption(crowT, "numStars", 0), showDescription ? DbValue_initStaticCopy(DbColumn_getName(column.column, name, 64)) : DbValue_initEmpty(), 0);
			return &rating->base;
		}

		case DbFormat_SLIDER:
		return GuiItemSlider_new(grid, DbValue_initOption(crowT, "min", 0), DbValue_initOption(crowT, "max", 0), DbValue_initOption(crowT, "jump", 0), column, showDescription ? DbValue_initStaticCopy(DbColumn_getName(column.column, name, 64)) : DbValue_initEmpty());

		case DbFormat_CHECK:
		return GuiItemCheck_new(grid, column, showDescription ? DbValue_initStaticCopy(DbColumn_getName(column.column, name, 64)) : DbValue_initEmpty());

		case DbFormat_MENU:
		{
			BIG optionsRow = DbRows_findSubType(crowT, "options");
			return GuiItemComboDynamic_new(grid, FALSE, DbRows_initLink((DbColumnN*)column.column, -1), DbValue_initOption(-1, "name", 0), DbRows_initLink(DbRoot_getColumnSubs(), optionsRow), showDescription ? DbValue_initStaticCopy(DbColumn_getName(column.column, name, 64)) : DbValue_initEmpty());
		}

		case DbFormat_TAGS:
		{
			BIG optionsRow = DbRows_findSubType(crowT, "options");

			//values
			DbValues columns = DbValues_init();
			DbValues_add(&columns, DbValue_initOption(optionsRow, "name", 0));

			GuiItem* ret = GuiItemTags_new(grid, DbRows_initLink((DbColumnN*)column.column, -1), columns, GuiItemTags_dialogTagsAdd, GuiItemTags_dialogTagsDetails, TRUE, TRUE, FALSE, TRUE, DbValue_initEmpty());
			DbValue_free(&column);
			return ret;
		}

		case DbFormat_LINK_1:
		case DbFormat_LINK_N:
		{
			DbTable* btable = DbColumn_getBTable(column.column);
			if (btable)
			{
				const BIG num_columns = Std_bmax(1, DbColumn_getOptionNumber(column.column, "numColumnsPreviews"));
				DbValues columns = DbRows_getSubs(DbTable_getRow(btable), "columns", TRUE, num_columns);

				GuiItem* ret = GuiItemTags_new(grid, DbRows_initLink((DbColumnN*)column.column, -1), columns, GuiItemTags_dialogLinksAdd, GuiItemTags_dialogLinksDetails, FALSE, TRUE, TRUE, TRUE, DbValue_initEmpty());
				DbValue_free(&column);
				return ret;
			}
			break;
		}

		case DbFormat_FILE_N:
		{
			DbValues columns = DbValues_init();
			GuiItem* ret = GuiItemTags_new(grid, DbRows_initLink((DbColumnN*)column.column, -1), columns, GuiItemTags_dialogFileAdd, GuiItemTags_dialogFileDetails, TRUE, TRUE, TRUE, TRUE, DbValue_initOption(crowT, "preview", 0));
			DbValue_free(&column);
			return ret;
		}

		case DbFormat_FILE_1:
		return GuiItemFile_new(grid, column, showDescription ? DbValue_initNumber(1) : DbValue_initOption(crowT, "preview", 0), FALSE);

		case DbFormat_TEXT:
		case DbFormat_PHONE:
		case DbFormat_URL:
		case DbFormat_EMAIL:
		case DbFormat_LOCATION:
		{
			GuiItem* edit;
			if (onlyRead)
			{
				edit = GuiItemText_new(grid, TRUE, column, showDescription ? DbValue_initStaticCopy(DbColumn_getName(column.column, name, 64)) : DbValue_initEmpty());
				GuiItemText_setWhiteBack((GuiItemText*)edit, TRUE);
			}
			else
				edit = GuiItemEdit_new(grid, column, showDescription ? DbValue_initStaticCopy(DbColumn_getName(column.column, name, 64)) : DbValue_initEmpty());
			return edit;
		}

		case DbFormat_DATE:
		{
			GuiItemButton* button = (GuiItemButton*)GuiItemButton_newWhiteEx(grid, column, &GuiItemTable_clickSelectCalendar);
			GuiItem_setAttribute(&button->base, "timeFormat", DbValue_getOptionNumber(crowT, "timeFormat", 0));

			if (showDescription)
				GuiItemButton_setDescription(button, DbValue_initStaticCopy(DbColumn_getName(column.column, name, 64)));
			return &button->base;
		}
		case DbFormat_ROW:
		{
			break;
		}
	}

	return GuiItemText_new(grid, TRUE, DbValue_initEmpty(), DbValue_initEmpty());;
}

GuiItemLayout* GuiItemTable_buildPage(BIG viewRow, BOOL showRowID, BOOL onlyRead)
{
	GuiItemLayout* skin = GuiItemLayout_new(Quad2i_init());
	GuiItemLayout_addColumn(skin, 0, 100);

	int y = 0;
	if (showRowID)
		GuiItem_addSubName((GuiItem*)skin, "row_id", GuiItemText_new(Quad2i_init4(0, y++, 1, 1), TRUE, DbValue_initGET(DbTable_getIdsColumn(DbRoot_findParentTable(viewRow)), -1), DbValue_initEmpty()));

	int N_COLS = DbRows_getSubsNum(viewRow, "columns", TRUE);
	BIG c;
	for (c = 0; c < N_COLS; c++)
	{
		DbValue value = DbRows_getSubsCell(viewRow, "columns", TRUE, c, -1);

		GuiItem* it = GuiItemTable_getCardSkinItem(Quad2i_init4(0, y, 1, 2), value, TRUE, onlyRead);
		if (it)
		{
			char nameId[64];
			snprintf(nameId, 64, "%lld_skin", c);

			GuiItem_addSubName((GuiItem*)skin, nameId, it);
			y += 2;
		}
	}

	return skin;
}

void GuiItemTable_clickShowPage(GuiItem* self)
{
	GuiItemTable* table = GuiItem_findParentType(self->parent, GuiItem_TABLE);
	if (table && self->type == GuiItem_BUTTON)
	{
		BIG row = GuiItem_getRow(self);

		GuiItemLayout* skin = GuiItemTable_buildPage(table->viewRow, TRUE, FALSE);

		GuiItem_setRow(&skin->base, row, 0);
		GuiItemRoot_addDialogLayout(GuiItemRoot_createDialogLayout(Vec2i_init2(15, GuiItemLayout_getSubMaxGrid(skin).y), DbValue_initLang("CARD"), (GuiItem*)skin, 0));
	}
}

void UiDialogLink_clickLinkDialogClear(GuiItem* self)
{
	GuiItemTable* linkTable = GuiItem_findNameType(self->parent, "btable", GuiItem_TABLE);
	if (linkTable)
	{
		BIG srcRow = GuiItemTable_getBaseRow(linkTable);

		DbColumnN* column = (DbColumnN*)GuiItemTable_getBaseColumn(linkTable);
		if (column)
			DbColumnN_clearRow(column, srcRow);
	}
}

void UiDialogLink_clickLinkDialogAddAll(GuiItem* self)
{
	GuiItemTable* linkTable = GuiItem_findNameType(self->parent, "btable", GuiItem_TABLE);
	if (linkTable)
	{
		BIG srcRow = GuiItemTable_getBaseRow(linkTable);

		DbColumnN* column = (DbColumnN*)GuiItemTable_getBaseColumn(linkTable);
		if (column)
			DbColumnN_addAllTable(column, srcRow);
	}
}

void UiDialogLink_clickLinkDialogAddOne(GuiItem* self)
{
	GuiItemTable* linkTable = GuiItem_findNameType(self->parent, "btable", GuiItem_TABLE);
	if (linkTable)
		DbRows_addLinkRow(&linkTable->filter, GuiItem_getRow(self));
}

GuiItemLayout* GuiItemTable_buildDialogLinks(DbColumn* column)
{
	BIG srcRow = -1;
	DbTable* btable = DbColumn_getBTable(column);
	BIG bRow = DbTable_getRow(btable);

	//layout
	GuiItemLayout* layout = GuiItemLayout_new(Quad2i_init4(0, 0, 1, 1));
	GuiItemLayout_addColumn(layout, 0, 10);
	GuiItemLayout_addColumn(layout, 1, 10);
	GuiItemLayout_addColumn(layout, 2, 10);
	GuiItemLayout_addRow(layout, 0, 2);
	GuiItemLayout_addRow(layout, 1, 2);
	GuiItemLayout_addRow(layout, 2, 10);
	GuiItemLayout_addRow(layout, 3, 2);
	GuiItemLayout_addRow(layout, 4, 10);

	//"Links" + [Clear]
	GuiItem_addSubName((GuiItem*)layout, "add_all", GuiItemButton_newClassicEx(Quad2i_init4(0, 0, 1, 1), DbValue_initLang("ADD_ALL"), &UiDialogLink_clickLinkDialogAddAll));
	GuiItem_addSubName((GuiItem*)layout, "clear", GuiItemButton_newClassicEx(Quad2i_init4(1, 0, 1, 1), DbValue_initLang("CLEAR"), &UiDialogLink_clickLinkDialogClear));

	//Table with current links
	GuiItem_addSubName((GuiItem*)layout, "links", GuiItemText_new(Quad2i_init4(0, 1, 3, 1), TRUE, DbValue_initLang("LINKS"), DbValue_initEmpty()));

	GuiItemTable* tableActual = (GuiItemTable*)GuiItem_addSubName((GuiItem*)layout, "btable", (GuiItem*)GuiItemTable_new(Quad2i_init4(0, 2, 3, 1), bRow, DbRows_initLink((DbColumnN*)column, srcRow), TRUE, FALSE, DbValue_initNumber(0), DbValue_initNumber(0), DbValue_initStaticCopyCHAR("0 0 0 0")));
	tableActual->drawBorder = TRUE;
	tableActual->showRemoveButton = TRUE;
	tableActual->showChangeOrder = TRUE;
	tableActual->changeOrderNameDst = Std_newCHAR("dst");
	tableActual->changeOrderNameSrc = Std_newCHAR("dst");

	//BTable
	UNI name[64];
	GuiItem_addSubName((GuiItem*)layout, "name", GuiItemText_new(Quad2i_init4(0, 3, 3, 1), TRUE, DbValue_initStaticCopy(DbTable_getName(btable, name, 64)), DbValue_initEmpty()));
	GuiItemTable* tableAdd = (GuiItemTable*)GuiItem_addSubName((GuiItem*)layout, "cTable", (GuiItem*)GuiItemTable_new(Quad2i_init4(0, 4, 3, 1), bRow, DbRows_initTable(btable), TRUE, TRUE, DbValue_initNumber(0), DbValue_initNumber(0), DbValue_initStaticCopyCHAR("0 0 0 0")));
	tableAdd->drawBorder = TRUE;
	tableAdd->showAddButton = TRUE;
	tableAdd->showChangeOrder = TRUE;
	tableAdd->changeOrderNameDst = Std_newCHAR("---");
	tableAdd->changeOrderNameSrc = Std_newCHAR("dst");

	return layout;
}

void GuiItemTable_draw(GuiItemTable* self, Image4* img, Quad2i coord, Win* win)
{
	GuiItemTable_repairSelect(self);

	//background
	Image4_drawBoxQuad(img, coord, self->base.back_cd);		//header
}

static Quad2i _GuiItemTable_getSelectGridCoord(const GuiItemTable* self, Quad2i q, BOOL noScroll)
{
	Quad2i ret = Quad2i_init();

	const GuiItem* layout = GuiItem_getSubConst(&self->base, 0);
	if (layout)
	{
		const GuiItem* it = GuiItem_getSubConst(layout, self->subs_start_rowCell);
		if (it)
		{
			Quad2i firstCell = noScroll ? it->coordMove : it->coordScreen;

			//x
			const int N_ROWS_VISIBLE = self->num_rows_max;
			BIG x;
			for (x = 0; x < Quad2i_end(q).x; x++)
			{
				const int s = GuiItem_getSubConst(layout, self->subs_start_rowCell + x * N_ROWS_VISIBLE)->coordMove.size.x; //coordMove;
				if (x < q.start.x)
					ret.start.x += s;
				else
					ret.size.x += s;
			}

			//y
			const int s = firstCell.size.y;
			const BIG WHEEL = GuiScroll_getWheelRow(&self->scroll);
			if ((q.start.y - WHEEL) < 0)
			{
				ret.size.y = (q.size.y + q.start.y - WHEEL) * s;
				ret.start.y = 0;
			}
			else
			{
				ret.start.y = (q.start.y - WHEEL) * s;
				ret.size.y = (q.size.y) * s;
			}

			ret.start = Vec2i_add(ret.start, firstCell.start);
		}
	}

	return ret;
}

static BOOL _GuiItemTable_getTouchCellPos(GuiItemTable* self, Vec2i touch_pos, Vec2i* out_pos)
{
	const GuiItem* layout = GuiItem_getSubConst(&self->base, 0);
	const UBIG WHEEL = GuiScroll_getWheelRow(&self->scroll);
	const int N_COLS = GuiItemTable_numColumns(self);
	const int N_ROWS_VISIBLE = self->num_rows_max;
	const int N_ROWS_REAL = self->num_rows_real;

	BIG x, y;
	for (y = 0; y < N_ROWS_REAL; y++)
	{
		for (x = 0; x < N_COLS; x++)
		{
			if (Quad2i_inside(GuiItem_getSubConst(layout, self->subs_start_rowCell + x * N_ROWS_VISIBLE + y)->coordMoveCut, touch_pos))	//coordMove
			{
				*out_pos = Vec2i_init2(x, y + WHEEL);
				return TRUE;
			}
		}
	}

	return FALSE;
}

static BOOL _GuiItemTable_getTouchCellPosClosest(GuiItemTable* self, Vec2i touch_pos, Vec2i* out_pos)
{
	const float max_r = 10000000;
	float r = max_r;

	const GuiItem* layout = GuiItem_getSubConst(&self->base, 0);
	const UBIG WHEEL = GuiScroll_getWheelRow(&self->scroll);
	const int N_COLS = GuiItemTable_numColumns(self);
	const int N_ROWS_VISIBLE = self->num_rows_max;
	const int N_ROWS_REAL = self->num_rows_real;

	BIG x, y;
	for (y = 0; y < N_ROWS_REAL; y++)
	{
		for (x = 0; x < N_COLS; x++)
		{
			Quad2i itCoord = GuiItem_getSubConst(layout, self->subs_start_rowCell + x * N_ROWS_VISIBLE + y)->coordMoveCut;
			float dist = Quad2i_inside(itCoord, touch_pos) ? 0 : Vec2i_distance(touch_pos, Quad2i_getMiddle(itCoord));
			if (dist < r)
			{
				r = dist;
				*out_pos = Vec2i_init2(x, y + WHEEL);
			}
		}
	}

	return (r < max_r);
}

static BOOL _GuiItemTable_isSelect(const GuiItemTable* self)
{
	Quad2i selectRect = GuiItemTable_getSelectRect(self);
	Quad2i g = _GuiItemTable_getSelectGridCoord(self, selectRect, TRUE);
	return (g.size.x > 0 && g.size.y > 0);
}

static BOOL _GuiItemTable_isCellExtendSelect(const GuiItemTable* self)
{
	Quad2i g = _GuiItemTable_getSelectGridCoord(self, self->cellExtendRect, TRUE);
	return _GuiItemTable_isSelect(self) && (g.size.x > 0 && g.size.y > 0);
}

static Quad2i _GuiItemTable_getCellExtendQuad(const GuiItemTable* self, Quad2i grid)
{
	const int cell = OsWinIO_cellSize();

	Quad2i q = _GuiItemTable_getSelectGridCoord(self, grid, TRUE);
	return Quad2i_initMid(Quad2i_end(q), Vec2i_init2(cell / 4, cell / 4));
}

static GuiItem* _GuiItemTable_getBaseLayout(GuiItemTable* self)
{
	return GuiItem_getSub(&self->base, 0);
}
static BOOL _GuiItemTable_hasHScroll(GuiItemTable* self)
{
	return GuiItemLayout_hasScrollH((GuiItemLayout*)_GuiItemTable_getBaseLayout(self));
}

void GuiItemTable_drawPost(GuiItemTable* self, Image4* img, Quad2i coord, Win* win)
{
	const int cell = OsWinIO_cellSize();
	const int scroll_width = GuiScroll_widthWin(win);

	if (_GuiItemTable_hasHScroll(self))
		coord.size.y -= scroll_width;

	if (_GuiItemTable_isSelect(self) && GuiItemTable_repairSelect(self))
	{
		Quad2i selectRect = GuiItemTable_getSelectRect(self);
		Quad2i q = _GuiItemTable_getSelectGridCoord(self, selectRect, TRUE);

		{
			Quad2i coord2 = coord;
			coord2.size.x -= GuiScroll_widthWin(win);
			q = Quad2i_getIntersect(q, coord2);	//cut it
		}

		Rgba cd = g_theme.selectEdit;
		if (selectRect.size.x == 1 && selectRect.size.y == 1)
		{
			//draw shadow line
			{
				Quad2i qs = q;
				qs.start.x = coord.start.x;
				qs.size.x = coord.size.x - scroll_width;
				Rgba cds = g_theme.main;
				cds.a = 20;
				Image4_drawBoxQuadAlpha(img, qs, cds);
			}

			cd.a = 255;
			Image4_drawBorder(img, q, 2, cd);
		}
		else
		{
			//only start rect
			cd.a = 50;
			Image4_drawBoxQuadAlpha(img, _GuiItemTable_getSelectGridCoord(self, Quad2i_init2(self->selectFirstTouch, Vec2i_init2(1, 1)), TRUE), cd);

			//complete rect
			cd.a = 20;
			Image4_drawBoxQuadAlpha(img, q, cd);

			//complete border
			cd.a = 255;
			Image4_drawBorder(img, q, 2, cd);
		}

		if (_GuiItemTable_isCellExtendSelect(self))
		{
			q = _GuiItemTable_getSelectGridCoord(self, self->cellExtendRect, TRUE);
			Rgba cd = g_theme.main;
			Image4_drawBorder(img, q, 2, cd);
		}

		//holder
		q = _GuiItemTable_getCellExtendQuad(self, _GuiItemTable_isCellExtendSelect(self) ? self->cellExtendRect : selectRect);
		Image4_drawBoxQuad(img, q, g_theme.white);
		cd.a = 255;
		Image4_drawBorder(img, q, 1, cd);
	}

	if (self->drawBorder)
		Image4_drawBorder(img, coord, 1, g_theme.black);

	//scroll
	UINT extra = GuiItemTable_scrollExtra(self, win);
	const int RS = GuiItemTable_getRowSize(self);
	//int N_ROWS_VISIBLE = (coord.size.y - GuiItemTable_scrollExtra(self, win)) / cell / RS;

	BOOL hScroll = _GuiItemTable_hasHScroll(self);	//is horizontal scroll on
	if (!hScroll)
		hScroll = (coord.size.y % cell) < cell * RS * 3 / 4;	//check if last is visible from 3/4
	GuiScroll_set(&self->scroll, (GuiItemTable_numRows(self) + self->showAddRecord + hScroll) * cell * RS, coord.size.y - extra, cell * RS);
	GuiScroll_drawV(&self->scroll, Vec2i_init2(coord.start.x + coord.size.x - scroll_width, coord.start.y + extra), img, win);
}

void GuiItemTable_update(GuiItemTable* self, Quad2i coord, Win* win)
{
	GuiItemTable_repairSelect(self);

	DbValue_hasChanged(&self->selectGrid);
	char gridC[64];
	snprintf(gridC, 64, "%d %d %d %d", self->selectFirstTouch.x, self->selectFirstTouch.y, self->selectLastTouch.x, self->selectLastTouch.y);
	gridC[63] = 0;
	UNI gridU[64];
	Std_copyUNI_char(gridU, 64, gridC);
	if (!Std_cmpUNI(gridU, self->selectGrid.result.str))
	{
		BOOL reset = !GuiItemRoot_hasChanges();
		DbValue_setTextCopy(&self->selectGrid, gridU);
		if (reset)
			GuiItemRoot_resetNumChanges();
	}

	DbRows_hasChanged(&self->filter);

	BIG oldNumRows = self->oldNumRows;
	self->oldNumRows = GuiItemTable_numRows(self);

	//update cells
	if (self->base.subs.num)
	{
		GuiItem* layout = _GuiItemTable_getBaseLayout(self);

		const UBIG N_ROWS_VISIBLE = self->num_rows_max;//Std_bmin(DbTable_numRowsReal(table), self->num_rows_max);
		const UBIG WHEEL = GuiScroll_getWheelRow(&self->scroll);

		self->num_rows_real = 0;

		int r, p;

		int last_y = 0;

		//cards
		p = self->subs_start_rowOptionCards;
		for (r = 0; r < N_ROWS_VISIBLE; r++)
		{
			GuiItem* it = GuiItem_getSub(layout, p++);
			GuiItem_setRow(it, DbRows_getRow(&self->filter, WHEEL + r), 0);

			BOOL show = DbRows_isRow(&self->filter, WHEEL + r);
			GuiItem_setShow(it, show);

			if (show)
				last_y = Std_max(last_y, it->grid.start.y + it->grid.size.y);
		}

		if (self->showHeader && last_y == 0)	//no cards
			last_y = 1;

		//options
		p = self->subs_start_rowOption;
		for (r = 0; r < N_ROWS_VISIBLE; r++)
		{
			GuiItem* it = GuiItem_getSub(layout, p++);

			GuiItem_setRow(it, DbRows_getRow(&self->filter, WHEEL + r), 0);

			BOOL show = DbRows_isRow(&self->filter, WHEEL + r);
			GuiItem_setShow(it, show);

			self->num_rows_real += show;
		}

		if (self->showChangeOrder)
		{
			p = self->subs_start_rowOptionOrder;
			for (r = 0; r < N_ROWS_VISIBLE; r++)
			{
				GuiItem* it = GuiItem_getSub(layout, p++);
				GuiItem_setRow(it, DbRows_getRow(&self->filter, WHEEL + r), 0);

				BOOL show = DbRows_isRow(&self->filter, WHEEL + r);
				GuiItem_setShow(it, show);
			}
		}

		if (self->showAddRecord)
		{
			GuiItem* addRecord = GuiItem_getSub(layout, self->subs_start_add);

			BOOL show = (DbColumns_num(DbTable_getColumns((GuiItemTable_getTable(self)))) > 1 && self->showAddRecord);

			GuiItem_setShow(addRecord, show);
			if (show)
				GuiItem_setGrid(addRecord, Quad2i_init4(0, last_y, 1, 1));
		}

		//Records data
		const int N_COLS = GuiItemTable_numColumns(self);
		Quad2i rectCoord = _GuiItemTable_getSelectGridCoord(self, GuiItemTable_getSelectRect(self), TRUE);
		int c;
		for (c = 0; c < N_COLS; c++)
		{
			for (r = 0; r < N_ROWS_VISIBLE; r++)
			{
				p = Std_min(self->subs_start_rowCell + c * self->num_rows_max + r, self->subs_end_rowCell);
				if (p < self->subs_end_rowCell)
				{
					GuiItem* it = GuiItem_getSub(layout, p);

					GuiItem_setRow(it, DbRows_getRow(&self->filter, WHEEL + r), 0);
					GuiItem_setShow(it, DbRows_isRow(&self->filter, WHEEL + r));

					GuiItem_setEnableOne(it, !GuiItemTable_isColumnLocked(self, c));
					GuiItem_setTouchRecommand(it, Quad2i_inside(rectCoord, Quad2i_getMiddle(it->coordMove)));
				}
			}
		}

		GuiItem_setRedraw(&self->base, (oldNumRows != self->oldNumRows || GuiScroll_getRedrawAndReset(&self->scroll)));
	}

	GuiItemText* numRecords = GuiItem_findNameType(&self->base, "#records", GuiItem_TEXT);
	if (numRecords)
	{
		UBIG old = DbValue_getNumber(&numRecords->text);
		UBIG nw = DbRows_getSize(&self->filter);

		if (old != nw)
		{
			UNI nmbr[64];
			Std_buildNumberUNI(nw, 0, nmbr);
			Std_separNumberThousands(nmbr, ' ');
			DbValue_setTextCopy(&numRecords->text, nmbr);
		}
	}
}

const UNI* GuiItemTable_getTextNoFormat(GuiItemTable* self, int c, UBIG r, StdString* out)
{
	return DbColumn_getStringCopyLong(GuiItemTable_getColumn(self, c), r, out);
}

const UNI* GuiItemTable_getTextWithFormat(GuiItemTable* self, int c, UBIG r, StdString* out)
{
	return DbColumn_getStringCopyWithFormatLong(GuiItemTable_getColumn(self, c), r, out);
}

static void _GuiItemTable_copy(GuiItemTable* self, Vec2i s, Vec2i e, BOOL format)
{
	UNI* clip = 0;

	StdString strr = StdString_init();

	BIG x, y;
	for (y = s.y; y < e.y; y++)
	{
		for (x = s.x; x < e.x; x++)
		{
			const UNI* str = format ? GuiItemTable_getTextWithFormat(self, x, DbRows_getRow(&self->filter, y), &strr) : GuiItemTable_getTextNoFormat(self, x, DbRows_getRow(&self->filter, y), &strr);
			clip = Std_addAfterUNI(clip, str);
			clip = Std_addAfterUNI(clip, (x + 1 < e.x) ? _UNI32("\t") : _UNI32("\r\n"));
		}
	}

	StdString_free(&strr);

	if (clip)
		Win_clipboard_setText(clip);
	Std_deleteUNI(clip);
}

static void _GuiItemTable_paste(GuiItemTable* self, Vec2i s, Vec2i e)
{
	UNI* paste = Win_clipboard_getText();
	UNI* paste_orig = paste;
	Std_removeLetterUNI(paste, '\r');

	BIG x, y;
	for (y = s.y; y < e.y; y++)
	{
		BIG line = Std_findUNI(paste, '\n');
		if (line >= 0)
			paste[line] = 0;

		int value_i = 0;
		const int value_max = Std_separNumItemsUNI(paste, '\t');

		for (x = s.x; x < e.x && x < s.x + value_max; x++)
		{
			if (!GuiItemTable_isColumnLocked(self, x))
			{
				UNI* value = Std_separGetItemUNI(paste, value_i, '\t');
				if (Std_sizeUNI(value))
				{
					DbColumn* column = GuiItemTable_getColumn(self, x);
					DbColumn_setStringCopy(column, DbRows_getRow(&self->filter, y), 0, value);
				}

				Std_deleteUNI(value);
			}

			value_i++;
		}

		if (line >= 0)
		{
			paste[line] = '\n';
			paste += line + 1;
		}
		else
			paste += Std_findUNI(paste, 0);
	}

	Std_deleteUNI(paste_orig);
}

void GuiItemTable_clickRemoveOption(GuiItem* self)
{
	if (self->type == GuiItem_BUTTON)
	{
		DbRoot_removeRow(((GuiItemButton*)self)->text.row);
	}
}

static void _GuiItemTable_delete(GuiItemTable* self, Vec2i s, Vec2i e)
{
	BIG x, y;
	for (y = s.y; y < e.y; y++)
	{
		for (x = s.x; x < e.x; x++)
			DbColumn_setStringCopy(GuiItemTable_getColumn(self, x), DbRows_getRow(&self->filter, y), 0, 0);
	}
}

void GuiItemTable_clickCopy(GuiItem* self)
{
	GuiItemTable* table = GuiItem_findParentType(self->parent, GuiItem_TABLE);
	if (table)
	{
		Quad2i selectRect = GuiItemTable_getSelectRect(table);
		Vec2i st = selectRect.start;
		Vec2i e = Quad2i_end(selectRect);

		_GuiItemTable_copy(table, st, e, TRUE);
		GuiItem_closeParentLevel(self);
	}
}
void GuiItemTable_clickPaste(GuiItem* self)
{
	GuiItemTable* table = GuiItem_findParentType(self->parent, GuiItem_TABLE);
	if (table)
	{
		Quad2i selectRect = GuiItemTable_getSelectRect(table);
		Vec2i st = selectRect.start;
		Vec2i e = Quad2i_end(selectRect);

		_GuiItemTable_paste(table, st, e);
		GuiItem_closeParentLevel(self);
	}
}
void GuiItemTable_clickDelete(GuiItem* self)
{
	GuiItemTable* table = GuiItem_findParentType(self->parent, GuiItem_TABLE);
	if (table)
	{
		Quad2i selectRect = GuiItemTable_getSelectRect(table);
		Vec2i st = selectRect.start;
		Vec2i e = Quad2i_end(selectRect);

		_GuiItemTable_delete(table, st, e);
		GuiItem_closeParentLevel(self);
	}
}

void GuiItemTable_clickGotoBtable(GuiItem* self)
{
	GuiItemTable* table = GuiItem_findParentType(self->parent, GuiItem_TABLE);
	if (table)
	{
		BIG c = GuiItem_findAttribute(self, "c");

		DbTable* btable = DbColumnN_getBTable((DbColumnN*)GuiItemTable_getColumn(table, c));

		BIG btableRow = DbTable_getRow(btable);

		if (DbRoot_getPanelLeft() == table->viewRow)
			DbRoot_setPanelLeft(btableRow);
		else
			DbRoot_setPanelRight(btableRow);
	}
}

void GuiItemTable_makeCellExtend(GuiItemTable* self)
{
	Quad2i selectRect = GuiItemTable_getSelectRect(self);
	Vec2i ss = selectRect.start;
	Vec2i ee = Quad2i_end(selectRect);

	Vec2i s = self->cellExtendRect.start;
	Vec2i e = Quad2i_end(self->cellExtendRect);

	BOOL horizontal = (self->cellExtendRect.size.y == selectRect.size.y);

	StdString strr = StdString_init();

	BIG x, y;
	BIG i;
	if (horizontal)
	{
		for (y = ss.y; y < ee.y; y++)
		{
			//Days/Months/etc.
			const UNI* find = GuiItemTable_getTextNoFormat(self, ss.x, DbRows_getRow(&self->filter, y), &strr);
			BIG n;
			UNI** src = Lang_getList(find, &n);
			if (!src)
			{
				n = ee.x - ss.x;
				src = Os_malloc(n * sizeof(UNI*));
				for (x = ss.x; x < ee.x; x++)
					src[x - ss.x] = Std_newUNI(GuiItemTable_getTextNoFormat(self, x, DbRows_getRow(&self->filter, y), &strr));
			}

			//set
			for (x = s.x; x < e.x; x++)
				if (!Quad2i_inside(selectRect, Vec2i_init2(x, y)))
				{
					BIG xx = Std_abs(x - s.x) % n;
					DbColumn_setStringCopy(GuiItemTable_getColumn(self, x), DbRows_getRow(&self->filter, y), 0, src[xx]);
				}

			//free
			for (i = 0; i < n; i++)
				Std_deleteUNI(src[i]);
			Os_free(src, n * sizeof(UNI*));
		}
	}
	else
	{
		for (x = ss.x; x < ee.x; x++)
		{
			DbColumn* column = GuiItemTable_getColumn(self, x);

			if (DbColumn_isType1(column))
			{
				//numbers

				//alloc
				BIG n = (ee.y - ss.y);
				double* src = Os_calloc(n, sizeof(double));
				for (y = ss.y; y < ee.y - 1; y++)
				{
					src[y - ss.y] = DbColumn_getFlt(column, DbRows_getRow(&self->filter, y + 1), 0) - DbColumn_getFlt(column, DbRows_getRow(&self->filter, y), 0);
				}

				//set
				double stValue = DbColumn_getFlt(column, DbRows_getRow(&self->filter, ee.y - 1), 0);
				UBIG p = 0;
				for (y = s.y; y < e.y; y++)
					if (!Quad2i_inside(selectRect, Vec2i_init2(x, y)))
					{
						stValue += src[p++];
						if (p >= n - 1)p = 0;
						DbColumn_setFlt(column, DbRows_getRow(&self->filter, y), 0, stValue);
					}
				//free
				Os_free(src, n * sizeof(double));
			}
			else
			{
				//Days/Months/etc.

				//alloc
				const UNI* find = GuiItemTable_getTextNoFormat(self, x, DbRows_getRow(&self->filter, ss.y), &strr);
				BIG n;
				UNI** src = Lang_getList(find, &n);
				if (!src)
				{
					//repeat
					n = ee.y - ss.y;
					src = Os_malloc(n * sizeof(UNI*));
					for (y = ss.y; y < ee.y; y++)
						src[y - ss.y] = Std_newUNI(GuiItemTable_getTextNoFormat(self, x, DbRows_getRow(&self->filter, y), &strr));
				}

				//set
				for (y = s.y; y < e.y; y++)
					if (!Quad2i_inside(selectRect, Vec2i_init2(x, y)))
					{
						BIG yy = Std_abs(y - s.y) % n;
						DbColumn_setStringCopy(column, DbRows_getRow(&self->filter, y), 0, src[yy]);
					}

				//free
				for (i = 0; i < n; i++)
					Std_deleteUNI(src[i]);
				Os_free(src, n * sizeof(UNI*));
			}
		}
	}

	StdString_free(&strr);
}

void GuiItemTable_showCellMenu(GuiItemTable* self)
{
	GuiItemLayout* layout = GuiItemLayout_new(Quad2i_init4(0, 0, 1, 1));
	layout->drawBorder = TRUE;
	GuiItemLayout_addColumn(layout, 0, 7);

	GuiItem* item;
	item = GuiItem_addSubName(&layout->base, "copy", GuiItemButton_newAlphaEx(Quad2i_init4(0, 0, 1, 1), DbValue_initLang("COPY"), &GuiItemTable_clickCopy));
	((GuiItemButton*)item)->textCenter = FALSE;

	item = GuiItem_addSubName(&layout->base, "paste", GuiItemButton_newAlphaEx(Quad2i_init4(0, 1, 1, 1), DbValue_initLang("PASTE"), &GuiItemTable_clickPaste));
	((GuiItemButton*)item)->textCenter = FALSE;

	item = GuiItem_addSubName(&layout->base, "delete", GuiItemButton_newAlphaEx(Quad2i_init4(0, 2, 1, 1), DbValue_initLang("DELETE"), &GuiItemTable_clickDelete));
	((GuiItemButton*)item)->textCenter = FALSE;

	item = GuiItem_addSubName(&layout->base, "delete_rows", GuiItemButton_newAlphaEx(Quad2i_init4(0, 4, 1, 1), DbValue_initLang("DELETE_ROWS"), &GuiItemTable_clickRemoveRows));
	((GuiItemButton*)item)->textCenter = FALSE;

	//Quad2i selectRect = GuiItemTable_getSelectRect(self);
	//GuiItemRoot_addDialogRel((GuiItem*)layout, (GuiItem*)self, _GuiItemTable_getSelectGridCoord(self, selectRect, FALSE), TRUE);

	GuiItemRoot_addDialogRel((GuiItem*)layout, (GuiItem*)self, Quad2i_init2(OsWinIO_getTouchPos(), Vec2i_init2(1, 1)), TRUE);
}

void GuiItemTable_touch(GuiItemTable* self, Quad2i coord, Win* win)
{
	const int cell = OsWinIO_cellSize();
	const int scroll_width = GuiScroll_widthWin(win);
	GuiItemTable_repairSelect(self);

	const int N_COLS = GuiItemTable_numColumns(self);
	const int N_ROWS = GuiItemTable_numRows(self);

	if (_GuiItemTable_hasHScroll(self))
		coord.size.y -= scroll_width;

	Quad2i selectRect = GuiItemTable_getSelectRect(self);

	Quad2i backup_selectRect = selectRect;
	Quad2i backup_cellExtendRect = self->cellExtendRect;

	Rgba back_cd = GuiItemTheme_getWhite_Background();
	Rgba front_cd = g_theme.black;

	self->cdSelect = g_theme.select;

	BOOL touchL = (OsWinIO_getTouch_action() == Win_TOUCH_DOWN_S);
	BOOL touchR = (OsWinIO_getTouch_action() == Win_TOUCH_FORCE_DOWN_S);
	BOOL endTouchL = (OsWinIO_getTouch_action() == Win_TOUCH_DOWN_E);
	//BOOL endTouchR = (OsWinIO_getTouch_action() == Win_TOUCH_FORCE_DOWN_E);

	if (self->base.touch && GuiItem_isEnable(&self->base) && OsWinIO_canActiveRenderItem(self))
	{
		UINT extra = GuiItemTable_scrollExtra(self, win);

		if (OsWinIO_isCursorEmpty())
			GuiScroll_touchV(&self->scroll, self, coord, Vec2i_init2(coord.start.x + coord.size.x - scroll_width, coord.start.y + extra), win);

		int width_sum = 1 + GuiItemTable_getColumnIdsWidth(self);
		width_sum = Std_max(0, width_sum - GuiItemLayout_getWheelH((GuiItemLayout*)_GuiItemTable_getBaseLayout(self)));

		BOOL insideColumnsHeader = FALSE;
		if (self->showHeader)
		{
			Quad2i q = coord;
			q.size.y = cell;
			insideColumnsHeader = Quad2i_inside(q, OsWinIO_getTouchPos());
		}
		BOOL insideRowsHeader = FALSE;
		if (self->showHeader)
		{
			Quad2i q = coord;
			q.size.x = width_sum * cell;
			insideRowsHeader = Quad2i_inside(q, OsWinIO_getTouchPos());
		}

		if (self->showHeader)
		{
			//cut top
			coord.start.y += cell;
			coord.size.y -= cell;

			//cut left
			coord.start.x += width_sum * cell;
			coord.size.x -= width_sum * cell;
		}

		BOOL inside = Quad2i_inside(coord, OsWinIO_getTouchPos());
		BOOL insideSelect = Quad2i_inside(_GuiItemTable_getSelectGridCoord(self, selectRect, TRUE), OsWinIO_getTouchPos());
		BOOL insideCellExtend = _GuiItemTable_isSelect(self) && Quad2i_inside(Quad2i_addSpace(_GuiItemTable_getCellExtendQuad(self, selectRect), -5), OsWinIO_getTouchPos());

		if (inside && touchR && !insideSelect)
		{
			//simulate left click => show Context menu
			insideSelect = TRUE;
			touchL = TRUE;
			endTouchL = TRUE;
		}

		if (inside && touchL)	//full touch
		{
			if (insideCellExtend)
			{
				self->cellExtendActive = TRUE;
				OsWinIO_setActiveRenderItem(self);
			}
			else
			{
				Vec2i curr;
				if (_GuiItemTable_getTouchCellPos(self, OsWinIO_getTouchPos(), &curr))
				{
					self->selectActive = TRUE;

					if (!(OsWinIO_getKeyExtra() & Win_EXTRAKEY_SHIFT))
						self->selectFirstTouch = curr;
				}
			}
		}

		if (self->cellExtendActive || insideCellExtend)
			Win_updateCursor(win, Win_CURSOR_FLEUR);

		if (self->cellExtendActive && inside)
		{
			Vec2i curr;
			if (_GuiItemTable_getTouchCellPosClosest(self, OsWinIO_getTouchPos(), &curr))
			{
				self->cellExtendRect.start = self->selectFirstTouch;
				self->cellExtendRect.size = Vec2i_sub(curr, self->selectFirstTouch);

				self->cellExtendRect = Quad2i_repair(self->cellExtendRect, 0);
				self->cellExtendRect.size.x++;
				self->cellExtendRect.size.y++;

				//vertical or horizontal
				if (Std_abs(self->cellExtendRect.size.x - selectRect.size.x) > Std_abs(self->cellExtendRect.size.y - selectRect.size.y))
				{
					//horizontal
					self->cellExtendRect.start.y = selectRect.start.y;
					self->cellExtendRect.size.y = selectRect.size.y;
				}
				else
				{
					//vertical
					self->cellExtendRect.start.x = selectRect.start.x;
					self->cellExtendRect.size.x = selectRect.size.x;
				}
			}
		}
		else
			if (self->selectActive && inside)	//update
			{
				_GuiItemTable_getTouchCellPosClosest(self, OsWinIO_getTouchPos(), &self->selectLastTouch);
			}

		if (insideSelect && touchR)
		{
			GuiItemTable_showCellMenu(self);
			OsWinIO_setTouch_action(Win_TOUCH_NONE);
		}

		if (insideColumnsHeader && touchR)
		{
			//select complete column
			if (_GuiItemTable_getTouchCellPosClosest(self, OsWinIO_getTouchPos(), &self->selectFirstTouch))
				self->selectLastTouch = Vec2i_init2(self->selectFirstTouch.x, N_ROWS - 1);
			OsWinIO_setTouch_action(Win_TOUCH_NONE);
		}
		if (insideRowsHeader && touchR)
		{
			//select complete row
			if (_GuiItemTable_getTouchCellPosClosest(self, OsWinIO_getTouchPos(), &self->selectFirstTouch))
				self->selectLastTouch = Vec2i_init2(N_COLS - 1, self->selectFirstTouch.y);

			OsWinIO_setTouch_action(Win_TOUCH_NONE);
		}

		if (!Quad2i_cmp(backup_selectRect, GuiItemTable_getSelectRect(self)))
		{
			GuiItemEdit_saveCache();	//close edit when selectRect changed
			//OsWinIO_resetTouch();
		}
	}

	if (endTouchL)
	{
		if (self->cellExtendActive)
			OsWinIO_resetActiveRenderItem();

		self->selectActive = FALSE;

		if (self->cellExtendActive)
			GuiItemTable_makeCellExtend(self);

		self->cellExtendActive = FALSE;
		self->cellExtendRect = Quad2i_init();
	}

	GuiItem_setRedraw(&self->base, !Quad2i_cmp(backup_selectRect, GuiItemTable_getSelectRect(self)) || !Quad2i_cmp(backup_cellExtendRect, self->cellExtendRect));

	_GuiItem_updateFinalCd(&self->base, back_cd, front_cd, coord, win);
}

void GuiItemTable_key(GuiItemTable* self, Quad2i coord, Win* win)
{
	if (!GuiItem_isEnable(&self->base))
		return;

	if (!OsWinIO_isCursorEmpty())
		return;

	const UBIG WHEEL = GuiScroll_getWheelRow(&self->scroll);
	const int N_COLS = GuiItemTable_numColumns(self);
	const int N_ROWS_VISIBLE = self->num_rows_max;
	const int N_ROWS = GuiItemTable_numRows(self);

	Quad2i selectRect = GuiItemTable_getSelectRect(self);
	Quad2i selectRectBackup = selectRect;

	UINT keyExtra = OsWinIO_getKeyExtra();

	{
		Vec2i s = selectRect.start;
		Vec2i e = Quad2i_end(selectRect);

		if (keyExtra & Win_EXTRAKEY_COPY)
		{
			_GuiItemTable_copy(self, s, e, FALSE);
		}
		else
			if (keyExtra & Win_EXTRAKEY_PASTE)
			{
				_GuiItemTable_paste(self, s, e);
			}
			else
				if (keyExtra & Win_EXTRAKEY_CUT)
				{
					_GuiItemTable_copy(self, s, e, FALSE);
					_GuiItemTable_delete(self, s, e);
				}
				else
					if (keyExtra & Win_EXTRAKEY_DELETE)
					{
						_GuiItemTable_delete(self, s, e);
					}
	}

	if (keyExtra & Win_EXTRAKEY_SHIFT)
	{
		if (keyExtra & Win_EXTRAKEY_RIGHT)
		{
			self->selectLastTouch.x = Std_min(N_COLS - 1, self->selectLastTouch.x + 1);
		}
		else
			if (keyExtra & Win_EXTRAKEY_LEFT)
			{
				self->selectLastTouch.x = Std_max(0, self->selectLastTouch.x - 1);
			}
			else
				if (keyExtra & Win_EXTRAKEY_UP)
				{
					self->selectLastTouch.y = Std_max(0, self->selectLastTouch.y - 1);
				}
				else
					if (keyExtra & Win_EXTRAKEY_DOWN)
					{
						self->selectLastTouch.y = Std_min(N_ROWS - 1, self->selectLastTouch.y + 1);
					}
					else
						if (keyExtra & Win_EXTRAKEY_HOME)
						{
							self->selectLastTouch.y = 0;
						}
						else
							if (keyExtra & Win_EXTRAKEY_END)
							{
								self->selectLastTouch.y = N_ROWS - 1;
							}
							else
								if (keyExtra & Win_EXTRAKEY_PAGE_U)
								{
									self->selectLastTouch.y = Std_max(0, self->selectLastTouch.y - N_ROWS_VISIBLE);
								}
								else
									if (keyExtra & Win_EXTRAKEY_PAGE_D)
									{
										self->selectLastTouch.y = Std_min(N_ROWS - 1, self->selectLastTouch.y + N_ROWS_VISIBLE);
									}
	}
	else
		if (keyExtra & Win_EXTRAKEY_RIGHT)
		{
			self->selectFirstTouch.x = Std_min(N_COLS - 1, self->selectFirstTouch.x + 1);
			self->selectLastTouch = self->selectFirstTouch;
		}
		else
			if (keyExtra & Win_EXTRAKEY_LEFT)
			{
				self->selectFirstTouch.x = Std_max(0, self->selectFirstTouch.x - 1);
				self->selectLastTouch = self->selectFirstTouch;
			}
			else
				if (keyExtra & Win_EXTRAKEY_UP)
				{
					self->selectFirstTouch.y = Std_max(0, self->selectFirstTouch.y - 1);
					self->selectLastTouch = self->selectFirstTouch;
				}
				else
					if (keyExtra & Win_EXTRAKEY_DOWN)
					{
						self->selectFirstTouch.y = Std_min(N_ROWS - 1, self->selectFirstTouch.y + 1);
						self->selectLastTouch = self->selectFirstTouch;
					}
					else
						if (keyExtra & Win_EXTRAKEY_HOME)
						{
							self->selectFirstTouch.y = 0;
							self->selectLastTouch = self->selectFirstTouch;
						}
						else
							if (keyExtra & Win_EXTRAKEY_END)
							{
								self->selectFirstTouch.y = N_ROWS - 1;
								self->selectLastTouch = self->selectFirstTouch;
							}
							else
								if (keyExtra & Win_EXTRAKEY_PAGE_U)
								{
									self->selectFirstTouch.y = Std_max(0, self->selectFirstTouch.y - N_ROWS_VISIBLE);
									self->selectLastTouch = self->selectFirstTouch;
								}
								else
									if (keyExtra & Win_EXTRAKEY_PAGE_D)
									{
										self->selectFirstTouch.y = Std_min(N_ROWS - 1, self->selectFirstTouch.y + N_ROWS_VISIBLE);
										self->selectLastTouch = self->selectFirstTouch;
									}

	if (keyExtra & Win_EXTRAKEY_SELECT_ALL)
	{
		//select all
		self->selectFirstTouch = Vec2i_init2(0, 0);
		self->selectLastTouch = Vec2i_init2(N_COLS - 1, N_ROWS - 1);
	}

	if (keyExtra & Win_EXTRAKEY_ENTER)
	{
		OsWinIO_resetKey();	//edit will not know about this 'enter'

		self->selectFirstTouch = self->selectLastTouch;

		Quad2i rect = _GuiItemTable_getSelectGridCoord(self, GuiItemTable_getSelectRect(self), TRUE);
		if (!Quad2i_isZero(rect))
		{
			GuiItem* it = GuiItem_findSubPos(&self->base, Quad2i_getMiddle(rect));
			if (it)
			{
				if (it->type == GuiItem_EDIT)
					GuiItemEdit_setCursor((GuiItemEdit*)it, TRUE);
				//else
				//if(it->type == GuiItem_TAGS)
				//	GuiItemTable_clickLinkDialog(it);
				else
					if (it->type == GuiItem_FILE)
						GuiItemFile_switchFullscreen((GuiItemFile*)it);
			}
		}
	}

	if (!Quad2i_cmp(selectRectBackup, GuiItemTable_getSelectRect(self)))
	{
		GuiItem_setRedraw(&self->base, TRUE);

		//move scroll
		if (self->selectLastTouch.y < WHEEL)
			GuiScroll_setWheelRow(&self->scroll, self->selectLastTouch.y);
		else
			if (self->selectLastTouch.y > WHEEL + N_ROWS_VISIBLE - 1)
				GuiScroll_setWheelRow(&self->scroll, self->selectLastTouch.y - (N_ROWS_VISIBLE - 1));
	}
}

GuiItemLayout* GuiItemTable_resize(GuiItemTable* self, GuiItemLayout* layout, Win* win)
{
	if (!self->base.resize)
		return (GuiItemLayout*)GuiItem_getSub(&self->base, 0);

	const int cell = OsWinIO_cellSize();

	GuiItem_freeSubs(&self->base);

	//layout(will hold table cells)
	layout = GuiItemLayout_newCoord(&self->base, FALSE, TRUE, win);
	GuiItemLayout_setDrawBackground(layout, FALSE);
	GuiItem_addSubName(&self->base, "layout_main", &layout->base);
	GuiItemLayout_setScrollH(layout, DbValue_initCopy(&self->scrollH));

	//cells will not be under scroll
	layout->base.coordScreen.size.x -= GuiScroll_widthWin(win);	//vertical

	const int N_COLS = GuiItemTable_numColumns(self);

	int width_sum = GuiItemTable_getColumnIdsWidth(self);
	BIG c;
	for (c = 0; c < N_COLS; c++)
		width_sum += GuiItemTable_getColumnWidth(self, c);
	const int header_x = 1 + self->showChangeOrder;	//[card_icon][order]
	const int header_y = self->showHeader;

	BOOL isFilter = !DbRoot_isType_table(self->viewRow);

	self->showAddRecord &= !isFilter;

	const int RS = GuiItemTable_getRowSize(self);

	Vec2i screenSize;
	OsScreen_getMonitorResolution(&screenSize);
	int N_ROWS_SCREEN = Std_max(1, screenSize.y / cell / RS);
	self->num_rows_max = N_ROWS_SCREEN;
	//self->num_rows_max = (self->base.coordMove.size.y - GuiItemTable_scrollExtra(self, win)) / cell / RS;
	//self->num_rows_max++;	//extra row

	BIG r;
	GuiItemLayout_addColumn(layout, self->showChangeOrder + 1, 2 + (self->showAddButton || self->showRemoveButton)); //row menus

	if (!isFilter)
	{
		//create new column
		GuiItemMenu* newColumn = (GuiItemMenu*)GuiItem_addSubName(&layout->base, "+column", GuiItemMenu_new(Quad2i_init4(header_x + width_sum, 0, 2, 1), DbValue_initStaticCopy(_UNI32("+")), FALSE));
		GuiItemMenu_setHighligthBackground(newColumn, TRUE);
		newColumn->base.drawTable = TRUE;
		newColumn->underline = FALSE;
		GuiItemMenu_setCloseAuto(newColumn, FALSE);
		GuiItemMenu_addItemIcon(newColumn, GuiImage_new1(UiIcons_init_column_file()), DbValue_initLang("COLUMN_NUMBER_1"), &GuiItemTable_clickAddColumn, FALSE, DbFormat_NUMBER_1);
		//GuiItemMenu_addItemIcon(newColumn, GuiImage_new1(UiIcons_init_column_file()), DbValue_initLang("COLUMN_NUMBER_N"), &GuiItemTable_clickAddColumn, FALSE, DbFormat_NUMBER_N);
		GuiItemMenu_addItemIcon(newColumn, GuiImage_new1(UiIcons_init_column_text()), DbValue_initLang("COLUMN_TEXT"), &GuiItemTable_clickAddColumn, FALSE, DbFormat_TEXT);
		GuiItemMenu_addItemIcon(newColumn, GuiImage_new1(UiIcons_init_column_link()), DbValue_initLang("COLUMN_LINK_1"), &GuiItemTable_clickAddColumn, FALSE, DbFormat_LINK_1);
		GuiItemMenu_addItemIcon(newColumn, GuiImage_new1(UiIcons_init_column_link()), DbValue_initLang("COLUMN_LINK_N"), &GuiItemTable_clickAddColumn, FALSE, DbFormat_LINK_N);
		GuiItemMenu_addItemIcon(newColumn, GuiImage_new1(UiIcons_init_column_file()), DbValue_initLang("COLUMN_FILE_1"), &GuiItemTable_clickAddColumn, FALSE, DbFormat_FILE_1);
		GuiItemMenu_addItemIcon(newColumn, GuiImage_new1(UiIcons_init_column_file()), DbValue_initLang("COLUMN_FILE_N"), &GuiItemTable_clickAddColumn, FALSE, DbFormat_FILE_N);
		GuiItemMenu_addItemIcon(newColumn, GuiImage_new1(UiIcons_init_column_currency()), DbValue_initLang("COLUMN_CURRENCY"), &GuiItemTable_clickAddColumn, FALSE, DbFormat_CURRENCY);
		GuiItemMenu_addItemIcon(newColumn, GuiImage_new1(UiIcons_init_column_percentage()), DbValue_initLang("COLUMN_PERCENTAGE"), &GuiItemTable_clickAddColumn, FALSE, DbFormat_PERCENTAGE);
		GuiItemMenu_addItemIcon(newColumn, GuiImage_new1(UiIcons_init_column_phone()), DbValue_initLang("COLUMN_PHONE"), &GuiItemTable_clickAddColumn, FALSE, DbFormat_PHONE);
		GuiItemMenu_addItemIcon(newColumn, GuiImage_new1(UiIcons_init_column_url()), DbValue_initLang("COLUMN_URL"), &GuiItemTable_clickAddColumn, FALSE, DbFormat_URL);
		GuiItemMenu_addItemIcon(newColumn, GuiImage_new1(UiIcons_init_column_email()), DbValue_initLang("COLUMN_EMAIL"), &GuiItemTable_clickAddColumn, FALSE, DbFormat_EMAIL);
		GuiItemMenu_addItemIcon(newColumn, GuiImage_new1(UiIcons_init_column_location()), DbValue_initLang("COLUMN_LOCATION"), &GuiItemTable_clickAddColumn, FALSE, DbFormat_LOCATION);
		GuiItemMenu_addItemIcon(newColumn, GuiImage_new1(UiIcons_init_column_rating()), DbValue_initLang("COLUMN_RATING"), &GuiItemTable_clickAddColumn, FALSE, DbFormat_RATING);
		GuiItemMenu_addItemIcon(newColumn, GuiImage_new1(UiIcons_init_column_date()), DbValue_initLang("COLUMN_DATE"), &GuiItemTable_clickAddColumn, FALSE, DbFormat_DATE);
		GuiItemMenu_addItemIcon(newColumn, GuiImage_new1(UiIcons_init_column_menu()), DbValue_initLang("COLUMN_MENU"), &GuiItemTable_clickAddColumn, FALSE, DbFormat_MENU);
		GuiItemMenu_addItemIcon(newColumn, GuiImage_new1(UiIcons_init_column_tags()), DbValue_initLang("COLUMN_TAGS"), &GuiItemTable_clickAddColumn, FALSE, DbFormat_TAGS);
		GuiItemMenu_addItemIcon(newColumn, GuiImage_new1(UiIcons_init_column_slider()), DbValue_initLang("COLUMN_SLIDER"), &GuiItemTable_clickAddColumn, FALSE, DbFormat_SLIDER);
		GuiItemMenu_addItemIcon(newColumn, GuiImage_new1(UiIcons_init_column_check()), DbValue_initLang("COLUMN_CHECK"), &GuiItemTable_clickAddColumn, FALSE, DbFormat_CHECK);

		if (N_COLS)
		{
			GuiItemMenu_addItemEmpty(newColumn);
			GuiItemMenu_addItem(newColumn, DbValue_initLang("NEW_RECORD"), &GuiItemTable_clickAddRecord);
		}
	}

	//num records
	{
		width_sum = GuiItemTable_getColumnIdsWidth(self);
		DbValue nrv = DbValue_initNumber(0);
		nrv.staticPost = Std_newUNI_char("x");
		GuiItem* numrecrods = GuiItem_addSubName(&layout->base, "#records", GuiItemText_new(Quad2i_init4(header_x, 0, width_sum, 1), TRUE, nrv, DbValue_initEmpty()));
		GuiItem_setChangeSize(numrecrods, TRUE, DbValue_initOption(self->viewRow, "width_ids", _UNI32("3")), FALSE);
	}

	//columns settings
	for (c = 0; c < N_COLS; c++)
	{
		int width = GuiItemTable_getColumnWidth(self, c);

		DbColumn* col = GuiItemTable_getColumn(self, c);
		BIG crow = GuiItemTable_getColumnRow(self, c);	//table/filter
		BIG crowT = DbColumn_getRow(col);	//table

		char crowName[64];
		snprintf(crowName, 64, "%lld_d", crow);

		GuiItemMenu* mn = (GuiItemMenu*)GuiItem_addSubName(&layout->base, crowName, GuiItemMenu_new(Quad2i_init4(header_x + width_sum, 0, width, 1), DbRows_getSubsOption(self->viewRow, "columns", TRUE, c, "name", TRUE), FALSE));
		GuiItemMenu_setHighligthBackground(mn, TRUE);
		mn->base.drawTable = TRUE;
		mn->textCenter = FALSE;
		GuiItem_setChangeSize(&mn->base, TRUE, DbValue_initOption(crow, "width", 0), FALSE);
		GuiItem_setDrop(&mn->base, "column", "column", TRUE, DbRows_getSubsArray(self->viewRow, "columns"), crow);

		GuiItemLayout* layColumn = GuiItemLayout_new(Quad2i_init());
		GuiItemLayout_addColumn(layColumn, 0, 5);
		GuiItemLayout_addColumn(layColumn, 1, 5);
		GuiItemLayout_addColumn(layColumn, 2, 5);

		GuiItem_setAttribute((GuiItem*)layColumn, "c", c);

		GuiItem* name = GuiItem_addSubName((GuiItem*)layColumn, "name", GuiItemEdit_new(Quad2i_init4(0, 0, 3, 1), DbRows_getSubsOption(self->viewRow, "columns", TRUE, c, "name", TRUE), DbValue_initLang("NAME")));

		int y = 1;

		DbFormatTYPE type = GuiItemTable_getColumnType(self, c);

		//this type
		GuiItemComboStatic* comboConvert = GuiItem_addSubName((GuiItem*)layColumn, "format", GuiItemComboStatic_newEx(Quad2i_init4(0, y, 3, 2), DbValue_initNumber(0), 0, DbValue_initLang("FORMAT"), &GuiItemTable_clickColumnConvert));
		y += 2;

		GuiItemComboStatic_addItemIcon(comboConvert, GuiImage_new1(GuiItem_getColumnIcon(type)), DbValue_initStaticCopy(Lang_find(DbColumnFormat_findColumnLang(type))));
		//convert types
		const int numConverts = DbColumnConvert_num(type);
		int co;
		for (co = 0; co < numConverts; co++)
		{
			const DbColumnConvert* cc = DbColumnConvert_get(type, co);
			if (cc)
				GuiItemComboStatic_addItemIcon(comboConvert, GuiImage_new1(GuiItem_getColumnIcon(cc->dstType)), DbValue_initStaticCopy(Lang_find(DbColumnFormat_findColumnLang(cc->dstType))));
		}

		y++;

		switch (type)
		{
			case DbFormat_NUMBER_1:
			case DbFormat_NUMBER_N:
			{
				GuiItem_setIcon(name, GuiImage_new1(UiIcons_init_column_number()));
				GuiItem_setIcon((GuiItem*)mn, GuiImage_new1(UiIcons_init_column_number()));

				GuiItem_addSubName((GuiItem*)layColumn, "precision", GuiItemEdit_newEx(Quad2i_init4(0, y, 2, 2), DbRows_getSubsOption(self->viewRow, "columns", TRUE, c, "precision", TRUE), DbValue_initLang("PRECISION"), &GuiItemTable_clickRebuild));

				y += 2;
				break;
			}

			case DbFormat_PERCENTAGE:
			{
				GuiItem_setIcon(name, GuiImage_new1(UiIcons_init_column_percentage()));
				GuiItem_setIcon((GuiItem*)mn, GuiImage_new1(UiIcons_init_column_percentage()));

				GuiItem_addSubName((GuiItem*)layColumn, "precision", GuiItemEdit_newEx(Quad2i_init4(0, y, 1, 2), DbRows_getSubsOption(self->viewRow, "columns", TRUE, c, "precision", TRUE), DbValue_initLang("PRECISION"), &GuiItemTable_clickRebuild));

				GuiItem_addSubName((GuiItem*)layColumn, "mult100", GuiItemCheck_newEx(Quad2i_init4(1, y, 2, 1), DbRows_getSubsOption(self->viewRow, "columns", TRUE, c, "mult100", TRUE), DbValue_initLang("MULTIPLAY_100"), &GuiItemTable_clickRebuild));
				GuiItem_addSubName((GuiItem*)layColumn, "bar", GuiItemCheck_newEx(Quad2i_init4(1, y + 1, 2, 1), DbRows_getSubsOption(self->viewRow, "columns", TRUE, c, "bar", TRUE), DbValue_initLang("PERCENTAGE_BACKGROUND"), &GuiItemTable_clickRebuild));
				y += 2;
				break;
			}

			case DbFormat_CURRENCY:
			{
				GuiItem_setIcon(name, GuiImage_new1(UiIcons_init_column_currency()));
				GuiItem_setIcon((GuiItem*)mn, GuiImage_new1(UiIcons_init_column_currency()));

				GuiItem_addSubName((GuiItem*)layColumn, "precision", GuiItemEdit_newEx(Quad2i_init4(0, y, 1, 2), DbRows_getSubsOption(self->viewRow, "columns", TRUE, c, "precision", TRUE), DbValue_initLang("PRECISION"), &GuiItemTable_clickRebuild));
				GuiItem_addSubName((GuiItem*)layColumn, "currency", GuiItemEdit_newEx(Quad2i_init4(1, y, 1, 2), DbRows_getSubsOption(self->viewRow, "columns", TRUE, c, "currency", TRUE), DbValue_initLang("CURRENCY"), &GuiItemTable_clickRebuild));
				y += 2;
				GuiItem_addSubName((GuiItem*)layColumn, "before", GuiItemCheck_newEx(Quad2i_init4(0, y++, 1, 1), DbRows_getSubsOption(self->viewRow, "columns", TRUE, c, "before", TRUE), DbValue_initLang("CURRENCY_BEFORE"), &GuiItemTable_clickRebuild));
				break;
			}

			case DbFormat_RATING:
			{
				GuiItem_setIcon(name, GuiImage_new1(UiIcons_init_column_rating()));
				GuiItem_setIcon((GuiItem*)mn, GuiImage_new1(UiIcons_init_column_rating()));

				GuiItem_addSubName((GuiItem*)layColumn, "num_stars", GuiItemEdit_newEx(Quad2i_init4(0, y, 1, 2), DbRows_getSubsOption(self->viewRow, "columns", TRUE, c, "numStars", TRUE), DbValue_initLang("NUMBER_STARS"), &GuiItemTable_clickRebuild));
				y += 2;
				break;
			}

			case DbFormat_MENU:
			case DbFormat_TAGS:
			{
				BIG optionsRow = DbRows_findSubType(crowT, "options");
				GuiItem_setIcon(name, GuiImage_new1(type == DbFormat_MENU ? UiIcons_init_column_menu() : UiIcons_init_column_tags()));
				GuiItem_setIcon((GuiItem*)mn, GuiImage_new1(type == DbFormat_MENU ? UiIcons_init_column_menu() : UiIcons_init_column_tags()));

				//+
				GuiItem_addSubName((GuiItem*)layColumn, "+", GuiItemButton_newClassicEx(Quad2i_init4(0, y, 3, 1), DbValue_initStaticCopyCHAR("+"), &GuiItemTable_clickAddOptionLine));
				y++;

				//skin
				GuiItemLayout* skin = GuiItemLayout_new(Quad2i_init4(0, 0, 1, 1));
				GuiItemLayout_addColumn(skin, 1, 100);

				GuiItem* drag = GuiItem_addSubName((GuiItem*)skin, "drag", GuiItemBox_newEmpty(Quad2i_init4(0, 0, 1, 1)));
				GuiItem_setIcon(drag, GuiImage_new1(UiIcons_init_reoder()));
				GuiItem_setDrop(drag, "option", "option", FALSE, DbRows_initLink(DbRoot_getColumnSubs(), optionsRow), -1);

				GuiItem_addSubName((GuiItem*)skin, "name", GuiItemEdit_new(Quad2i_init4(1, 0, 1, 1), DbValue_initOption(-1, "name", 0), DbValue_initEmpty()));

				//list
				GuiItemList* list = (GuiItemList*)GuiItem_addSubName((GuiItem*)layColumn, "list", GuiItemList_new(Quad2i_init4(0, y, 3, 5), DbRows_initLink(DbRoot_getColumnSubs(), optionsRow), (GuiItem*)skin, DbValue_initEmpty()));
				GuiItemList_setShowRemove(list, TRUE);
				GuiItemList_setClickRemove(list, &GuiItemTable_clickRemoveOption);

				y += 5;

				break;
			}

			case DbFormat_SLIDER:
			{
				GuiItem_setIcon(name, GuiImage_new1(UiIcons_init_column_slider()));
				GuiItem_setIcon((GuiItem*)mn, GuiImage_new1(UiIcons_init_column_slider()));

				//min & max & jump
				GuiItem_addSubName((GuiItem*)layColumn, "min", GuiItemEdit_newEx(Quad2i_init4(0, y, 1, 2), DbRows_getSubsOption(self->viewRow, "columns", TRUE, c, "min", TRUE), DbValue_initLang("MINIMUM"), &GuiItemTable_clickRebuild));
				GuiItem_addSubName((GuiItem*)layColumn, "max", GuiItemEdit_newEx(Quad2i_init4(1, y, 1, 2), DbRows_getSubsOption(self->viewRow, "columns", TRUE, c, "max", TRUE), DbValue_initLang("MAXIMUM"), &GuiItemTable_clickRebuild));
				GuiItem_addSubName((GuiItem*)layColumn, "jump", GuiItemEdit_newEx(Quad2i_init4(2, y, 1, 2), DbRows_getSubsOption(self->viewRow, "columns", TRUE, c, "jump", TRUE), DbValue_initLang("JUMP"), &GuiItemTable_clickRebuild));

				y += 2;
				break;
			}

			case DbFormat_CHECK:
			{
				GuiItem_setIcon(name, GuiImage_new1(UiIcons_init_column_check()));
				GuiItem_setIcon((GuiItem*)mn, GuiImage_new1(UiIcons_init_column_check()));

				y += 2;
				break;
			}

			case DbFormat_TEXT:
			{
				GuiItem_setIcon(name, GuiImage_new1(UiIcons_init_column_text()));
				GuiItem_setIcon((GuiItem*)mn, GuiImage_new1(UiIcons_init_column_text()));

				//password
				GuiItem_addSubName((GuiItem*)layColumn, "password", GuiItemCheck_newEx(Quad2i_init4(0, y++, 1, 1), DbRows_getSubsOption(self->viewRow, "columns", TRUE, c, "password", TRUE), DbValue_initLang("PASSWORD"), &GuiItemTable_clickRebuild));
				break;
			}

			case DbFormat_PHONE:
			{
				GuiItem_setIcon(name, GuiImage_new1(UiIcons_init_column_phone()));
				GuiItem_setIcon((GuiItem*)mn, GuiImage_new1(UiIcons_init_column_phone()));
				break;
			}

			case DbFormat_URL:
			{
				GuiItem_setIcon(name, GuiImage_new1(UiIcons_init_column_url()));
				GuiItem_setIcon((GuiItem*)mn, GuiImage_new1(UiIcons_init_column_url()));
				break;
			}

			case DbFormat_EMAIL:
			{
				GuiItem_setIcon(name, GuiImage_new1(UiIcons_init_column_email()));
				GuiItem_setIcon((GuiItem*)mn, GuiImage_new1(UiIcons_init_column_email()));
				break;
			}

			case DbFormat_LOCATION:
			{
				GuiItem_setIcon(name, GuiImage_new1(UiIcons_init_column_location()));
				GuiItem_setIcon((GuiItem*)mn, GuiImage_new1(UiIcons_init_column_location()));
				break;
			}

			case DbFormat_DATE:
			{
				GuiItem_setIcon(name, GuiImage_new1(UiIcons_init_column_date()));
				GuiItem_setIcon((GuiItem*)mn, GuiImage_new1(UiIcons_init_column_date()));

				GuiItem_addSubName((GuiItem*)layColumn, "time_format", GuiItemComboStatic_newEx(Quad2i_init4(0, y++, 2, 1), DbRows_getSubsOption(self->viewRow, "columns", TRUE, c, "timeFormat", TRUE), Lang_find("CALENDAR_FORMAT_TIME"), DbValue_initLang("TIME_FORMAT"), &GuiItemTable_clickRebuild));
				break;
			}

			case DbFormat_FILE_1:
			case DbFormat_FILE_N:
			{
				GuiItem_setIcon(name, GuiImage_new1(UiIcons_init_column_file()));
				GuiItem_setIcon((GuiItem*)mn, GuiImage_new1(UiIcons_init_column_file()));

				GuiItem_addSubName((GuiItem*)layColumn, "preview", GuiItemCheck_newEx(Quad2i_init4(0, y++, 2, 1), DbRows_getSubsOption(self->viewRow, "columns", TRUE, c, "preview", TRUE), DbValue_initLang("PREVIEW"), &GuiItemTable_clickRebuild));
				break;
			}

			case DbFormat_LINK_1:
			case DbFormat_LINK_N:
			{
				GuiItem_setIcon(name, GuiImage_new1(UiIcons_init_column_link()));
				GuiItem_setIcon((GuiItem*)mn, GuiImage_new1(UiIcons_init_column_link()));

				DbValue name = DbValue_initEmpty();
				DbTable* btable = DbColumn_getBTable(col);
				if (btable)
					name = DbValue_initOption(DbTable_getRow(btable), "name", 0);
				name.staticPre = Std_addUNI(Lang_find("SHOW"), _UNI32(" "));

				GuiItem_addSubName((GuiItem*)layColumn, "goto_table", GuiItemButton_newClassicEx(Quad2i_init4(0, y++, 2, 1), name, &GuiItemTable_clickGotoBtable));
				GuiItem_addSubName((GuiItem*)layColumn, "num_column_previews", GuiItemEdit_newEx(Quad2i_init4(0, y, 2, 2), DbRows_getSubsOption(self->viewRow, "columns", TRUE, c, "numColumnPreviews", TRUE), DbValue_initLang("COLUMN_PREVIEWS"), &GuiItemTable_clickRebuild));
				y += 2;

				//Move to table
				GuiItemMenu* mnMove = (GuiItemMenu*)GuiItem_addSubName((GuiItem*)layColumn, "column_move", GuiItemMenu_new(Quad2i_init4(0, y, 2, 1), DbValue_initLang("COLUMN_MOVE"), FALSE));
				GuiItemMenu_setCenter(mnMove, FALSE);
				GuiItemMenu_addItem(mnMove, DbValue_initLang("YES_IAM_SURE"), &GuiItemTable_clickMoveLink);

				//Add to column
				GuiItem_addSubName((GuiItem*)layColumn, "copy_into_column", GuiItemButton_newClassicEx(Quad2i_init4(2, y, 2, 1), DbValue_initLang("COPY_INTO_COLUMN"), &GuiItemTable_clickSendLinks));

				y++;

				//column to render ...
				//none = first ...
				break;
			}
			case DbFormat_ROW:
			{
				break;
			}
		}

		y++;
		GuiItem_addSubName((GuiItem*)layColumn, "lock", GuiItemCheck_new(Quad2i_init4(0, y++, 3, 1), DbRows_getSubsOption(self->viewRow, "columns", TRUE, c, "lock", FALSE), DbValue_initLang("LOCK")));
		GuiItem_addSubName((GuiItem*)layColumn, "hide", GuiItemButton_newNoCenterEx(Quad2i_init4(0, y++, 3, 1), DbValue_initLang("HIDE"), &GuiItemTable_clickHideColumn));
		GuiItem_addSubName((GuiItem*)layColumn, "duplicate", GuiItemButton_newNoCenterEx(Quad2i_init4(0, y++, 3, 1), DbValue_initLang("DUPLICATE"), &GuiItemTable_clickDuplicateColumn));

		GuiItemMenu* mnRemove = (GuiItemMenu*)GuiItem_addSubName((GuiItem*)layColumn, "remove", GuiItemMenu_new(Quad2i_init4(0, y++, 3, 1), DbValue_initLang("REMOVE"), FALSE));
		GuiItemMenu_setCenter(mnRemove, FALSE);
		GuiItemMenu_addItem(mnRemove, DbValue_initLang("YES_IAM_SURE"), &GuiItemTable_clickRemoveColumn);

		GuiItemMenu_setContext(mn, layColumn);

		width_sum += width;
	}

	//order
	if (self->showChangeOrder)
	{
		self->subs_start_rowOptionOrder = layout->base.subs.num;
		for (r = 0; r < N_ROWS_SCREEN; r++)
		{
			char nameId[64];
			snprintf(nameId, 64, "%lld_o", r);

			GuiItem* drag = GuiItem_addSubName(&layout->base, nameId, GuiItemBox_newEmpty(Quad2i_init4(0, header_y + r * RS, 1, RS)));
			GuiItem_setIcon(drag, GuiImage_new1(UiIcons_init_reoder()));
			GuiItem_setDrop(drag, self->changeOrderNameSrc, self->changeOrderNameDst, FALSE, DbRows_initCopy(&self->filter), -1);
			drag->dropDontRemove = TRUE;
		}
	}

	//cards
	self->subs_start_rowOptionCards = layout->base.subs.num;
	for (r = 0; r < N_ROWS_SCREEN; r++)
	{
		char nameId[64];
		snprintf(nameId, 64, "%lld_k", r);

		const Quad2i grid = Quad2i_init4(self->showChangeOrder, header_y + r * RS, 1, RS);

		//card button
		GuiItemButton* b = GuiItem_addSubName(&layout->base, nameId, GuiItemButton_newImage(grid, GuiImage_new1(UiIcons_init_card()), &GuiItemTable_clickShowPage));
		b->imageSpace = cell / 10;
	}

	width_sum = GuiItemTable_getColumnIdsWidth(self);
	//rows ids(first column)
	self->subs_start_rowOption = layout->base.subs.num;
	for (r = 0; r < N_ROWS_SCREEN; r++)
	{
		char nameId[64];
		snprintf(nameId, 64, "%lld_i", r);

		const Quad2i grid = Quad2i_init4(self->showChangeOrder + 1, header_y + r * RS, width_sum, RS);
		if (self->showAddButton)
		{
			GuiItem_addSubName(&layout->base, nameId, GuiItemButton_newClassicEx(grid, DbValue_initLang("ADD"), &UiDialogLink_clickLinkDialogAddOne));
		}
		else
			if (self->showRemoveButton)
			{
				GuiItem_addSubName(&layout->base, nameId, GuiItemButton_newClassicEx(grid, DbValue_initStaticCopy(_UNI32("X")), &GuiItemTable_clickRemoveRow));
			}
			else
			{
				GuiItemMenu* mn = (GuiItemMenu*)GuiItem_addSubName(&layout->base, nameId, GuiItemMenu_new(grid, DbValue_initGET(DbTable_getIdsColumn(GuiItemTable_getTable(self)), -1), FALSE));
				GuiItemMenu_addItem(mn, DbValue_initLang("CARD"), &GuiItemTable_clickShowPage);
				GuiItemMenu_addItem(mn, DbValue_initLang("REMOVE"), &GuiItemTable_clickRemoveRow);
				GuiItemMenu_setHighligthBackground(mn, TRUE);
				mn->base.drawTable = TRUE;
			}
	}

	//cells
	self->subs_start_rowCell = layout->base.subs.num;
	//width_sum = 0;
	for (c = 0; c < N_COLS; c++)
	{
		const int width = GuiItemTable_getColumnWidth(self, c);
		for (r = 0; r < N_ROWS_SCREEN; r++)
		{
			char nameId[64];
			snprintf(nameId, 64, "%lld_%lld_rec", c, r);

			Quad2i q = Quad2i_init4(header_x + width_sum, header_y + r * RS, width, RS);

			GuiItem* itt = GuiItem_addSubName(&layout->base, nameId, GuiItemTable_getCardSkinItem(q, DbRows_getSubsCell(self->viewRow, "columns", TRUE, c, -1), FALSE, FALSE));
			itt->drawTable = TRUE;
		}

		width_sum += width;
	}
	self->subs_end_rowCell = layout->base.subs.num;

	if (self->showAddRecord)
	{
		self->subs_start_add = layout->base.subs.num;
		GuiItemButton* b = (GuiItemButton*)GuiItem_addSubName(&layout->base, "+record", GuiItemButton_newClassicEx(Quad2i_init4(0, self->num_rows_max, 1, 1), DbValue_initStaticCopyCHAR("+"), &GuiItemTable_clickAddRecord));
		GuiItemButton_setCircle(b, TRUE);
		GuiItem_setShow(&b->base, FALSE);	//update will set it visible
	}

	return layout;
}
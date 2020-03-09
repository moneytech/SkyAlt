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

typedef struct GuiItemTable_s
{
	GuiItem base;
	BIG viewRow;
	DbRows filter;

	DbValue searchHighlight;

	BIG oldNumRows;

	UBIG num_rows_max;		//visible
	UBIG num_rows_real;

	UBIG subs_ids_start_rowOptionOrder;
	UBIG subs_ids_start_rowOptionCards;
	UBIG subs_ids_start_rowOption;
	UBIG subs_ids_add_record;

	UBIG subs_cells_start;
	UBIG subs_cells_end;

	GuiScroll scrollV;

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

	DbColumn* choosenSourceColumn;

	BOOL showAddButton;
	BOOL showRemoveButton;

	char* changeOrderNameDst;
	char* changeOrderNameSrc;

	DbValue scrollH;
}GuiItemTable;

char* _GuiItemTable_getNameId(char* nameId, const char* format, BIG value)
{
	snprintf(nameId, 64, format, value);
	return nameId;
}

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

BIG GuiItemTable_getColumnBaseRow(const GuiItemTable* self, UBIG i)
{
	return DbColumn_getRow(GuiItemTable_getColumn(self, i));
}

UBIG GuiItemTable_numRows(GuiItemTable* self)
{
	return DbRows_getSize(&self->filter);
}

static GuiItemLayout* _GuiItemTable_getIdsLayout(const GuiItemTable* self)
{
	return GuiItem_findName((GuiItem*)self, "ids_layout");
}
static GuiItemLayout* _GuiItemTable_getCellsLayout(const GuiItemTable* self)
{
	return GuiItem_findName((GuiItem*)self, "cells_layout");
}

BOOL GuiItemTable_repairSelect(GuiItemTable* self)
{
	const BIG NUM_COLS = GuiItemTable_numColumns(self);
	const BIG NUM_ROWS = GuiItemTable_numRows(self);

	const BOOL loaded = DbTable_isLoaded(GuiItemTable_getTable(self));

	BOOL ok = (loaded && NUM_COLS > 0 && NUM_ROWS > 0);

	self->selectFirstTouch.x = Std_clamp(self->selectFirstTouch.x, 0, NUM_COLS ? NUM_COLS - 1 : 0);
	self->selectLastTouch.x = Std_clamp(self->selectLastTouch.x, 0, NUM_COLS ? NUM_COLS - 1 : 0);

	if (loaded)
	{
		self->selectFirstTouch.y = Std_clamp(self->selectFirstTouch.y, 0, NUM_ROWS ? NUM_ROWS - 1 : 0);
		self->selectLastTouch.y = Std_clamp(self->selectLastTouch.y, 0, NUM_ROWS ? NUM_ROWS - 1 : 0);
	}
	return ok;
}

GuiItemTable* GuiItemTable_new(Quad2i grid, BIG viewRow, DbRows filter, BOOL showHeader, BOOL showAddRecord, DbValue scrollV, DbValue scrollH, DbValue selectGrid, DbValue searchHighlight)
{
	GuiItemTable* self = Os_malloc(sizeof(GuiItemTable));
	self->base = GuiItem_init(GuiItem_TABLE, grid);

	self->viewRow = viewRow;
	self->filter = filter;
	DbRows_hasChanged(&self->filter);

	self->searchHighlight = searchHighlight;

	self->oldNumRows = -1;

	self->num_rows_max = 0;
	self->num_rows_real = 0;

	self->subs_ids_start_rowOptionOrder = 0;
	self->subs_ids_start_rowOptionCards = 0;
	self->subs_ids_start_rowOption = 0;
	self->subs_ids_add_record = 0;
	self->subs_cells_start = 0;
	self->subs_cells_end = 0;

	self->scrollV = GuiScroll_init(scrollV);
	self->scrollH = scrollH;

	self->showChangeOrder = FALSE;
	self->showHeader = showHeader;
	self->showAddRecord = showAddRecord;

	self->drawBorder = FALSE;

	self->selectedRow = -1;
	self->selectActive = FALSE;

	self->cellExtendRect = Quad2i_init();
	self->cellExtendActive = FALSE;

	//self->rowSize = DbValue_initOption(tableRow, "height", 0);

	self->choosenSourceColumn = 0;

	//self->group = DbRows_initEmpty();

	self->showAddButton = FALSE;
	self->showRemoveButton = FALSE;

	self->changeOrderNameDst = 0;
	self->changeOrderNameSrc = 0;

	self->selectGrid = selectGrid;
	DbValue_hasChanged(&self->selectGrid);

	char gridC[64];
	Std_copyCHAR_uni(gridC, 64, DbValue_result(&self->selectGrid));
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
	self->searchHighlight = DbValue_initCopy(&src->searchHighlight);

	//self->group = DbRows_initCopy(&src->group);

	self->oldNumRows = -1;
	self->scrollV = GuiScroll_initCopy(&src->scrollV);
	self->scrollH = DbValue_initCopy(&src->scrollH);

	self->selectGrid = DbValue_initCopy(&src->selectGrid);

	self->changeOrderNameDst = Std_newCHAR(src->changeOrderNameDst);
	self->changeOrderNameSrc = Std_newCHAR(src->changeOrderNameSrc);

	return (GuiItem*)self;
}

BOOL GuiItemTable_isSummaryTable(const GuiItemTable* self)
{
	return DbTable_isSummary(GuiItemTable_getTable(self));
}

BOOL GuiItemTable_isColumnLocked(const GuiItemTable* self, UBIG c)
{
	return DbRows_getSubsOptionNumber(self->viewRow, "columns", TRUE, c, "lock", FALSE) != 0;
}

int GuiItemTable_getColumnIdsWidth(const GuiItemTable* self)
{
	DbValue v = DbValue_initOption(self->viewRow, "width_ids", _UNI32("4"));
	int width = DbValue_getNumber(&v);
	DbValue_free(&v);
	return Std_bmax(1, width);
}

int GuiItemTable_getColumnWidth(const GuiItemTable* self, UBIG c)
{
	return Std_bmax(1, DbRows_getSubsOptionNumber(self->viewRow, "columns", TRUE, c, "width", FALSE));
}

DbFormatTYPE GuiItemTable_getColumnType(GuiItemTable* self, UBIG c)
{
	return DbColumnFormat_findColumn(GuiItemTable_getColumn(self, c));
}
BOOL GuiItemTable_isColumnRemote(GuiItemTable* self, UBIG c)
{
	return DbColumn_isRemote(GuiItemTable_getColumn(self, c));
}

void GuiItemTable_delete(GuiItemTable* self)
{
	DbRows_free(&self->filter);
	DbValue_free(&self->searchHighlight);

	Std_deleteCHAR(self->changeOrderNameDst);
	Std_deleteCHAR(self->changeOrderNameSrc);

	DbValue_free(&self->scrollH);
	GuiScroll_free(&self->scrollV);

	DbValue_free(&self->selectGrid);

	GuiItem_free(&self->base);
	Os_free(self, sizeof(GuiItemTable));
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
	int rs = DbValue_getOptionNumber(self->viewRow, "height", 0);
	rs += 1;

	if (rs >= 2)
		rs *= 2;
	return rs;
}

Quad2i GuiItemTable_getSelectRectEx(Vec2i first, Vec2i last)
{
	Quad2i q;
	q.start = first;
	q.size = Vec2i_sub(last, first);

	q = Quad2i_repair(q, 0);
	q.size.x++;
	q.size.y++;

	return q;
}

Quad2i GuiItemTable_getSelectRect(const GuiItemTable* self)
{
	return GuiItemTable_getSelectRectEx(self->selectFirstTouch, self->selectLastTouch);
}

static int GuiItemTable_scrollExtra(const GuiItemTable* self, Win* win)
{
	return (self->showHeader) * OsWinIO_cellSize();
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

	GuiItemRoot_closeLevels();
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

	GuiItemRoot_closeLevels();
}

void GuiItemTable_clickNewColumnLinkMirroredSET(GuiItem* self)
{
	GuiItemTable* table = GuiItem_findParentType(self->parent, GuiItem_TABLE);
	if (table && self->type == GuiItem_BUTTON)
	{
		BIG dstColumnRow = ((GuiItemButton*)self)->text.row;
		DbColumn* dstColumn = DbRoot_findColumn(dstColumnRow);
		if (dstColumn)
		{
			DbColumn* newColumn = DbTable_createColumnFormat(GuiItemTable_getTable(table), DbFormat_LINK_MIRRORED, 0, DbColumn_getTable(dstColumn));

			//set mirror column
			BIG mirrorRow = DbRows_findOrCreateSubType(DbColumn_getRow(newColumn), "mirror");
			DbColumn1_set(DbRoot_ref(), mirrorRow, dstColumnRow);
		}
	}

	GuiItemRoot_closeLevels();
}

void GuiItemTable_clickNewColumnLinkFilteredSET(GuiItem* self)
{
	GuiItemTable* table = GuiItem_findParentType(self->parent, GuiItem_TABLE);
	if (table && self->type == GuiItem_BUTTON)
	{
		BIG dstColumnRow = ((GuiItemButton*)self)->text.row;
		DbColumn* dstColumn = DbRoot_findColumn(dstColumnRow);
		if (dstColumn)
		{
			DbColumn* newColumn = DbTable_createColumnFormat(GuiItemTable_getTable(table), DbFormat_LINK_FILTERED, 0, DbColumn_getTable(dstColumn));

			//set column
			BIG sourceRow = DbRows_findOrCreateSubType(DbColumn_getRow(newColumn), "source_column");
			DbColumn1_set(DbRoot_ref(), sourceRow, dstColumnRow);
		}
	}

	GuiItemRoot_closeLevels();
}

void GuiItemTable_clickNewColumnLinkJointedSET(GuiItem* self)
{
	GuiItemTable* table = GuiItem_findParentType(self->parent, GuiItem_TABLE);
	if (table && self->type == GuiItem_BUTTON)
	{
		DbTable* btable = DbRoot_findParentTable(((GuiItemButton*)self)->text.row);
		if (btable)
			DbTable_createColumnFormat(GuiItemTable_getTable(table), DbFormat_LINK_JOINTED, 0, btable);
	}

	GuiItemRoot_closeLevels();
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
			GuiItemLayout* layout = GuiStruct_create(GuiStruct_SHOW_TABLES, DbTable_getRow(tab), DbValue_initLang("TABLES"), (format == DbFormat_LINK_1 ? &GuiItemTable_clickNewColumnLinkSET_1 : &GuiItemTable_clickNewColumnLinkSET_N), -1, -1, -1, FALSE);
			GuiItemRoot_addDialogRelLayout(layout, self, self->coordMove, TRUE);
			close = FALSE;
		}
		else
			if (format == DbFormat_LINK_MIRRORED)
			{
				GuiItemLayout* layout = GuiStruct_create(GuiStruct_SHOW_COLUMNS, DbTable_getRow(tab), DbValue_initLang("COLUMNS"), &GuiItemTable_clickNewColumnLinkMirroredSET, -1, DbTable_getRow(tab), -1, FALSE);
				GuiItemRoot_addDialogRelLayout(layout, self, self->coordMove, TRUE);
				close = FALSE;
			}
			else
				if (format == DbFormat_LINK_FILTERED)
				{
					GuiItemLayout* layout = GuiStruct_create(GuiStruct_SHOW_COLUMNS, DbTable_getRow(tab), DbValue_initLang("COLUMNS"), &GuiItemTable_clickNewColumnLinkFilteredSET, DbTable_getRow(tab), -1, -1, TRUE);
					GuiItemRoot_addDialogRelLayout(layout, self, self->coordMove, TRUE);
					close = FALSE;
				}
				else
					if (format == DbFormat_LINK_JOINTED)
					{
						GuiItemLayout* layout = GuiStruct_create(GuiStruct_SHOW_TABLES, DbTable_getRow(tab), DbValue_initLang("TABLES"), &GuiItemTable_clickNewColumnLinkJointedSET, -1, -1, -1, FALSE);
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

BIG _GuiItemTable_addOptionLine(BIG parentRow, Rgba cd)
{
	DbRows rows = DbRows_initLinkN(DbRoot_subs(), parentRow);
	BIG r = DbRows_addNewRow(&rows);

	DbValue_setOptionNumber(r, "cd", Rgba_asNumber(cd));

	DbRows_free(&rows);
	return r;
}
void GuiItemTable_clickAddOptionLine(GuiItem* self)
{
	GuiItemTable* table = GuiItem_findParentType(self->parent, GuiItem_TABLE);
	if (table)
	{
		BIG c = GuiItem_findAttribute(self, "c");
		BIG crowT = GuiItemTable_getColumnBaseRow(table, c);

		BIG optionsRow = DbRows_findSubType(crowT, "options");

		BIG cdn = GuiItem_findAttribute(self, "cd");
		if (cdn >= 0)
			_GuiItemTable_addOptionLine(optionsRow, Rgba_initFromNumber(cdn));
	}
}

void GuiItemTable_clickInsertItemToOptionLineSET(GuiItem* self)
{
	GuiItemTable* table = GuiItem_findParentType(self->parent, GuiItem_TABLE);
	if (table && self->type == GuiItem_BUTTON)
	{
		BIG c = GuiItem_findAttribute(self, "c");
		BIG crowT = GuiItemTable_getColumnBaseRow(table, c);

		BIG pathRow = DbRows_findOrCreateSubType(crowT, "path");

		DbTable* btable = DbRoot_findParentTable(((GuiItemButton*)self)->text.row);
		if (btable && pathRow >= 0)
		{
			BIG pos = GuiItem_findAttribute(self, "pos");
			BIG r = DbRoot_addSubRow(pathRow, pos);

			BIG propRow = DbRoot_findOrCreateChildType(r, "btable");
			DbColumn1_set(DbRoot_ref(), propRow, DbTable_getRow(btable));
		}
	}
}

void GuiItemTable_clickRemoteItem(GuiItem* self)
{
	BIG row = GuiItem_findAttribute(self, "row");
	DbRoot_removeRow(row);
}

void GuiItemTable_clickInsertItemToOptionLine(GuiItem* self)
{
	GuiItemTable* table = GuiItem_findParentType(self->parent, GuiItem_TABLE);
	BIG c = GuiItem_findAttribute(self, "c");
	BIG crowT = GuiItemTable_getColumnBaseRow(table, c);
	//BIG pos = GuiItem_findAttribute(self, "pos");

	GuiItemLayout* layout = GuiStruct_create(GuiStruct_SHOW_TABLES, DbRoot_findParentTableRow(crowT), DbValue_initLang("TABLES"), &GuiItemTable_clickInsertItemToOptionLineSET, -1, -1, -1, FALSE);
	GuiItemRoot_addDialogRelLayout(layout, self, self->coordMove, TRUE);
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

				GuiItemLayout* layout = GuiStruct_create(GuiStruct_SHOW_COLUMNS, columnSrcRow, DbValue_initLang("COLUMNS"), &GuiItemTable_clickSendLinksSET, DbTable_getRow(GuiItemTable_getTable(table)), DbTable_getRow(btable), columnSrcRow, FALSE);
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

		const DbColumnConvert* cc = DbColumnConvert_get(type, co, GuiItemTable_isColumnRemote(table, c));
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
		const DbColumnConvert* cc = DbColumnConvert_get(type, co, GuiItemTable_isColumnRemote(table, c));
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
		const DbColumnConvert* cc = DbColumnConvert_get(type, co, GuiItemTable_isColumnRemote(table, c));
		if (cc)
		{
			DbColumn* bTableColumn = 0;
			if (cc->dstType == DbFormat_LINK_1 || cc->dstType == DbFormat_LINK_N)
			{
				if (cc->srcType != DbFormat_LINK_1 && cc->srcType != DbFormat_LINK_N)
				{
					GuiItem_setAttribute(self, "dstType", co);
					GuiItemLayout* layout = GuiStruct_create(GuiStruct_SHOW_COLUMNS, GuiItemTable_getColumnBaseRow(table, c), DbValue_initLang("COLUMNS"), &GuiItemTable_clickColumnConvertLINK, -1, -1, -1, FALSE);
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

void GuiItemTable_clickShortColumn(GuiItem* self)
{
	GuiItemTable* table = GuiItem_findParentType(self->parent, GuiItem_TABLE);
	if (table && self->type == GuiItem_BUTTON)
	{
		BIG c = GuiItem_findAttribute(self, "c");

		DbRows shortRows = DbRows_initLinkN(DbRoot_subs(), DbRows_findSubType(table->viewRow, "short"));

		BIG r = DbRows_addNewRow(&shortRows);
		DbColumn1_set(DbRoot_ref(), r, GuiItemTable_getColumnBaseRow(table, c));
		DbValue_setOptionNumber(r, "ascending", 0);	//0 is "a->z"

		DbRows_free(&shortRows);
	}

	GuiItem_closeParentLevel(self);

	GuiItemMenu* shortt = GuiItem_findNameType(self->parent, "short_menu", GuiItem_MENU);
	if (shortt)
		GuiItemMenu_showContext(shortt);
}

void GuiItemTable_clickFilterColumn(GuiItem* self)
{
	GuiItemTable* table = GuiItem_findParentType(self->parent, GuiItem_TABLE);
	if (table && self->type == GuiItem_BUTTON)
	{
		BIG c = GuiItem_findAttribute(self, "c");

		DbRows selectRows = DbRows_initLinkN(DbRoot_subs(), DbRows_findSubType(table->viewRow, "select"));
		BIG r = DbRows_addNewRow(&selectRows);	//new filter line

		//set line properties, in this case "column"
		BIG rr = DbRoot_findOrCreateChildType(r, "column");
		DbColumn1_set(DbRoot_ref(), rr, GuiItemTable_getColumnBaseRow(table, c));

		DbRows_free(&selectRows);
	}

	GuiItem_closeParentLevel(self);

	GuiItemMenu* filterr = GuiItem_findNameType(self->parent, "filter_menu", GuiItem_MENU);
	if (filterr)
		GuiItemMenu_showContext(filterr);
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

void GuiItemTable_clickAddRecord(GuiItem* item)
{
	GuiItemTable* self = GuiItem_findParentType(item->parent, GuiItem_TABLE);
	if (self)
	{
		DbRows_addNewRow(&self->filter);

		const UBIG N_ROWS = GuiItemTable_numRows(self);
		self->selectFirstTouch = self->selectLastTouch = Vec2i_init2(0, N_ROWS - 1);
		GuiScroll_setWheelRow(&self->scrollV, N_ROWS - 1);
	}
}

void GuiItemTable_clickRemoveRow(GuiItem* self)
{
	GuiItemTable* table = GuiItem_findParentType(self->parent, GuiItem_TABLE);
	if (table && self->type == GuiItem_BUTTON)
		DbRows_removeRow(&table->filter, GuiItem_getRow(self));
}

void GuiItemTable_clickRemoveRows(GuiItem* self)
{
	GuiItemTable* table = GuiItem_findParentType(self->parent, GuiItem_TABLE);
	if (table)
	{
		DbTable* tab = GuiItemTable_getTable(table);

		Quad2i selectRect = GuiItemTable_getSelectRect(table);
		Vec2i st = selectRect.start;
		Vec2i e = Quad2i_end(selectRect);

		BIG y;
		for (y = e.y - 1; y >= st.y; y--)
			//DbTable_removeRow(tab, DbRows_getRow(&table->filter, y))
			DbRows_removeRowDirect(tab, DbRows_getRow(&table->filter, y));
	}
}

static Quad2i _GuiItemTable_getSelectGridCoordBase(const GuiItemTable* self, Quad2i q, BOOL noScroll)
{
	Quad2i ret = Quad2i_init();

	const GuiItem* layoutCells = (GuiItem*)_GuiItemTable_getCellsLayout(self);
	if (layoutCells)
	{
		const GuiItem* it = GuiItem_getSubConst(layoutCells, self->subs_cells_start);
		if (it)
		{
			Quad2i firstCell = noScroll ? it->coordMove : it->coordScreen;

			//x
			const int N_ROWS_VISIBLE = self->num_rows_max;
			BIG x;
			for (x = 0; x < Quad2i_end(q).x; x++)
			{
				//const int s = GuiItem_getSubConst(layoutCells, self->subs_cells_start + x * N_ROWS_VISIBLE)->coordMove.size.x; //coordMove;
				const int s = GuiItemTable_getColumnWidth(self, x) * OsWinIO_cellSize();
				if (x < q.start.x)
					ret.start.x += s;
				else
					ret.size.x += s;
			}

			//y
			const int s = firstCell.size.y;
			const BIG WHEEL = GuiScroll_getWheelRow(&self->scrollV);
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
static Quad2i _GuiItemTable_getSelectGridCoord(const GuiItemTable* self, Quad2i q, BOOL noScroll)
{
	Quad2i ret = _GuiItemTable_getSelectGridCoordBase(self, q, noScroll);

	Quad2i coordLayout = _GuiItemTable_getCellsLayout(self)->base.coordMoveCut;
	ret = Quad2i_getIntersect(ret, coordLayout);	//cut it
	return ret;
}

static BOOL _GuiItemTable_getTouchCellPos(const GuiItemTable* self, Vec2i touch_pos, BOOL noScroll, Vec2i* out_pos)
{
	const GuiItem* layoutCells = (GuiItem*)_GuiItemTable_getCellsLayout(self);
	const UBIG WHEEL = GuiScroll_getWheelRow(&self->scrollV);
	const int N_COLS = GuiItemTable_numColumns(self);
	const int N_ROWS_VISIBLE = self->num_rows_max;
	const int N_ROWS_REAL = self->num_rows_real;

	BIG x, y;
	for (y = 0; y < N_ROWS_REAL; y++)
	{
		for (x = 0; x < N_COLS; x++)
		{
			const GuiItem* it = GuiItem_getSubConst(layoutCells, self->subs_cells_start + x * N_ROWS_VISIBLE + y);
			if (Quad2i_inside(noScroll ? it->coordMove : it->coordMoveCut, touch_pos))
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

	const GuiItem* layoutCells = (GuiItem*)_GuiItemTable_getCellsLayout(self);
	const UBIG WHEEL = GuiScroll_getWheelRow(&self->scrollV);
	const int N_COLS = GuiItemTable_numColumns(self);
	const int N_ROWS_VISIBLE = self->num_rows_max;
	const int N_ROWS_REAL = self->num_rows_real;

	BIG x, y;
	for (y = 0; y < N_ROWS_REAL; y++)
	{
		for (x = 0; x < N_COLS; x++)
		{
			Quad2i itCoord = GuiItem_getSubConst(layoutCells, self->subs_cells_start + x * N_ROWS_VISIBLE + y)->coordMoveCut;
			float dist = Quad2i_inside(itCoord, touch_pos) ? 0 : Vec2i_distance(touch_pos, Quad2i_getMiddle(itCoord));
			if (dist < r)
			{
				r = dist;
				*out_pos = Vec2i_init2(x, y + WHEEL);

				if (dist == 0)
				{
					printf("[%d %d] %f %d %lld, [%lld %lld]\n", touch_pos.x, touch_pos.y, r, out_pos->y, WHEEL, x, y);
					Quad2i_print(itCoord, "itCoord");
				}
			}
		}
	}

	return (r < max_r);
}

static BOOL _GuiItemTable_isSelect(const GuiItemTable* self, Win* win)
{
	Quad2i selectRect = GuiItemTable_getSelectRect(self);
	Quad2i g = _GuiItemTable_getSelectGridCoord(self, selectRect, TRUE);
	return (g.size.x > 0 && g.size.y > 0);
}

static BOOL _GuiItemTable_isCellExtendSelect(const GuiItemTable* self, Win* win)
{
	Quad2i g = _GuiItemTable_getSelectGridCoord(self, self->cellExtendRect, TRUE);
	return _GuiItemTable_isSelect(self, win) && (g.size.x > 0 && g.size.y > 0);
}

static Quad2i _GuiItemTable_getCellExtendQuad(const GuiItemTable* self, Quad2i grid, Win* win)
{
	const int cell = OsWinIO_cellSize();

	Quad2i q = _GuiItemTable_getSelectGridCoord(self, grid, TRUE);
	return Quad2i_initMid(Quad2i_end(q), Vec2i_init2(cell / 4, cell / 4));
}

/*static GuiItem* _GuiItemTable_getBaseLayout(GuiItemTable* self)
{
	return GuiItem_getSub(&self->base, 0);
}*/

void GuiItemTable_scrollToCell(GuiItemTable* self, Vec2i cell)
{
	const UBIG WHEEL = GuiScroll_getWheelRow(&self->scrollV);
	const int N_COLS = GuiItemTable_numColumns(self);
	const int N_ROWS_VISIBLE = self->num_rows_max;

	//Vertical
	if (cell.y < WHEEL)
		GuiScroll_setWheelRow(&self->scrollV, cell.y);
	else
		if (cell.y > WHEEL + N_ROWS_VISIBLE - 1)
			GuiScroll_setWheelRow(&self->scrollV, cell.y - (N_ROWS_VISIBLE - 3));

	//Horizontal
	GuiItemLayout* lay = _GuiItemTable_getCellsLayout(self);
	Quad2i coord = lay->base.coordScreen;
	if (GuiItemLayout_hasScrollH(lay))
	{
		Quad2i rect = _GuiItemTable_getSelectGridCoordBase(self, Quad2i_init2(cell, Vec2i_init2(1, 1)), TRUE);

		//left
		if (rect.start.x < coord.start.x)
			GuiScroll_setWheel(&lay->scrollH, GuiScroll_getWheel(&lay->scrollH) - (coord.start.x - rect.start.x));

		//right
		if (rect.start.x + rect.size.x > coord.start.x + coord.size.x)
			GuiScroll_setWheel(&lay->scrollH, GuiScroll_getWheel(&lay->scrollH) + (rect.start.x + rect.size.x) - (coord.start.x + coord.size.x));
	}



/*	const BIG Y = GuiScroll_getWheel(&self->scrollV);

	if (cell.y < Y || cell.y > Y + self->num_rows_max - 2)
		GuiScroll_setWheelRow(&self->scrollV, cell.y);*/




	/*for (c = 0; c < N_COLS; c++)
	{
		DbFormatTYPE type = GuiItemTable_getColumnType(self, c);
		int width = GuiItemTable_getColumnWidth(self, c);

	GuiItemLayout* cells = _GuiItemTable_getCellsLayout(self);
	const BIG X = GuiItemLayout_getWheelH(cells);
	int W = cells->base.coordScreen.size.x;
	cell.x *= OsWinIO_cellSize();
	if (cell.x < X || cell.x > X + W)
		GuiItemLayout_setWheelH(cells, cell.x);*/
}

BOOL GuiItemTable_findForward(GuiItemTable* self, DbValues* columns, Vec2i startPos, const UNI* findStr, BOOL onlyReplaceColumns)
{
	const UBIG N = DbRows_getSize(&self->filter);
	BIG x = startPos.x;
	BIG y;
	for (y = startPos.y; y < N; y++)
	{
		DbValues_setRow(columns, DbRows_getRow(&self->filter, y));

		for (; x < columns->num; x++)
		{
			DbValue* v = &columns->values[x];

			if (!onlyReplaceColumns || DbColumn_isFindAndReplace(v->column))
			{
				DbValue_hasChanged(v);
				if (Std_subUNI(v->result.str, findStr) >= 0)
				{
					self->selectFirstTouch = self->selectLastTouch = Vec2i_init2(x, y);
					return TRUE;
				}
			}
		}
		x = 0;
	}
	return FALSE;
}

BOOL GuiItemTable_findBackward(GuiItemTable* self, DbValues* columns, Vec2i startPos, const UNI* findStr)
{
	const UBIG N = DbRows_getSize(&self->filter);
	BIG x = startPos.x;
	BIG y;
	for (y = startPos.y; y >= 0; y--)
	{
		DbValues_setRow(columns, DbRows_getRow(&self->filter, y));

		for (; x >= 0; x--)
		{
			DbValue_hasChanged(&columns->values[x]);
			if (Std_subUNI(columns->values[x].result.str, findStr) >= 0)
			{
				self->selectFirstTouch = self->selectLastTouch = Vec2i_init2(x, y);
				return TRUE;
			}
		}
		x = columns->num - 1;
	}
	return FALSE;
}

void GuiItemTable_clickFindForward(GuiItem* item)
{
	GuiItemTable* self = GuiItem_findType(item, GuiItem_TABLE);
	GuiItemEdit* search = GuiItem_findNameType(item, "search", GuiItem_EDIT);
	if (self && search)
	{
		const UNI* findStr = GuiItemEdit_getText(search);
		if (Std_sizeUNI(findStr))
		{
			const Vec2i pos = self->selectLastTouch;
			DbValues columns = DbRows_getSubs(self->viewRow, "columns", TRUE, -1);

			BOOL found = GuiItemTable_findForward(self, &columns, Vec2i_init2(pos.x + 1, pos.y), findStr, FALSE);
			if (!found)
				found = GuiItemTable_findForward(self, &columns, Vec2i_init2(0, 0), findStr, FALSE);

			if (found)
			{
				GuiItemTable_scrollToCell(self, self->selectLastTouch);
				GuiItem_setRedraw(&self->base, TRUE);
			}

			DbValues_free(&columns);
		}
	}
}

void GuiItemTable_clickFindBackward(GuiItem* item)
{
	GuiItemTable* self = GuiItem_findType(item, GuiItem_TABLE);
	GuiItemEdit* search = GuiItem_findNameType(item, "search", GuiItem_EDIT);
	if (self && search)
	{
		const UNI* findStr = GuiItemEdit_getText(search);
		if (Std_sizeUNI(findStr))
		{
			const Vec2i pos = self->selectFirstTouch;
			DbValues columns = DbRows_getSubs(self->viewRow, "columns", TRUE, -1);

			BOOL found = GuiItemTable_findBackward(self, &columns, Vec2i_init2(pos.x - 1, pos.y), findStr);
			if (!found)
				found = GuiItemTable_findBackward(self, &columns, Vec2i_init2(columns.num - 1, DbRows_getSize(&self->filter) - 1), findStr);

			if (found)
			{
				GuiItemTable_scrollToCell(self, self->selectLastTouch);
				GuiItem_setRedraw(&self->base, TRUE);
			}

			DbValues_free(&columns);
		}
	}
}


void GuiItemTable_findAndReplaceCell(DbValue* v, const UNI* findStr, const UNI* replaceStr, BOOL rewriteCells)
{
	if (rewriteCells)
		DbValue_setTextCopy(v, replaceStr);
	else
	{
		UNI* new_str = Std_newUNI(v->result.str);
		Std_replaceInsideUNI(&new_str, findStr, replaceStr);
		DbValue_setText(v, new_str);
	}
}

void GuiItemTable_moveOnRight(GuiItemTable* self)
{
	self->selectLastTouch.x++;
	if (self->selectLastTouch.x >= GuiItemTable_numColumns(self))
	{
		self->selectLastTouch.x = 0;
		self->selectLastTouch.y++;
	}

	if (self->selectLastTouch.y >= DbRows_getSize(&self->filter))
		self->selectLastTouch.y = 0;

	self->selectFirstTouch = self->selectLastTouch;	//one cell
}


void GuiItemTable_clickFindReplaceOne(GuiItem* item)
{
	GuiItemTable* self = GuiItem_findType(item, GuiItem_TABLE);

	const UNI* findStr = GuiItemEdit_getText(GuiItem_findNameType(item, "find_str", GuiItem_EDIT));
	const UNI* replaceStr = GuiItemEdit_getText(GuiItem_findNameType(item, "replace_str", GuiItem_EDIT));
	BOOL rewriteCells = GuiItemCheck_isActive(GuiItem_findNameType(item, "replace_cell", GuiItem_CHECK));

	const Vec2i pos = self->selectLastTouch;
	DbValues columns = DbRows_getSubs(self->viewRow, "columns", TRUE, -1);

	BOOL found = GuiItemTable_findForward(self, &columns, Vec2i_init2(pos.x+1, pos.y), findStr, TRUE);
	if (!found)
		found = GuiItemTable_findForward(self, &columns, Vec2i_init2(0, 0), findStr, TRUE);

	if (found)
	{
		DbValues_setRow(&columns, DbRows_getRow(&self->filter, self->selectLastTouch.y));

		GuiItemTable_findAndReplaceCell(&columns.values[self->selectLastTouch.x], findStr, replaceStr, rewriteCells);

		//GuiItemTable_moveOnRight(self);

		GuiItemTable_scrollToCell(self, self->selectLastTouch);
		GuiItem_setRedraw(&self->base, TRUE);
	}


	DbValues_free(&columns);
}

void GuiItemTable_clickFindReplaceAll(GuiItem* item)
{
	GuiItemTable* self = GuiItem_findType(item, GuiItem_TABLE);

	const UNI* findStr = GuiItemEdit_getText(GuiItem_findNameType(item, "find_str", GuiItem_EDIT));
	const UNI* replaceStr = GuiItemEdit_getText(GuiItem_findNameType(item, "replace_str", GuiItem_EDIT));
	BOOL rewriteCells = GuiItemCheck_isActive(GuiItem_findNameType(item, "replace_cell", GuiItem_CHECK));

	DbValues columns = DbRows_getSubs(self->viewRow, "columns", TRUE, -1);

	const UBIG N = DbRows_getSize(&self->filter);
	BIG x, y;
	for (y = 0; y < N; y++)
	{
		DbValues_setRow(&columns, DbRows_getRow(&self->filter, y));

		for (x = 0; x < columns.num; x++)
		{
			DbValue* v = &columns.values[x];

			if (DbColumn_isFindAndReplace(v->column))
			{
				DbValue_hasChanged(v);
				if (Std_subUNI(v->result.str, findStr) >= 0)
					GuiItemTable_findAndReplaceCell(v, findStr, replaceStr, rewriteCells);
			}
		}
	}
	DbValues_free(&columns);

	GuiItem_setRedraw(&self->base, TRUE);
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
			IOFiles_readSingle(path, &col, N++);
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
			IOFiles_readNet(path, &col, N++);
			Std_deleteCHAR(path);
		}

		DbValue_free(&col);
		Std_deleteCHAR(paths);
	}
}

static void _GuiItemTable_setPageShow(BIG propRow, BOOL show)
{
	DbValues values = DbRows_getOptions(propRow, "enable", FALSE);

	BIG i;
	for (i = 0; i < values.num; i++)
		DbValue_setNumber(&values.values[i], show);

	DbValues_free(&values);
}

void GuiItemTable_clickPageShowAll(GuiItem* item)
{
	_GuiItemTable_setPageShow(GuiItem_findAttribute(item, "columnsRow"), TRUE);
}

void GuiItemTable_clickPageHideAll(GuiItem* item)
{
	_GuiItemTable_setPageShow(GuiItem_findAttribute(item, "columnsRow"), FALSE);
}

void GuiItemTable_clickRemoveSummaryItem(GuiItem* item)
{
	GuiItemTable* table = GuiItem_findParentType(item->parent, GuiItem_TABLE);
	if (table)
	{
		BIG c = GuiItem_findAttribute(item, "c");
		BIG it = GuiItem_findAttribute(item, "it");
		BIG crowT = GuiItemTable_getColumnBaseRow(table, c);

		DbRows rows = DbRows_initSubLink(crowT, "summary");
		DbRows_removeRow(&rows, it);
		DbRows_free(&rows);
	}
}

void GuiItemTable_callListIcon(GuiItem* item)
{
	DbColumn* column = DbRoot_findColumn(GuiItem_getRow(item));
	if (column)
	{
		DbFormatTYPE type = DbColumnFormat_findColumn(column);
		Image1 icon = GuiItem_getColumnIcon(type);
		if (Image1_is(&icon))
			GuiItem_setIcon(item, GuiImage_new1(icon));
	}
}

void GuiItemTable_rebuildShowedList(GuiItem* item)
{
	GuiItem_freeSubs(item);

	GuiItemLayout* layColumn = (GuiItemLayout*)item;

	const BIG row = GuiItem_findAttribute(item, "row");
	const BIG columnsRow = GuiItem_findAttribute(item, "columnsRow");
	const BOOL isNameDirect = GuiItem_findAttribute(item, "isNameDirect");
	const BOOL editable = GuiItem_findAttribute(item, "editable");

	int y = 0;
	UBIG i = 0;
	BIG it;
	while ((it = DbColumnN_jump(DbRoot_subs(), columnsRow, &i, 1)) >= 0)
	{
		GuiItemLayout* skin = GuiItemLayout_new(Quad2i_init4(0, y++, 1, 1));
		GuiItem_setAttribute((GuiItem*)skin, "row", row);
		GuiItemLayout_addColumn(skin, 2, 99);

		GuiItem* drag = GuiItem_addSubName((GuiItem*)skin, "drag", GuiItemBox_newEmpty(Quad2i_init4(0, 0, 1, 1)));
		GuiItem_setIcon(drag, GuiImage_new1(UiIcons_init_reoder()));
		GuiItem_setDrop(drag, "columns", 0, FALSE, DbRows_initLinkN(DbRoot_subs(), columnsRow), -1);

		GuiItem_addSubName((GuiItem*)skin, "on", GuiItemCheck_newEx(Quad2i_init4(1, 0, 1, 1), DbValue_initOptionEnable(-1), DbValue_initEmpty(), 0));

		GuiItem* name;
		if (isNameDirect)
		{
			name = GuiItem_addSubName((GuiItem*)skin, "name", GuiItemText_new(Quad2i_init4(2, 0, 1, 1), FALSE, DbValue_initOption(-1, "name", 0), DbValue_initEmpty()));
			GuiItem_setEnableCallback(name, &GuiItem_enableEnableAttr);
		}
		else
		{
			name = GuiItemText_new(Quad2i_init4(2, 0, 1, 1), FALSE, DbValue_initOption(-1, "name", 0), DbValue_initEmpty());

			GuiItemList* listName = (GuiItemList*)GuiItem_addSubName((GuiItem*)skin, "list", GuiItemList_new(Quad2i_init4(2, 0, 1, 1), DbRows_initLink1(DbRoot_ref(), -1), name, DbValue_initEmpty()));
			GuiItemList_setShowBorder(listName, FALSE);
			GuiItemList_setShowScroll(listName, FALSE);
			GuiItem_setEnableCallback((GuiItem*)listName, &GuiItem_enableEnableAttr);
		}
		GuiItem_setIconCallback(name, &GuiItemTable_callListIcon);
		GuiItem_setDrop(&skin->base, 0, "columns", FALSE, DbRows_initLinkN(DbRoot_subs(), columnsRow), -1);

		GuiItem_setRow(&skin->base, it, 0);
		char nameId[64];
		GuiItem_addSubName((GuiItem*)layColumn, _GuiItemTable_getNameId(nameId, "%lld_line", i), &skin->base);

		//GuiItemList* list = (GuiItemList*)GuiItem_addSubName((GuiItem*)layColumn, "list", GuiItemList_new(Quad2i_init4(0, 0, 1, 1), DbRows_initLinkN(DbRoot_subs(), columnsRow), (GuiItem*)skin, DbValue_initEmpty()));
		//GuiItemList_setShowBorder(list, FALSE);
		//GuiItem_setCallClick((GuiItem*)list, 0);
		i++;
	}

	y++;

	//Hide all
	GuiItem_addSubName((GuiItem*)layColumn, "hideAll", GuiItemButton_newClassicEx(Quad2i_init4(0, y++, 1, 1), DbValue_initLang("HIDE_ALL"), &GuiItemTable_clickPageHideAll));

	//Show All
	GuiItem_addSubName((GuiItem*)layColumn, "showAll", GuiItemButton_newClassicEx(Quad2i_init4(0, y++, 1, 1), DbValue_initLang("SHOW_ALL"), &GuiItemTable_clickPageShowAll));

	if (editable)
	{
		GuiItem_addSubName((GuiItem*)layColumn, "edit", GuiItemCheck_new(Quad2i_init4(0, y++, 1, 1), DbValue_initOption(columnsRow, "edit", 0), DbValue_initLang("ALLOW_EDIT")));
	}
}

static GuiItemLayout* _GuiItemTable_buildShowedList(Quad2i grid, BIG row, const char* subType, BOOL isNameDirect, BOOL editable)
{
	GuiItemLayout* layout = GuiItemLayout_new(grid);
	GuiItem_setAttribute((GuiItem*)layout, "row", row);
	GuiItem_setAttribute((GuiItem*)layout, "columnsRow", DbRows_findOrCreateSubType(row, subType));
	GuiItem_setAttribute((GuiItem*)layout, "isNameDirect", isNameDirect);
	GuiItem_setAttribute((GuiItem*)layout, "editable", editable);

	GuiItemLayout_addColumn(layout, 0, 10);
	GuiItemLayout_setResize(layout, &GuiItemTable_rebuildShowedList);
	return layout;
}

GuiItemLayout* GuiItemTable_buildShowedList(Quad2i grid, BIG row)
{
	return _GuiItemTable_buildShowedList(grid, row, "columns", (DbRoot_findParentTableRow(row) == row), FALSE);
}

GuiItemLayout* GuiItemTable_buildIDShowedList(Quad2i grid, BIG row)
{
	return _GuiItemTable_buildShowedList(grid, row, "id_columns", FALSE, TRUE);
}

/*void GuiItemTable_clickRemoveIdColumLine(GuiItem* item)
{
	BIG row = GuiItem_getRow(item);
	DbRoot_removeRow(row);
}

void GuiItemTable_rebuildIdColumnList(GuiItem* item)
{
	GuiItem_freeSubs(item);

	GuiItemLayout* layColumn = (GuiItemLayout*)item;
	BIG row = GuiItem_findAttribute(item, "row");
	BIG isColumnRow = DbRows_findOrCreateSubType(row, "id_columns");

	int y = 0;
	GuiItem_addSubName((GuiItem*)layColumn, "info", GuiItemText_new(Quad2i_init4(0, y++, 1, 1), TRUE, DbValue_initLang("DRAG_DROP_COLUMN_HERE"), DbValue_initEmpty()));

	GuiItem_addSubName((GuiItem*)layColumn, "edit", GuiItemCheck_new(Quad2i_init4(0, y++, 1, 1), DbValue_initOption(isColumnRow, "edit", 0), DbValue_initLang("ALLOW_EDIT")));

	UBIG i = 0;
	BIG it;
	while ((it = DbColumnN_jump(DbRoot_subs(), isColumnRow, &i, 1)) >= 0)
	{
		GuiItemLayout* skin = GuiItemLayout_new(Quad2i_init4(0, y++, 1, 1));
		GuiItem_setAttribute((GuiItem*)skin, "row", row);
		GuiItemLayout_addColumn(skin, 2, 99);

		GuiItem* drag = GuiItem_addSubName((GuiItem*)skin, "drag", GuiItemBox_newEmpty(Quad2i_init4(0, 0, 1, 1)));
		GuiItem_setIcon(drag, GuiImage_new1(UiIcons_init_reoder()));
		GuiItem_setDrop(drag, "columns", 0, FALSE, DbRows_initLinkN(DbRoot_subs(), isColumnRow), -1);

		GuiItem_addSubName((GuiItem*)skin, "on", GuiItemCheck_newEx(Quad2i_init4(1, 0, 1, 1), DbValue_initOptionEnable(-1), DbValue_initEmpty(), 0));

		BOOL isFilter = DbRoot_findParentTableRow(row) != row;
		GuiItem* name = GuiItemText_new(Quad2i_init4(2, 0, 1, 1), FALSE, DbValue_initOption(-1, "name", 0), DbValue_initEmpty());

		GuiItemList* listName = (GuiItemList*)GuiItem_addSubName((GuiItem*)skin, "list", GuiItemList_new(Quad2i_init4(2, 0, 1, 1), DbRows_initLink1(DbRoot_ref(), -1), name, DbValue_initEmpty()));
		GuiItemList_setShowBorder(listName, FALSE);
		GuiItemList_setShowScroll(listName, FALSE);
		GuiItem_setEnableCallback((GuiItem*)listName, &GuiItem_enableEnableAttr);

		GuiItem_setIconCallback(name, &GuiItemTable_callListIcon);
		GuiItem_setDrop(&skin->base, 0, "columns", FALSE, DbRows_initLinkN(DbRoot_subs(), isColumnRow), -1);

		//remove
		GuiItem_addSubName((GuiItem*)skin, "remove", GuiItemButton_newBlackEx(Quad2i_init4(3, 0, 1, 1), DbValue_initStaticCopyCHAR("X"), &GuiItemTable_clickRemoveIdColumLine));

		GuiItem_setRow(&skin->base, it, 0);
		char nameId[64];
		GuiItem_addSubName((GuiItem*)layColumn, _GuiItemTable_getNameId(nameId, "%lld_line", i), &skin->base);

		i++;
	}
}

GuiItemLayout* GuiItemTable_buildIdColumnList(Quad2i grid, BIG row)
{
	GuiItemLayout* layout = GuiItemLayout_new(grid);
	GuiItem_setAttribute((GuiItem*)layout, "row", row);
	GuiItemLayout_addColumn(layout, 0, 10);
	GuiItemLayout_setResize(layout, &GuiItemTable_rebuildIdColumnList);
	return layout;
}*/

GuiItem* GuiItemTable_getCardSkinItem(Quad2i grid, DbValue column, BOOL showDescription, const DbValue searchHighlight)//, BOOL onlyRead)
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
			/*if (onlyRead)
			{
				edit = GuiItemText_new(grid, TRUE, column, showDescription ? DbValue_initStaticCopy(DbColumn_getName(column.column, name, 64)) : DbValue_initEmpty());
				GuiItemText_setWhiteBack((GuiItemText*)edit, TRUE);
				((GuiItemText*)edit)->drawBackground_procentage_visualize = DbValue_getOptionNumber(crowT, "back_vis", 0);
				((GuiItemText*)edit)->drawBackground_procentage_visualize_mult100 = DbValue_getOptionNumber(crowT, "mult100", 0);
			}
			else*/
			{
				edit = GuiItemEdit_new(grid, column, showDescription ? DbValue_initStaticCopy(DbColumn_getName(column.column, name, 64)) : DbValue_initEmpty());
				((GuiItemEdit*)edit)->drawBackground_procentage_visualize = DbValue_getOptionNumber(crowT, "back_vis", 0);
				((GuiItemEdit*)edit)->drawBackground_procentage_visualize_mult100 = DbValue_getOptionNumber(crowT, "mult100", 0);

				GuiItemEdit_setHighlightFind((GuiItemEdit*)edit, DbValue_initCopy(&searchHighlight));
			}

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
		{
			GuiItemCheck* check = (GuiItemCheck*)GuiItemCheck_new(grid, column, showDescription ? DbValue_initStaticCopy(DbColumn_getName(column.column, name, 64)) : DbValue_initEmpty());

			UNI* whiteCd = Std_newNumber(Rgba_asNumber(g_theme.white));
			GuiItemCheck_setColors(check, DbValue_initOption(crowT, "cd_enable", whiteCd), DbValue_initOption(crowT, "cd_on", whiteCd), DbValue_initOption(crowT, "cd_off", whiteCd));
			Std_deleteUNI(whiteCd);

			return (GuiItem*)check;
		}

		case DbFormat_MENU:
		{
			BIG optionsRow = DbRows_findSubType(crowT, "options");
			GuiItemComboDynamic* mn = (GuiItemComboDynamic*)GuiItemComboDynamic_new(grid, FALSE, DbRows_initLinkN((DbColumnN*)column.column, -1), DbValue_initOption(-1, "name", 0), DbRows_initLinkN(DbRoot_subs(), optionsRow), showDescription ? DbValue_initStaticCopy(DbColumn_getName(column.column, name, 64)) : DbValue_initEmpty());
			GuiItemComboDynamic_setColor(mn, DbValue_initOption(-1, "cd", 0), DbValue_initOption(crowT, "cd_enable", 0));
			return (GuiItem*)mn;
		}

		case DbFormat_TAGS:
		{
			BIG optionsRow = DbRows_findSubType(crowT, "options");

			//values
			DbValues columns = DbValues_init();
			DbValues_add(&columns, DbValue_initOption(optionsRow, "name", 0));

			GuiItem* ret = GuiItemTags_new(grid, DbRows_initLinkN((DbColumnN*)column.column, -1), columns, DbValue_initOption(crowT, "cd_enable", 0), GuiItemTags_dialogTagsAdd, GuiItemTags_dialogTagsDetails, TRUE, TRUE, FALSE, TRUE, DbValue_initEmpty());
			DbValue_free(&column);
			return ret;
		}

		case DbFormat_LINK_1:
		case DbFormat_LINK_N:
		case DbFormat_LINK_MIRRORED:
		case DbFormat_LINK_JOINTED:
		case DbFormat_LINK_FILTERED:
		{
			DbTable* btable = DbColumn_getBTable(column.column);
			if (btable)
			{
				const BIG num_columns = Std_bmax(1, DbColumn_getOptionNumber(column.column, "numColumnsPreviews"));
				DbValues columns = DbRows_getSubs(DbTable_getRow(btable), "columns", TRUE, num_columns);

				const BOOL generated = (type == DbFormat_LINK_JOINTED || type == DbFormat_LINK_FILTERED);

				GuiItemTagsCallback* callbackAdd = generated ? 0 : GuiItemTags_dialogLinksAdd;
				GuiItemTagsCallback* callbackItem = GuiItemTags_dialogLinksDetails;

				GuiItem* ret = GuiItemTags_new(grid, DbRows_initLinkN((DbColumnN*)column.column, -1), columns, DbValue_initEmpty(), callbackAdd, callbackItem, FALSE, TRUE, TRUE, TRUE, DbValue_initEmpty());
				((GuiItemTags*)ret)->showClose = !generated;

				DbValue_free(&column);
				return ret;
			}
			break;
		}

		case DbFormat_FILE_N:
		{
			DbValues columns = DbValues_init();
			GuiItem* ret = GuiItemTags_new(grid, DbRows_initLinkN((DbColumnN*)column.column, -1), columns, DbValue_initEmpty(), GuiItemTags_dialogFileAdd, GuiItemTags_dialogFileDetails, TRUE, TRUE, TRUE, TRUE, DbValue_initOption(crowT, "preview", 0));
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
			/*if (onlyRead)
			{
				edit = GuiItemText_new(grid, TRUE, column, showDescription ? DbValue_initStaticCopy(DbColumn_getName(column.column, name, 64)) : DbValue_initEmpty());
				GuiItemText_setWhiteBack((GuiItemText*)edit, TRUE);
			}
			else*/
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

		case DbFormat_SUMMARY:
		{
			GuiItem* text = GuiItemText_new(grid, TRUE, column, showDescription ? DbValue_initStaticCopy(DbColumn_getName(column.column, name, 64)) : DbValue_initEmpty());
			GuiItemText_setWhiteBack((GuiItemText*)text, TRUE);
			return text;
		}

		case DbFormat_ROW:
		{
			break;
		}
	}

	return GuiItemText_new(grid, TRUE, DbValue_initEmpty(), DbValue_initEmpty());;
}

void GuiItemTable_resizePage(GuiItem* item)
{
	BIG row = GuiItem_getRow(item);

	GuiItem_freeSubs(item);

	BIG viewRow = GuiItem_findAttribute(item, "viewRow");
	BOOL showRowID = GuiItem_findAttribute(item, "showRowID");
	BOOL showHidden = GuiItem_findAttribute(item, "showHidden");
	//BOOL onlyRead = GuiItem_findAttribute(item, "onlyRead");

	GuiItemLayout* skin = (GuiItemLayout*)item;

	int y = 0;
	if (showHidden)
	{
		//Column list
		GuiItemMenu* columns = (GuiItemMenu*)GuiItem_addSubName((GuiItem*)skin, "columns", GuiItemMenu_new(Quad2i_init4(0, 0, 1, 1), DbValue_initLang("COLUMNS"), FALSE));
		GuiItemMenu_setContext(columns, GuiItemTable_buildShowedList(Quad2i_init(), viewRow));
		GuiItemMenu_setHighligthBackground(columns, DbRows_hasColumnsSubDeactive(viewRow, "columns"), 0.5f);
		GuiItemMenu_setTransparent(columns, FALSE);
		GuiItem_setIcon((GuiItem*)columns, GuiImage_new1(UiIcons_init_table_hide()));
		GuiItemMenu_setCenter(columns, FALSE);
	}

	if (showRowID)
	{
		GuiItem_addSubName((GuiItem*)skin, "row_id", GuiItemText_new(Quad2i_init4(showHidden ? 1 : 0, y++, showHidden ? 1 : 2, 1), TRUE, DbValue_initGET(DbTable_getIdsColumn(DbRoot_findParentTable(viewRow)), -1), DbValue_initEmpty()));
	}

	int N_COLS = DbRows_getSubsNum(viewRow, "columns", TRUE);
	BIG c;
	for (c = 0; c < N_COLS; c++)
	{
		DbValue value = DbRows_getSubsCell(viewRow, "columns", TRUE, c, -1);

		GuiItem* it = GuiItemTable_getCardSkinItem(Quad2i_init4(0, y, 2, 2), value, TRUE, DbValue_initEmpty());//, onlyRead);
		if (it)
		{
			char nameId[64];
			snprintf(nameId, 64, "%lld_skin", c);

			GuiItem_addSubName((GuiItem*)skin, nameId, it);
			y += 2;
		}
	}

	GuiItem_setRow(item, row, 0);

	//disable touch for summary table
	DbTable* table = DbRoot_findParentTable(viewRow);
	if (table)
		GuiItem_setTouchRecommand(item, !DbTable_isSummary(table));
}

GuiItemLayout* GuiItemTable_buildPage(BIG viewRow, BIG row, BOOL showRowID, BOOL showHidden)//, BOOL onlyRead)
{
	GuiItemLayout* skin = GuiItemLayout_new(Quad2i_init());
	GuiItemLayout_addColumn(skin, 0, 3);
	GuiItemLayout_addColumn(skin, 1, 100);
	GuiItemLayout_setResize(skin, &GuiItemTable_resizePage);

	GuiItem_setAttribute((GuiItem*)skin, "viewRow", viewRow);
	GuiItem_setAttribute((GuiItem*)skin, "showRowID", showRowID);
	GuiItem_setAttribute((GuiItem*)skin, "showHidden", showHidden);
	//GuiItem_setAttribute((GuiItem*)skin, "onlyRead", onlyRead);

	GuiItem_setRow(&skin->base, row, 0);

	GuiItemTable_resizePage((GuiItem*)skin);

	return skin;
}

void GuiItemTable_clickShowPage(GuiItem* self)
{
	GuiItemTable* table = GuiItem_findParentType(self->parent, GuiItem_TABLE);
	if (table && self->type == GuiItem_BUTTON)
	{
		BIG row = GuiItem_getRow(self);
		GuiItemLayout* skin = GuiItemTable_buildPage(table->viewRow, row, TRUE, TRUE);// , FALSE);
		GuiItemRoot_addDialogLayout(GuiItemRoot_createDialogLayout(Vec2i_init2(15, GuiItemLayout_getSubMaxGrid(skin).y), DbValue_initLang("CARD"), (GuiItem*)skin, 0));
	}
}

void UiDialogLink_clickLinkDialogClear(GuiItem* self)
{
	GuiItemTable* linkTable = GuiItem_findNameType(self->parent, "btable", GuiItem_TABLE);
	if (linkTable)
	{
		BIG srcRow = GuiItemTable_getBaseRow(linkTable);

		DbColumn* column = GuiItemTable_getBaseColumn(linkTable);
		if (column)
			DbColumn_deleteRowData(column, srcRow);
	}
}

void UiDialogLink_clickLinkDialogAddAll(GuiItem* self)
{
	GuiItemTable* linkTable = GuiItem_findNameType(self->parent, "btable", GuiItem_TABLE);
	if (linkTable)
	{
		BIG srcRow = GuiItemTable_getBaseRow(linkTable);

		DbColumn* column = GuiItemTable_getBaseColumn(linkTable);
		if (column)
			DbColumn_addAllTable(column, srcRow);
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

	GuiItemTable* tableActual = (GuiItemTable*)GuiItem_addSubName((GuiItem*)layout, "btable", (GuiItem*)GuiItemTable_new(Quad2i_init4(0, 2, 3, 1), bRow, DbRows_initLinkN((DbColumnN*)column, srcRow), TRUE, FALSE, DbValue_initNumber(0), DbValue_initNumber(0), DbValue_initStaticCopyCHAR("0 0 0 0"), DbValue_initEmpty()));
	tableActual->drawBorder = TRUE;
	tableActual->showRemoveButton = TRUE;
	tableActual->showChangeOrder = TRUE;
	tableActual->changeOrderNameDst = Std_newCHAR("dst");
	tableActual->changeOrderNameSrc = Std_newCHAR("dst");

	//BTable
	UNI name[64];
	GuiItem_addSubName((GuiItem*)layout, "name", GuiItemText_new(Quad2i_init4(0, 3, 3, 1), TRUE, DbValue_initStaticCopy(DbTable_getName(btable, name, 64)), DbValue_initEmpty()));
	GuiItemTable* tableAdd = (GuiItemTable*)GuiItem_addSubName((GuiItem*)layout, "cTable", (GuiItem*)GuiItemTable_new(Quad2i_init4(0, 4, 3, 1), bRow, DbRows_initTable(btable), TRUE, TRUE, DbValue_initNumber(0), DbValue_initNumber(0), DbValue_initStaticCopyCHAR("0 0 0 0"), DbValue_initEmpty()));
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



static BOOL _GuiItemTable_hasHScroll(const GuiItemTable* self)
{
	return GuiItemLayout_hasScrollH(_GuiItemTable_getCellsLayout(self));
}

static void _GuiItemTable_setScroll(GuiItemTable* self, Quad2i coord, Win* win)
{
	const int cell = OsWinIO_cellSize();
	const int RS = GuiItemTable_getRowSize(self);
	const UINT extra = GuiItemTable_scrollExtra(self, win);
	const int scroll_width = GuiScroll_widthWin(win);

	BOOL hScroll = _GuiItemTable_hasHScroll(self);	//is horizontal scroll on
	if (!hScroll)
		hScroll = (coord.size.y % cell) < cell * RS * 3 / 4;	//check if last is visible from 3/4
	GuiScroll_set(&self->scrollV, (GuiItemTable_numRows(self) + self->showAddRecord + hScroll) * cell * RS, coord.size.y - extra - cell + scroll_width, cell * RS);
}

void GuiItemTable_drawPost(GuiItemTable* self, Image4* img, Quad2i coord, Win* win)
{
	const int cell = OsWinIO_cellSize();
	const int scroll_width = GuiScroll_widthWin(win);

	if (_GuiItemTable_hasHScroll(self))
		coord.size.y -= scroll_width;

	//DbValue_getOptionNumber(self->viewRow, "width_ids", 4);
	{
		Quad2i q = _GuiItemTable_getIdsLayout(self)->base.coordScreen;
		q.start.x += q.size.x - 2;
		q.size.x = 4;
		Image4_drawBoxQuadAlpha(img, q, g_theme.main);
		//Image4_drawBoxQuadAlpha(img, q, Rgba_aprox(g_theme.background, g_theme.black, 0.4f));
	}

	if (_GuiItemTable_isSelect(self, win) && GuiItemTable_repairSelect(self))
	{
		//GuiItem* cells_layout = GuiItem_findName((GuiItem*)self, "cells_layout");
		//const Quad2i layoutCoord = cells_layout->coordMoveCut;

		Quad2i selectRect = GuiItemTable_getSelectRect(self);
		Quad2i q = _GuiItemTable_getSelectGridCoord(self, selectRect, TRUE);

		Rgba cd = g_theme.edit;
		if (selectRect.size.x == 1 && selectRect.size.y == 1)
		{
			//draw shadow line
			{
				Quad2i qs = q;
				qs.start.x = coord.start.x;
				qs.size.x = coord.size.x - scroll_width;
				Rgba cds = g_theme.main;
				cds.a = 35;
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

		if (_GuiItemTable_isCellExtendSelect(self, win))
		{
			q = _GuiItemTable_getSelectGridCoord(self, self->cellExtendRect, TRUE);
			Rgba cd = g_theme.main;
			Image4_drawBorder(img, q, 3, cd);
		}
		else
			if (!GuiItemTable_isSummaryTable(self))
			{
				//holder
				q = _GuiItemTable_getCellExtendQuad(self, _GuiItemTable_isCellExtendSelect(self, win) ? self->cellExtendRect : selectRect, win);
				Image4_drawBoxQuad(img, q, g_theme.white);
				cd.a = 255;
				Image4_drawBorder(img, q, 1, cd);
			}
	}

	if (self->drawBorder)
		Image4_drawBorder(img, coord, 1, g_theme.black);

	//scroll
	UINT extra = GuiItemTable_scrollExtra(self, win);
	_GuiItemTable_setScroll(self, coord, win);
	GuiScroll_drawV(&self->scrollV, Vec2i_init2(coord.start.x + coord.size.x - scroll_width, coord.start.y + extra), img, win);
}

void GuiItemTable_saveSelect(GuiItemTable* self)
{
	GuiItemTable_repairSelect(self);

	DbValue_hasChanged(&self->selectGrid);
	char gridC[64];
	snprintf(gridC, 64, "%d %d %d %d", self->selectFirstTouch.x, self->selectFirstTouch.y, self->selectLastTouch.x, self->selectLastTouch.y);
	gridC[63] = 0;
	UNI gridU[64];
	Std_copyUNI_char(gridU, 64, gridC);
	if (!Std_cmpUNI(gridU, DbValue_result(&self->selectGrid)))
	{
		BOOL reset = !GuiItemRoot_hasChanges();
		DbValue_setTextCopy(&self->selectGrid, gridU);
		if (reset)
			GuiItemRoot_resetNumChanges();
	}
}

void GuiItemTable_update(GuiItemTable* self, Quad2i coord, Win* win)
{
	GuiItemTable_saveSelect(self);

	DbRows_hasChanged(&self->filter);

	BIG oldNumRows = self->oldNumRows;
	self->oldNumRows = GuiItemTable_numRows(self);

	//update cells
	if (self->base.subs.num)
	{
		GuiItem* ids_layout = (GuiItem*)_GuiItemTable_getIdsLayout(self);
		GuiItem* cells_layout = (GuiItem*)_GuiItemTable_getCellsLayout(self);

		const UBIG N_ROWS_VISIBLE = self->num_rows_max;//Std_bmin(DbTable_numRowsReal(table), self->num_rows_max);

		if (_GuiItemTable_hasHScroll(self))
			coord.size.y -= GuiScroll_widthWin(win);
		_GuiItemTable_setScroll(self, coord, win);
		const UBIG WHEEL = GuiScroll_getWheelRow(&self->scrollV);

		self->num_rows_real = 0;

		int r, p;

		int last_y = 0;

		//cards
		p = self->subs_ids_start_rowOptionCards;
		for (r = 0; r < N_ROWS_VISIBLE; r++)
		{
			GuiItem* it = GuiItem_getSub(ids_layout, p++);
			GuiItem_setRow(it, DbRows_getRow(&self->filter, WHEEL + r), 0);

			BOOL show = DbRows_isRow(&self->filter, WHEEL + r);
			GuiItem_setShow(it, show);

			if (show)
				last_y = Std_max(last_y, it->grid.start.y + it->grid.size.y);
		}

		if (self->showHeader && last_y == 0)	//no cards
			last_y = 1;

		//options
		p = self->subs_ids_start_rowOption;
		for (r = 0; r < N_ROWS_VISIBLE; r++)
		{
			GuiItem* it = GuiItem_getSub(ids_layout, p++);

			GuiItem_setRow(it, DbRows_getRow(&self->filter, WHEEL + r), 0);

			BOOL show = DbRows_isRow(&self->filter, WHEEL + r);
			GuiItem_setShow(it, show);

			self->num_rows_real += show;
		}

		if (self->showChangeOrder)
		{
			p = self->subs_ids_start_rowOptionOrder;
			for (r = 0; r < N_ROWS_VISIBLE; r++)
			{
				GuiItem* it = GuiItem_getSub(ids_layout, p++);
				GuiItem_setRow(it, DbRows_getRow(&self->filter, WHEEL + r), 0);

				BOOL show = DbRows_isRow(&self->filter, WHEEL + r);
				GuiItem_setShow(it, show);
			}
		}

		if (self->showAddRecord)
		{
			GuiItem* addRecord = GuiItem_getSub(ids_layout, self->subs_ids_add_record);

			BOOL show = (DbColumns_num(DbTable_getColumns((GuiItemTable_getTable(self)))) > 1 && self->showAddRecord);

			GuiItem_setShow(addRecord, show);
			if (show)
				GuiItem_setGrid(addRecord, Quad2i_init4(1, last_y, self->showChangeOrder + 1, 1));
		}

		//Records data
		const int N_COLS = GuiItemTable_numColumns(self);
		Quad2i rectCoord = _GuiItemTable_getSelectGridCoord(self, GuiItemTable_getSelectRect(self), TRUE);
		int c;
		for (c = 0; c < N_COLS; c++)
		{
			const BOOL modeSummary = GuiItemTable_isSummaryTable(self);// && !DbColumn_isSummaryLinks(col);
			for (r = 0; r < N_ROWS_VISIBLE; r++)
			{
				p = Std_min(self->subs_cells_start + c * self->num_rows_max + r, self->subs_cells_end);
				if (p < self->subs_cells_end)
				{
					GuiItem* it = GuiItem_getSub(cells_layout, p);

					GuiItem_setRow(it, DbRows_getRow(&self->filter, WHEEL + r), 0);
					GuiItem_setShow(it, DbRows_isRow(&self->filter, WHEEL + r));

					GuiItem_setEnableOne(it, !GuiItemTable_isColumnLocked(self, c));

					GuiItem_setTouchRecommand(it, !modeSummary && Quad2i_inside(rectCoord, Quad2i_getMiddle(it->coordMove)));
				}
			}
		}

		GuiItem_setRedraw(&self->base, (oldNumRows != self->oldNumRows || GuiScroll_getRedrawAndReset(&self->scrollV) || DbValue_hasChanged(&self->searchHighlight)));
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

void GuiItemTable_makeCellExtend(GuiItemTable* self, const BOOL EXTRA_DICTIONARY)
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
			UNI** src = EXTRA_DICTIONARY ? Lang_getList(find, &n) : 0;
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
				UNI** src = EXTRA_DICTIONARY ? Lang_getList(find, &n) : 0;
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

	//const int N_COLS = GuiItemTable_numColumns(self);
	//const int N_ROWS = GuiItemTable_numRows(self);

	Quad2i tableCoord = coord;
	coord = _GuiItemTable_getCellsLayout(self)->base.coordMoveCut;

	if (_GuiItemTable_hasHScroll(self))
		coord.size.y -= scroll_width;

	Quad2i selectRect = GuiItemTable_getSelectRect(self);

	Quad2i backup_selectRect = selectRect;
	Quad2i backup_cellExtendRect = self->cellExtendRect;

	Rgba back_cd = GuiItemTheme_getWhite_Background();
	Rgba front_cd = g_theme.black;

	BOOL touchL = (OsWinIO_getTouch_action() == Win_TOUCH_DOWN_S);
	BOOL touchR = (OsWinIO_getTouch_action() == Win_TOUCH_FORCE_DOWN_S);
	BOOL endTouchL = (OsWinIO_getTouch_action() == Win_TOUCH_DOWN_E);
	//BOOL endTouchR = (OsWinIO_getTouch_action() == Win_TOUCH_FORCE_DOWN_E);

	if (self->base.touch && GuiItem_isEnable(&self->base) && OsWinIO_canActiveRenderItem(self))
	{
		UINT extra = GuiItemTable_scrollExtra(self, win);

		if (OsWinIO_isCursorEmpty())
			GuiScroll_touchV(&self->scrollV, self, tableCoord, Vec2i_init2(coord.start.x + coord.size.x/* - scroll_width*/, coord.start.y + extra), win);

		if (self->showHeader)
		{
			//cut top
			coord.start.y += cell;
			coord.size.y -= cell;
		}

		BOOL inside = Quad2i_inside(coord, OsWinIO_getTouchPos());
		BOOL insideSelect = Quad2i_inside(_GuiItemTable_getSelectGridCoord(self, selectRect, TRUE), OsWinIO_getTouchPos());
		BOOL insideCellExtend = _GuiItemTable_isSelect(self, win) && Quad2i_inside(Quad2i_addSpace(_GuiItemTable_getCellExtendQuad(self, selectRect, win), -5), OsWinIO_getTouchPos());

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
				if (!GuiItemTable_isSummaryTable(self))
				{
					self->cellExtendActive = TRUE;
					OsWinIO_setActiveRenderItem(self);
				}
			}
			else
			{
				Vec2i curr;
				if (_GuiItemTable_getTouchCellPos(self, OsWinIO_getTouchPos(), FALSE, &curr))
				{
					self->selectActive = TRUE;

					if (!(OsWinIO_getKeyExtra() & Win_EXTRAKEY_SHIFT))
						self->selectFirstTouch = curr;
				}
			}
		}

		if (!GuiItemTable_isSummaryTable(self) && (self->cellExtendActive || insideCellExtend))
			Win_updateCursor(win, Win_CURSOR_FLEUR);

		if (self->cellExtendActive && inside)
		{
			Vec2i curr;
			if (_GuiItemTable_getTouchCellPosClosest(self, OsWinIO_getTouchPos(), &curr))
			{
				Vec2i end = Quad2i_end(selectRect);
				end.x--;
				end.y--;

				if (curr.y > selectRect.start.y)
					self->cellExtendRect = GuiItemTable_getSelectRectEx(selectRect.start, curr);
				else
					self->cellExtendRect = GuiItemTable_getSelectRectEx(end, curr);

				BOOL vertical = Std_isBetween(curr.x, selectRect.start.x, end.x);
				BOOL horizontal = Std_isBetween(curr.y, selectRect.start.y, end.y);

				if (vertical && horizontal)
				{
					self->cellExtendRect = selectRect;
				}
				else
					if (horizontal || Std_abs(self->cellExtendRect.size.x - selectRect.size.x) > Std_abs(self->cellExtendRect.size.y - selectRect.size.y))
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

		//auto-scroll: if touch position is on edge
		if (self->selectActive || self->cellExtendActive)
		{
			Quad2i q = Quad2i_addSpace(coord, cell);

			//vertical
			BOOL up = OsWinIO_getTouchPos().y < q.start.y;
			BOOL down = OsWinIO_getTouchPos().y > q.start.y + q.size.y;
			if (up || down)
			{
				if (GuiScroll_tryDragScroll(&self->scrollV, 3, up ? -1 : 1))
				{
					GuiItem_update(&self->base, win);
					GuiItem_setRedraw(&self->base, TRUE);
				}
			}

			//horizontal
			GuiItemLayout* layout = _GuiItemTable_getCellsLayout(self);
			if (GuiItemLayout_hasScrollH(layout))
			{
				BOOL left = OsWinIO_getTouchPos().x < q.start.x;
				BOOL right = OsWinIO_getTouchPos().x > q.start.x + q.size.x;
				if (left || right)
				{
					if (GuiScroll_tryDragScroll(&layout->scrollH, 3, left ? -1 : 1))
					{
						GuiItem_update(&self->base, win);
						GuiItem_setRedraw(&self->base, TRUE);
					}
				}
			}
		}

		if (insideSelect && touchR)
		{
			GuiItemTable_showCellMenu(self);
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
			GuiItemTable_makeCellExtend(self, OsWinIO_getKeyExtra() & Win_EXTRAKEY_CTRL);

		self->cellExtendActive = FALSE;
		self->cellExtendRect = Quad2i_init();
	}

	GuiItem_setRedraw(&self->base, !Quad2i_cmp(backup_selectRect, GuiItemTable_getSelectRect(self)) || !Quad2i_cmp(backup_cellExtendRect, self->cellExtendRect));

	_GuiItem_updateFinalCd(&self->base, back_cd, front_cd, coord, win);
}

void GuiItemTable_key(GuiItemTable* self, Quad2i coord, Win* win)
{
	if (/*!self->base.touch || */!GuiItem_isEnable(&self->base))
		return;

	UNI keyId = OsWinIO_getKeyID();
	UBIG keyExtra = OsWinIO_getKeyExtra();

	const int N_ROWS = GuiItemTable_numRows(self);

	if (!OsWinIO_isCursorEmpty())
	{
		if ((keyExtra & Win_EXTRAKEY_ENTER) && ((GuiItem*)OsWinIO_getCursorRenderItem())->type == GuiItem_EDIT)
		{
			self->selectFirstTouch.y = Std_min(N_ROWS - 1, self->selectFirstTouch.y + 1);
			self->selectLastTouch = self->selectFirstTouch;

			GuiItem_setRedraw(&self->base, TRUE);
		}
		return;
	}

	const UBIG WHEEL = GuiScroll_getWheelRow(&self->scrollV);
	const int N_COLS = GuiItemTable_numColumns(self);
	const int N_ROWS_VISIBLE = self->num_rows_max;

	Quad2i selectRect = GuiItemTable_getSelectRect(self);
	Quad2i selectRectBackup = selectRect;

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

	if (keyExtra & Win_EXTRAKEY_SELECT_ROW)
	{
		//select row
		self->selectFirstTouch = Vec2i_init2(0, self->selectFirstTouch.y);
		self->selectLastTouch = Vec2i_init2(N_COLS - 1, self->selectLastTouch.y);
	}
	else
		if (keyExtra & Win_EXTRAKEY_SELECT_COLUMN)
		{
			//select column
			self->selectFirstTouch = Vec2i_init2(self->selectFirstTouch.x, 0);
			self->selectLastTouch = Vec2i_init2(self->selectLastTouch.x, N_ROWS - 1);
		}
		else
			if ((keyExtra & Win_EXTRAKEY_ENTER))	//jump down
			{
				//OsWinIO_resetKey();	//edit will not know about this 'enter'

				self->selectFirstTouch.y = Std_min(N_ROWS - 1, self->selectFirstTouch.y + 1);
				self->selectLastTouch = self->selectFirstTouch;
			}

	//if (keyExtra & Win_EXTRAKEY_ENTER)
	if (keyId)	//start writing
	{
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

	//move scroll
	if (!Quad2i_cmp(selectRectBackup, GuiItemTable_getSelectRect(self)))
	{
		GuiItem_setRedraw(&self->base, TRUE);

		GuiItemTable_scrollToCell(self, self->selectLastTouch);
		/*//Vertical
		if (self->selectLastTouch.y < WHEEL)
			GuiScroll_setWheelRow(&self->scrollV, self->selectLastTouch.y);
		else
			if (self->selectLastTouch.y > WHEEL + N_ROWS_VISIBLE - 1)
				GuiScroll_setWheelRow(&self->scrollV, self->selectLastTouch.y - (N_ROWS_VISIBLE - 1));

		//Horizontal
		GuiItemLayout* lay = _GuiItemTable_getCellsLayout(self);
		if (GuiItemLayout_hasScrollH(lay))
		{
			Quad2i rect = _GuiItemTable_getSelectGridCoord(self, Quad2i_init2(self->selectLastTouch, Vec2i_init2(1, 1)), TRUE);

			//left
			if (rect.start.x < coord.start.x)
				GuiScroll_setWheel(&lay->scrollH, GuiScroll_getWheel(&lay->scrollH) - (coord.start.x - rect.start.x));

			//right
			if (rect.start.x + rect.size.x > coord.start.x + coord.size.x)
				GuiScroll_setWheel(&lay->scrollH, GuiScroll_getWheel(&lay->scrollH) + (rect.start.x + rect.size.x) - (coord.start.x + coord.size.x));
		}*/
	}
}

void GuiItemTable_clickLinkMirrorColumnSET(GuiItem* self)
{
	GuiItemTable* table = GuiItem_findParentType(self->parent, GuiItem_TABLE);
	if (table && self->type == GuiItem_BUTTON)
	{
		BIG c = GuiItem_findAttribute(self, "c");
		BIG crowT = GuiItemTable_getColumnBaseRow(table, c);
		//DbColumn* columnSrc = GuiItemTable_getColumn(table, c);

		BIG dstColumnRow = ((GuiItemButton*)self)->text.row;
		//DbColumn* columnDst = DbRoot_findColumn(dstColumnRow);

		//if (columnSrc && dstColumnRow >= 0)
		{
			BIG mirrorRow = DbRows_findOrCreateSubType(crowT, "mirror");
			DbColumn1_set(DbRoot_ref(), mirrorRow, dstColumnRow);
		}
	}
}
void GuiItemTable_clickLinkMirrorColumn(GuiItem* self)
{
	GuiItemTable* table = GuiItem_findParentType(self->parent, GuiItem_TABLE);
	if (table && self->type == GuiItem_BUTTON)
	{
		BIG c = GuiItem_findAttribute(self, "c");
		BIG crowT = GuiItemTable_getColumnBaseRow(table, c);
		DbColumn* columnSrc = GuiItemTable_getColumn(table, c);
		if (columnSrc)
		{
			DbTable* tab = GuiItemTable_getTable(table);
			if (tab)
			{
				BIG columnSrcRow = DbColumn_getRow(columnSrc);

				BIG mirrorRow = DbRows_findOrCreateSubType(crowT, "mirror");
				BIG mirrorColumnRow = DbRoot_ref_row(mirrorRow);

				GuiItemLayout* layout = GuiStruct_create(GuiStruct_SHOW_COLUMNS, mirrorColumnRow, DbValue_initLang("COLUMNS"), &GuiItemTable_clickLinkMirrorColumnSET, -1, DbTable_getRow(tab), columnSrcRow, FALSE);
				GuiItemRoot_addDialogRelLayout(layout, self, self->coordMove, TRUE);
			}
		}
	}
}

void GuiItemTable_buildColumnDialogProperties(GuiItem* item, GuiItemTable* self)
{
	GuiItemLayout* properLayout = (GuiItemLayout*)item;
	GuiItem_freeSubs(&properLayout->base);

	//GuiItemTable* self = GuiItem_findParentType(item->parent, GuiItem_TABLE);
	DbTable* table = GuiItemTable_getTable(self);

	BIG c = GuiItem_findAttribute(item, "c");
	//BIG crow = GuiItemTable_getColumnRow(self, c);

	DbColumn* col = GuiItemTable_getColumn(self, c);
	BIG crowT = DbColumn_getRow(col);	//table

	DbFormatTYPE type = GuiItemTable_getColumnType(self, c);
	switch (type)
	{
		case DbFormat_NUMBER_1:
		case DbFormat_NUMBER_N:
		{
			GuiItemLayout_addColumn(properLayout, 0, 99);
			GuiItemLayout_addColumn(properLayout, 1, 99);
			GuiItem_addSubName((GuiItem*)properLayout, "precision", GuiItemEdit_newEx(Quad2i_init4(0, 0, 1, 2), DbRows_getSubsOption(self->viewRow, "columns", TRUE, c, "precision", TRUE), DbValue_initLang("PRECISION"), &GuiItemTable_clickRebuild));
			GuiItem_addSubName((GuiItem*)properLayout, "units", GuiItemComboStatic_newEx(Quad2i_init4(1, 0, 1, 2), DbRows_getSubsOption(self->viewRow, "columns", TRUE, c, "units", TRUE), Lang_find("UNIT_OPTIONS"), DbValue_initLang("UNITS"), &GuiItemTable_clickRebuild));
			break;
		}

		case DbFormat_PERCENTAGE:
		{
			GuiItemLayout_addColumn(properLayout, 0, 99);
			GuiItemLayout_addColumn(properLayout, 1, 99);
			GuiItem_addSubName((GuiItem*)properLayout, "precision", GuiItemEdit_newEx(Quad2i_init4(0, 0, 1, 2), DbRows_getSubsOption(self->viewRow, "columns", TRUE, c, "precision", TRUE), DbValue_initLang("PRECISION"), &GuiItemTable_clickRebuild));

			GuiItem_addSubName((GuiItem*)properLayout, "mult100", GuiItemCheck_newEx(Quad2i_init4(1, 0, 2, 1), DbRows_getSubsOption(self->viewRow, "columns", TRUE, c, "mult100", TRUE), DbValue_initLang("MULTIPLAY_100"), &GuiItemTable_clickRebuild));
			GuiItem_addSubName((GuiItem*)properLayout, "bar", GuiItemCheck_newEx(Quad2i_init4(1, 1, 2, 1), DbRows_getSubsOption(self->viewRow, "columns", TRUE, c, "back_vis", TRUE), DbValue_initLang("PERCENTAGE_BACKGROUND"), &GuiItemTable_clickRebuild));
			break;
		}

		case DbFormat_CURRENCY:
		{
			GuiItemLayout_addColumn(properLayout, 0, 99);
			GuiItemLayout_addColumn(properLayout, 1, 99);
			GuiItem_addSubName((GuiItem*)properLayout, "precision", GuiItemEdit_newEx(Quad2i_init4(0, 0, 1, 2), DbRows_getSubsOption(self->viewRow, "columns", TRUE, c, "precision", TRUE), DbValue_initLang("PRECISION"), &GuiItemTable_clickRebuild));
			GuiItem_addSubName((GuiItem*)properLayout, "units", GuiItemComboStatic_newEx(Quad2i_init4(1, 0, 1, 2), DbRows_getSubsOption(self->viewRow, "columns", TRUE, c, "units", TRUE), Lang_find("UNIT_OPTIONS"), DbValue_initLang("UNITS"), &GuiItemTable_clickRebuild));

			GuiItem_addSubName((GuiItem*)properLayout, "currency_before", GuiItemEdit_newEx(Quad2i_init4(0, 2, 1, 2), DbRows_getSubsOption(self->viewRow, "columns", TRUE, c, "currency_before", TRUE), DbValue_initLang("CURRENCY_BEFORE"), &GuiItemTable_clickRebuild));
			GuiItem_addSubName((GuiItem*)properLayout, "currency_after", GuiItemEdit_newEx(Quad2i_init4(1, 2, 1, 2), DbRows_getSubsOption(self->viewRow, "columns", TRUE, c, "currency_after", TRUE), DbValue_initLang("CURRENCY_AFTER"), &GuiItemTable_clickRebuild));

			break;
		}

		case DbFormat_RATING:
		{
			GuiItemLayout_addColumn(properLayout, 0, 99);
			GuiItem_addSubName((GuiItem*)properLayout, "num_stars", GuiItemEdit_newEx(Quad2i_init4(0, 0, 1, 2), DbRows_getSubsOption(self->viewRow, "columns", TRUE, c, "numStars", TRUE), DbValue_initLang("NUMBER_STARS"), &GuiItemTable_clickRebuild));
			break;
		}

		case DbFormat_MENU:
		case DbFormat_TAGS:
		{
			GuiItemLayout_addColumn(properLayout, 0, 99);
			GuiItemLayout_addColumn(properLayout, 1, 99);

			BIG optionsRow = DbRows_findSubType(crowT, "options");

			DbValue colors = DbValue_initOption(crowT, "cd_enable", 0);
			const BOOL isColors = DbValue_getNumber(&colors);

			//skin
			GuiItemLayout* skin = GuiItemLayout_new(Quad2i_init4(0, 0, 1, 1));
			{
				GuiItemLayout_addColumn(skin, 1, 100);
				if (isColors)
					GuiItemLayout_addColumn(skin, 2, 2);

				GuiItem* drag = GuiItem_addSubName((GuiItem*)skin, "drag", GuiItemBox_newEmpty(Quad2i_init4(0, 0, 1, 1)));
				GuiItem_setIcon(drag, GuiImage_new1(UiIcons_init_reoder()));
				GuiItem_setDrop(drag, "option", "option", FALSE, DbRows_initLinkN(DbRoot_subs(), optionsRow), -1);

				GuiItem_addSubName((GuiItem*)skin, "name", GuiItemEdit_new(Quad2i_init4(1, 0, 1, 1), DbValue_initOption(-1, "name", 0), DbValue_initEmpty()));

				if (isColors)
					GuiItem_addSubName((GuiItem*)skin, "cd", (GuiItem*)GuiItemColor_new(Quad2i_init4(2, 0, 1, 1), DbValue_initOption(-1, "cd", 0), FALSE));
			}

			//colors enable
			GuiItem_addSubName((GuiItem*)properLayout, "cd_enable", GuiItemCheck_new(Quad2i_init4(0, 0, 3, 1), colors, DbValue_initLang("COLORS_ENABLE")));

			//list
			GuiItemList* list = (GuiItemList*)GuiItem_addSubName((GuiItem*)properLayout, "list", GuiItemList_new(Quad2i_init4(0, 1, 3, 5), DbRows_initLinkN(DbRoot_subs(), optionsRow), (GuiItem*)skin, DbValue_initEmpty()));
			GuiItemList_setShowRemove(list, TRUE);
			GuiItemList_setDrawBackground(list, TRUE);
			GuiItemList_setClickRemove(list, &GuiItemTable_clickRemoveOption);

			//+
			GuiItem* add = GuiItem_addSubName((GuiItem*)properLayout, "+", GuiItemButton_newClassicEx(Quad2i_init4(0, 6, 3, 1), DbValue_initStaticCopyCHAR("+"), &GuiItemTable_clickAddOptionLine));
			GuiItem_setAttribute(add, "cd", Rgba_asNumber(type == DbFormat_MENU ? Rgba_initWhite() : Rgba_initGreyDark()));

			break;
		}

		case DbFormat_SLIDER:
		{
			GuiItemLayout_addColumn(properLayout, 0, 99);
			GuiItemLayout_addColumn(properLayout, 1, 99);
			GuiItemLayout_addColumn(properLayout, 2, 99);

			//min & max & jump
			GuiItem_addSubName((GuiItem*)properLayout, "min", GuiItemEdit_newEx(Quad2i_init4(0, 0, 1, 2), DbRows_getSubsOption(self->viewRow, "columns", TRUE, c, "min", TRUE), DbValue_initLang("MINIMUM"), &GuiItemTable_clickRebuild));
			GuiItem_addSubName((GuiItem*)properLayout, "max", GuiItemEdit_newEx(Quad2i_init4(1, 0, 1, 2), DbRows_getSubsOption(self->viewRow, "columns", TRUE, c, "max", TRUE), DbValue_initLang("MAXIMUM"), &GuiItemTable_clickRebuild));
			GuiItem_addSubName((GuiItem*)properLayout, "jump", GuiItemEdit_newEx(Quad2i_init4(2, 0, 1, 2), DbRows_getSubsOption(self->viewRow, "columns", TRUE, c, "jump", TRUE), DbValue_initLang("JUMP"), &GuiItemTable_clickRebuild));

			break;
		}

		case DbFormat_CHECK:
		{
			GuiItemLayout_addColumn(properLayout, 0, 99);
			GuiItemLayout_addColumn(properLayout, 1, 99);

			//colors enable
			DbValue colors = DbValue_initOption(crowT, "cd_enable", 0);
			const BOOL isColors = DbValue_getNumber(&colors);
			GuiItem_addSubName((GuiItem*)properLayout, "cd_enable", GuiItemCheck_new(Quad2i_init4(0, 0, 2, 1), colors, DbValue_initLang("COLORS_ENABLE")));

			UNI* whiteCd = Std_newNumber(Rgba_asNumber(g_theme.white));

			GuiItem* ont = GuiItem_addSubName((GuiItem*)properLayout, "text_on", GuiItemText_new(Quad2i_init4(0, 1, 1, 1), TRUE, DbValue_initLang("CHECK_ON"), DbValue_initEmpty()));
			GuiItem* on = GuiItem_addSubName((GuiItem*)properLayout, "cd_on", (GuiItem*)GuiItemColor_new(Quad2i_init4(0, 2, 1, 1), DbValue_initOption(crowT, "cd_on", whiteCd), FALSE));

			GuiItem* offt = GuiItem_addSubName((GuiItem*)properLayout, "text_off", GuiItemText_new(Quad2i_init4(1, 1, 1, 1), TRUE, DbValue_initLang("CHECK_OFF"), DbValue_initEmpty()));
			GuiItem* off = GuiItem_addSubName((GuiItem*)properLayout, "cd_off", (GuiItem*)GuiItemColor_new(Quad2i_init4(1, 2, 1, 1), DbValue_initOption(crowT, "cd_off", whiteCd), FALSE));

			Std_deleteUNI(whiteCd);

			GuiItem_setEnable(ont, isColors);
			GuiItem_setEnable(on, isColors);
			GuiItem_setEnable(offt, isColors);
			GuiItem_setEnable(off, isColors);

			break;
		}

		case DbFormat_TEXT:
		{
			GuiItemLayout_addColumn(properLayout, 0, 99);
			//password
			GuiItem_addSubName((GuiItem*)properLayout, "password", GuiItemCheck_newEx(Quad2i_init4(0, 0, 1, 1), DbRows_getSubsOption(self->viewRow, "columns", TRUE, c, "password", TRUE), DbValue_initLang("PASSWORD"), &GuiItemTable_clickRebuild));
			break;
		}

		case DbFormat_PHONE:
		{
			break;
		}

		case DbFormat_URL:
		{
			break;
		}

		case DbFormat_EMAIL:
		{
			break;
		}

		case DbFormat_LOCATION:
		{
			break;
		}

		case DbFormat_DATE:
		{
			GuiItemLayout_addColumn(properLayout, 0, 99);
			GuiItem_addSubName((GuiItem*)properLayout, "time_format", GuiItemComboStatic_newEx(Quad2i_init4(0, 0, 2, 1), DbRows_getSubsOption(self->viewRow, "columns", TRUE, c, "timeFormat", TRUE), Lang_find("CALENDAR_FORMAT_TIME"), DbValue_initLang("TIME_FORMAT"), &GuiItemTable_clickRebuild));
			break;
		}

		case DbFormat_FILE_1:
		case DbFormat_FILE_N:
		{
			GuiItemLayout_addColumn(properLayout, 0, 99);

			GuiItem_addSubName((GuiItem*)properLayout, "preview", GuiItemCheck_newEx(Quad2i_init4(0, 0, 2, 1), DbRows_getSubsOption(self->viewRow, "columns", TRUE, c, "preview", TRUE), DbValue_initLang("PREVIEW"), &GuiItemTable_clickRebuild));
			break;
		}

		case DbFormat_LINK_1:
		case DbFormat_LINK_N:
		{
			GuiItemLayout_addColumn(properLayout, 0, 99);
			//GuiItemLayout_addColumn(properLayout, 1, 99);
			//GuiItemLayout_addColumn(properLayout, 2, 99);

			DbValue name = DbValue_initEmpty();
			DbTable* btable = DbColumn_getBTable(col);
			if (btable)
				name = DbValue_initOption(DbTable_getRow(btable), "name", 0);
			name.staticPre = Std_addUNI(Lang_find("SHOW"), _UNI32(" "));

			GuiItem_addSubName((GuiItem*)properLayout, "goto_table", GuiItemButton_newBlackEx(Quad2i_init4(0, 0, 1, 1), name, &GuiItemTable_clickGotoBtable));
			GuiItem_addSubName((GuiItem*)properLayout, "num_column_previews", GuiItemEdit_newEx(Quad2i_init4(0, 1, 1, 2), DbRows_getSubsOption(self->viewRow, "columns", TRUE, c, "numColumnPreviews", TRUE), DbValue_initLang("COLUMN_PREVIEWS"), &GuiItemTable_clickRebuild));

			//Move to table
			GuiItemMenu* mnMove = (GuiItemMenu*)GuiItem_addSubName((GuiItem*)properLayout, "column_move", GuiItemMenu_new(Quad2i_init4(0, 3, 1, 1), DbValue_initLang("COLUMN_MOVE"), FALSE));
			GuiItemMenu_setCenter(mnMove, FALSE);
			GuiItemMenu_addItem(mnMove, DbValue_initLang("YES_IAM_SURE"), &GuiItemTable_clickMoveLink);

			//Add to column
			GuiItem_addSubName((GuiItem*)properLayout, "copy_into_column", GuiItemButton_newBlackEx(Quad2i_init4(0, 4, 1, 1), DbValue_initLang("COPY_INTO_COLUMN"), &GuiItemTable_clickSendLinks));

			//column to render ...
			//none = first ...
			break;
		}

		case DbFormat_LINK_MIRRORED:
		{
			GuiItemLayout_addColumn(properLayout, 0, 99);

			UNI name[64];

			BIG mirrorRow = DbRows_findOrCreateSubType(crowT, "mirror");
			DbColumn* column = DbRoot_ref_column(mirrorRow);

			DbValue vnm = DbValue_initStaticCopy(column ? DbColumn_getPath(column, name, 64) : _UNI32("---"));

			//choose other Link Column mirror
			GuiItemButton* bt = (GuiItemButton*)GuiItem_addSubName((GuiItem*)properLayout, "other_link_column", GuiItemButton_newBlackEx(Quad2i_init4(0, 0, 1, 1), vnm, &GuiItemTable_clickLinkMirrorColumn));
			GuiItemButton_setWarningCd(bt, column == 0);
			break;
		}

		case DbFormat_LINK_JOINTED:
		{
			GuiItemLayout_addColumn(properLayout, 0, 99);

			DbTable* btable = DbColumn_getBTable(col);

			char nameId[64];

			int lnX = 0;

			BIG inColumnRow = DbRows_findOrCreateSubType(crowT, "in");
			BIG outColumnRow = DbRows_findOrCreateSubType(crowT, "out");

			//check:
				//src should be       table or null ...
				//dst should be from btable or null ...

			//src column
			GuiItem_addSubName((GuiItem*)properLayout, _GuiItemTable_getNameId(nameId, "%lld_src", crowT), GuiItemComboDynamic_new(Quad2i_init4(lnX, 0, 1, 2), TRUE, DbRows_initLink1(DbRoot_ref(), inColumnRow), DbValue_initOption(-1, "name", 0), DbRows_initSubsEx(table, "columns", FALSE, TRUE, TRUE, FALSE), DbValue_initOption(DbTable_getRow(table), "name", 0)));
			GuiItemLayout_addColumn(properLayout, lnX, 5);
			lnX++;

			//equal and +
			{
				GuiItem_addSubName((GuiItem*)properLayout, "=", GuiItemText_new(Quad2i_init4(lnX, 0, 1, 2), TRUE, DbValue_initStaticCopyCHAR("="), DbValue_initEmpty()));
				GuiItemButton* add = (GuiItemButton*)GuiItem_addSubName((GuiItem*)properLayout, _GuiItemTable_getNameId(nameId, "%lld_add", crowT), GuiItemButton_newBlackEx(Quad2i_init4(lnX, 2, 1, 1), DbValue_initStaticCopyCHAR("+"), &GuiItemTable_clickInsertItemToOptionLine));
				GuiItem_setAttribute((GuiItem*)add, "pos", 0);
				GuiItemButton_setCircle(add, TRUE);
				GuiItemLayout_addColumn(properLayout, lnX, -5);
				lnX++;
			}

			BIG pathRow = DbRows_findOrCreateSubType(crowT, "path");
			BIG it;
			UBIG ii = 0;
			int p = 1;
			while ((it = DbColumnN_jump(DbRoot_subs(), pathRow, &ii, 1)) >= 0)
			{
				BIG btableRow = DbRoot_ref_row(DbRows_findOrCreateSubType(it, "btable"));
				DbTable* tab = DbRoot_findTable(btableRow);

				BIG inRow = DbRows_findOrCreateSubType(it, "in");
				BIG outRow = DbRows_findOrCreateSubType(it, "out");

				//btable
				GuiItem_addSubName((GuiItem*)properLayout, _GuiItemTable_getNameId(nameId, "%lld_table", it), GuiItemText_new(Quad2i_init4(lnX, 0, 2, 1), TRUE, DbValue_initOption(btableRow, "name", 0), DbValue_initEmpty()));

				//columns
				GuiItem_addSubName((GuiItem*)properLayout, _GuiItemTable_getNameId(nameId, "%lld_in", it), GuiItemComboDynamic_new(Quad2i_init4(lnX + 0, 1, 1, 1), TRUE, DbRows_initLink1(DbRoot_ref(), inRow), DbValue_initOption(-1, "name", 0), DbRows_initSubsEx(tab, "columns", FALSE, TRUE, TRUE, FALSE), DbValue_initEmpty()));
				GuiItem_addSubName((GuiItem*)properLayout, _GuiItemTable_getNameId(nameId, "%lld_out", it), GuiItemComboDynamic_new(Quad2i_init4(lnX + 1, 1, 1, 1), TRUE, DbRows_initLink1(DbRoot_ref(), outRow), DbValue_initOption(-1, "name", 0), DbRows_initSubsEx(tab, "columns", FALSE, TRUE, TRUE, FALSE), DbValue_initEmpty()));

				//close
				{
					GuiItemButton* close = (GuiItemButton*)GuiItem_addSubName((GuiItem*)properLayout, _GuiItemTable_getNameId(nameId, "%lld_remove", it), GuiItemButton_newBlackEx(Quad2i_init4(lnX, 2, 2, 1), DbValue_initStaticCopyCHAR("X"), &GuiItemTable_clickRemoteItem));
					GuiItem_setAttribute((GuiItem*)close, "row", it);
					GuiItemButton_setCircle(close, TRUE);
				}
				//check: they should be from same table ...

				GuiItemLayout_addColumn(properLayout, lnX + 0, 5);
				GuiItemLayout_addColumn(properLayout, lnX + 1, 5);
				lnX += 2;

				//equal and +
				{
					GuiItem_addSubName((GuiItem*)properLayout, "=", GuiItemText_new(Quad2i_init4(lnX, 0, 1, 2), TRUE, DbValue_initStaticCopyCHAR("="), DbValue_initEmpty()));
					GuiItemButton* add = (GuiItemButton*)GuiItem_addSubName((GuiItem*)properLayout, _GuiItemTable_getNameId(nameId, "%lld_add", it), GuiItemButton_newBlackEx(Quad2i_init4(lnX, 2, 1, 1), DbValue_initStaticCopyCHAR("+"), &GuiItemTable_clickInsertItemToOptionLine));
					GuiItem_setAttribute((GuiItem*)add, "pos", p);
					GuiItemButton_setCircle(add, TRUE);
					GuiItemLayout_addColumn(properLayout, lnX, -5);
					lnX++;
				}

				p++;
				ii++;
			}

			//dst column
			GuiItem_addSubName((GuiItem*)properLayout, _GuiItemTable_getNameId(nameId, "%lld_dst", crowT), GuiItemComboDynamic_new(Quad2i_init4(lnX, 0, 1, 2), TRUE, DbRows_initLink1(DbRoot_ref(), outColumnRow), DbValue_initOption(-1, "name", 0), DbRows_initSubsEx(btable, "columns", FALSE, TRUE, TRUE, FALSE), DbValue_initOption(DbTable_getRow(btable), "name", 0)));
			GuiItemLayout_addColumn(properLayout, lnX, 5);
			lnX++;

			break;
		}

		case DbFormat_LINK_FILTERED:
		{
			GuiItemLayout_addColumn(properLayout, 0, 99);

			BIG sourceRow = DbRows_findOrCreateSubType(crowT, "source_column");
			BIG filteredColumnRow = DbRoot_ref_row(sourceRow);
			DbColumn* column = DbRoot_findColumn(filteredColumnRow);

			//source column info
			GuiItem_addSubName((GuiItem*)properLayout, "other_link_column", GuiItemText_new(Quad2i_init4(0, 0, 1, 1), TRUE, DbValue_initOption(filteredColumnRow, "name", 0), DbValue_initEmpty()));

			//filter
			GuiItemLayout* filterLayout = GuiItemTable_buildSelectList(Quad2i_init4(0, 1, 1, 6), crowT, DbColumn_getBTable(column));
			GuiItemLayout_setDrawBackground(filterLayout, FALSE);
			GuiItemLayout* list = GuiItem_findName((GuiItem*)filterLayout, "list");
			list->whiteScroll = TRUE;

			GuiItem_addSubName((GuiItem*)properLayout, "filter", (GuiItem*)filterLayout);

			break;
		}

		case DbFormat_SUMMARY:
		{
			int posX = 0;

			//function
			GuiItemComboStatic* func = GuiItem_addSubName((GuiItem*)properLayout, "summary_func", GuiItemComboStatic_new(Quad2i_init4(posX, 0, 1, 2), DbValue_initOption(crowT, "insight_func", 0), 0, DbValue_initLang("INSIGHT_FUNCTION")));
			//GuiItem_setIcon(&func->base, GuiImage_new1(UiIcons_init_column_insight()));
			GuiItemLayout_addColumn(properLayout, posX, 3);
			posX++;
			int ii;
			for (ii = 0; ii < DbInsightSelectFunc_num(); ii++)
				GuiItemComboStatic_addItem(func, DbValue_initLang(DbInsightSelectFunc_getName(ii)));

			//path
			BIG propRow = DbRows_findOrCreateSubType(crowT, "summary");
			DbTable* btable = table;
			BIG it;
			UBIG i = 0;
			while (btable && (it = DbColumnN_jump(DbRoot_subs(), propRow, &i, 1)) >= 0)
			{
				UBIG ii = i + 1;
				BOOL isLast = (DbColumnN_jump(DbRoot_subs(), propRow, &ii, 1) < 0);

				char nameId[64];
				snprintf(nameId, 64, "%lld_column", it);

				BIG columnRow = DbRoot_ref_row(it);
				DbColumn* column = DbRoot_findColumn(columnRow);

				UNI nameBtable[64];
				DbTable_getName(btable, nameBtable, 64);

				if (isLast)
				{
					//combo
					GuiItem_addSubName((GuiItem*)properLayout, nameId, GuiItemComboDynamic_new(Quad2i_init4(posX, 0, 1, 2), TRUE, DbRows_initLink1(DbRoot_ref(), it), DbValue_initOption(-1, "name", 0), DbRows_initSubsEx(btable, "columns", FALSE, posX > 0, posX > 0, posX >= 0), DbValue_initStaticCopy(nameBtable)));
					GuiItemLayout_addColumn(properLayout, posX, 5);
					posX++;
				}
				else
				{
					//text
					GuiItem_addSubName((GuiItem*)properLayout, nameId, GuiItemText_new(Quad2i_init4(posX, 0, 1, 2), TRUE, DbValue_initOption(columnRow, "name", 0), DbValue_initStaticCopy(nameBtable)));
					GuiItemLayout_addColumn(properLayout, posX, 5);

					//remove
					snprintf(nameId, 64, "%lld_del", it);
					GuiItem* remove = GuiItem_addSubName((GuiItem*)properLayout, nameId, GuiItemButton_newBlackEx(Quad2i_init4(posX, 2, 1, 1), DbValue_initStaticCopyCHAR("x"), &GuiItemTable_clickRemoveSummaryItem));	//callback ...
					GuiItem_setAttribute(remove, "it", it);
					posX++;
				}

				btable = column ? DbColumn_getBTable(column) : 0;

				//arrow
				if (!isLast)
				{
					GuiItem_addSubName((GuiItem*)properLayout, "arrow", GuiItemText_new(Quad2i_init4(posX, 0, 1, 2), TRUE, DbValue_initStaticCopyCHAR("=>"), DbValue_initEmpty()));
					posX++;
				}

				i++;
			}

			break;
		}

		case DbFormat_ROW:
		{
			break;
		}
	}
}
void GuiItemTable_resizeColumnDialogProperties(GuiItem* item)
{
	GuiItemTable* self = GuiItem_findParentType(item->parent, GuiItem_TABLE);
	if (self)
		GuiItemTable_buildColumnDialogProperties(item, self);
}

void GuiItemTable_clickGotoRow(GuiItem* item)
{
	GuiItemEdit* edit = GuiItem_findNameType(item, "edit", GuiItem_EDIT);
	GuiItemTable* self = GuiItem_findParentType(item, GuiItem_TABLE);
	if (edit && self)
	{
		BIG row = Std_bmax(0, GuiItemEdit_getNumber(edit));

		BIG pos = DbRows_findRowScroll(&self->filter, row);
		//BIG pos = DbTable_findRowScroll(GuiItemTable_getTable(self), row);
		if (pos >= 0)
		{
			const int N_COLS = GuiItemTable_numColumns(self);
			self->selectFirstTouch = Vec2i_init2(0, pos);
			self->selectLastTouch = Vec2i_init2(N_COLS - 1, pos);

			GuiScroll_setWheelRow(&self->scrollV, pos);

			GuiItemTable_saveSelect(self);

			OsWinIO_resetKeyEXTRA();
			GuiItemRoot_closeLevelTop();
		}
		else
			GuiItemEdit_setHighlightIfContent(edit, TRUE);
	}
}
void GuiItemTable_clickOpenGotoRowDialog(GuiItem* self)
{
	GuiItemLayout* layout = GuiItemLayout_new(Quad2i_init());
	layout->drawBorder = TRUE;
	GuiItemLayout_addColumn(layout, 0, 10);

	GuiItem_addSubName(&layout->base, "edit", GuiItemEdit_newEx(Quad2i_init4(0, 0, 1, 1), DbValue_initNumber(1), DbValue_initEmpty(), &GuiItemTable_clickGotoRow));
	GuiItem* b = GuiItem_addSubName(&layout->base, "button", GuiItemButton_newClassicEx(Quad2i_init4(0, 1, 1, 1), DbValue_initLang("GOTO_ROW"), &GuiItemTable_clickGotoRow));
	//GuiItem_setShortcutKey(b, FALSE, Win_EXTRAKEY_ENTER, 0, &GuiItemTable_clickGoto);

	const int cell = OsWinIO_cellSize();
	GuiItemRoot_addDialogRel((GuiItem*)layout, (GuiItem*)self, Quad2i_getSub(self->coordMoveCut, Vec2i_init2(10 * cell, 2 * cell)), TRUE);
}

void GuiItemTable_callbackDropIdColumn(GuiItem* dstItem, BIG dstRow, BIG srcRow, BOOL findIn)
{
	GuiItemTable* self = GuiItem_findParentType(dstItem, GuiItem_TABLE);
	if (self)
	{
		UBIG idColumnsRow = DbRoot_findOrCreateChildType(self->viewRow, "id_columns");

		DbColumn* column = DbRoot_findColumn(srcRow);
		if (!column)
			column = DbRoot_findColumn(DbRoot_ref_row(srcRow));

		BIG line = DbRoot_findSubLineRefRow(idColumnsRow, DbColumn_getRow(column));
		DbRoot_setEnable(line, TRUE);

		GuiItemMenu_showContext(((GuiItemMenu*)dstItem));
	}
}

GuiItemLayout* GuiItemTable_resize(GuiItemTable* self, GuiItemLayout* layout, Win* win)
{
	if (!self->base.resize)
		return (GuiItemLayout*)GuiItem_getSub(&self->base, 0);

	//_GuiItemTable_clearInsights(self);

	const int cell = OsWinIO_cellSize();

	GuiItem_freeSubs(&self->base);

	//layout(will hold table cells)
	//layout = GuiItemLayout_newCoord(&self->base, FALSE, TRUE, win);
	layout = GuiItemLayout_newCoord(&self->base, FALSE, FALSE, win);
	GuiItemLayout_setDrawBackground(layout, FALSE);
	GuiItem_addSubName(&self->base, "main_layout", &layout->base);
	//GuiItemLayout_setScrollH(layout, DbValue_initCopy(&self->scrollH));
	GuiItemLayout_addRow(layout, 0, 99);
	GuiItemLayout_addColumn(layout, 0, GuiItemTable_getColumnIdsWidth(self));
	GuiItemLayout_addColumn(layout, 1, 99);

	GuiItemLayout* ids_layout = GuiItemLayout_new(Quad2i_init4(0, 0, 1, 1));
	GuiItemLayout_setDrawBackground(ids_layout, TRUE);
	//GuiItemLayout_setBackgroundBlack(ids_layout, TRUE);
	GuiItemLayout_showScroll(ids_layout, FALSE, FALSE);
	GuiItem_addSubName(&layout->base, "ids_layout", &ids_layout->base);
	GuiItem_setShortcutKey((GuiItem*)layout, FALSE, Win_EXTRAKEY_CTRL | Win_EXTRAKEY_GOTO, 0, &GuiItemTable_clickOpenGotoRowDialog);
	GuiItem_setChangeSize(&ids_layout->base, TRUE, DbValue_initOption(self->viewRow, "width_ids", _UNI32("4")), FALSE, FALSE, Rgba_initBlack());

	GuiItemLayout* cells_layout = GuiItemLayout_new(Quad2i_init4(1, 0, 1, 1));
	GuiItemLayout_setDrawBackground(cells_layout, FALSE);
	GuiItemLayout_showScroll(cells_layout, FALSE, TRUE);
	GuiItemLayout_setScrollH(cells_layout, DbValue_initCopy(&self->scrollH));
	GuiItem_addSubName(&layout->base, "cells_layout", &cells_layout->base);

	//cells will not be under scroll
	layout->base.coordScreen.size.x -= GuiScroll_widthWin(win);
	//cells_layout->base.coordScreen.size.y -= GuiScroll_widthWin(win);

	GuiItem_setShortcutKey((GuiItem*)ids_layout, FALSE, Win_EXTRAKEY_CTRL | Win_EXTRAKEY_ADD_RECORD, 0, &GuiItemTable_clickAddRecord);

	const int N_COLS = GuiItemTable_numColumns(self);

	int width_sum = 0;// GuiItemTable_getColumnIdsWidth(self);
	BIG c;
	//for (c = 0; c < N_COLS; c++)
	//	width_sum += GuiItemTable_getColumnWidth(self, c);
	const int header_x = 0;// 1 + self->showChangeOrder;	//[card_icon][order]
	const int header_y = self->showHeader;

	BOOL isFilter = !DbRoot_isType_table(self->viewRow);

	self->showAddRecord &= !isFilter;

	const int RS = GuiItemTable_getRowSize(self);

	//Vec2i screenSize;
	//OsScreen_getMonitorResolution(&screenSize);
	//int N_ROWS_SCREEN = Std_max(1, screenSize.y / cell / RS);
	//self->num_rows_max = N_ROWS_SCREEN;

	self->num_rows_max = (self->base.coordMove.size.y - GuiItemTable_scrollExtra(self, win)) / cell / RS;
	self->num_rows_max++;	//extra row

	BIG r;
	//GuiItemLayout_addColumn(ids_layout, self->showChangeOrder + 1, 2 + (self->showAddButton || self->showRemoveButton)); //row menus

	const BOOL mode_summary = GuiItemTable_isSummaryTable(self);

	//columns settings
	for (c = 0; c < N_COLS; c++)
	{
		DbFormatTYPE type = GuiItemTable_getColumnType(self, c);
		int width = GuiItemTable_getColumnWidth(self, c);

		DbColumn* col = GuiItemTable_getColumn(self, c);
		BIG crow = GuiItemTable_getColumnRow(self, c);	//table/filter
		const BOOL remote = DbColumn_isRemote(col);
		//BIG crowT = DbColumn_getRow(col);	//table

		//const BOOL modeSummaryFunc = mode_summary && !DbColumn_isSummaryLinks(col) && !DbColumn_isSummaryGroup(col);
		const BOOL modeSummaryLinks = mode_summary && DbColumn_isSummaryLinks(col);
		const BOOL modeSummaryGroup = mode_summary && DbColumn_isSummaryGroup(col);
		const BOOL modeSummaryFunc = mode_summary && !modeSummaryLinks && !modeSummaryGroup;

		char crowName[64];
		snprintf(crowName, 64, "%lld_d", crow);

		GuiItemLayout* headColumn = GuiItemLayout_new(Quad2i_init4(header_x + width_sum, 0, width, 1));
		headColumn->extraSpace = 1;
		GuiItemLayout_addColumn(headColumn, 0, 20);
		if (modeSummaryFunc)
			GuiItemLayout_addColumn(headColumn, 1, 10);
		GuiItem_addSubName(&cells_layout->base, crowName, &headColumn->base);

		if (modeSummaryGroup)
			GuiItemLayout_setBackgroundWhite(headColumn, TRUE);
		else
			GuiItemLayout_setBackgroundMain(headColumn, TRUE);

		GuiItem_setChangeSize(&headColumn->base, TRUE, DbValue_initOption(crow, "width", 0), FALSE, FALSE, g_theme.white);
		GuiItem_setDrop(&headColumn->base, "column", "column", TRUE, DbRows_getSubsArray(self->viewRow, "columns"), crow);
		GuiItem_setIcon(&headColumn->base, GuiImage_new1(GuiItem_getColumnIcon(type)));

		if (modeSummaryFunc)
		{
			//choose function
			GuiItemComboStatic* func = GuiItem_addSubName((GuiItem*)headColumn, "summary_func", GuiItemComboStatic_new(Quad2i_init4(1, 0, 1, 1), DbValue_initOption(crow, "insight_func", 0), 0, DbValue_initEmpty()));
			int i;
			for (i = 0; i < DbInsightSelectFunc_num(); i++)
				GuiItemComboStatic_addItem(func, DbValue_initLang(DbInsightSelectFunc_getName(i)));
		}

		GuiItemMenu* mn = (GuiItemMenu*)GuiItem_addSubName(&headColumn->base, "name", GuiItemMenu_new(Quad2i_init4(0, 0, 1, 1), DbRows_getSubsOption(self->viewRow, "columns", TRUE, c, "name", TRUE), FALSE));
		if (!modeSummaryGroup)
			GuiItemMenu_setHighligthBackground(mn, TRUE, 1.0f);
		mn->textCenter = FALSE;

		if (DbColumn_isErr(col))
			mn->err = TRUE;

		GuiItemLayout* settingsColumn = GuiItemLayout_new(Quad2i_init());
		GuiItemLayout_addColumn(settingsColumn, 0, 5);
		GuiItemLayout_addColumn(settingsColumn, 1, 5);
		GuiItemLayout_addColumn(settingsColumn, 2, 5);
		GuiItemMenu_setContext(mn, settingsColumn);

		GuiItem_setAttribute((GuiItem*)settingsColumn, "c", c);

		int y = 0;
		if (!mode_summary)
		{
			GuiItem* name = GuiItem_addSubName((GuiItem*)settingsColumn, "name", GuiItemEdit_new(Quad2i_init4(0, y++, 3, 1), DbRows_getSubsOption(self->viewRow, "columns", TRUE, c, "name", TRUE), DbValue_initLang("NAME")));
			GuiItem_setIcon(name, GuiImage_new1(GuiItem_getColumnIcon(type)));
			//GuiItem_setAlternativeIconCd(name, remote);
			GuiItem_setEnable(name, !remote);

			//description
			GuiItem_addSubName((GuiItem*)settingsColumn, "description", GuiItemEdit_new(Quad2i_init4(0, y, 3, 2), DbRows_getSubsOption(self->viewRow, "columns", TRUE, c, "description", TRUE), DbValue_initLang("DESCRIPTION")));
			y += 2;
		}

		//convert types
		if (!mode_summary)
		{
			GuiItemComboStatic* comboConvert = GuiItem_addSubName((GuiItem*)settingsColumn, "format", GuiItemComboStatic_newEx(Quad2i_init4(0, y, 3, 2), DbValue_initNumber(0), 0, DbValue_initLang("FORMAT"), &GuiItemTable_clickColumnConvert));
			GuiItemComboStatic_addItemIcon(comboConvert, GuiImage_new1(GuiItem_getColumnIcon(type)), DbValue_initStaticCopy(Lang_find(DbColumnFormat_findColumnLang(type))));

			const int numConverts = DbColumnConvert_num(type, remote);
			int co;
			for (co = 0; co < numConverts; co++)
			{
				const DbColumnConvert* cc = DbColumnConvert_get(type, co, remote);
				if (cc)
					GuiItemComboStatic_addItemIcon(comboConvert, GuiImage_new1(GuiItem_getColumnIcon(cc->dstType)), DbValue_initStaticCopy(Lang_find(DbColumnFormat_findColumnLang(cc->dstType))));
			}

			y += 2;
		}

		if (!mode_summary)
		{
			y++;	//space

			GuiItemLayout* propertiesLayout = GuiItemLayout_new(Quad2i_init4(0, y, 3, 1));
			//GuiItemLayout_setBackgroundMain(propertiesLayout, TRUE);
			GuiItemLayout_setBackgroundGrey(propertiesLayout, TRUE);

			GuiItemLayout_setResize(propertiesLayout, &GuiItemTable_resizeColumnDialogProperties);
			GuiItem_addSubName((GuiItem*)settingsColumn, "propertiesLayout", (GuiItem*)propertiesLayout);

			GuiItemTable_buildColumnDialogProperties((GuiItem*)propertiesLayout, self);
			propertiesLayout->base.grid.size.y = GuiItem_getSubMaxGrid((GuiItem*)propertiesLayout).size.y;
			y += propertiesLayout->base.grid.size.y + 1;
		}

		GuiItem_addSubName((GuiItem*)settingsColumn, "short", GuiItemButton_newNoCenterEx(Quad2i_init4(0, y++, 3, 1), DbValue_initLang("ADD_TO_SHORT"), &GuiItemTable_clickShortColumn));
		if (isFilter)
			GuiItem_addSubName((GuiItem*)settingsColumn, "filter", GuiItemButton_newNoCenterEx(Quad2i_init4(0, y++, 3, 1), DbValue_initLang("ADD_TO_FILTER"), &GuiItemTable_clickFilterColumn));
		GuiItem_addSubName((GuiItem*)settingsColumn, "hide", GuiItemButton_newNoCenterEx(Quad2i_init4(0, y++, 3, 1), DbValue_initLang("HIDE"), &GuiItemTable_clickHideColumn));

		if (!mode_summary)
			GuiItem_addSubName((GuiItem*)settingsColumn, "lock", GuiItemCheck_new(Quad2i_init4(0, y++, 3, 1), DbRows_getSubsOption(self->viewRow, "columns", TRUE, c, "lock", FALSE), DbValue_initLang("LOCK")));

		if (!mode_summary)
		{
			GuiItem* duplicate = GuiItem_addSubName((GuiItem*)settingsColumn, "duplicate", GuiItemButton_newNoCenterEx(Quad2i_init4(0, y++, 3, 1), DbValue_initLang("DUPLICATE"), &GuiItemTable_clickDuplicateColumn));
			GuiItem_setEnable(duplicate, !remote);
		}

		if (!mode_summary)
		{
			GuiItemMenu* mnRemove = (GuiItemMenu*)GuiItem_addSubName((GuiItem*)settingsColumn, "remove", GuiItemMenu_new(Quad2i_init4(0, y++, 3, 1), DbValue_initLang("REMOVE"), FALSE));
			GuiItemMenu_setCenter(mnRemove, FALSE);
			GuiItemMenu_addItem(mnRemove, DbValue_initLang("YES_IAM_SURE"), &GuiItemTable_clickRemoveColumn);
			GuiItem_setEnable((GuiItem*)mnRemove, !remote);
		}

		width_sum += width;
	}

	if (!isFilter)
	{
		//create new column
		GuiItemMenu* newColumn = (GuiItemMenu*)GuiItem_addSubName(&cells_layout->base, "+column", GuiItemMenu_new(Quad2i_init4(header_x + width_sum, 0, 2, 1), DbValue_initStaticCopy(_UNI32("+")), FALSE));
		GuiItemMenu_setHighligthBackground(newColumn, TRUE, 1.0f);
		GuiItemMenu_setTransparent(newColumn, FALSE);
		newColumn->base.drawTable = TRUE;
		newColumn->underline = FALSE;
		GuiItemMenu_setCloseAuto(newColumn, FALSE);
		GuiItemMenu_addItemIcon(newColumn, GuiImage_new1(UiIcons_init_column_text()), DbValue_initLang("COLUMN_TEXT"), &GuiItemTable_clickAddColumn, FALSE, FALSE, DbFormat_TEXT);
		GuiItemMenu_addItemIcon(newColumn, GuiImage_new1(UiIcons_init_column_number()), DbValue_initLang("COLUMN_NUMBER_1"), &GuiItemTable_clickAddColumn, FALSE, TRUE, DbFormat_NUMBER_1);
		//GuiItemMenu_addItemIcon(newColumn, GuiImage_new1(UiIcons_init_column_file()), DbValue_initLang("COLUMN_NUMBER_N"), &GuiItemTable_clickAddColumn, FALSE, DbFormat_NUMBER_N);
		//GuiItemMenu_addItemEmpty(newColumn);
		GuiItemMenu_addItemIcon(newColumn, GuiImage_new1(UiIcons_init_column_currency()), DbValue_initLang("COLUMN_CURRENCY"), &GuiItemTable_clickAddColumn, FALSE, FALSE, DbFormat_CURRENCY);
		GuiItemMenu_addItemIcon(newColumn, GuiImage_new1(UiIcons_init_column_percentage()), DbValue_initLang("COLUMN_PERCENTAGE"), &GuiItemTable_clickAddColumn, FALSE, FALSE, DbFormat_PERCENTAGE);
		GuiItemMenu_addItemIcon(newColumn, GuiImage_new1(UiIcons_init_column_rating()), DbValue_initLang("COLUMN_RATING"), &GuiItemTable_clickAddColumn, FALSE, TRUE, DbFormat_RATING);
		//GuiItemMenu_addItemEmpty(newColumn);
		GuiItemMenu_addItemIcon(newColumn, GuiImage_new1(UiIcons_init_column_phone()), DbValue_initLang("COLUMN_PHONE"), &GuiItemTable_clickAddColumn, FALSE, FALSE, DbFormat_PHONE);
		GuiItemMenu_addItemIcon(newColumn, GuiImage_new1(UiIcons_init_column_url()), DbValue_initLang("COLUMN_URL"), &GuiItemTable_clickAddColumn, FALSE, FALSE, DbFormat_URL);
		GuiItemMenu_addItemIcon(newColumn, GuiImage_new1(UiIcons_init_column_email()), DbValue_initLang("COLUMN_EMAIL"), &GuiItemTable_clickAddColumn, FALSE, FALSE, DbFormat_EMAIL);
		GuiItemMenu_addItemIcon(newColumn, GuiImage_new1(UiIcons_init_column_location()), DbValue_initLang("COLUMN_LOCATION"), &GuiItemTable_clickAddColumn, FALSE, TRUE, DbFormat_LOCATION);
		//GuiItemMenu_addItemEmpty(newColumn);
		GuiItemMenu_addItemIcon(newColumn, GuiImage_new1(UiIcons_init_column_date()), DbValue_initLang("COLUMN_DATE"), &GuiItemTable_clickAddColumn, FALSE, FALSE, DbFormat_DATE);
		GuiItemMenu_addItemIcon(newColumn, GuiImage_new1(UiIcons_init_column_menu()), DbValue_initLang("COLUMN_MENU"), &GuiItemTable_clickAddColumn, FALSE, FALSE, DbFormat_MENU);
		GuiItemMenu_addItemIcon(newColumn, GuiImage_new1(UiIcons_init_column_tags()), DbValue_initLang("COLUMN_TAGS"), &GuiItemTable_clickAddColumn, FALSE, FALSE, DbFormat_TAGS);
		GuiItemMenu_addItemIcon(newColumn, GuiImage_new1(UiIcons_init_column_slider()), DbValue_initLang("COLUMN_SLIDER"), &GuiItemTable_clickAddColumn, FALSE, FALSE, DbFormat_SLIDER);
		GuiItemMenu_addItemIcon(newColumn, GuiImage_new1(UiIcons_init_column_check()), DbValue_initLang("COLUMN_CHECK"), &GuiItemTable_clickAddColumn, FALSE, TRUE, DbFormat_CHECK);
		//GuiItemMenu_addItemEmpty(newColumn);
		GuiItemMenu_addItemIcon(newColumn, GuiImage_new1(UiIcons_init_column_insight()), DbValue_initLang("COLUMN_SUMMARY"), &GuiItemTable_clickAddColumn, FALSE, TRUE, DbFormat_SUMMARY);
		//GuiItemMenu_addItemEmpty(newColumn);
		GuiItemMenu_addItemIcon(newColumn, GuiImage_new1(UiIcons_init_column_link()), DbValue_initLang("COLUMN_LINK_1"), &GuiItemTable_clickAddColumn, FALSE, FALSE, DbFormat_LINK_1);
		GuiItemMenu_addItemIcon(newColumn, GuiImage_new1(UiIcons_init_column_link()), DbValue_initLang("COLUMN_LINK_N"), &GuiItemTable_clickAddColumn, FALSE, FALSE, DbFormat_LINK_N);
		GuiItemMenu_addItemIcon(newColumn, GuiImage_new1(UiIcons_init_column_link()), DbValue_initLang("COLUMN_LINK_MIRRORED"), &GuiItemTable_clickAddColumn, FALSE, FALSE, DbFormat_LINK_MIRRORED);
		GuiItemMenu_addItemIcon(newColumn, GuiImage_new1(UiIcons_init_column_link()), DbValue_initLang("COLUMN_LINK_JOINTED"), &GuiItemTable_clickAddColumn, FALSE, FALSE, DbFormat_LINK_JOINTED);
		GuiItemMenu_addItemIcon(newColumn, GuiImage_new1(UiIcons_init_column_link()), DbValue_initLang("COLUMN_LINK_FILTERED"), &GuiItemTable_clickAddColumn, FALSE, TRUE, DbFormat_LINK_FILTERED);
		//GuiItemMenu_addItemEmpty(newColumn);
		GuiItemMenu_addItemIcon(newColumn, GuiImage_new1(UiIcons_init_column_file()), DbValue_initLang("COLUMN_FILE_1"), &GuiItemTable_clickAddColumn, FALSE, FALSE, DbFormat_FILE_1);
		GuiItemMenu_addItemIcon(newColumn, GuiImage_new1(UiIcons_init_column_file()), DbValue_initLang("COLUMN_FILE_N"), &GuiItemTable_clickAddColumn, FALSE, FALSE, DbFormat_FILE_N);
	}

	//top add record
	{
		GuiItem_addSubName(&ids_layout->base, "+record_top", GuiItemButton_newAlphaEx(Quad2i_init4(0, 0, 1, 1), DbValue_initStaticCopyCHAR("@"), &GuiItemTable_clickAddRecord));
	}

	//Id description
	{
		GuiItemMenu* mn = (GuiItemMenu*)GuiItem_addSubName(&ids_layout->base, "id_columns", GuiItemMenu_new(Quad2i_init4(1, 0, self->showChangeOrder + 1, 1), DbValue_initLang("ROWS_TITLES"), FALSE));
		GuiItemMenu_setContext(mn, GuiItemTable_buildIDShowedList(Quad2i_init(), self->viewRow));

		GuiItem_setDropIN(&mn->base, "column", DbRows_initEmpty());
		GuiItem_setDropCallback(&mn->base, &GuiItemTable_callbackDropIdColumn);
		mn->base.dropDontRemove = TRUE;
	}

	//order
	if (self->showChangeOrder)
	{
		self->subs_ids_start_rowOptionOrder = ids_layout->base.subs.num;
		for (r = 0; r < self->num_rows_max; r++)
		{
			char nameId[64];
			snprintf(nameId, 64, "%lld_o", r);

			GuiItem* drag = GuiItem_addSubName(&ids_layout->base, nameId, GuiItemBox_newEmpty(Quad2i_init4(0, header_y + r * RS, 1, RS)));
			GuiItem_setIcon(drag, GuiImage_new1(UiIcons_init_reoder()));
			GuiItem_setDrop(drag, self->changeOrderNameSrc, self->changeOrderNameDst, FALSE, DbRows_initCopy(&self->filter), -1);
			drag->dropDontRemove = TRUE;
		}
	}

	//cards
	self->subs_ids_start_rowOptionCards = ids_layout->base.subs.num;
	for (r = 0; r < self->num_rows_max; r++)
	{
		char nameId[64];
		snprintf(nameId, 64, "%lld_k", r);

		const Quad2i grid = Quad2i_init4(self->showChangeOrder, header_y + r * RS, 1, RS);

		//card button
		GuiItem_addSubName(&ids_layout->base, nameId, GuiItemButton_newImage(grid, GuiImage_new1(UiIcons_init_card()), TRUE, &GuiItemTable_clickShowPage));
	}

	//rows ids(first column)
	self->subs_ids_start_rowOption = ids_layout->base.subs.num;
	for (r = 0; r < self->num_rows_max; r++)
	{
		char nameId[64];
		snprintf(nameId, 64, "%lld_i", r);

		const Quad2i grid = Quad2i_init4(self->showChangeOrder + 1, header_y + r * RS, 1, RS);
		if (self->showAddButton)
		{
			GuiItem_addSubName(&ids_layout->base, nameId, GuiItemButton_newClassicEx(grid, DbValue_initLang("ADD"), &UiDialogLink_clickLinkDialogAddOne));
		}
		else
			if (self->showRemoveButton)
			{
				GuiItem_addSubName(&ids_layout->base, nameId, GuiItemButton_newClassicEx(grid, DbValue_initStaticCopy(_UNI32("X")), &GuiItemTable_clickRemoveRow));
			}
			else
			{
				GuiItemLayout* headColumn = GuiItemLayout_new(grid);
				GuiItem_addSubName(&ids_layout->base, nameId, &headColumn->base);

				//text
				GuiItemMenu* mn = (GuiItemMenu*)GuiItem_addSubName(&headColumn->base, "id", GuiItemMenu_new(Quad2i_init4(0, 0, 1, RS), DbValue_initGET(DbTable_getIdsColumn(GuiItemTable_getTable(self)), -1), FALSE));
				GuiItemLayout_addColumn(headColumn, 0, 99);
				GuiItemMenu_addItem(mn, DbValue_initLang("CARD"), &GuiItemTable_clickShowPage);
				if (!mode_summary)
					GuiItemMenu_addItem(mn, DbValue_initLang("REMOVE"), &GuiItemTable_clickRemoveRow);
				GuiItemMenu_setHighligthBackground(mn, TRUE, 1.0f);
				GuiItemMenu_setTransparent(mn, FALSE);
				mn->base.drawTable = TRUE;

				BIG idColumnsRow = DbRows_findOrCreateSubType(self->viewRow, "id_columns");
				int ic = 0;
				UBIG i = 0;
				BIG it;
				while ((it = DbColumnN_jump(DbRoot_subs(), idColumnsRow, &i, 1)) >= 0)
				{
					if (DbRows_isEnable(it))
					{
						DbColumn* idColumn = DbRoot_ref_column(it);

						GuiItemLayout_addColumn(headColumn, 1 + ic, 99);
						GuiItem* name = GuiItem_addSubName((GuiItem*)headColumn, "value", GuiItemTable_getCardSkinItem(Quad2i_init4(1 + ic, 0, 1, RS), DbValue_initGET(idColumn, -1), FALSE, self->searchHighlight));
						name->drawTable = TRUE;
						if (name->type == GuiItem_TEXT)
							((GuiItemText*)name)->drawBackground = FALSE;
						GuiItem_setTouchRecommand(name, DbValue_getOptionNumber(idColumnsRow, "edit", 0));

						ic++;
					}

					i++;
				}
			}

		GuiItemLayout_addColumn(ids_layout, grid.start.x, 99);
	}

	if (self->showAddRecord && !mode_summary)
	{
		self->subs_ids_add_record = ids_layout->base.subs.num;
		GuiItemButton* b = (GuiItemButton*)GuiItem_addSubName(&ids_layout->base, "+record", GuiItemButton_newClassicEx(Quad2i_init(), DbValue_initStaticCopyCHAR("+"), &GuiItemTable_clickAddRecord));
		GuiItem_setShow(&b->base, FALSE);	//update will set it visible
	}

	//cells
	self->subs_cells_start = cells_layout->base.subs.num;
	width_sum = 0;// GuiItemTable_getColumnIdsWidth(self);
	for (c = 0; c < N_COLS; c++)
	{
		const int width = GuiItemTable_getColumnWidth(self, c);
		for (r = 0; r < self->num_rows_max; r++)
		{
			char nameId[64];
			snprintf(nameId, 64, "%lld_%lld_rec", c, r);

			Quad2i q = Quad2i_init4(header_x + width_sum, header_y + r * RS, width, RS);

			GuiItem* itt = GuiItem_addSubName(&cells_layout->base, nameId, GuiItemTable_getCardSkinItem(q, DbRows_getSubsCell(self->viewRow, "columns", TRUE, c, -1), FALSE, self->searchHighlight));// , FALSE));
			itt->drawTable = TRUE;
		}

		width_sum += width;
	}
	self->subs_cells_end = cells_layout->base.subs.num;

	//insights
	{
		GuiItemLayout* ids_layout2 = GuiItemLayout_new(Quad2i_init4(0, 1, 1, 1));
		//GuiItemLayout_setDrawBackground(ids_layout2, FALSE);
		//GuiItemLayout_setBackgroundBlack(ids_layout2, TRUE);
		GuiItemLayout_showScroll(ids_layout2, FALSE, FALSE);
		GuiItemLayout_addColumn(ids_layout2, 0, 99);
		GuiItem_addSubName(&layout->base, "ids_layout", &ids_layout2->base);

		GuiItemLayout* cells_layout2 = GuiItemLayout_new(Quad2i_init4(1, 1, 1, 1));
		//GuiItemLayout_setDrawBackground(cells_layout2, FALSE);
		GuiItemLayout_showScroll(cells_layout2, FALSE, FALSE);
		GuiItemLayout_setScrollH(cells_layout2, DbValue_initCopy(&self->scrollH));
		GuiItem_addSubName(&layout->base, "cells_layout", &cells_layout2->base);

		//num records
		{
			DbValue nrv = DbValue_initNumber(0);
			nrv.staticPost = Std_newUNI_char("x");
			GuiItemText* t = (GuiItemText*)GuiItem_addSubName(&ids_layout2->base, "#records", GuiItemText_new(Quad2i_init4(0, 0, 1, 1), TRUE, nrv, DbValue_initEmpty()));
			GuiItemText_setWhiteBack(t, TRUE);
		}

		//columns
		width_sum = 0;
		for (c = 0; c < N_COLS; c++)
		{
			const int width = GuiItemTable_getColumnWidth(self, c);
			DbColumn* col = GuiItemTable_getColumn(self, c);
			BIG crow = GuiItemTable_getColumnRow(self, c);	//table/filter

			char nameId[64];
			snprintf(nameId, 64, "%lld_%lld_insight", c, r);

			{
				GuiItemLayout* headColumn = GuiItemLayout_new(Quad2i_init4(width_sum, 0, width, 1));
				GuiItemLayout_setBackgroundWhite(headColumn, TRUE);
				headColumn->extraSpace = 2;

				GuiItemLayout_addColumn(headColumn, 0, 99);
				GuiItemLayout_addColumn(headColumn, 1, 3);
				GuiItem_addSubName(&cells_layout2->base, nameId, &headColumn->base);

				//text
				DbValue val = DbValue_initInsight(1, &self->filter, col);
				GuiItem_addSubName(&headColumn->base, "value", GuiItemText_new(Quad2i_init4(0, 0, 1, 1), TRUE, val, DbValue_initEmpty()));

				//function
				GuiItemComboStatic* func = GuiItem_addSubName(&headColumn->base, "summary_func", GuiItemComboStatic_new(Quad2i_init4(1, 0, 1, 1), DbValue_initOption(crow, "insight_head_func", 0), 0, DbValue_initEmpty()));
				int i;
				for (i = 0; i < DbInsightSelectFunc_num(); i++)
					GuiItemComboStatic_addItem(func, DbValue_initLang(DbInsightSelectFunc_getName(i)));
			}

			width_sum += width;
		}

		if (!isFilter)
			GuiItem_addSubName(&cells_layout2->base, "+column_empty_box", GuiItemBox_newEmpty(Quad2i_init4(width_sum, 0, 2, 1)));

	}

	return layout;
}

void GuiItemTable_clickSelectRemove(GuiItem* item)
{
	BIG row = GuiItem_getRow(item);
	DbRoot_removeRow(row);
}

BOOL GuiItemTable_highlightEdit(GuiItem* item)
{
	GuiItemEdit* self = (GuiItemEdit*)item;
	return GuiItemEdit_getNumber(self) > 0;
}

void GuiItemTable_rebuildSelectList(GuiItem* item)
{
	GuiItem_freeSubs(item);

	GuiItemLayout* layout = (GuiItemLayout*)item;
	BIG selectRow = GuiItem_findAttribute(item, "row");
	DbTable* table = DbRoot_findParentTable(GuiItem_findAttribute(item, "tableRow"));

	GuiItemLayout_addColumn(layout, 0, 2);
	GuiItemLayout_addColumn(layout, 1, 20);

	int y = 0;

	//enable all
	GuiItem_addSubName((GuiItem*)layout, "on", GuiItemCheck_new(Quad2i_init4(0, y++, 2, 1), DbValue_initOptionEnable(selectRow), DbValue_initLang("SELECT_ENABLE")));

	UBIG i = 0;
	BIG it;
	while ((it = DbColumnN_jump(DbRoot_subs(), selectRow, &i, 1)) >= 0)
	{
		//skin
		GuiItemLayout* skin = GuiItemLayout_new(Quad2i_init4(0, y++, 2, 1));
		GuiItem_setAttribute((GuiItem*)skin, "row", it);
		GuiItemLayout_addColumn(skin, 2, 3);
		GuiItemLayout_addColumn(skin, 3, 6);
		GuiItemLayout_addColumn(skin, 4, 4);
		GuiItemLayout_addColumn(skin, 5, 6);

		//drag
		GuiItem* drag = GuiItem_addSubName((GuiItem*)skin, "drag", GuiItemBox_newEmpty(Quad2i_init4(0, 0, 1, 1)));
		GuiItem_setRow(drag, it, 0);
		GuiItem_setIcon(drag, GuiImage_new1(UiIcons_init_reoder()));
		GuiItem_setDrop(drag, "select", "select", FALSE, DbRows_initLinkN(DbRoot_subs(), selectRow), -1);

		//on/off
		GuiItem* on = GuiItem_addSubName((GuiItem*)skin, "on", GuiItemCheck_new(Quad2i_init4(1, 0, 1, 1), DbValue_initOptionEnable(it), DbValue_initEmpty()));

		//and/or
		GuiItem* andOr = 0;
		if (y > 2)
			andOr = GuiItem_addSubName((GuiItem*)skin, "ascending", GuiItemComboStatic_new(Quad2i_init4(2, 0, 1, 1), DbValue_initOption(it, "ascending", 0), Lang_find("BOOLEAN_OPTIONS"), DbValue_initEmpty()));

		//column
		DbRows rows = DbRows_initLink1(DbRoot_ref(), DbRoot_findOrCreateChildType(it, "column"));
		BIG columnRow = DbRows_getRow(&rows, 0);
		DbColumn* column = DbRoot_findColumn(columnRow);
		GuiItem* cbb = GuiItem_addSubName((GuiItem*)skin, "columns", GuiItemComboDynamic_new(Quad2i_init4(3, 0, 1, 1), TRUE, rows, DbValue_initOption(-1, "name", 0), DbRows_initSubs(table, "columns", TRUE), DbValue_initEmpty()));

		Quad2i editGrid = Quad2i_init4(5, 0, 1, 1);
		Quad2i removeGrid = Quad2i_init4(6, 0, 1, 1);

		GuiItem* type = 0;
		GuiItem* edit = 0;
		//funcType
		if (column)
		{
			DbValue tp = DbValue_initOption(it, "func", 0);
			const int fnIndex = DbValue_getNumber(&tp);
			UNI* options = DbFilterSelectFunc_getList(DbColumnFormat_findColumn(column));
			type = GuiItem_addSubName((GuiItem*)skin, "func", GuiItemComboStatic_new(Quad2i_init4(4, 0, 1, 1), tp, options, DbValue_initEmpty()));
			Std_deleteUNI(options);

			const char* funcName = DbFilterSelectFunc_getName(DbColumnFormat_findColumn(column), fnIndex);

			if (!Std_cmpCHAR(funcName, "FILTER_FUNC_EMPTY") && !Std_cmpCHAR(funcName, "FILTER_FUNC_NOT_EMPTY"))
			{
				DbFormatTYPE format = DbColumnFormat_findColumn(column);
				if (format == DbFormat_MENU)
				{
					BIG optionsRow = DbRows_findSubType(columnRow, "options");
					edit = GuiItemComboDynamic_new(editGrid, TRUE, DbRows_initLinkN(DbRoot_subs(), DbRoot_findOrCreateChildType(it, "option")), DbValue_initOption(-1, "name", 0), DbRows_initLinkN(DbRoot_subs(), optionsRow), DbValue_initEmpty());
				}
				else
					if (format == DbFormat_TAGS)
					{
						BIG optionsRow = DbRows_findSubType(columnRow, "options");
						edit = GuiItemComboDynamic_new(editGrid, TRUE, DbRows_initLinkN(DbRoot_subs(), DbRoot_findOrCreateChildType(it, "option")), DbValue_initOption(-1, "name", 0), DbRows_initLinkN(DbRoot_subs(), optionsRow), DbValue_initEmpty());
					}
					else
						if (format == DbFormat_DATE)
						{
							if (Std_cmpCHAR(funcName, "FILTER_FUNC_TODAY_LESS") || Std_cmpCHAR(funcName, "FILTER_FUNC_TODAY_GREATER"))
							{
								edit = GuiItemEdit_new(editGrid, DbValue_initOption(it, "value", 0), DbValue_initEmpty());

								//combo(years, months, days, ...)
								GuiItemComboStatic* combo = (GuiItemComboStatic*)GuiItemComboStatic_new(Quad2i_init4(6, 0, 1, 1), DbValue_initOption(it, "valueEx", 0), 0, DbValue_initEmpty());
								GuiItemComboStatic_addItem(combo, DbValue_initLang("YEARS"));
								GuiItemComboStatic_addItem(combo, DbValue_initLang("MONTHS"));
								GuiItemComboStatic_addItem(combo, DbValue_initLang("DAYS"));
								GuiItemComboStatic_addItem(combo, DbValue_initLang("HOURS"));
								GuiItemComboStatic_addItem(combo, DbValue_initLang("MINUTES"));
								GuiItemComboStatic_addItem(combo, DbValue_initLang("SECONDS"));

								GuiItem_addSubName((GuiItem*)skin, "valueEx", (GuiItem*)combo);
								GuiItemLayout_addColumn(skin, 6, 4);
								removeGrid.start.x = 7;
							}
							else
							{
								//copy timeFormat from Column settings
								const OsDateTimeTYPE timeFormat = DbValue_getOptionNumber(DbColumn_getRow(column), "timeFormat", 0);
								//DbValue_setOptionNumber(it, "timeFormat", timeFormat);

								DbValue v = DbValue_initOption(it, "value", 0);
								v.optionFormat = DbFormat_DATE;
								edit = GuiItemButton_newWhiteEx(editGrid, v, &GuiItemTable_clickSelectCalendar);
								GuiItem_setAttribute(edit, "timeFormat", timeFormat);
							}
						}
						else
							if (format == DbFormat_CHECK)
							{
								edit = GuiItemCheck_new(editGrid, DbValue_initOption(it, "value", 0), DbValue_initEmpty());
								((GuiItemCheck*)edit)->center = TRUE;
							}
							else
								edit = GuiItemEdit_new(editGrid, DbValue_initOption(it, "value", 0), DbValue_initEmpty());
			}
			if (edit)
				GuiItem_addSubName((GuiItem*)skin, "value", edit);
		}

		//remove
		GuiItem* remove = GuiItem_addSubName((GuiItem*)skin, "remove", GuiItemButton_newClassicEx(removeGrid, DbValue_initStaticCopyCHAR("X"), &GuiItemTable_clickSelectRemove));
		GuiItem_setRow(remove, it, 0);

		if (andOr)	GuiItem_setEnableCallback((GuiItem*)andOr, &GuiItem_enableEnableAttr);
		if (cbb)	GuiItem_setEnableCallback((GuiItem*)cbb, &GuiItem_enableEnableAttr);
		if (type)	GuiItem_setEnableCallback((GuiItem*)type, &GuiItem_enableEnableAttr);
		if (edit)	GuiItem_setEnableCallback((GuiItem*)edit, &GuiItem_enableEnableAttr);

		//GuiItem_setRow((GuiItem*)skin, it, 0);
		GuiItem_setEnableCallback((GuiItem*)skin, &GuiItem_enableEnableParentAttr);
		char nameId[64];
		snprintf(nameId, 64, "%lld_line", i);
		GuiItem_addSubName((GuiItem*)layout, nameId, (GuiItem*)skin);

		i++;
	}

	//add
	GuiItemButton* add = (GuiItemButton*)GuiItem_addSubName((GuiItem*)layout, "+", GuiItemButton_newBlackEx(Quad2i_init4(0, y++, 1, 1), DbValue_initStaticCopy(_UNI32("+")), &GuiItemTable_clickAddSubLine));
	y++;

	//maxRecords
	GuiItem* maxRecords = GuiItem_addSubName((GuiItem*)layout, "maxRecords", GuiItemEdit_newEx(Quad2i_init4(0, y, 2, 2), DbValue_initOption(selectRow, "maxRecords", 0), DbValue_initLang("MAX_RECORDS"), 0));
	GuiItemEdit_setHighlightCallback((GuiItemEdit*)maxRecords, &GuiItemTable_highlightEdit);

	GuiItem_setEnableCallback((GuiItem*)add, &GuiItem_enableEnableAttr);
	GuiItem_setEnableCallback((GuiItem*)maxRecords, &GuiItem_enableEnableAttr);
}

void GuiItemTable_clickAddSubLine(GuiItem* item)
{
	BIG propRow = GuiItem_findAttribute(item, "row");

	DbRows rows = DbRows_initLinkN(DbRoot_subs(), propRow);
	BIG r = DbRows_addNewRow(&rows);

	//set random color
	DbValue_setOptionNumber(r, "cd", Rgba_asNumber(Rgba_initHSL(OsCrypto_random01() * 360, 0.8f, 0.6f)));

	DbRows_free(&rows);
}

GuiItemLayout* GuiItemTable_buildSelectList(Quad2i grid, UBIG row, DbTable* table)
{
	GuiItemLayout* layout = GuiItemLayout_new(grid);

	BIG selectRow = DbRows_findOrCreateSubType(row, "select");
	GuiItem_setAttribute((GuiItem*)layout, "row", selectRow);
	GuiItem_setAttribute((GuiItem*)layout, "tableRow", DbTable_getRow(table));

	GuiItemLayout_setResize(layout, &GuiItemTable_rebuildSelectList);
	return layout;
}
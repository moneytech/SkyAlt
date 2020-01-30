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

typedef struct DbTable_s
{
	DbTables* parent;
	FileRow fileId;

	DbColumns* columns;

	UBIG num_rows;
	UBIG num_rows_real; //less_equal than num_rows

	StdBigs removed_rows;	//save ranges(less memory and faster iter) ...

	DbColumn1* rows;
	StdBigs rowsMoves;

	UBIG lastRow;

	DbTable* copyTableAsk;

	BIG remoteRow;

	BOOL summary;
	//OsDate remote_next_refresh;
} DbTable;

DbColumn* DbTable_getIdsColumn(const DbTable* self)
{
	return &self->rows->base;
}
BOOL DbTable_isIdsColumn(const DbTable* self, const DbColumn* column)
{
	return DbTable_getIdsColumn(self) == column;
}

BOOL DbTable_isSummary(const DbTable* self)
{
	return self->summary;
}

DbTable* DbTable_new(FileRow fileId, DbTables* parent)
{
	DbTable* self = Os_malloc(sizeof(DbTable));
	self->parent = parent;
	self->fileId = fileId;
	self->columns = DbColumns_new(self);

	self->num_rows = 0;
	self->num_rows_real = 0;
	self->removed_rows = StdBigs_init();

	self->rowsMoves = StdBigs_init();

	self->rows = 0;	//must be here!
	self->rows = DbColumns_addColumn1(self->columns, 0);

	self->lastRow = 1;

	self->copyTableAsk = 0;

	self->remoteRow = -1;

	self->summary = FALSE;

	return self;
}

void DbTable_clear(DbTable* self)
{
	DbColumns_clear(self->columns);
	self->num_rows = 0;
	self->num_rows_real = 0;
	self->lastRow = 1;

	StdBigs_free(&self->removed_rows);
	StdBigs_free(&self->rowsMoves);
}

void DbTable_delete(DbTable* self)
{
	DbTable_clear(self);
	DbColumns_delete(self->columns);
	Os_free(self, sizeof(DbTable));
}

FileRow DbTable_getFileId(const DbTable* self)
{
	return self->fileId;
}

BIG DbTable_getRow(const DbTable* self)
{
	return DbRoot_getRow(self->fileId);
}

DbColumns* DbTable_getColumns(const DbTable* self)
{
	return self->columns;
}

DbColumn1* DbTable_getColumnRows(const DbTable* self)
{
	return self->rows;
}

UBIG DbTable_numRows(const DbTable* self)
{
	return self->num_rows;
}

UBIG DbTable_numRowsReal(const DbTable* self)
{
	return self->num_rows_real;
}

UBIG DbTable_numCells(const DbTable* self, BOOL realRows)
{
	return (realRows ? DbTable_numRowsReal(self) : DbTable_numRows(self)) * DbColumns_num(DbTable_getColumns(self));
}

const UNI* DbTable_getName(const DbTable* self, UNI* out, const UBIG outMaxSize)
{
	return DbRoot_getName(DbTable_getRow(self), out, outMaxSize);
}
void DbTable_setName(DbTable* self, const UNI* name)
{
	DbRoot_setName(DbTable_getRow(self), name);
}

void DbTable_setRemote(DbTable* self, BIG remoteRow)
{
	self->remoteRow = remoteRow;
}

BOOL DbTable_isRemoteSaveItIntoFile(const DbTable* self)
{
	return self->remoteRow >= 0 ? DbRoot_isRemoteSaveItIntoFile(self->remoteRow) : FALSE;
}

BIG DbTable_jumpRowsFrom0(DbTable* self, UBIG index) //try to jump over deleted lines
{
	if (index >= self->num_rows_real)
		return -1;

	const UBIG N = self->num_rows - self->num_rows_real;
	UBIG i;
	for (i = 0; i < N; i++)
		if (self->removed_rows.ptrs[i] <= index)
			index++;
	return index;
}

BOOL DbTable_isRowActive(const DbTable* self, const BIG r)
{
	return r >= 0 && r < DbTable_numRows(self) && DbColumn1_isValid(self->rows, r);
}

BIG DbTable_jumpRows(DbTable* self, UBIG* pos, BIG jumps) //try to jump over deleted lines
{
	UBIG i;
	for (i = *pos; i < self->num_rows; i++)
	{
		if (DbTable_isRowActive(self, i))
		{
			jumps--;
			if (jumps == 0)
				break;
		}
	}

	*pos = i;
	return jumps == 0 ? i : -1;
}

BIG DbTable_jumpLinks(DbTable* self, const double* links, UBIG* pos, BIG jumps) //try to jump over deleted lines
{
	if (links)
	{
		const UBIG N = links[0];
		links++; //move after link size

		while (*pos < N)
		{
			if (links[*pos] >= 0 && DbTable_isRowActive(self, links[*pos]))
			{
				jumps--;
				if (jumps == 0)
					break;
			}
			(*pos)++;
		}
	}

	return(links && jumps == 0) ? links[*pos] : -1;
}

BIG DbTable_jumpLinksFirst(DbTable* self, const double* links) //try to jump over deleted lines
{
	UBIG pos = 0;
	return DbTable_jumpLinks(self, links, &pos, 1);
}

BIG DbTable_firstRow(DbTable* self)
{
	UBIG i = 0;
	for (; i < self->num_rows; i++)
	{
		if (DbTable_isRowActive(self, i))
			return i;
	}
	return -1;
}

BIG DbTable_findRow(const DbTable* self, FileRow row)
{
	return self->rows && FileRow_is(row) ? DbColumn1_findRowPos(self->rows, row) : -1;
}

UBIG DbTable_addRowEx(DbTable* self, FileRow row)
{
	UBIG r = self->num_rows;

	self->num_rows++;
	self->num_rows_real++;

	//columns
	DbColumns_updateNumberOfRows(self->columns, self->num_rows - 1);

	//marks
	DbColumn1_setFileId(self->rows, r, row);

	return r;
}

void DbTable_setMaxRow(DbTable* self, FileRow lastRow)
{
	if (FileRow_is(lastRow))
	{
		self->lastRow = Std_bmax(self->lastRow, lastRow.row + 1);
	}
}

UBIG DbTable_addRow(DbTable* self)
{
	return DbTable_addRowEx(self, FileRow_init(self->lastRow++));
}

void DbTable_addRows(DbTable* self, UBIG n)
{
	while (n > 0)
	{
		DbTable_addRow(self);
		n--;
	}
}

void DbTable_checkForCopyAsk(DbTable* self)
{
	UBIG c;

	//load if needed
	BOOL load = (self->copyTableAsk != 0);
	for (c = 0; c < DbColumns_num(self->columns); c++)
		if (DbColumns_get(self->columns, c)->copyColumnAsk)
			load = TRUE;
	if (load)
		DbTable_loadLast(self);

	//table
	if (self->copyTableAsk)
	{
		DbTable_loadLast(self->copyTableAsk);
		DbTable_addRows(self, DbTable_numRows(self->copyTableAsk));

		BIG i;
		for (i = 0; i < self->copyTableAsk->removed_rows.num; i++)
			DbTable_removeRow(self, self->copyTableAsk->removed_rows.ptrs[i]);

		self->copyTableAsk = 0;
	}

	//column
	for (c = 0; c < DbColumns_num(self->columns); c++)
	{
		DbColumn* column = DbColumns_get(self->columns, c);
		if (column->copyColumnAsk)
		{
			DbColumn_copyContent(column, column->copyColumnAsk);
			column->copyColumnAsk = 0;
		}
	}
}

void DbTable_removeRow(DbTable* self, UBIG row)
{
	if (DbColumn1_isValid(self->rows, row))
	{
		DbColumns_deleteRowData(self->columns, row);

		DbColumn1_invalidate(self->rows, row);
		self->num_rows_real--;

		StdBigs_insertShort(&self->removed_rows, row);
	}
}

void DbTable_copyRow(DbTable* self, UBIG dstRow, UBIG srcRow)
{
	DbColumns_copyRow(self->columns, dstRow, srcRow);
}

DbColumn* DbTable_createColumnFormat(DbTable* self, DbFormatTYPE format, const UNI* name, DbTable* btable)
{
	DbColumn* column = 0;

	if (format == DbFormat_MENU || format == DbFormat_TAGS)
		btable = DbRoot_getInfoTable();

	if (!name)
		name = Lang_find(DbColumnFormat_findColumnLang(format));

	DbColumnTYPE columnType = DbColumnFormat_findColumnType(format);
	switch (columnType)
	{
		case DbColumn_1: column = &DbColumns_createColumn1(self->columns, name, btable)->base;	break;
		case DbColumn_N: column = &DbColumns_createColumnN(self->columns, name, btable)->base;	break;
		case DbColumn_STRING_32: column = &DbColumns_createColumnString32(self->columns, name)->base;	break;
	}

	if (column)
	{
		UNI* val = Std_newUNI_char(DbColumnFormat_findColumnName(format));
		DbColumn_setOptionString(column, "format", val);
		DbColumn_setOptionNumber(column, "width", 8);
		Std_deleteUNI(val);

		DbColumn_setDefaultOptions(DbColumn_getRow(column), format);
	}

	return column;
}

void DbTable_computeTempMoves(DbTable* self)
{
	StdBigs_resize(&self->rowsMoves, self->num_rows);

	UBIG move = 0;
	UBIG i;
	for (i = 0; i < self->num_rows; i++)
	{
		if (!DbColumn1_isValid(self->rows, i))
		{
			self->rowsMoves.ptrs[i] = -1; //invalid
			move++;
		}
		else
			self->rowsMoves.ptrs[i] = move;
	}
}

void DbTable_freeTempMoves(DbTable* self)
{
	StdBigs_free(&self->rowsMoves);
}

BIG* DbTable_getTempMoves(DbTable* self)
{
	return self->rowsMoves.ptrs;
}

BOOL DbTable_isLoaded(DbTable* self)
{
	const int N_COLS = DbColumns_num(self->columns);

	int i;
	for (i = 0; i < N_COLS; i++)	//exclude first
		if (DbColumns_get(self->columns, i) != &self->rows->base)
			if (DbColumns_get(self->columns, i)->loaded)
				return TRUE;	//at least one is => loading right now

	return FALSE;
}

void DbTable_maintenance(DbTable* self)
{
	if (DbTable_isLoaded(self) && self->num_rows_real != self->num_rows)
	{
		DbTable_computeTempMoves(self);

		//Columns which link to this table
		UBIG ti;
		for (ti = 0; ti < DbTables_num(DbRoot_getTables()); ti++)
		{
			DbTable* table = DbTables_get(DbRoot_getTables(), ti);
			UBIG ci;
			for (ci = 0; ci < DbColumns_num(table->columns); ci++)
			{
				DbColumn* column = DbColumns_get(table->columns, ci);
				if (DbColumn_getBTable(column) == self)
				{
					if (column->type == DbColumn_1)
						DbColumn1_maintenanceContent((DbColumn1*)column);
					if (column->type == DbColumn_N)
						DbColumnN_maintenanceContent((DbColumnN*)column);
				}
			}
		}

		//This table
		DbColumns_maintenance(self->columns, self->rows);

		DbColumn1_maintenanceMarks(self->rows);

		self->num_rows = self->num_rows_real;

		StdBigs_free(&self->removed_rows);

		DbTable_freeTempMoves(self);
	}
}

//This will destroy all rowID "pointers"!
BOOL DbTable_isChangedSave(const DbTable* self)
{
	int i;
	for (i = 0; i < DbColumns_num(self->columns); i++)
		if (DbColumn_isChangedSave(DbColumns_get(self->columns, i)))
			return TRUE;
	return FALSE;
}
BOOL DbTable_isChangedExe(const DbTable* self)
{
	int i;
	for (i = 0; i < DbColumns_num(self->columns); i++)
		if (DbColumn_isChangedExe(DbColumns_get(self->columns, i)))
			return TRUE;
	return FALSE;
}

UBIG DbTable_numChanges(const DbTable* self)
{
	if (self->summary)
		return 0;

	UBIG n = 0;
	int i;
	for (i = 0; i < DbColumns_num(self->columns); i++)
		n += DbColumn_numChanges(DbColumns_get(self->columns, i));
	return n;
}

void DbTable_addBTablesList(DbTable* self, StdArr* out)
{
	StdArr_add(out, self);

	int i;
	for (i = 0; i < DbColumns_num(self->columns); i++)
	{
		DbColumn* col = DbColumns_get(self->columns, i);
		if (col->type == DbColumn_N)
		{
			DbTable* btable = DbColumnN_getBTable((DbColumnN*)col);
			if (btable)
			{
				if (StdArr_find(out, btable) < 0)
					DbTable_addBTablesList(btable, out);
			}
		}
	}
}

void DbTable_save(DbTable* self, UBIG* doneCells, const UBIG numAllCells)
{
	const double date = Os_timeUTC();
	int i;
	for (i = 0; i < DbColumns_num(self->columns) && StdProgress_is(); i++)
	{
		DbColumn* column = DbColumns_get(self->columns, i);
		if (column != &self->rows->base)
			DbColumn_save(column, date, doneCells, numAllCells);
		else
			*doneCells += DbTable_numRows(self);
	}

	DbColumns_reset_save(self->columns, FALSE);
}

void DbTable_unloadHard(DbTable* self)
{
	DbTable_clear(self);

	int i;
	for (i = 0; i < DbColumns_num(self->columns); i++)
		DbColumn_unload(DbColumns_get(self->columns, i));
}

void DbTable_unload(DbTable* self)
{
	if (DbTable_isChangedSave(self))
		return;

	DbTable_unloadHard(self);
}

void DbTable_load(DbTable* self, double currDate)
{
	BIG i;

	if (DbTable_isLoaded(self))
		return;

	if (currDate <= 0)
		currDate = Std_maxDouble();

	OsODBC* odbc = self->remoteRow >= 0 ? DbRoot_connectRemote(self->remoteRow, FALSE) : 0;

	UBIG filesDone = 0;
	UBIG filesSizes = 0;	//not including BTables ...
	for (i = 0; i < DbColumns_num(self->columns) && StdProgress_is(); i++)
		if (DbColumns_get(self->columns, i) != &self->rows->base)
			filesSizes += FileProject_bytesColumns(DbColumns_get(self->columns, i)->fileId);

	for (i = 0; i < DbColumns_num(self->columns) && StdProgress_is(); i++)
		if (DbColumns_get(self->columns, i) != &self->rows->base)
			DbColumn_load(DbColumns_get(self->columns, i), currDate, &filesDone, filesSizes, odbc);

	if (odbc)
		OsODBC_delete(odbc);

	for (i = 0; i < DbColumns_num(self->columns) && StdProgress_is(); i++)
	{
		DbColumn* col = DbColumns_get(self->columns, i);

		if (col->type == DbColumn_1)
		{
			DbTable* btable = DbColumn1_getBTable((DbColumn1*)col);
			if (btable)
			{
				//load btable
				//if (!DbTable_isLoaded(btable))
				DbTable_load(btable, currDate);

				//convert links to btable to poses
				DbColumn1_convertToPos((DbColumn1*)col);
			}
		}
		else
			if (col->type == DbColumn_N)
			{
				DbTable* btable = DbColumnN_getBTable((DbColumnN*)col);
				if (btable)
				{
					//load btable
					//if (!DbTable_isLoaded(btable))
					DbTable_load(btable, currDate);

					//convert links to btable to poses
					DbColumnN_convertToPos((DbColumnN*)col);
				}
			}

		if (col->insight)
		{
			BIG ii = 0;
			DbColumn* ic;
			while ((ic = DbInsight_getItemColumn(col->insight, ii++)))
			{
				DbTable* btable = DbColumn_getBTable(ic);
				if (btable)
					DbTable_load(btable, currDate);
			}
		}

		//to samé co platí pro Insight, platí i pro Links(mirror/generated) ...
	}

	DbColumns_reset_save(self->columns, TRUE);
}

void DbTable_loadLast(DbTable* self)
{
	DbTable_load(self, 0);
}

void DbTable_refreshRemote(DbTable* self)
{
	DbTable_unloadHard(self);
	DbTable_loadLast(self);
}

UBIG DbTable_bytes(DbTable* self)
{
	UBIG sum = 0;
	sum += DbColumns_bytes(self->columns);
	return sum;
}

void DbTable_getArrayPoses(const DbTable* self, StdBigs* out)
{
	StdBigs_clear(out);

	const UBIG NUM_ROWS = DbTable_numRows(self);
	StdBigs_resize(out, NUM_ROWS);

	UBIG n = 0;
	BIG i;
	for (i = 0; i < NUM_ROWS; i++)
	{
		if (DbTable_isRowActive(self, i))
			out->ptrs[n++] = i;
	}

	StdBigs_resize(out, n);
}

typedef struct DbTables_s
{
	StdArr tables;	//<DbTable*>
} DbTables;

DbTables* DbTables_new(void)
{
	DbTables* self = Os_malloc(sizeof(DbTables));
	self->tables = StdArr_init();
	return self;
}

void DbTables_freeFilterAndInsight(DbTables* self)
{
	UBIG i;
	for (i = 0; i < DbTables_num(self); i++)
		DbColumns_freeFilterAndInsight(DbTable_getColumns(DbTables_get(self, i)));
}

void DbTables_delete(DbTables* self)
{
	StdArr_freeFn(&self->tables, (StdArrFREE)&DbTable_delete);

	Os_free(self, sizeof(DbTables));
}

BOOL DbTables_removeRow(DbTables* self, DbTable* table)
{
	BOOL found = StdArr_removeFind(&self->tables, table);
	if (found)
	{
		DbRoot_removeRow(DbTable_getRow(table));
		DbTable_delete(table);
	}
	return found;
}

UBIG DbTables_num(const DbTables* self)
{
	return self->tables.num;
}

DbTable* DbTables_get(const DbTables* self, UBIG i)
{
	return self->tables.ptrs[i];
}

DbTable* DbTables_add(DbTables* self, FileRow fileId)
{
	DbTable* table = DbTable_new(fileId, self);
	StdArr_add(&self->tables, table);
	return table;
}

DbTable* DbTables_create(DbTables* self, BIG parentRow)
{
	DbTable* table = DbTables_add(self, FileRow_initEmpty());
	_DbRoot_createTableRow(table, parentRow);
	return table;
}

BOOL DbTables_findBTable(DbTables* self, DbTable* btable)
{
	UBIG i;
	for (i = 0; i < DbTables_num(self); i++)
	{
		DbTable* table = DbTables_get(self, i);
		if (table != btable && DbColumns_hasLinkBTable(table->columns, btable))
			return TRUE;
	}
	return FALSE;
}

BIG DbTables_findPos(const DbTables* self, const FileRow fileId)
{
	if (!FileRow_is(fileId))
		return -1;

	UBIG i;
	for (i = 0; i < DbTables_num(self); i++)
	{
		DbTable* table = DbTables_get(self, i);
		if (FileRow_cmp(DbTable_getFileId(table), fileId))
			return i;
	}
	return -1;
}
DbTable* DbTables_find(const DbTables* self, const FileRow fileId)
{
	BIG i = DbTables_findPos(self, fileId);
	return i >= 0 ? DbTables_get(self, i) : 0;
}

DbTable* DbTables_findOrAdd(DbTables* self, FileRow fileId)
{
	DbTable* table = DbTables_find(self, fileId);
	if (!table)
		table = DbTables_add(self, fileId);
	return table;
}

DbTable* DbTables_findName(const DbTables* self, const UNI* name)
{
	UBIG i;
	for (i = 0; i < DbTables_num(self); i++)
	{
		DbTable* table = DbTables_get(self, i);
		if (DbRoot_cmpName(DbTable_getRow(table), name))
			return table;
	}
	return 0;
}

DbTable* DbTables_findNameRemote(const DbTables* self, const UNI* name, BIG remoteRow)
{
	UBIG i;
	for (i = 0; i < DbTables_num(self); i++)
	{
		DbTable* table = DbTables_get(self, i);
		if (table->remoteRow == remoteRow && DbRoot_cmpName(DbTable_getRow(table), name))
			return table;
	}
	return 0;
}

DbColumn* DbTables_findColumn(const DbTables* self, const FileRow fileId)
{
	if (!FileRow_is(fileId))
		return 0;

	UBIG i;
	for (i = 0; i < DbTables_num(self); i++)
	{
		DbTable* table = DbTables_get(self, i);
		DbColumn* col = DbColumns_find(DbTable_getColumns(table), fileId);
		if (col)
			return col;
	}
	return 0;
}

StdBigs DbTables_getLinks(const DbTables* self)
{
	StdBigs arr = StdBigs_init();

	UBIG i;
	for (i = 0; i < DbTables_num(self); i++)
		StdBigs_add(&arr, DbTable_getRow(DbTables_get(self, i)));

	return arr;
}

void DbTable_checkExists(DbTables* self)
{
	BIG i;
	for (i = DbTables_num(self) - 1; i >= 0; i--)
	{
		DbTable* table = DbTables_get(self, i);

		if (DbTable_getRow(table) < 0)
			StdArr_removeFind(&self->tables, table);

		DbColumns_checkExists(table->columns);

		if (DbColumns_num(table->columns) == 1)	//no columns(1=ID_Column)
			DbTable_clear(table);
	}
}

void DbTables_maintenance(DbTables* self)
{
	UBIG i;
	for (i = 0; i < DbTables_num(self); i++)
		DbTable_maintenance(DbTables_get(self, i));
}

UBIG DbTables_bytes(const DbTables* self)
{
	UBIG sum = 0;
	UBIG i;
	for (i = 0; i < DbTables_num(self); i++)
		sum += DbTable_bytes(DbTables_get(self, i));
	return sum;
}

UBIG DbTables_numColumns(const DbTables* self)
{
	UBIG n = 0;
	UBIG i;
	for (i = 0; i < DbTables_num(self); i++)
		n += DbColumns_num(DbTable_getColumns(DbTables_get(self, i)));
	return n;
}

UBIG DbTables_numRecords(const DbTables* self)
{
	UBIG n = 0;
	UBIG i;
	for (i = 0; i < DbTables_num(self); i++)
		n += DbTable_numRowsReal(DbTables_get(self, i));
	return n;
}

UBIG DbTables_numCells(const DbTables* self, BOOL realRows)
{
	UBIG n = 0;
	UBIG i;
	for (i = 0; i < DbTables_num(self); i++)
		n += DbTable_numCells(DbTables_get(self, i), realRows);

	return n;
}

UBIG DbTables_numChanges(const DbTables* self)
{
	UBIG n = 0;
	UBIG i;
	for (i = 0; i < DbTables_num(self); i++)
		n += DbTable_numChanges(DbTables_get(self, i));
	return n;
}

void DbTables_save(DbTables* self)
{
	UBIG doneCells = 0;
	UBIG numAllCells = DbTables_numCells(self, FALSE);

	UBIG i;
	for (i = 0; i < DbTables_num(self); i++)
	{
		DbTable* t = DbTables_get(self, i);
		if (!t->summary)
			DbTable_save(t, &doneCells, numAllCells);
	}
}

BOOL DbTables_isChangedSave(const DbTables* self)
{
	int i;
	for (i = 0; i < DbTables_num(self); i++)
	{
		DbTable* t = DbTables_get(self, i);
		if (!t->summary)
			if (DbTable_isChangedSave(t))
				return TRUE;
	}
	return FALSE;
}

BOOL DbTables_isChangedExe(const DbTables* self)
{
	int i;
	for (i = 0; i < DbTables_num(self); i++)
		if (DbTable_isChangedExe(DbTables_get(self, i)))
			return TRUE;
	return FALSE;
}

void DbTables_unloadUnlisted(const DbTables* self, StdArr* keepTables)
{
	int i;
	for (i = 0; i < DbTables_num(self); i++)
	{
		DbTable* table = DbTables_get(self, i);
		if (StdArr_find(keepTables, table) < 0)
			DbTable_unload(table);
	}
}

void DbTables_refreshRemote(DbTables* self, BIG remoteRow)
{
	int i;
	for (i = 0; i < DbTables_num(self); i++)
	{
		DbTable* table = DbTables_get(self, i);
		if (table->remoteRow == remoteRow)
			DbTable_refreshRemote(table);
	}
}

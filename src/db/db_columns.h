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

typedef struct DbColumns_s
{
	DbTable* parent;
	FileRow fileId;
	StdArr columns;
} DbColumns;

DbColumns* DbColumns_new(DbTable* parent)
{
	DbColumns* self = Os_malloc(sizeof(DbColumns));
	self->parent = parent;
	self->fileId = FileRow_initEmpty();
	self->columns = StdArr_init();
	return self;
}

BIG DbColumns_getRow(const DbColumns* self)
{
	return DbRoot_getRow(self->fileId);
}

const UBIG DbColumns_num(const DbColumns* self)
{
	return self->columns.num;
}

DbColumn* DbColumns_get(const DbColumns* self, const UINT i)
{
	return self->columns.ptrs[i];
}

void DbColumns_clear(DbColumns* self)
{
	UBIG i;
	for (i = 0; i < DbColumns_num(self); i++)
		DbColumn_clear(DbColumns_get(self, i));
}

void DbColumns_delete(DbColumns* self)
{
	UBIG i;
	for (i = 0; i < DbColumns_num(self); i++)
		DbColumn_delete(DbColumns_get(self, i));

	StdArr_freeBase(&self->columns);
	Os_free(self, sizeof(DbColumns));
}

DbTable* DbColumns_getTable(const DbColumns* self)
{
	return self->parent;
}

void DbColumns_checkExists(DbColumns* self)
{
	BIG i;
	for (i = DbColumns_num(self) - 1; i >= 0; i--)
	{
		DbColumn* column = DbColumns_get(self, i);
		if (!DbTable_isIdsColumn(self->parent, column) && DbColumn_getRow(column) < 0)
			StdArr_removeFind(&self->columns, column);
	}
}

BOOL DbColumns_hasLinkBTable(DbColumns* self, const DbTable* btable)
{
	UBIG i;
	for (i = 0; i < DbColumns_num(self); i++)
	{
		DbColumn* column = DbColumns_get(self, i);
		if (column->type == DbColumn_N && ((DbColumnN*)column)->btable == btable)
			return TRUE;
	}
	return FALSE;
}

DbColumn* DbColumns_findName(const DbColumns* self, const UNI* name)
{
	UBIG i;
	for (i = 0; i < DbColumns_num(self); i++)
	{
		if (DbRoot_cmpName(DbColumns_getRow(self), name))
			return DbColumns_get(self, i);
	}
	return 0;
}

void* DbColumns_findNameType(const DbColumns* self, const UNI* name, DbColumnTYPE type)
{
	UBIG i;
	for (i = 0; i < DbColumns_num(self); i++)
	{
		DbColumn* column = DbColumns_get(self, i);
		if (DbRoot_cmpName(DbColumns_getRow(self), name) && column->type == type)
			return column;
	}
	return 0;
}

BIG DbColumns_findPos(const DbColumns* self, FileRow fileId)
{
	if (!FileRow_is(fileId))
		return -1;

	UBIG i;
	for (i = 0; i < DbColumns_num(self); i++)
		if (FileRow_cmp(DbColumns_get(self, i)->fileId, fileId))
			return i;
	return -1;
}

DbColumn* DbColumns_find(const DbColumns* self, FileRow fileId)
{
	BIG i = DbColumns_findPos(self, fileId);
	return(i >= 0 ? DbColumns_get(self, i) : 0);
}

BIG DbColumns_findNamePos(const DbColumns* self, const UNI* name)
{
	UBIG i;
	for (i = 0; i < DbColumns_num(self); i++)
	{
		if (DbRoot_cmpName(DbColumns_getRow(self), name))
			return i;
	}
	return -1;
}

BIG DbColumns_findColumnPos(const DbColumns* self, DbColumn* column)
{
	return StdArr_find(&self->columns, column);
}

static void _DbColumns_add(DbColumns* self, DbColumn* newColumn)
{
	StdArr_add(&self->columns, newColumn);
	DbColumn_updateNumberOfRows(newColumn, 0);
}

DbColumn1* DbColumns_addColumn1(DbColumns* self, DbTable* btable)
{
	DbColumn1* column = DbColumn1_new(self, (btable ? DbFormat_LINK_1 : DbFormat_NUMBER_1), btable);
	_DbColumns_add(self, &column->base);
	return column;
}
DbColumnN* DbColumns_addColumnN(DbColumns* self, DbTable* btable)
{
	DbColumnN* column = DbColumnN_new(self, (btable ? DbFormat_LINK_N : DbFormat_NUMBER_N), btable);
	_DbColumns_add(self, &column->base);
	return column;
}
DbColumnString32* DbColumns_addColumnString32(DbColumns* self)
{
	DbColumnString32* column = DbColumnString32_new(self, DbFormat_TEXT);
	_DbColumns_add(self, &column->base);
	return column;
}

DbColumn1* DbColumns_createColumn1(DbColumns* self, const UNI* name, DbTable* btable)
{
	DbColumn1* column = DbColumns_addColumn1(self, btable);
	_DbRoot_createColumnRow(&column->base);
	DbColumn_setName(&column->base, name);
	return column;
}
DbColumnN* DbColumns_createColumnN(DbColumns* self, const UNI* name, DbTable* btable)
{
	DbColumnN* column = DbColumns_addColumnN(self, btable);
	_DbRoot_createColumnRow(&column->base);
	DbColumn_setName(&column->base, name);
	return column;
}
DbColumnString32* DbColumns_createColumnString32(DbColumns* self, const UNI* name)
{
	DbColumnString32* column = DbColumns_addColumnString32(self);
	_DbRoot_createColumnRow(&column->base);
	DbColumn_setName(&column->base, name);
	return column;
}

/*static void _DbColumns_remove(DbColumns* self, DbColumn* column)
{
	DbColumn_delete(column);
	StdArr_removeFind(&self->columns, column);
}*/

void DbColumns_updateNumberOfRows(DbColumns* self, const UBIG old_num_rows)
{
	UBIG i;
	for (i = 0; i < DbColumns_num(self); i++)
		DbColumn_updateNumberOfRows(DbColumns_get(self, i), old_num_rows);
}

void DbColumns_move(DbColumns* self, int oldPos, int newPos)
{
	StdArr_insert(&self->columns, newPos, DbColumns_get(self, oldPos));
	StdArr_remove(&self->columns, (newPos <= oldPos) ? (oldPos + 1) : oldPos);
}

void DbColumns_maintenance(DbColumns* self, DbColumn1* active)
{
	UBIG i;
	for (i = 0; i < DbColumns_num(self); i++)
	{
		if (DbColumns_get(self, i) != &active->base)
			DbColumn_maintenance(DbColumns_get(self, i));
	}
}

UBIG DbColumns_bytes(DbColumns* self)
{
	UBIG sum = 0;

	UBIG i;
	for (i = 0; i < DbColumns_num(self); i++)
		sum += DbColumn_bytes(DbColumns_get(self, i));

	return sum;
}

void DbColumns_reset_save(DbColumns* self)
{
	UBIG i;
	for (i = 0; i < DbColumns_num(self); i++)
		DbColumn_reset_save(DbColumns_get(self, i));
}

void DbColumns_deleteRowData(DbColumns* self, const UBIG r)
{
	UBIG i;
	for (i = 1; i < DbColumns_num(self); i++)
		DbColumn_deleteRowData(DbColumns_get(self, i), r);
}

DbColumn* DbColumns_addUnloadType(DbColumns* self, FileRow fileId, DbColumnTYPE type, DbFormatTYPE format, DbTable* btable)
{
	DbColumn* column = 0;
	switch (type)
	{
		case DbColumn_1:			column = &DbColumn1_new(self, format, btable)->base;		break;
		case DbColumn_N:			column = &DbColumnN_new(self, format, btable)->base;		break;
		case DbColumn_STRING_32:	column = &DbColumnString32_new(self, format)->base;		break;
	}

	if (column)
	{
		column->fileId = fileId;
		column->loaded = DbTable_isLoaded(self->parent);
		column->format = format;

		_DbColumns_add(self, column);
	}
	return column;
}

DbColumn* DbColumns_findOrAddUnloadType(DbColumns* self, FileRow fileId, DbColumnTYPE type, DbFormatTYPE format, DbTable* btable)
{
	DbColumn* column = DbColumns_find(self, fileId);
	if (!column)
		column = DbColumns_addUnloadType(self, fileId, type, format, btable);
	else
		column->format = format;
	return column;
}

void DbColumns_copyRow(DbColumns* self, UBIG dstRow, UBIG srcRow)
{
	int i;
	for (i = 1; i < DbColumns_num(self); i++)	//first column is 'id', which is not copied
	{
		DbColumn* c = DbColumns_get(self, i);

		switch (c->type)
		{
			case DbColumn_1:
			DbColumn1_copyRow((DbColumn1*)c, dstRow, (DbColumn1*)c, srcRow);
			break;

			case DbColumn_N:
			DbColumnN_copyRow((DbColumnN*)c, dstRow, (DbColumnN*)c, srcRow);
			break;

			case DbColumn_STRING_32:
			DbColumnString32_copyRow((DbColumnString32*)c, dstRow, (DbColumnString32*)c, srcRow);
			break;
		}
	}
}

BOOL DbColumn_copyContent(DbColumn* dst, const DbColumn* src)
{
	const UBIG NUM_ROWS = DbColumn_numRows(src);

	if (NUM_ROWS != DbColumn_numRows(dst))
		return FALSE;

	if (src->type != dst->type)
		return FALSE;

	BIG r;
	switch (src->type)
	{
		case DbColumn_1:
		{
			for (r = 0; r < NUM_ROWS; r++)
				DbColumn1_copyRow((DbColumn1*)dst, r, (DbColumn1*)src, r);
			break;
		}

		case DbColumn_N:
		{
			for (r = 0; r < NUM_ROWS; r++)
				DbColumnN_copyRow((DbColumnN*)dst, r, (DbColumnN*)src, r);
			break;
		}

		case DbColumn_STRING_32:
		{
			for (r = 0; r < NUM_ROWS; r++)
				DbColumnString32_copyRow((DbColumnString32*)dst, r, (DbColumnString32*)src, r);
			break;
		}
	}

	return TRUE;
}

DbColumn* DbColumns_getFirstColumn(DbColumns* self)
{
	return DbColumns_num(self) ? DbColumns_get(self, 0) : 0;
}

void DbColumn_save(DbColumn* self, const double date, UBIG* doneCells, const UBIG numAllCells)
{
	const DbColumn1* colRows = DbColumn_getActive(self);
	const UBIG num_rows = DbColumn_numRows(self);

	if (!self->loaded)
	{
		*doneCells += num_rows;
		return;
	}

	BIG i;
	//head
	UBIG num_changed = 0;
	for (i = 0; i < num_rows; i++)
		num_changed += DbColumn_isChange(self, i);

	if (num_changed == 0)
		return;

	const FileUser* user = FileProject_getUserMe();
	FileFile* file = FileFile_newColumn(user, self->fileId, TRUE);
	if (file)
	{
		FileFile_seekEnd(file);

		FileFile_writeItemHeader(file, date, num_changed);

		volatile StdProgress* progress = DbRoot_getProgress();

		double* convertArray = 0;
		BIG convertArray_maxBytes = 0;

		switch (self->type)
		{
			case DbColumn_1:
			{
				for (i = 0; i < num_rows && DbRoot_getProgress()->running; i++)
				{
					if (DbColumn_isChange(self, i))
					{
						double v = DbColumn1_getBTable((DbColumn1*)self) ? DbColumn1_getLinkFileId((DbColumn1*)self, i) : DbColumn1_get((DbColumn1*)self, i);
						FileFile_writeItem_1(file, DbColumn1_getFileId(colRows, i), v);
					}

					progress->done = (*doneCells)++ / (float)numAllCells;
				}
				break;
			}

			case DbColumn_N:
			{
				for (i = 0; i < num_rows && DbRoot_getProgress()->running; i++)
				{
					if (DbColumn_isChange(self, i))
					{
						const double* arr;
						if (DbColumnN_getBTable((DbColumnN*)self))
							arr = convertArray = DbColumnN_exportRowArray((DbColumnN*)self, i, convertArray, &convertArray_maxBytes);
						else
							arr = DbColumnN_getArray((DbColumnN*)self, i);

						FileFile_writeItem_n(file, DbColumn1_getFileId(colRows, i), arr);
					}

					progress->done = (*doneCells)++ / (float)numAllCells;
				}
				break;
			}

			case DbColumn_STRING_32:
			{
				for (i = 0; i < num_rows && DbRoot_getProgress()->running; i++)
				{
					if (DbColumn_isChange(self, i))
						FileFile_writeItemText_32(file, DbColumn1_getFileId(colRows, i), DbColumnString32_get((DbColumnString32*)self, i));

					progress->done = (*doneCells)++ / (float)numAllCells;
				}
				break;
			}

			default:
			break;
		}

		Os_free(convertArray, convertArray_maxBytes);

		FileFile_delete(file);
	}
}

void DbColumn_unload(DbColumn* self)
{
	self->loaded = FALSE;
}

static BOOL _DbColumn_loadRow(DbColumn* self, StdBIndex* index, FileRow row, BIG* r)
{
	BIG bg = FileRow_getBIG(row);

	BOOL valid = FileRow_is(row);
	if (valid)
	{
		*r = StdBIndex_search(index, bg);
		if (*r < 0)
		{
			*r = DbTable_addRowEx(DbColumn_getTable(self), row);
			StdBIndex_add(index, bg, *r);
		}
	}
	else
	{
		*r = StdBIndex_search(index, bg);
		if (*r >= 0)
			DbTable_removeRow(DbColumn_getTable(self), *r);
	}

	return valid;
}

void DbColumn_load(DbColumn* self, const double currDate, UBIG* filesDone, const UBIG filesSizes)
{
	if (self->loaded)
		return;

	volatile StdProgress* progress = DbRoot_getProgress();
	DbTable* table = DbColumn_getTable(self);

	const DbColumn1* colRows = DbColumn_getActive(self);
	StdBIndex* index = DbColumn1_createIndex(colRows);

	StdArr files = FileProject_openColumns(self->fileId, FALSE);

	double* fileDates = Os_calloc(files.num, sizeof(double));
	UBIG* fileNumItems = Os_calloc(files.num, sizeof(UBIG));

	BIG i;
	for (i = 0; i < files.num; i++)
		FileFile_readItemHeader(files.ptrs[i], &fileDates[i], &fileNumItems[i]);

	while (files.num && DbRoot_getProgress()->running)
	{
		BIG file_i = -1;

		double minDate = Std_minDouble();
		for (i = 0; i < files.num; i++) //find minimum
		{
			if (files.ptrs[i])
			{
				if (minDate == Std_minDouble())
					minDate = fileDates[i];

				if (fileDates[i] <= minDate)
					file_i = i;
			}
		}

		if (file_i < 0 || fileDates[file_i] > currDate) //end
			break;

		FileRow row;
		BIG r = 0;
		switch (self->type)
		{
			case DbColumn_1:
			{
				for (i = 0; i < fileNumItems[file_i] && DbRoot_getProgress()->running; i++)
				{
					double number;
					FileFile_readItem_1(files.ptrs[file_i], &row, &number);
					DbTable_setMaxRow(table, row);

					if (_DbColumn_loadRow(self, index, row, &r))
					{
						DbColumn1_set((DbColumn1*)self, r, number);

						if (DbColumn1_getBTable((DbColumn1*)self))
							((DbColumn1*)self)->isConverted = FALSE;
					}
					progress->done = (*filesDone += 16) / (float)filesSizes;
				}

				break;
			}

			case DbColumn_N:
			{
				for (i = 0; i < fileNumItems[file_i] && DbRoot_getProgress()->running; i++)
				{
					double* numbers;
					FileFile_readItem_n(files.ptrs[file_i], &row, &numbers);
					DbTable_setMaxRow(table, row);

					if (_DbColumn_loadRow(self, index, row, &r))
					{
						DbColumnN_setArray((DbColumnN*)self, r, numbers);

						if (DbColumnN_getBTable((DbColumnN*)self))
							((DbColumnN*)self)->isConverted = FALSE;
					}

					progress->done = (*filesDone += 16 + DbColumnN_array_bytes(numbers)) / (float)filesSizes;
				}

				break;
			}

			case DbColumn_STRING_32:
			{
				for (i = 0; i < fileNumItems[file_i] && DbRoot_getProgress()->running; i++)
				{
					UNI* text;
					FileFile_readItemText_32(files.ptrs[file_i], &row, &text);
					DbTable_setMaxRow(table, row);

					if (_DbColumn_loadRow(self, index, row, &r))
						DbColumnString32_setEqFree((DbColumnString32*)self, r, text);

					progress->done = (*filesDone += 16 + Std_bytesUNI(text)) / (float)filesSizes;	//not accurate
				}
				break;
			}
		}

		if (!FileFile_readItemHeader(files.ptrs[file_i], &fileDates[file_i], &fileNumItems[file_i]))
		{
			FileFile_delete(files.ptrs[file_i]);
			files.ptrs[file_i] = 0;
		}
		*filesDone += 16;
	}

	self->loaded = TRUE;

	Os_free(fileDates, files.num * sizeof(double));
	Os_free(fileNumItems, files.num * sizeof(UBIG));

	StdBIndex_delete(index);

	StdArr_freeFn(&files, (StdArrFREE)&FileFile_delete);
}

void DbColumn_short(DbColumn* self, StdBigs* poses, const BIG start, const BIG end, const BOOL ascending)
{
	switch (self->type)
	{
		case DbColumn_1:			DbColumn1_qshort((DbColumn1*)self, poses, start, end, ascending);	break;
		case DbColumn_N:			DbColumnN_qshort((DbColumnN*)self, poses, start, end, ascending);	break;
		case DbColumn_STRING_32:	DbColumnString32_qshort((DbColumnString32*)self, poses, start, end, ascending);	break;
	}
}

BOOL DbColumn_cmpRow(DbColumn* self, UBIG i, UBIG j)
{
	switch (self->type)
	{
		case DbColumn_1:
		return DbColumn1_cmpRow((DbColumn1*)self, (DbColumn1*)self, i, j);

		case DbColumn_N:
		return DbColumnN_cmpBigRow((DbColumnN*)self, (DbColumnN*)self, i, j);

		case DbColumn_STRING_32:
		return DbColumnString32_cmpRow((DbColumnString32*)self, (DbColumnString32*)self, i, j);
	}
	return FALSE;
}

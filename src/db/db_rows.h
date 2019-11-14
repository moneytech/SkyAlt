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

static DbRows _DbRows_init(DbTable* table, DbColumn* column)
{
	DbRows self;

	self.table = table;
	self.column = column;
	self.row = -1;

	self.array = StdBigs_init();
	self.arrayStatic = FALSE;

	self.filter = 0;

	return self;
}

DbRows DbRows_initCopy(const DbRows* src)
{
	DbRows self = *src;
	self.array = StdBigs_initCopy(&src->array);

	if (src->filter)
		self.filter = DbRoot_addFilter(src->filter);

	return self;
}

void DbRows_free(DbRows* self)
{
	StdBigs_free(&self->array);

	DbRoot_removeFilter(self->filter);

	Os_memset(self, sizeof(DbRows));
}

DbRows DbRows_initEmpty(void)
{
	return _DbRows_init(0, 0);
}

DbRows DbRows_initTable(DbTable* table)
{
	return _DbRows_init(table, 0);
}

DbRows DbRows_initLink(DbColumnN* column, BIG row)
{
	DbRows self = _DbRows_init(0, &column->base);
	self.row = row;
	return self;
}

DbRows DbRows_initArray(DbTable* table, StdBigs array)
{
	DbRows self = _DbRows_init(table, 0);
	self.array = array;
	self.arrayStatic = TRUE;
	return self;
}

BOOL DbRows_hasChanged(DbRows* self)
{
	if (self->filter)
		return DbFilter_execute(self->filter);
	return FALSE;
}

void DbRows_setBaseRow(DbRows* self, BIG row)
{
	self->row = row;
}

BIG DbRows_getBaseRow(const DbRows* self)
{
	return self->row;
}

DbTable* DbRows_getTable(const DbRows* self)
{
	if (self->table)
		return self->table;
	else
		if (self->column)
			return DbColumn_getTable(self->column);
		else
			if (self->row >= 0)
				return DbRoot_findParentTable(self->row);

	return 0;
}

BOOL DbRows_isColumnValid(const DbRows* self)
{
	return self->column && self->row >= 0 && DbColumn_getBTable(self->column);
}

BOOL DbRows_is(const DbRows* self)
{
	return (self->column || self->table) && self->row >= 0;
}

FileRow DbRows_getFileId(const DbRows* self, UBIG pos)
{
	FileRow fileId = FileRow_initEmpty();

	if (self->column && self->row >= 0)
		fileId = DbColumn_fileGetPos(self->column, self->row, pos);
	return fileId;
}

BIG DbRows_getRow(const DbRows* self, UBIG pos)
{
	if (self->filter)
	{
		return pos < self->filter->rows.num ? self->filter->rows.ptrs[pos] : -1;
	}
	else
		if (self->arrayStatic)	//must be first!
		{
			return pos < self->array.num ? self->array.ptrs[pos] : -1;
		}
		else
			if (self->table)
			{
				return (pos < DbRows_getSize(self)) ? DbTable_jumpRowsFrom0(self->table, pos) : -1;
			}
			else
				if (DbRows_isColumnValid(self))
				{
					return (pos < DbRows_getSize(self)) ? DbColumn_getIndex(self->column, self->row, pos) : -1;
				}

	return -1;
}

BOOL DbRows_isRow(const DbRows* self, UBIG pos)
{
	return DbRows_getRow(self, pos) >= 0;
}

UBIG DbRows_getSize(const DbRows* self)
{
	if (self->filter)
	{
		return self->filter->rows.num;
	}
	else
		if (self->arrayStatic) //must be first!
			return self->array.num;
		else
			if (self->table)
			{
				return DbTable_numRowsReal(self->table);
			}
			else
				if (self->column && self->row >= 0)
				{
					return DbColumn_sizeActive(self->column, self->row);
				}

	return 0;
}

BOOL DbRows_isInside(const DbRows* self, UBIG pos)
{
	return pos < DbRows_getSize(self);
}

BIG DbRows_findSubType(BIG row, const char* subType)
{
	return DbRoot_findChildType(row, subType);
}

DbRows DbRows_getSubsArray(BIG row, const char* subType)
{
	UBIG columnsRow = DbRoot_findOrCreateChildType(row, subType);
	return DbRows_initLink(DbRoot_getColumnSubs(), columnsRow);
}

BIG DbRows_getAddSubsLine(BIG row, const char* subType)
{
	DbRows rows = DbRows_getSubsArray(row, subType);

	BIG r = DbRows_addNewRow(&rows);
	_DbRoot_setOptionNumber(r, "width", 7);


	DbRows_free(&rows);

	return r;
}

static BIG _DbRows_getSubsIndex(const BIG row, const char* subType, const BOOL onlyEnable, const BIG index, DbColumn** out_column, BIG* out_row)
{
	UBIG num = 0;

	DbColumnN* subs = DbRoot_getColumnSubs();

	BIG propRow = DbRows_findSubType(row, subType);
	DbTable* table = DbRoot_findParentTable(row);

	if (table && propRow >= 0)
	{
		UBIG i = 0;
		BIG it;
		while ((it = DbColumnN_jump(subs, propRow, &i, 1)) >= 0)
		{
			if (!onlyEnable || _DbRoot_isEnable(it))
			{
				DbColumn* column = DbColumns_find(table->columns, DbRoot_getFileId(it));
				if (!column)
					column = DbColumns_find(table->columns, DbRoot_getFileId(DbColumnN_getFirstRow(subs, it)));

				if (index == num)
				{
					*out_column = column;
					*out_row = it;
					return num;
				}
				num++;
			}
			i++;
		}
	}

	return index < 0 ? num : -1;
}

DbRows DbRows_initSubs(DbTable* table, const char* subType, BOOL onlyEnable)
{
	StdBigs arr = StdBigs_init();

	DbColumn* column;
	BIG row2;
	BIG index = 0;
	while (_DbRows_getSubsIndex(DbTable_getRow(table), subType, onlyEnable, index, &column, &row2) == index)
	{
		StdBigs_add(&arr, row2);
		index++;
	}

	return DbRows_initArray(DbRoot_getInfoTable(), arr);
}

DbValues DbRows_getSubs(BIG row, const char* subType, BOOL onlyEnable, int maxN)
{
	DbValues values = DbValues_init();

	DbColumn* column;
	BIG row2;
	BIG index = 0;
	while (_DbRows_getSubsIndex(row, subType, onlyEnable, index, &column, &row2) == index && (maxN < 0 || index < maxN))
	{
		DbValues_add(&values, DbValue_initGET(column, -1));
		index++;
	}

	return values;
}

DbValue DbRows_getSubsCell(BIG row, const char* subType, BOOL onlyEnable, BIG index, BIG cellRow)
{
	DbColumn* column = DbRows_getSubsColumn(row, subType, onlyEnable, index);
	return column ? DbValue_initGET(column, cellRow) : DbValue_initEmpty();
}

DbValue DbRows_getSubsOption(BIG row, const char* subType, BOOL onlyEnable, BIG index, const char* optionName, BOOL columnDirect)
{
	DbColumn* column;
	BIG row2;
	return _DbRows_getSubsIndex(row, subType, onlyEnable, index, &column, &row2) == index ? DbValue_initOption(columnDirect ? DbColumn_getRow(column) : row2, optionName, 0) : DbValue_initEmpty();
}

DbColumn* DbRows_getSubsColumn(BIG row, const char* subType, BOOL onlyEnable, BIG index)
{
	DbColumn* column;
	BIG row2;
	return _DbRows_getSubsIndex(row, subType, onlyEnable, index, &column, &row2) == index ? column : 0;
}

BIG DbRows_getSubsRow(BIG row, const char* subType, BOOL onlyEnable, BIG index)
{
	DbColumn* column;
	BIG row2;
	return _DbRows_getSubsIndex(row, subType, onlyEnable, index, &column, &row2) == index ? row2 : -1;
}

UBIG DbRows_getSubsNum(BIG row, const char* subType, BOOL onlyEnable)
{
	DbColumn* column;
	BIG row2;
	return _DbRows_getSubsIndex(row, subType, onlyEnable, -1, &column, &row2);
}

DbValues DbRows_getOptions(BIG row, const char* subType, const char* valueType, BOOL onlyEnable)
{
	DbValues values = DbValues_init();

	DbColumnN* subs = DbRoot_getColumnSubs();

	BIG propRow = DbRows_findSubType(row, subType);
	DbTable* table = DbRoot_findParentTable(row);

	if (table && propRow >= 0)
	{
		UBIG i = 0;
		BIG it;
		while ((it = DbColumnN_jump(subs, propRow, &i, 1)) >= 0)
		{
			if (!onlyEnable || _DbRoot_isEnable(it))
				DbValues_add(&values, DbValue_initOption(it, valueType, 0));
			i++;
		}
	}

	return values;
}

DbRows DbRows_initTables(void)
{
	DbRows self = DbRows_initArray(0, DbRoot_getTableLinks());
	return self;
}

void _DbRows_addFilter(DbFilter** filterOrig, BIG row)
{
	DbColumnN* subs = DbRoot_getColumnSubs();
	DbTable* table = DbRoot_findParentTable(row);

	if (table)
	{
		BIG selectRow = DbRows_findSubType(row, "select");
		BIG groupRow = DbRows_findSubType(row, "group");
		BIG shortRow = DbRows_findSubType(row, "short");

		BOOL selectOk = (selectRow >= 0 && _DbRoot_isEnable(selectRow));
		BOOL groupOk = (groupRow >= 0 && _DbRoot_isEnable(groupRow));
		BOOL shortOk = (shortRow >= 0 && _DbRoot_isEnable(shortRow));

		if (selectOk || groupOk || shortOk)
		{
			DbFilter* filter = DbFilter_new(table, DbRoot_getFileId(DbTable_getRow(table)));
			if (!*filterOrig)
				*filterOrig = filter;
			else
				DbFilter_addParent(*filterOrig, filter);

			UBIG i;
			BIG it;
			if (shortOk)
			{
				i = 0;
				while ((it = DbColumnN_jump(subs, shortRow, &i, 1)) >= 0)
				{
					DbColumn* column = DbColumns_find(table->columns, DbRoot_getFileId(DbColumnN_getFirstRow(subs, it)));
					if (column && _DbRoot_isEnable(it))
						DbFilter_addShort(filter, column, _DbRoot_isAscending(it));
					i++;
				}
			}

			if (groupOk)
			{
				//group
				i = 0;
				while ((it = DbColumnN_jump(subs, groupRow, &i, 1)) >= 0)
				{
					DbColumn* column = DbColumns_find(table->columns, DbRoot_getFileId(DbColumnN_getFirstRow(subs, it)));
					if (column && _DbRoot_isEnable(it))
						DbFilter_addGroup(filter, column, _DbRoot_isAscending(it), FALSE);
					i++;
				}
			}

			if (selectOk)
			{
				DbFilter_setMaxRecords(filter, _DbRoot_getOptionNumber(selectRow, "maxRecords", 0));

				//select
				i = 0;
				while ((it = DbColumnN_jump(subs, selectRow, &i, 1)) >= 0)
				{
					BIG colRow = DbRoot_findOrCreateChildType(it, "column");
					DbColumn* column = DbColumns_find(table->columns, DbRoot_getFileId(DbColumnN_getFirstRow(subs, colRow)));
					if (column && _DbRoot_isEnable(it))
					{
						DbFormatTYPE format = DbColumnFormat_findColumn(column);

						UNI value[32];
						if (format == DbFormat_MENU || format == DbFormat_TAGS)
						{
							BIG row = DbColumnN_getFirstRow(DbRoot_getColumnSubs(), DbRoot_findOrCreateChildType(it, "option"));
							Std_buildNumberUNI(row, 0, value);
						}
						else
							_DbRoot_getOptionString(it, "value", 0, value, 32);

						UINT funcIndex = _DbRoot_getOptionNumber(it, "func", 0);
						DbFilter_addSelect(filter, _DbRoot_isAND(it), column, DbFilterSelectFunc_get(format, funcIndex), value);
					}
					i++;
				}
			}
		}

		if (!DbRoot_isType_table(row))
		{
			BIG parentRow = DbRoot_findParent(row);
			if (parentRow >= 0)
				_DbRows_addFilter(filterOrig, parentRow);
		}

	}
}

DbRows DbRows_initFilter(BIG row)
{
	DbFilter* filter = 0;
	_DbRows_addFilter(&filter, row);

	//add search
	UNI search[64];
	_DbRoot_getOptionString(row, "search", 0, search, 64);
	if (Std_sizeUNI(search))
	{
		DbTable* table = DbRoot_findParentTable(row);
		if (!filter)
			filter = DbFilter_new(table, DbRoot_getFileId(row));

		int i;
		for (i = 0; i < DbColumns_num(table->columns); i++)
			DbFilter_addSelect(filter, FALSE, DbColumns_get(table->columns, i), DbFilterSelectFunc_find(DbFormat_TEXT, "FILTER_FUNC_CONTAIN"), search);
	}

	DbRows self;
	if (!DbFilter_isEmpty(filter))
	{
		self = DbRows_initEmpty();
		self.filter = DbRoot_addFilter(filter);
		self.table = DbRoot_findParentTable(row);
	}
	else
	{
 		DbTable* table = DbRoot_findParentTable(row);
		self = DbRows_initTable(table);
	}
	self.row = row;

	return self;
}

void DbRows_forceEmptyFilter(DbRows* self)
{
	DbTable* table = DbRows_getTable(self);
	if (!self->filter && table)
		self->filter = DbFilter_new(table, DbRoot_getFileId(self->row));
}

BOOL DbRows_hasFilterSubActive(BIG row, const char* subName)
{
	DbColumnN* subs = DbRoot_getColumnSubs();

	DbTable* table = DbRoot_findParentTable(row);
	if (table)
	{
		BIG subRow = DbRows_findSubType(row, subName);
		if (subRow >= 0)
		{
			if (!_DbRoot_isEnable(subRow))
				return FALSE;

			UBIG i = 0;
			BIG it;
			while ((it = DbColumnN_jump(subs, subRow, &i, 1)) >= 0)
			{
				if (_DbRoot_isEnable(it))
					return TRUE;
				i++;
			}
		}
	}

	return FALSE;
}

BOOL DbRows_hasColumnsSubDeactive(BIG row, const char* columnsName)
{
	DbColumnN* subs = DbRoot_getColumnSubs();

	BIG columnsRow = DbRows_findSubType(row, columnsName);
	if (columnsRow >= 0)
	{
		//short
		UBIG i = 0;
		BIG it;
		while ((it = DbColumnN_jump(subs, columnsRow, &i, 1)) >= 0)
		{
			if (!_DbRoot_isEnable(it))
				return TRUE;
			i++;
		}
	}

	return FALSE;
}

void DbRows_addLinkRow(DbRows* self, BIG row)
{
	if (DbRows_isColumnValid(self))
		DbColumn_add(self->column, self->row, row);
	else
	{
		StdBigs_add(&self->array, row);
		self->arrayStatic = TRUE;
	}
}

void DbRows_setLinkRow(DbRows* self, BIG row)
{
	if (DbRows_isColumnValid(self))
		DbColumn_set(self->column, self->row, row);
	else
	{
		StdBigs_resize(&self->array, 1);
		self->array.ptrs[0] = row;
		self->arrayStatic = TRUE;
	}
}

void DbRows_insertIDbefore(DbRows* self, BIG row, BIG findRow)
{
	if (DbRows_isColumnValid(self))
		DbColumn_insert_before(self->column, self->row, row, findRow);
}
void DbRows_insertIDafter(DbRows* self, BIG row, BIG findRow)
{
	if (DbRows_isColumnValid(self))
		DbColumn_insert_after(self->column, self->row, row, findRow);
}

BIG DbRows_addNewRow(DbRows* self)
{
	BIG row = -1;

	DbTable* table = DbRows_getTable(self);

	if (table)
	{
		row = DbTable_addRow(table);

		if (DbRows_isColumnValid(self))
			DbRows_addLinkRow(self, row);
	}

	return row;
}

void DbRows_removeRow(DbRows* self, BIG row)
{
	if (self->arrayStatic)
	{
		//self->array.ptrs[pos];	//...
		//DbTable_removeRow(self->table, row);
	}
	else
		if (self->table)
			DbTable_removeRow(self->table, row);
		else
			if (self->column && self->row >= 0)
				DbColumn_remove(self->column, self->row, row);
}

void DbRows_removeFile(DbRows* self, FileRow fileRow)
{
	if (self->column && self->row >= 0)
		DbColumn_remove(self->column, self->row, FileRow_getDouble(fileRow));
}

BOOL DbRows_isSubChild(DbRows* self, BIG origRow, BIG findRow)
{
	if (origRow == findRow)
		return TRUE;

	UBIG i = 0;
	BIG it;
	while ((it = DbColumn_jump(self->column, origRow, &i, 1)) >= 0)
	{
		if (DbRows_isSubChild(self, it, findRow))
			return TRUE;
		i++;
	}

	return FALSE;
}

BIG DbRows_findLinkPos(DbRows* self, BIG findRow)
{
	if (DbRows_isColumnValid(self))
		return DbColumnN_findBigPos((DbColumnN*)self->column, self->row, findRow);
	return -1;
}

BOOL DbRows_getColumnMinMax(DbRows* self, DbColumn1* column, double* mn, double* mx)
{
	const UBIG N = DbRows_getSize(self);
	UBIG i;
	for (i = 0; i < N; i++)
	{
		UBIG it = DbRows_getRow(self, i);

		double v = DbColumn1_get(column, it);
		*mn = Std_dmin(*mn, v);
		*mx = Std_dmax(*mx, v);
	}

	return (N > 0);
}

BOOL DbRows_getColumnsMinMax(DbRows* self, DbColumn1** columns, double* mn, double* mx)
{
	if (!columns)
		return FALSE;

	const UBIG N = DbRows_getSize(self);
	UBIG i;
	for (i = 0; i < N; i++)
	{
		UBIG it = DbRows_getRow(self, i);

		UBIG v = 0;
		DbColumn1* column = *columns;
		while (column)
			v += DbColumn1_get(column, it);

		*mn = Std_dmin(*mn, v);
		*mx = Std_dmax(*mx, v);
	}

	return (N > 0 && *columns);
}

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

void _DbRows_setFilter(DbRows* self, DbFilter* filter)
{
	//if (self->filter)
	//	DbRoot_removeFilter(self->filter);

	self->filter = (!filter || DbFilter_isEmpty(filter)) ? 0 : filter;	//DbRoot_addFilter(filter);

	if (filter && !self->filter)
		self->table = DbFilter_getTable(filter);
}

DbRows DbRows_initCopy(const DbRows* src)
{
	DbRows self = *src;
	self.array = StdBigs_initCopy(&src->array);

	if (src->filter)
		self.filter = src->filter;	//DbRoot_addFilter(src->filter);

	return self;
}

void DbRows_free(DbRows* self)
{
	StdBigs_free(&self->array);

	//	DbRoot_removeFilter(self->filter);

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

DbRows DbRows_initLink1(DbColumn1* column, BIG row)
{
	DbRows self = _DbRows_init(0, &column->base);
	self.row = row;
	return self;
}
DbRows DbRows_initLinkN(DbColumnN* column, BIG row)
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
	//mohu si uložit num_changes pro Table nebo Column ...

	//if (self->filter)
	//	return DbFilter_execute(self->filter);
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
	if (self->arrayStatic)	//must be first!
	{
		BIG i;
		for (i = 0; i < self->array.num; i++)
		{
			BIG r = self->array.ptrs[i];
			if (DbTable_isRowActive(self->table, r))
			{
				if (pos == 0)
					return r;
				pos--;
			}
		}
		return -1;
	}
	else
		if (self->filter)
		{
			return pos < self->filter->rows.num ? self->filter->rows.ptrs[pos] : -1;
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

void DbRows_getArrayPoses(const DbRows* self, StdBigs* out)
{
	StdBigs_clear(out);

	if (self->arrayStatic)	//must be first!
	{
		StdBigs_copy(out, &self->array);
	}
	else
		if (self->filter)
		{
			StdBigs_copy(out, &self->filter->rows);
		}
		else
			if (self->table)
			{
				DbTable_getArrayPoses(self->table, out);
			}
}

BOOL DbRows_isRow(const DbRows* self, UBIG pos)
{
	return DbRows_getRow(self, pos) >= 0;
}

UBIG DbRows_getSize(const DbRows* self)
{
	if (self->arrayStatic) //must be first!
	{
		UBIG n = 0;
		BIG i;
		for (i = 0; i < self->array.num; i++)
		{
			if (DbTable_isRowActive(self->table, self->array.ptrs[i]))
				n++;
		}
		return n;
	}

	else
		if (self->filter)
		{
			return self->filter->rows.num;
		}
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
BIG DbRows_findOrCreateSubType(BIG row, const char* subType)
{
	return DbRoot_findOrCreateChildType(row, subType);
}

DbRows DbRows_getSubsArray(BIG row, const char* subType)
{
	UBIG columnsRow = DbRoot_findOrCreateChildType(row, subType);
	return DbRows_initLinkN(DbRoot_subs(), columnsRow);
}

BIG DbRows_addSubsLine(BIG row, const char* subType)
{
	DbRows rows = DbRows_getSubsArray(row, subType);

	BIG r = DbRows_addNewRow(&rows);
	_DbRoot_setOptionNumber(r, "width", 8);

	DbRows_free(&rows);

	return r;
}

static BIG _DbRows_getSubsIndex(const BIG row, const char* subType, const BOOL onlyEnable, const BIG index, DbColumn** out_column, BIG* out_row)
{
	UBIG num = 0;

	//DbColumnN* subs = DbRoot_getColumnSubs();

	BIG propRow = DbRows_findSubType(row, subType);
	DbTable* table = DbRoot_findParentTable(row);

	if (table && propRow >= 0 && (!onlyEnable || DbRows_isEnable(propRow)))
	{
		UBIG i = 0;
		BIG it;
		while ((it = DbColumnN_jump(DbRoot_subs(), propRow, &i, 1)) >= 0)
		{
			if (!onlyEnable || _DbRoot_isEnable(it))
			{
				DbColumn* column = DbColumns_find(table->columns, DbRoot_getFileId(it));
				if (!column)
					column = DbRoot_ref_column(it);

				if (!onlyEnable || column)	//!onlyEnable for group-lanes
				{
					if (index == num)
					{
						*out_column = column;
						*out_row = it;
						return num;
					}
					num++;
				}
			}
			i++;
		}
	}

	return index < 0 ? num : -1;
}

DbRows DbRows_initRefLink(BIG row, const char* subType)
{
	return DbRows_initLink1(DbRoot_ref(), DbRows_findOrCreateSubType(row, subType));
}

DbRows DbRows_initSubLink(BIG row, const char* subType)
{
	return DbRows_initLinkN(DbRoot_subs(), DbRows_findOrCreateSubType(row, subType));
}
DbValue DbRows_getSubOption(BIG row, const char* subType, const char* optionName, const UNI* defValue)
{
	return DbValue_initOption(DbRows_findOrCreateSubType(row, subType), optionName, defValue);
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

DbRows DbRows_initSubsEx(DbTable* table, const char* subType, BOOL onlyEnable, BOOL typeNumber, BOOL typeString, BOOL typeLink)
{
	StdBigs arr = StdBigs_init();

	DbColumn* column;
	BIG it;
	BIG index = 0;
	while (_DbRows_getSubsIndex(DbTable_getRow(table), subType, onlyEnable, index, &column, &it) == index)
	{
		BOOL add = FALSE;
		add |= typeLink && DbColumn_getBTable(column);
		add |= typeNumber && !DbColumn_getBTable(column) && (column->type == DbColumn_1 || column->type == DbColumn_N);
		add |= typeString && column->type == DbColumn_STRING_32;

		if (add)
			StdBigs_add(&arr, it);

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

double DbRows_getSubsOptionNumber(BIG row, const char* subType, BOOL onlyEnable, BIG index, const char* optionName, BOOL columnDirect)
{
	DbValue v = DbRows_getSubsOption(row, subType, onlyEnable, index, optionName, columnDirect);
	double value = DbValue_getNumber(&v);
	DbValue_free(&v);
	return value;
}
void DbRows_setSubsOptionNumber(BIG row, const char* subType, BOOL onlyEnable, BIG index, const char* optionName, BOOL columnDirect, double value)
{
	DbValue v = DbRows_getSubsOption(row, subType, onlyEnable, index, optionName, columnDirect);
	DbValue_setNumber(&v, value);
	DbValue_free(&v);
}

DbColumn* DbRows_getSubsColumn(BIG row, const char* subType, BOOL onlyEnable, BIG index)
{
	DbColumn* column;
	BIG row2;
	return _DbRows_getSubsIndex(row, subType, onlyEnable, index, &column, &row2) == index ? column : 0;
}

StdArr DbRows_getSubsColumns(BIG row, const char* subType, BOOL onlyEnable)
{
	StdArr arr = StdArr_init();

	StdArr_resize(&arr, DbRows_getSubsNum(row, subType, onlyEnable));

	BIG i;
	for (i = 0; i < arr.num; i++)
		arr.ptrs[i] = DbRows_getSubsColumn(row, subType, onlyEnable, i);

	return arr;
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

DbValues DbRows_getOptions(BIG propRow, const char* valueType, BOOL onlyEnable)
{
	DbValues values = DbValues_init();

	//BIG propRow = DbRows_findSubType(row, subType);
	DbTable* table = DbRoot_findParentTable(propRow);

	if (table && propRow >= 0)
	{
		UBIG i = 0;
		BIG it;
		while ((it = DbColumnN_jump(DbRoot_subs(), propRow, &i, 1)) >= 0)
		{
			if (!onlyEnable || _DbRoot_isEnable(it))
				DbValues_add(&values, DbValue_initOption(it, valueType, 0));
			i++;
		}
	}

	return values;
}

BOOL DbRows_isEnable(BIG row)
{
	return _DbRoot_isEnable(row);
}
void DbRows_setEnable(BIG row, BOOL enable)
{
	DbRoot_setEnable(row, enable);
}

DbRows DbRows_initTables(void)
{
	DbRows self = DbRows_initArray(0, DbRoot_getTableLinks());
	return self;
}

DbRows DbRows_initFilter(BIG row)
{
	DbFilter* filter = _DbRoot_findFilterParent(row);	//DbRoot_findFilter(row);

	DbRows self;
	if (!filter || DbFilter_isEmpty(filter))
	{
		self = DbRows_initTable(DbRoot_findParentTable(row));
	}
	else
	{
		self = DbRows_initEmpty();
		_DbRows_setFilter(&self, filter);
		self.table = DbRoot_findParentTable(row);
	}
	self.row = row;
	return self;
}

BOOL DbRows_hasFilterSubActive(BIG row, const char* subName)
{
	DbTable* table = DbRoot_findParentTable(row);
	if (table)
	{
		BIG subRow = DbRows_findSubType(row, subName);
		if (subRow >= 0)
		{
			if (!_DbRoot_isEnable(subRow))
				return FALSE;

			if (Std_cmpCHAR(subName, "select") && _DbRoot_getOptionNumber(subRow, "maxRecords", 0) > 0)
				return TRUE;

			UBIG i = 0;
			BIG it;
			while ((it = DbColumnN_jump(DbRoot_subs(), subRow, &i, 1)) >= 0)
			{
				if (_DbRoot_isEnable(it))
				{
					DbColumn* column = DbRoot_ref_column(it);
					if (!column)
					{
						BIG colRow = DbRoot_findChildType(it, "column");
						if (colRow >= 0)
							column = DbRoot_ref_column(colRow);
					}
					return column != 0;
					//return TRUE;
				}
				i++;
			}
		}
	}

	return FALSE;
}

BOOL DbRows_hasColumnsSubDeactive(BIG row, const char* columnsName)
{
	//DbColumnN* subs = DbRoot_getColumnSubs();

	BIG columnsRow = DbRows_findSubType(row, columnsName);
	if (columnsRow >= 0)
	{
		//short
		UBIG i = 0;
		BIG it;
		while ((it = DbColumnN_jump(DbRoot_subs(), columnsRow, &i, 1)) >= 0)
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
	if (self->table)
		DbTable_removeRow(self->table, row);
	else
		if (self->column && self->row >= 0)
			DbColumn_remove(self->column, self->row, row);
}

void DbRows_removeRowDirect(DbTable* table, BIG row)
{
	if (table)
		DbTable_removeRow(table, row);
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

BOOL DbRows_getColumnMinMax(DbRows* self, DbColumn* column, double* mn, double* mx)
{
	const UBIG N = DbRows_getSize(self);
	if (N == 0)
		return FALSE;

	UBIG i;
	for (i = 0; i < N; i++)
	{
		UBIG it = DbRows_getRow(self, i);

		double v = DbColumn_getFlt(column, it, 0);
		*mn = Std_dmin(*mn, v);
		*mx = Std_dmax(*mx, v);
	}

	return (N > 0);
}

BOOL DbRows_getColumnsMinMax(DbRows* self, DbColumn** columns, double* mn, double* mx)
{
	if (!columns)
		return FALSE;

	const UBIG N = DbRows_getSize(self);
	UBIG i;
	for (i = 0; i < N; i++)
	{
		UBIG r = DbRows_getRow(self, i);

		UBIG sum = 0;
		DbColumn** it = columns;
		while (*it)
		{
			sum += Std_dmax(0, DbColumn_getFlt(*it, r, 0));	//no negative values
			it++;
		}

		*mn = Std_dmin(*mn, sum);
		*mx = Std_dmax(*mx, sum);
	}

	return (N > 0);
}

BIG DbRows_findRowScroll(DbRows* self, BIG findRow)
{
	if (self->arrayStatic)	//must be first!
	{
		UBIG pos = 0;
		BIG i;
		for (i = 0; i < self->array.num; i++)
		{
			BIG r = self->array.ptrs[i];
			if (DbTable_isRowActive(self->table, r))
			{
				if (FileRow_isRow(DbTable_getFileRow(self->table, r), findRow))
					return pos;
				pos++;	//only for valid rows
			}
		}
	}
	else
		if (self->filter)
		{
			UBIG pos = 0;
			BIG i;
			for (i = 0; i < self->filter->rows.num; i++)
			{
				BIG r = self->filter->rows.ptrs[i];
				if (DbTable_isRowActive(self->table, r))
				{
					if (FileRow_isRow(DbTable_getFileRow(self->table, r), findRow))
						return pos;
					pos++;	//only for valid rows
				}
			}
		}
		else
			if (self->table)
			{
				return DbTable_findRowScroll(self->table, findRow);
			}
			else
				if (DbRows_isColumnValid(self))
				{
					const UBIG N = DbRows_getSize(self);
					UBIG pos = 0;
					BIG i;
					for (i = 0; i < N; i++)
					{
						BIG r = DbRows_getRow(self, i);
						if (DbTable_isRowActive(self->table, r))
						{
							if (FileRow_isRow(DbTable_getFileRow(self->table, r), findRow))
								return pos;
							pos++;	//only for valid rows
						}
					}
				}
					
	return -1;
}


/*const UBIG NUM_ROWS = DbColumn_numRows(&self->base);
UBIG pos = 0;
UBIG i;
for (i = 0; i < NUM_ROWS; i++)
{
	FileRow row = DbColumn1_getFileId(self, i);
	if (FileRow_is(row))	//only valid
	{
		if (FileRow_isRow(row, r))
			return pos;
		pos++;
	}
}
return -1;*/


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

typedef struct DbColumn_s
{
	FileRow fileId;

	DbColumns* parent;

	TableItems data;
	TableItems changes;

	DbColumnTYPE type;
	DbFormatTYPE format;
	BOOL loaded;

	UBIG numChanges;
	DbColumn* copyColumnAsk;
} DbColumn;

DbColumn DbColumn_init(DbColumnTYPE type, DbFormatTYPE format, DbColumns* parent, double defValue)
{
	DbColumn self;

	self.fileId = FileRow_initEmpty();
	self.parent = parent;
	self.data = TableItems_init(defValue);
	self.changes = TableItems_init(0);
	self.type = type;
	self.format = format;
	self.loaded = TRUE;
	self.numChanges = 0;
	self.copyColumnAsk = 0;

	return self;
}

BIG DbColumn_getRow(const DbColumn* self)
{
	return DbRoot_getRow(self->fileId);
}

double DbColumn_getDefValue(const DbColumn* self)
{
	return self->data.defValue;
}

const UNI* DbColumn_getName(const DbColumn* self, UNI* out, const UBIG outMaxSize)
{
	return DbRoot_getName(DbColumn_getRow(self), out, outMaxSize);
}

void DbColumn_setName(DbColumn* self, const UNI* name)
{
	DbRoot_setName(DbColumn_getRow(self), name);
}

void DbColumn_free(DbColumn* self)
{
	TableItems_free(&self->data);
	TableItems_free(&self->changes);
}

void DbColumn_clearItems(DbColumn* self)
{
	TableItems_clear(&self->data);
	TableItems_clear(&self->changes);
}

TableItems* DbColumn_getHalds(DbColumn* self)
{
	return &self->data;
}

TableItem* DbColumn_getItem(DbColumn* self, const UBIG r)
{
	return TableItems_get(&self->data, r);
}
const TableItem* DbColumn_getItemConst(const DbColumn* self, const UBIG r)
{
	return TableItems_getConst(&self->data, r);
}

DbTable* DbColumn_getTable(const DbColumn* self)
{
	return self->parent ? DbColumns_getTable(self->parent) : 0;
}

DbColumnTYPE DbColumn_getType(const DbColumn* self)
{
	return self->type;
}

UBIG DbColumn_numRows(const DbColumn* self)
{
	DbTable* table = DbColumn_getTable(self);
	return table ? DbTable_numRows(table) : 0;
}

DbColumn1* DbColumn_getActive(const DbColumn* self)
{
	DbTable* table = DbColumn_getTable(self);
	return table ? DbTable_getColumnRows(table) : 0;
}

void DbColumn_setChange(DbColumn* self, UBIG i)
{
	TableItems_setBitTo1(&self->changes, i);
	self->numChanges++;
}

BOOL DbColumn_isChange(DbColumn* self, UBIG i)
{
	return TableItems_getBit(&self->changes, i);
}

void DbColumn_reset_save(DbColumn* self)
{
	TableItems_resetAllTo0(&self->changes, 0);
	self->numChanges = 0;
}

void DbColumn_updateNumberOfRows(DbColumn* self, const UBIG old_num_rows)
{
	const UBIG NUM_ROWS = DbColumn_numRows(self);
	TableItems_add(&self->data, NUM_ROWS);

	TableItems_addBits(&self->changes, NUM_ROWS);

	//set new-ones to 1
	BIG i;
	for (i = old_num_rows; i < NUM_ROWS; i++)
		DbColumn_setChange(self, i);

	if (old_num_rows != NUM_ROWS)
		self->numChanges++;
}

/*void DbColumn_resetChanges(DbColumn* self)
{
	TableItems_resetAllTo1(&self->changes, 0);
	self->numChanges++;
}*/

BOOL DbColumn_isChangedSave(const DbColumn* self)
{
	return TableItems_isChangedSave(&self->changes);
}
BOOL DbColumn_isChangedExe(const DbColumn* self)
{
	return TableItems_isChangedExe(&self->changes);
}

UBIG DbColumn_numChanges(const DbColumn* self)
{
	return self->numChanges;
}

double DbColumn_getOptionNumber(const DbColumn* self, const char* name)
{
	return _DbRoot_getOptionNumber(DbColumn_getRow(self), name, 0);
}
UNI* DbColumn_getOption(const DbColumn* self, const char* name, UNI* out, const UBIG outMaxSize)
{
	return _DbRoot_getOptionString(DbColumn_getRow(self), name, 0, out, outMaxSize);
}
void DbColumn_setOptionString(DbColumn* self, const char* name, const UNI* value)
{
	_DbRoot_setOptionString(DbColumn_getRow(self), name, value);
}
void DbColumn_setOptionNumber(DbColumn* self, const char* name, double value)
{
	_DbRoot_setOptionNumber(DbColumn_getRow(self), name, value);
}
BOOL DbColumn_cmpOption(const DbColumn* self, const char* name, const UNI* compare)
{
	return _DbRoot_cmpOptionString(DbColumn_getRow(self), name, 0, compare);
}

BOOL DbColumn_isType1(const DbColumn* self)
{
	return self->type == DbColumn_1;
}
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

typedef struct DbInsight_s DbInsight;
typedef struct DbInsightItem_s DbInsightItem;

typedef void DbInsightFuncCallback(DbInsight* self, BIG row);
typedef struct DbInsightFunc_s
{
	const char* name;
	DbInsightFuncCallback* func;
} DbInsightFunc;

typedef struct DbInsightItem_s
{
	DbColumn* column;
	//const DbInsightFunc* func;
	BIG column_numChanges;
	DbInsightItem* next;
}DbInsightItem;

DbInsightItem* DbInsightItem_new(DbColumn* column)
{
	DbInsightItem* self = Os_malloc(sizeof(DbInsightItem));
	self->column = column;
	//self->func = 0;
	self->column_numChanges = -1;
	self->next = 0;
	return self;
}

DbInsightItem* DbInsightItem_newCopy(const DbInsightItem* src)
{
	DbInsightItem* self = Os_malloc(sizeof(DbInsightItem));
	*self = *src;

	if (src->next)
		self->next = DbInsightItem_newCopy(src->next);

	return self;
}

void DbInsightItem_delete(DbInsightItem* self)
{
	if (self->next)
		DbInsightItem_delete(self->next);

	Os_free(self, sizeof(DbInsightItem));
}

BOOL DbInsightItem_cmp(const DbInsightItem* a, const DbInsightItem* b)
{
	BOOL same = (a->next && b->next) || (!a->next && !b->next);

	same &= a->column == b->column;

	if (same && a->next)
		same &= DbInsightItem_cmp(a->next, b->next);

	return same;
}

BOOL DbInsightItem_needUpdate(DbInsightItem* self)
{
	BOOL update = (self->column->numChanges != self->column_numChanges);
	self->column_numChanges = self->column->numChanges;

	if (self->next)
		update |= DbInsightItem_needUpdate(self->next);

	return update;
}

typedef struct DbInsight_s
{
	//BIG row;

	DbTable* srcTable;
	DbFilter* srcFilter;

	//BIG srcFilterGroupRow;
	//DbRows* rows;

	DbInsightItem* items;

	BIG numChanges;

	//BIG refs;
	//double ref_time;

	double temp_sum;
	double temp_min;
	double temp_max;
	double temp_mean;
	UBIG temp_num;

	StdBigs arr;

	StdString result_string;
	double result_number;
	double result_number2;
	DbColumn* result_column;

	const DbInsightFunc* func;

	int precision;
}DbInsight;

DbInsight* DbInsight_new(const DbInsightFunc* func, DbTable* srcTable, DbFilter* srcFilter, DbColumn* result_column)
{
	DbInsight* self = Os_malloc(sizeof(DbInsight));
	//self->row = row;
	self->func = func;
	self->srcTable = srcTable;
	self->srcFilter = srcFilter;
	self->result_column = result_column;
	self->precision = 0;

	self->items = 0;

	self->numChanges = -1;

	//self->refs = 0;
	//self->ref_time = 0;

	self->result_string = StdString_init();
	self->result_number = 0;
	return self;
}

DbInsight* DbInsight_newCopy(const DbInsight* src)
{
	DbInsight* self = DbInsight_new(src->func, src->srcTable, src->srcFilter, src->result_column);

	if (src->items)
		self->items = DbInsightItem_newCopy(src->items);

	return self;
}

void DbInsight_delete(DbInsight* self)
{
	if (self->items)
		DbInsightItem_delete(self->items);

	StdString_free(&self->result_string);

	Os_free(self, sizeof(DbInsight));
}

BOOL DbInsight_cmp(const DbInsight* a, const DbInsight* b)
{
	BOOL same = TRUE;

	same &= a->srcTable == b->srcTable;

	same &= (a->items && b->items) || (!a->items && !b->items);
	same &= a->func == b->func;

	if (same && a->items)
		same &= DbInsightItem_cmp(a->items, b->items);

	return same;
}

BOOL DbInsight_isEmpty(const DbInsight* self)
{
	return self && !self->items;
}

DbInsightFuncCallback* DbInsight_getFinalFunc(const DbInsight* self)
{
	return self->func ? self->func->func : 0;
}

static DbInsightItem* _DbInsight_getFinalItem(const DbInsight* self)
{
	DbInsightItem* it = self->items;
	while (it)
	{
		if (!it->next)
			return it;
		it = it->next;
	}
	return 0;
}
DbColumn* DbInsight_getFinalColumn(const DbInsight* self)
{
	DbInsightItem* it = _DbInsight_getFinalItem(self);
	return it ? it->column : 0;
}

BOOL DbInsight_hasError(const DbInsight* self)
{
	DbColumn* column = DbInsight_getFinalColumn(self);
	return column ? DbColumn_getBTable(column) != 0 : TRUE;	//last can be Link
}

DbColumn* DbInsight_getItemColumn(const DbInsight* self, BIG i)
{
	DbInsightItem* it = self->items;
	while (it)
	{
		if (i == 0)
			return it->column;
		i--;

		it = it->next;
	}
	return 0;
}

const int DbInsight_getFinalPresicion(const DbInsight* self)
{
	DbInsightItem* it = _DbInsight_getFinalItem(self);
	return it ? DbColumn_getFormatPrecision(it->column) : 0;
}

void DbInsight_addItem(DbInsight* self, DbColumn* column)
{
	if (column)
	{
		DbInsightItem** next = &self->items;
		while (*next)
			next = &(*next)->next;
		*next = DbInsightItem_new(column);
	}
}

static BOOL DbInsight_needUpdate(DbInsight* self)
{
	BOOL update = FALSE;

	BIG numChanges = DbTable_numChanges(self->srcTable);
	update |= (self->numChanges != numChanges);

	self->numChanges = numChanges;

	if (self->items)
		update |= DbInsightItem_needUpdate(self->items);

	return update;
}

BOOL DbInsight_execute(DbInsight* self)
{
	if (!DbInsight_needUpdate(self))
		return FALSE;

	if (!self->items)
		return FALSE;

	self->precision = DbInsight_getFinalPresicion(self);

	DbInsightFuncCallback* func = DbInsight_getFinalFunc(self);

	BIG i;
	if (self->result_column)
	{
		//call it for every row and save it to resultColumn[row]

		if (self->srcFilter)
		{
			const StdBigs* rows = DbFilter_getRows(self->srcFilter);
			for (i = 0; i < rows->num; i++)
				func(self, rows->ptrs[i]);
		}
		else
		{
			const UBIG NUM_ROWS = DbTable_numRows(self->srcTable);
			BIG i;
			for (i = 0; i < NUM_ROWS; i++)
			{
				if (DbTable_isRowActive(self->srcTable, i))
					func(self, i);
			}
		}
	}
	else
		func(self, -1);	//call is for all record in table/filter => one result

	return TRUE;
}

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

typedef struct DbJointedItem_s
{
	DbColumn* in;
	DbColumn* out;

	BIG in_numChanges;
	BIG out_numChanges;

	struct DbJointedItem_s* next;
}DbJointedItem;

DbJointedItem* DbJointedItem_new(DbColumn* in, DbColumn* out)
{
	DbJointedItem* self = Os_malloc(sizeof(DbJointedItem));
	self->in = in;
	self->out = out;

	self->in_numChanges = -1;
	self->out_numChanges = -1;

	self->next = 0;
	return self;
}

DbJointedItem* DbJointedItem_newCopy(const DbJointedItem* src)
{
	DbJointedItem* self = Os_malloc(sizeof(DbJointedItem));
	*self = *src;

	if (src->next)
		self->next = DbJointedItem_newCopy(src->next);

	return self;
}

void DbJointedItem_delete(DbJointedItem* self)
{
	if (self->next)
		DbJointedItem_delete(self->next);

	Os_free(self, sizeof(DbJointedItem));
}

BOOL DbJointedItem_cmp(const DbJointedItem* a, const DbJointedItem* b)
{
	BOOL same = (a->next && b->next) || (!a->next && !b->next);

	same &= a->in == b->in;
	same &= a->out == b->out;

	if (same && a->next)
		same &= DbJointedItem_cmp(a->next, b->next);

	return same;
}

BOOL DbJointedItem_needUpdate(DbJointedItem* self)
{
	BOOL update = (self->in->numChanges != self->in_numChanges || self->out->numChanges != self->out_numChanges);

	self->in_numChanges = self->in->numChanges;
	self->out_numChanges = self->out->numChanges;

	if (self->next)
		update |= DbJointedItem_needUpdate(self->next);

	return update;
}

typedef struct DbJointed_s
{
	DbColumnN* jointedColumn;
	DbJointedItem* srcDst;

	DbJointedItem* items;

	StdBigs rows;
}DbJointed;

DbJointed* DbJointed_new(DbColumnN* jointedColumn, DbColumn* src, DbColumn* dst)
{
	DbJointed* self = Os_malloc(sizeof(DbJointed));
	self->jointedColumn = jointedColumn;
	self->srcDst = DbJointedItem_new(src, dst);
	self->items = 0;
	return self;
}

DbJointed* DbJointed_newCopy(const DbJointed* src)
{
	DbJointed* self = DbJointed_new(src->jointedColumn, src->srcDst->in, src->srcDst->out);

	if (src->items)
		self->items = DbJointedItem_newCopy(src->items);

	return self;
}

void DbJointed_delete(DbJointed* self)
{
	DbJointedItem_delete(self->srcDst);

	if (self->items)
		DbJointedItem_delete(self->items);

	Os_free(self, sizeof(DbJointed));
}

BOOL DbJointed_cmp(const DbJointed* a, const DbJointed* b)
{
	BOOL same = TRUE;

	same &= (a->items && b->items) || (!a->items && !b->items);

	same &= (a->jointedColumn == b->jointedColumn);

	same &= DbJointedItem_cmp(a->srcDst, b->srcDst);

	if (same && a->items)
		same &= DbJointedItem_cmp(a->items, b->items);

	return same;
}

void DbJointed_addItem(DbJointed* self, DbColumn* in, DbColumn* out)
{
	if (in && out)
	{
		DbJointedItem** next = &self->items;
		while (*next)
			next = &(*next)->next;
		*next = DbJointedItem_new(in, out);
	}
}

BOOL DbJointed_needUpdate(DbJointed* self)
{
	BOOL update = FALSE;

	update |= DbJointedItem_needUpdate(self->srcDst);

	if (self->items)
		update |= DbJointedItem_needUpdate(self->items);

	return update;
}

static void _DbJointed_executeNumbers(DbColumn* in, DbColumn* out, StdBigs* inRows, StdBigs* outRows)
{
	DbTable* outTable = DbColumn_getTable(out);
	const UBIG OUT_NUM_ROWS = DbTable_numRows(outTable);

	BIG i, j;
	for (i = 0; i < inRows->num; i++)
	{
		double inValue = DbColumn_getFlt(in, inRows->ptrs[i], 0);	//co když je string? ... uložit typ do DbJointedItem?

		for (j = 0; j < OUT_NUM_ROWS; j++)
		{
			if (DbTable_isRowActive(outTable, j))
			{
				double outValue = DbColumn_getFlt(out, j, 0);
				if (inValue == outValue)
					StdBigs_add(outRows, j);
			}
		}
	}
}
static void _DbJointed_executeStrings(DbColumnString32* in, DbColumnString32* out, StdBigs* inRows, StdBigs* outRows)
{
	DbTable* outTable = DbColumn_getTable(&out->base);
	const UBIG OUT_NUM_ROWS = DbTable_numRows(outTable);

	BIG i, j;
	for (i = 0; i < inRows->num; i++)
	{
		const UNI* inValue = DbColumnString32_get(in, inRows->ptrs[i]);	//co když je string? ... uložit typ do DbJointedItem?

		for (j = 0; j < OUT_NUM_ROWS; j++)
		{
			if (DbTable_isRowActive(outTable, j))
			{
				const UNI* outValue = DbColumnString32_get(out, j);
				if (Std_cmpUNI(inValue, outValue))
					StdBigs_add(outRows, j);
			}
		}
	}
}

static void _DbJointed_executeEx(DbColumn* in, DbColumn* out, StdBigs* inRows, StdBigs* outRows)
{
	if (in->type == DbColumn_STRING_32 && out->type == DbColumn_STRING_32)
		_DbJointed_executeStrings((DbColumnString32*)in, (DbColumnString32*)out, inRows, outRows);
	else
		_DbJointed_executeNumbers(in, out, inRows, outRows);
}

BOOL DbJointed_execute(DbJointed* self)
{
	//if (!DbJointed_needUpdate(self))
	//	return FALSE;

	//if (!self->items)
	//	return FALSE;

	StdBigs inRows = StdBigs_init();
	StdBigs outRows = StdBigs_init();

	DbTable* inTable = DbColumn_getTable(self->srcDst->in);
	const UBIG IN_NUM_ROWS = DbTable_numRows(inTable);
	BIG i;
	for (i = 0; i < IN_NUM_ROWS; i++)
	{
		StdBigs_clear(&inRows);
		StdBigs_clear(&outRows);

		if (DbTable_isRowActive(inTable, i))
		{
			StdBigs_add(&inRows, i);

			DbColumn* in = self->srcDst->in;

			DbJointedItem* next = self->items;
			while (next)
			{
				DbColumn* out = next->in;

				_DbJointed_executeEx(in, out, &inRows, &outRows);

				in = next->out;

				next = next->next;

				StdBigs t = inRows;
				inRows = outRows;
				outRows = t;
			}

			//last one
			{
				DbColumn* out = self->srcDst->out;
				_DbJointed_executeEx(in, out, &inRows, &outRows);
			}

			//write links
			DbColumnN_setArrayBigs(self->jointedColumn, i, &outRows);
		}
	}

	StdBigs_free(&inRows);
	StdBigs_free(&outRows);

	return TRUE;
}

BOOL DbJointed_check(DbColumnN* jointedColumn)
{
	BIG row = DbColumn_getRow(&jointedColumn->base);

	DbColumn* src = DbRoot_ref_column(DbRows_findOrCreateSubType(row, "in"));
	DbColumn* dst = DbRoot_ref_column(DbRows_findOrCreateSubType(row, "out"));

	if (!src || !dst)
		return FALSE;

	DbTable* srcTable = DbRoot_findParentTable(row);
	DbTable* dstTable = DbColumn_getBTable(&jointedColumn->base);

	if (!srcTable || !dstTable)
		return FALSE;

	if (srcTable != DbColumn_getTable(src) || dstTable != DbColumn_getTable(dst))
		return FALSE;

	BIG pathRow = DbRows_findOrCreateSubType(row, "path");
	BIG it;
	UBIG ii = 0;
	while ((it = DbColumnN_jump(DbRoot_subs(), pathRow, &ii, 1)) >= 0)
	{
		DbColumn* in = DbRoot_ref_column(DbRows_findOrCreateSubType(it, "in"));
		DbColumn* out = DbRoot_ref_column(DbRows_findOrCreateSubType(it, "out"));

		if (!in || !out)
			return FALSE;

		DbTable* btable = DbRoot_ref_table(DbRows_findOrCreateSubType(it, "btable"));

		if (!btable)
			return FALSE;

		if (btable != DbColumn_getTable(in) || btable != DbColumn_getTable(out))
			return FALSE;

		ii++;
	}

	return TRUE;
}

DbJointed* DbJointed_build(DbColumnN* column)
{
	BIG row = DbColumn_getRow(&column->base);

	DbColumn* src = DbRoot_ref_column(DbRows_findOrCreateSubType(row, "in"));
	DbColumn* dst = DbRoot_ref_column(DbRows_findOrCreateSubType(row, "out"));

	DbJointed* jointed = DbJointed_new((DbColumnN*)column, src, dst);

	BIG pathRow = DbRows_findOrCreateSubType(row, "path");

	BIG it;
	UBIG ii = 0;
	while ((it = DbColumnN_jump(DbRoot_subs(), pathRow, &ii, 1)) >= 0)
	{
		DbColumn* in = DbRoot_ref_column(DbRows_findOrCreateSubType(it, "in"));
		DbColumn* out = DbRoot_ref_column(DbRows_findOrCreateSubType(it, "out"));
		DbJointed_addItem(jointed, in, out);

		ii++;
	}

	return jointed;
}
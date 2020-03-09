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

typedef struct DbFilterGroup_s
{
	DbColumn* column;
	BOOL ascending;
	BOOL addOnlyOneRow;

	BIG column_numChanges;
	struct DbFilterGroup_s* next;
}DbFilterGroup;

DbFilterGroup* DbFilterGroup_new(DbColumn* column, BOOL ascending, BOOL addOnlyOneRow)
{
	DbFilterGroup* self = Os_malloc(sizeof(DbFilterGroup));
	self->column = column;
	self->column_numChanges = -1;
	self->ascending = ascending;
	self->addOnlyOneRow = addOnlyOneRow;
	self->next = 0;
	return self;
}

DbFilterGroup* DbFilterGroup_newCopy(const DbFilterGroup* src)
{
	DbFilterGroup* self = Os_malloc(sizeof(DbFilterGroup));
	*self = *src;

	if (src->next)
		self->next = DbFilterGroup_newCopy(src->next);

	return self;
}

void DbFilterGroup_delete(DbFilterGroup* self)
{
	if (self->next)
		DbFilterGroup_delete(self->next);

	Os_free(self, sizeof(DbFilterGroup));
}

BOOL DbFilterGroup_cmp(const DbFilterGroup* a, const DbFilterGroup* b)
{
	BOOL same = (a->next && b->next) || (!a->next && !b->next);

	same &= a->column == b->column && a->ascending == b->ascending && a->addOnlyOneRow == b->addOnlyOneRow;

	if (same && a->next)
		same &= DbFilterGroup_cmp(a->next, b->next);

	return same;
}

BOOL DbFilterGroup_needUpdate(DbFilterGroup* self, BOOL saveChanges)
{
	BOOL update = (self->column->numChanges != self->column_numChanges);
	if (saveChanges)
		self->column_numChanges = self->column->numChanges;

	if (self->next)
		update |= DbFilterGroup_needUpdate(self->next, saveChanges);

	return update;
}

UBIG DbFilterGroup_num(const DbFilterGroup* self)
{
	UBIG n = 0;
	const DbFilterGroup* t = self;
	while (t)
	{
		n++;
		t = t->next;
	}
	return n;
}

BIG DbFilterGroup_execute(DbFilterGroup* self, StdBigs* poses, const BIG start, const BIG end, DbTable* groupTable, DbColumnN* group_subs, DbColumnN* group_rows, DbColumn1* group_count)
{
	if (start >= end)
		return -1;

	UBIG i;
	DbColumn_short(self->column, poses, start, end, self->ascending);

	UBIG r = DbTable_addRow(groupTable);
	DbColumn1_set(group_count, r, (end - start));

	if (self->addOnlyOneRow)
		DbColumnN_add(group_rows, r, poses->ptrs[start]);
	else
	{
		//DbColumnN_add_bigs(poses, start, end); //faster ...
		for (i = start; i < end && StdProgress_is(); i++)
			DbColumnN_add(group_rows, r, poses->ptrs[i]);
	}

	UBIG last = start;
	for (i = start + 1; i < end && StdProgress_is(); i++)
	{
		//Std_printlnUNI(DbColumnString32_get((DbColumnString32*)self->column, poses->ptrs[i]));

		if (!DbColumn_cmpRow(self->column, poses->ptrs[i - 1], poses->ptrs[i]))
		{
			if (self->next)
			{
				UBIG rr = DbFilterGroup_execute(self->next, poses, last, i, groupTable, group_subs, group_rows, group_count);
				DbColumnN_add(group_subs, r, rr);
			}
			else
			{
				UBIG rr = DbTable_addRow(groupTable);
				DbColumnN_add(group_subs, r, rr);
				UBIG ii;
				for (ii = last; ii < i; ii++)
					DbColumnN_add(group_rows, rr, poses->ptrs[ii]);
				DbColumn1_set(group_count, rr, (i - last));	//count
			}
			last = i;
		}
	}

	if (StdProgress_is())
	{
		if (self->next)
		{
			UBIG rr = DbFilterGroup_execute(self->next, poses, last, i, groupTable, group_subs, group_rows, group_count);
			DbColumnN_add(group_subs, r, rr);
		}
		else
		{
			UBIG rr = DbTable_addRow(groupTable);
			DbColumnN_add(group_subs, r, rr);
			UBIG ii;
			for (ii = last; ii < i && StdProgress_is(); ii++)
				DbColumnN_add(group_rows, rr, poses->ptrs[ii]);
			DbColumn1_set(group_count, rr, (i - last));
		}
	}

	return r;
}

typedef struct DbFilterShort_s
{
	DbColumn* column;
	BOOL ascending;

	BIG column_numChanges;
	struct DbFilterShort_s* next;
}DbFilterShort;

DbFilterShort* DbFilterShort_new(DbColumn* column, BOOL ascending)
{
	DbFilterShort* self = Os_malloc(sizeof(DbFilterShort));
	self->column = column;
	self->column_numChanges = -1;
	self->ascending = ascending;
	self->next = 0;
	return self;
}

DbFilterShort* DbFilterShort_newCopy(const DbFilterShort* src)
{
	DbFilterShort* self = Os_malloc(sizeof(DbFilterShort));
	*self = *src;

	if (src->next)
		self->next = DbFilterShort_newCopy(src->next);
	return self;
}

void DbFilterShort_delete(DbFilterShort* self)
{
	if (self->next)
		DbFilterShort_delete(self->next);

	Os_free(self, sizeof(DbFilterShort));
}

UBIG DbFilterShort_num(const DbFilterShort* self)
{
	UBIG n = 0;
	const DbFilterShort* t = self;
	while (t)
	{
		n++;
		t = t->next;
	}
	return n;
}

BOOL DbFilterShort_cmp(const DbFilterShort* a, const DbFilterShort* b)
{
	BOOL same = (a->next && b->next) || (!a->next && !b->next);

	same &= a->column == b->column && a->ascending == b->ascending;

	if (same && a->next)
		same &= DbFilterShort_cmp(a->next, b->next);

	return same;
}

BOOL DbFilterShort_needUpdate(DbFilterShort* self, BOOL saveChanges)
{
	BOOL update = (self->column->numChanges != self->column_numChanges);
	if (saveChanges)
		self->column_numChanges = self->column->numChanges;

	if (self->next)
		update |= DbFilterShort_needUpdate(self->next, saveChanges);

	return update;
}

void DbFilterShort_execute(DbFilterShort* self, StdBigs* poses, const BIG start, const BIG end)
{
	if (start >= end)
		return;

	DbColumn_short(self->column, poses, start, end, self->ascending);

	if (self->next && StdProgress_is())
	{
		UBIG last = start;
		UBIG i;
		for (i = start + 1; i < end && StdProgress_is(); i++)
		{
			if (!DbColumn_cmpRow(self->column, poses->ptrs[i - 1], poses->ptrs[i]))
			{
				DbFilterShort_execute(self->next, poses, last, i);
				last = i;
			}
		}

		DbFilterShort_execute(self->next, poses, last, i);
	}
}

typedef struct DbFilterSelect_s
{
	DbColumn* column;
	const DbFilterSelectFunc* func;
	BOOL andd;
	UNI* value;
	double valueEx;

	BIG column_numChanges;
	struct DbFilterSelect_s* next;
}DbFilterSelect;

DbFilterSelect* DbFilterSelect_new(BOOL andd, DbColumn* column, const DbFilterSelectFunc* func, const UNI* value, double valueEx)
{
	DbFilterSelect* self = Os_malloc(sizeof(DbFilterSelect));
	self->column = column;
	self->column_numChanges = -1;
	self->func = func;
	self->value = Std_newUNI(value);
	self->valueEx = valueEx;
	self->andd = andd;
	self->next = 0;
	return self;
}
DbFilterSelect* DbFilterSelect_newCopy(const DbFilterSelect* src)
{
	DbFilterSelect* self = Os_malloc(sizeof(DbFilterSelect));
	*self = *src;

	self->value = Std_newUNI(src->value);
	if (src->next)
		self->next = DbFilterSelect_newCopy(src->next);
	return self;
}
void DbFilterSelect_delete(DbFilterSelect* self)
{
	Std_deleteUNI(self->value);

	if (self->next)
		DbFilterSelect_delete(self->next);
	Os_free(self, sizeof(DbFilterSelect));
}

UBIG DbFilterSelect_num(const DbFilterSelect* self)
{
	UBIG n = 0;
	const DbFilterSelect* t = self;
	while (t)
	{
		n++;
		t = t->next;
	}
	return n;
}

BOOL DbFilterSelect_cmp(const DbFilterSelect* a, const DbFilterSelect* b)
{
	BOOL same = (a->next && b->next) || (!a->next && !b->next);

	same &= a->column == b->column && a->func == b->func && a->andd == b->andd && Std_cmpUNI(a->value, b->value) && a->valueEx == b->valueEx;

	if (same && a->next)
		same &= DbFilterSelect_cmp(a->next, b->next);

	return same;
}

BOOL DbFilterSelect_needUpdate(DbFilterSelect* self, BOOL saveChanges)
{
	BOOL update = (self->column->numChanges != self->column_numChanges);
	if (saveChanges)
		self->column_numChanges = self->column->numChanges;

	if (self->next)
		update |= DbFilterSelect_needUpdate(self->next, saveChanges);

	return update;
}

void DbFilterSelect_executeEx(DbFilterSelect* self, StdBigs* poses, UCHAR* marks)
{
	if (self->func)
		self->func->func(self->column, self->value, self->valueEx, self->andd, poses, marks);

	//next
	if (self->next && StdProgress_is())
		DbFilterSelect_executeEx(self->next, poses, marks);
}

void DbFilterSelect_execute(DbFilterSelect* self, StdBigs* poses)
{
	UBIG oldN = poses->num;
	UCHAR* marks = Os_calloc(oldN, sizeof(UCHAR));
	DbFilterSelect_executeEx(self, poses, marks);

	//remove unmarked "poses"
	UBIG p = 0;
	UBIG i;
	for (i = 0; i < oldN && StdProgress_is(); i++)
		if (marks[i])
			poses->ptrs[p++] = poses->ptrs[i];
	StdBigs_resize(poses, p);

	//clean
	Os_free(marks, oldN * sizeof(UCHAR));
}

typedef struct DbFilter_s
{
	BIG row;

	DbFilterGroup* group;
	DbFilterShort* shortt;
	DbFilterSelect* select;

	StdBigs rows;

	DbTable* groupTable;
	DbColumnN* group_subs;
	DbColumnN* group_rows;
	DbColumn1* group_count;

	BIG numChanges;
	struct DbFilter_s* parent;

	UBIG maxRecords;

	//BIG refs;
	//double ref_time;
}DbFilter;

DbFilter* DbFilter_new(BIG row)
{
	DbFilter* self = Os_malloc(sizeof(DbFilter));

	self->row = row;
	self->group = 0;
	self->shortt = 0;
	self->select = 0;

	self->rows = StdBigs_init();

	self->maxRecords = 0;

	self->groupTable = DbRoot_addTableNotSave();
	self->group_subs = DbColumns_addColumnN(self->groupTable->columns, self->groupTable);
	self->group_rows = DbColumns_addColumnN(self->groupTable->columns, DbFilter_getTable(self));
	self->group_count = DbColumns_addColumn1(self->groupTable->columns, 0);

	self->numChanges = -1;
	self->parent = 0;

	//self->refs = 0;
	//self->ref_time = 0;

	return self;
}

DbFilter* DbFilter_newCopy(const DbFilter* src)
{
	DbFilter* self = DbFilter_new(src->row);

	StdBigs_free(&self->rows);
	self->rows = StdBigs_initCopy(&src->rows);

	if (src->group)		self->group = DbFilterGroup_newCopy(src->group);
	if (src->shortt)	self->shortt = DbFilterShort_newCopy(src->shortt);
	if (src->select)	self->select = DbFilterSelect_newCopy(src->select);

	if (src->parent)
		self->parent = DbFilter_newCopy(src->parent);

	return self;
}

void DbFilter_delete(DbFilter* self)
{
	if (self->group)
		DbFilterGroup_delete(self->group);
	if (self->shortt)
		DbFilterShort_delete(self->shortt);
	if (self->select)
		DbFilterSelect_delete(self->select);

	StdBigs_free(&self->rows);

	DbRoot_removeTable(self->groupTable);

	//if (self->parent)
	//	DbFilter_delete(self->parent);

	Os_free(self, sizeof(DbFilter));
}

const StdBigs* DbFilter_getRows(const DbFilter* self)
{
	return &self->rows;
}

DbTable* DbFilter_getTable(const DbFilter* self)
{
	const BOOL isSummary = DbRoot_isTypeView_summary(self->row);
	return DbRoot_findParentTable(isSummary ? DbRoot_findParent(self->row) : self->row);
}

void DbFilter_addParent(DbFilter* self, DbFilter* parent)
{
	DbFilter** p = &self->parent;
	while (*p)
		p = &(*p)->parent;

	*p = parent;
}

BOOL DbFilter_cmp(const DbFilter* a, const DbFilter* b)
{
	BOOL same = TRUE;

	same &= a->row == b->row;
	//same &= a->srcTable == b->srcTable;

	same &= a->maxRecords == b->maxRecords;

	same &= (a->shortt && b->shortt) || (!a->shortt && !b->shortt);
	same &= (a->group && b->group) || (!a->group && !b->group);
	same &= (a->select && b->select) || (!a->select && !b->select);
	same &= (a->parent && b->parent) || (!a->parent && !b->parent);

	if (same && a->shortt)
		same &= DbFilterShort_cmp(a->shortt, b->shortt);
	if (same && a->group)
		same &= DbFilterGroup_cmp(a->group, b->group);
	if (same && a->select)
		same &= DbFilterSelect_cmp(a->select, b->select);

	if (same && a->parent)
		same &= DbFilter_cmp(a->parent, b->parent);

	return same;
}

void DbFilter_clearGroups(DbFilter* self)
{
	if (self->group)
		DbFilterGroup_delete(self->group);
	self->group = 0;

	if (self->parent)
		DbFilter_clearGroups(self->parent);
}

UBIG DbFilter_numGroupTable(const DbFilter* self)
{
	return DbTable_numRowsReal(self->groupTable);
}

DbColumnN* DbFilter_getColumnGroupSubs(const DbFilter* self)
{
	return self->group_subs;
}
DbColumnN* DbFilter_getColumnGroupRows(const DbFilter* self)
{
	return self->group_rows;
}
DbColumn1* DbFilter_getColumnGroupCount(const DbFilter* self)
{
	return self->group_count;
}

void DbFilter_getMinMaxCount(DbFilter* self, double* minV, double* maxV)
{
	UBIG i = 0;
	while (DbTable_jumpRows(self->groupTable, &i, 1) >= 0)
	{
		if (DbColumnN_sizeActive(self->group_subs, i) == 0)
		{
			double v = DbColumn1_get(self->group_count, i);
			*minV = Std_dmin(*minV, v);
			*maxV = Std_dmax(*maxV, v);
		}
		i++;
	}
}

UBIG DbFilter_numGroups(const DbFilter* self)
{
	return DbFilterGroup_num(self->group);
}

BOOL DbFilter_isEmpty(const DbFilter* self)
{
	return self && !self->group && !self->shortt && !self->select && !self->parent && self->maxRecords <= 0;
}

void DbFilter_setMaxRecords(DbFilter* self, UBIG maxRecords)
{
	self->maxRecords = maxRecords;
}

void DbFilter_addGroup(DbFilter* self, DbColumn* column, BOOL ascending, BOOL addOnlyOneRow)
{
	if (column)
	{
		DbFilterGroup** next = &self->group;
		while (*next)
			next = &(*next)->next;
		*next = DbFilterGroup_new(column, ascending, addOnlyOneRow);
	}
}
void DbFilter_addShort(DbFilter* self, DbColumn* column, BOOL ascending)
{
	if (column)
	{
		DbFilterShort** next = &self->shortt;
		while (*next)
			next = &(*next)->next;
		*next = DbFilterShort_new(column, ascending);
	}
}
void DbFilter_addSelect(DbFilter* self, BOOL andd, DbColumn* column, const DbFilterSelectFunc* func, const UNI* value, double valueEx)
{
	if (column)
	{
		DbFilterSelect** next = &self->select;
		while (*next)
			next = &(*next)->next;
		*next = DbFilterSelect_new((!self->select ? FALSE : andd), column, func, value, valueEx);	//first is always OR
	}
}

void DbFilter_addShortCopy(DbFilter* self, DbFilter* src)
{
	if (src->shortt)
	{
		DbFilterShort** next = &self->shortt;
		while (*next)
			next = &(*next)->next;
		*next = DbFilterShort_newCopy(src->shortt);
	}
}

static BOOL _DbFilter_needUpdateEx(DbFilter* self, BOOL saveChanges)
{
	BOOL update = FALSE;

	DbTable* srcTable = DbFilter_getTable(self);

	BIG numChanges = DbTable_numChanges(srcTable);
	update |= (self->numChanges != numChanges);

	if (saveChanges)
		self->numChanges = numChanges;

	//columns insights
	if (DbRoot_isTypeView_summary(self->row))
	{
		int c;
		UBIG N_COLS = DbRows_getSubsNum(self->row, "columns", TRUE);
		for (c = 0; c < N_COLS; c++)
		{
			DbColumn* column = DbRows_getSubsColumn(self->row, "columns", TRUE, c);
			if (column->insightTable)
				update |= DbInsight_needUpdate(column->insightTable);
		}
	}

	if (self->select)
		update |= DbFilterSelect_needUpdate(self->select, saveChanges);
	if (self->shortt)
		update |= DbFilterShort_needUpdate(self->shortt, saveChanges);
	if (self->group)
		update |= DbFilterGroup_needUpdate(self->group, saveChanges);

	if (self->parent)
		update |= _DbFilter_needUpdateEx(self->parent, FALSE);

	return update;
}
static BOOL _DbFilter_needUpdate(DbFilter* self)
{
	return _DbFilter_needUpdateEx(self, TRUE);
}

UBIG DbFilter_num(const DbFilter* self)
{
	UBIG n = 0;
	const DbFilter* t = self;
	while (t)
	{
		n += DbFilterShort_num(t->shortt) + DbFilterGroup_num(t->group) + DbFilterSelect_num(t->select);
		t = t->parent;
	}
	return n;
}

static void _DbFilter_buildSummaryTable(DbFilter* self)
{
	DbTable* newTable = DbRoot_findTable(self->row);

	DbColumnN* subs = DbFilter_getColumnGroupSubs(self);
	DbColumnN* rows = DbFilter_getColumnGroupRows(self);
	if (subs && rows)
	{
		DbColumn* linksColumn = 0;

		UBIG N_COLS = DbRows_getSubsNum(self->row, "columns", TRUE);
		BIG c;
		for (c = 0; c < N_COLS; c++)
		{
			DbColumn* column = DbRows_getSubsColumn(self->row, "columns", TRUE, c);
			if (column->summary_links)		linksColumn = column;
		}

		for (c = 0; c < N_COLS; c++)
		{
			DbColumn* column = DbRows_getSubsColumn(self->row, "columns", TRUE, c);
			BIG crow = DbRows_getSubsRow(self->row, "columns", TRUE, c);
			if (!DbColumn_isSummaryLinks(column) && !DbColumn_isSummaryGroup(column))
			{
				const DbInsightFunc* func = DbInsightSelectFunc_get(DbValue_getOptionNumber(crow, "insight_func", 0));

				DbInsight* insight = DbInsight_new(func, newTable, 0, column);
				DbInsight_addItem(insight, linksColumn);
				DbInsight_addItem(insight, column->summary_origColumn);

				DbColumn_setInsightTable(column, insight);
			}
		}
	}
}

static void _DbFilter_rewriteSummaryTable(DbFilter* self)
{
	DbTable* newTable = DbRoot_findTable(self->row);
	DbTable_clear(newTable);

	DbColumnN* subs = DbFilter_getColumnGroupSubs(self);
	DbColumnN* rows = DbFilter_getColumnGroupRows(self);

	if (subs && rows && DbFilter_numGroupTable(self))
	{
		DbColumn* groupColumn = 0;
		DbColumn* linksColumn = 0;

		UBIG N_COLS = DbRows_getSubsNum(self->row, "columns", TRUE);
		BIG c;
		for (c = 0; c < N_COLS; c++)
		{
			DbColumn* column = DbRows_getSubsColumn(self->row, "columns", TRUE, c);
			if (column->summary_group)		groupColumn = column;
			if (column->summary_links)		linksColumn = column;
		}

		//add rows and fill links
		BIG it;
		UBIG i = 0;
		while ((it = DbColumnN_jump(subs, 0, &i, 1)) >= 0)
		{
			UBIG newRow = DbTable_addRow(newTable);

			//copy "group column" value
			if (groupColumn)	//is visible
				DbColumn_copyRowEx(groupColumn, groupColumn->summary_origColumn, newRow, DbColumnN_getFirstRow(rows, it));

			//copy links
			if (linksColumn)
				DbColumnN_copyRow((DbColumnN*)linksColumn, newRow, rows, it);
			i++;
		}

		//columns insights
		for (c = 0; c < N_COLS; c++)
		{
			DbColumn* column = DbRows_getSubsColumn(self->row, "columns", TRUE, c);
			if (column->insightTable)
				DbInsight_execute(column->insightTable);
		}
	}

	StdBigs_clear(&self->rows);
	DbTable_getArrayPoses(newTable, &self->rows);
}

static void _DbFilter_executeEx(DbFilter* self, StdBigs* poses)
{
	DbTable_clear(self->groupTable);

	if (self->group)
		DbFilterGroup_execute(self->group, poses, 0, poses->num, self->groupTable, self->group_subs, self->group_rows, self->group_count);

	//needs to be call after Group and before short!
	if (DbRoot_isTypeView_summary(self->row))
		_DbFilter_rewriteSummaryTable(self);

	if (self->select)
		DbFilterSelect_execute(self->select, poses);

	if (self->shortt)
		DbFilterShort_execute(self->shortt, poses, 0, poses->num);

	if (self->maxRecords > 0 && self->maxRecords < poses->num)
		StdBigs_resize(poses, self->maxRecords);
}

BOOL DbFilter_execute(DbFilter* self)
{
	if (_DbFilter_needUpdate(self))
	{
		StdBigs_clear(&self->rows);

		if (self->parent)
		{
			DbFilter_execute(self->parent);
			StdBigs_free(&self->rows);
			self->rows = StdBigs_initCopy(&self->parent->rows);
		}
		else
		{
			DbTable* srcTable = DbFilter_getTable(self);
			if (srcTable)
				DbTable_getArrayPoses(srcTable, &self->rows);
		}

		_DbFilter_executeEx(self, &self->rows);

		_DbFilter_needUpdate(self);	//reset numChanges - so SummaryTable will NOT re-update!
		return TRUE;
	}
	return FALSE;
}
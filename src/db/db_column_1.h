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

typedef struct DbColumn1_s
{
	DbColumn base;
	DbTable* btable;
	BOOL isConverted;
} DbColumn1;

DbColumn1* DbColumn1_new(DbColumns* parent, DbFormatTYPE format, DbTable* btable)
{
	DbColumn1* self = Os_malloc(sizeof(DbColumn1));
	self->base = DbColumn_init(DbColumn_1, format, parent, btable ? -1 : 0);
	self->btable = btable;
	self->isConverted = TRUE;
	return self;
}

void DbColumn1_clear(DbColumn1* self)
{
	DbColumn_clearItems(&self->base);
}

void DbColumn1_delete(DbColumn1* self)
{
	DbColumn1_clear(self);
	DbColumn_free((DbColumn*)self);
	Os_free(self, sizeof(DbColumn1));
}

void DbColumn1_setBTable(DbColumn1* self, DbTable* btable)
{
	self->base.data.defValue = (btable ? -1 : 0);
	self->btable = btable;
}

double DbColumn1_get(const DbColumn1* self, const UBIG r)
{
	return DbColumn_getItemConst(&self->base, r)->flt;
}

void DbColumn1_set(DbColumn1* self, const UBIG r, double value)
{
	double* v = &DbColumn_getItem(&self->base, r)->flt;
	if (*v != value)
		DbColumn_setChange(&self->base, r);
	*v = value;
}

double DbColumn1_getLink(const DbColumn1* self, const UBIG r)
{
	double row = DbColumn_getItemConst(&self->base, r)->flt;
	return DbTable_isRowActive(self->btable, row) ? row : -1;
}
UBIG DbColumn1_sizeActive(const DbColumn1* self, UBIG r)
{
	if (self->btable)
		return DbColumn1_getLink(self, r) >= 0;
	return 1;
}
void DbColumn1_addLink(DbColumn1* self, const UBIG rSrc, const UBIG rDst)
{
	DbColumn1_set(self, rSrc, rDst);
}
void DbColumn1_removeLink(DbColumn1* self, const UBIG rSrc, const double rDst)
{
	double row = DbColumn1_getLink(self, rSrc);
	if (row == rDst)
		DbColumn1_set(self, rSrc, self->btable ? -1 : 0);
}

BIG DbColumn1_jump(DbColumn1* self, UBIG r, UBIG* pos, BIG jumps)
{
	return (*pos == 0 && jumps == 1) ? DbColumn1_getLink(self, r) : -1;
}

void DbColumn1_setFileId(DbColumn1* self, const UBIG r, FileRow value)
{
	FileRow* v = &DbColumn_getItem(&self->base, r)->fileId;
	if (!FileRow_cmp(*v, value))
		DbColumn_setChange(&self->base, r);
	*v = value;
}

FileRow DbColumn1_getFileId(const DbColumn1* self, const BIG r)
{
	return r >= 0 ? DbColumn_getItemConst(&self->base, r)->fileId : FileRow_initEmpty();
}

void DbColumn1_copyRow(DbColumn1* self, const UBIG rDst, DbColumn1* copy, const UBIG rSrc)
{
	TableItem* dst = DbColumn_getItem(&self->base, rDst);
	TableItem* src = DbColumn_getItem(&copy->base, rSrc);

	if (dst->flt != dst->flt)
		DbColumn_setChange(&self->base, rDst);

	*dst = *src;
}

DbTable* DbColumn1_getBTable(const DbColumn1* self)
{
	return self->btable;
}

BOOL DbColumn1_isValid(const DbColumn1* self, const UBIG r)
{
	return TableItems_isValid(&self->base.data, r);
}
void DbColumn1_invalidate(DbColumn1* self, const UBIG r)
{
	TableItems_invalidate(&self->base.data, r);
}

StdBIndex* DbColumn1_createIndex(const DbColumn1* self)
{
	StdBIndex* index = StdBIndex_new();

	const UBIG NUM_ROWS = DbColumn_numRows(&self->base);
	UBIG i;
	for (i = 0; i < NUM_ROWS; i++)
		if (DbColumn1_isValid(self, i))
			StdBIndex_add(index, FileRow_getBIG(DbColumn1_getFileId(self, i)), i);
	return index;
}

BIG DbColumn1_findRowPos(const DbColumn1* self, FileRow row)
{
	const UBIG NUM_ROWS = DbColumn_numRows(&self->base);
	UBIG i;
	for (i = 0; i < NUM_ROWS; i++)
		if (FileRow_cmp(DbColumn1_getFileId(self, i), row))
			return i;

	return -1;
}

void DbColumn1_deleteRowData(DbColumn1* self, const UBIG r)
{
	DbColumn1_set(self, r, 0);
	DbColumn_setChange(&self->base, r);
}

double DbColumn1_getLinkFileId(DbColumn1* self, const UBIG r)
{
	BIG row = DbColumn1_getLink(self, r);
	FileRow fr = DbColumn1_getFileId(DbTable_getColumnRows(self->btable), row);
	return *(double*)&fr;
}

void DbColumn1_convertToPos(DbColumn1* self)
{
	if (!self->isConverted)
	{
		StdBIndex* index = DbColumn1_createIndex(DbTable_getColumnRows(self->btable));

		const UBIG NUM_ROWS = DbColumn_numRows(&self->base);
		UBIG r;
		for (r = 0; r < NUM_ROWS; r++)
		{
			double link = DbColumn1_get(self, r);
			FileRow fr = *(FileRow*)&link;
			if (FileRow_is(fr))
				DbColumn1_set(self, r, StdBIndex_search(index, FileRow_getBIG(fr)));
		}
		self->isConverted = TRUE;

		StdBIndex_delete(index);
	}
}

void DbColumn1_maintenanceContent(DbColumn1* self)
{
	if (!self->btable)
		return;

	BIG* tempMoves = DbTable_getTempMoves(self->btable);
	const UBIG NUM_ROWS = DbColumn_numRows(&self->base);
	UBIG i;
	for (i = 0; i < NUM_ROWS; i++)
	{
		BIG row = DbColumn1_getLink(self, i);

		BIG move = tempMoves ? tempMoves[row] : 0;
		if (row >= 0 && move >= 0)	//link is valid && row is valid
		{
			if (move)
				row -= move;	//change link to moved record
		}
		else
			row = -1;

		DbColumn1_set(self, i, row);
	}
}

void DbColumn1_maintenanceMarks(DbColumn1* self)
{
	const UBIG NUM_ROWS = DbColumn_numRows(&self->base);
	UBIG act = 0;
	UBIG i;
	for (i = 0; i < NUM_ROWS; i++)
	{
		if (DbColumn1_isValid(self, i))
		{
			DbColumn1_set(self, act, DbColumn1_get(self, i));
			act++;
		}
	}
}

UBIG DbColumn1_bytes(DbColumn1* self)
{
	const UBIG NUM_ROWS = DbColumn_numRows(&self->base);
	return TableItems_bytes(&self->base.data, NUM_ROWS) + sizeof(DbColumn1);
}

BIG DbColumn1_search(const DbColumn1* self, double value)
{
	DbColumn1* active = DbColumn_getActive(&self->base);
	const UBIG NUM_ROWS = DbColumn_numRows(&self->base);

	UBIG i;
	for (i = 0; i < NUM_ROWS; i++)
	{
		if (DbColumn1_isValid(active, i))
		{
			if (DbColumn1_get(self, i) == value)
				return i;
		}
	}
	return -1;
}

double DbColumn1_max(DbColumn1* self)
{
	DbColumn1* active = DbColumn_getActive(&self->base);
	const UBIG NUM_ROWS = DbColumn_numRows(&self->base);
	double maxx = 0;

	if (NUM_ROWS > 0)
	{
		BOOL first = TRUE;
		UBIG i;
		for (i = 0; i < NUM_ROWS; i++)
		{
			if (DbColumn1_isValid(active, i))
			{
				if (first)
					maxx = DbColumn1_get(self, i), first = FALSE;
				else
					maxx = Std_dmax(DbColumn1_get(self, i), maxx);
			}
		}
	}
	return maxx;
}

BOOL DbColumn1_cmpRow(DbColumn1* a, DbColumn1* b, UBIG ai, UBIG bi)
{
	return DbColumn1_get(a, ai) == DbColumn1_get(b, bi);
}

BOOL DbColumn1_cmp(DbColumn1* a, DbColumn1* b)
{
	const UBIG NUM_ROWS = DbColumn_numRows(&a->base);
	return TableItems_cmp(&a->base.data, &b->base.data, NUM_ROWS);
}

void DbColumn1_toString(const DbColumn1* self, DbColumn* dst)
{
	volatile StdProgress* progress = DbRoot_getProgress();

	const UBIG NUM_ROWS = DbColumn_numRows(&self->base);
	UBIG i;
	for (i = 0; i < NUM_ROWS && progress->running; i++)
	{
		TableItems_get(&dst->data, i)->string = Std_newNumber(TableItems_getConst(&self->base.data, i)->flt);
		DbRoot_getProgress()->done = i / (float)NUM_ROWS;
	}
}

//slow, very slow!
void DbColumn1_shortFlt(const DbColumn1* self, StdBigs* poses, const BIG start, const BIG end, const BOOL ascending)
{
	volatile StdProgress* progress = DbRoot_getProgress();

	BIG i, j;
	for (i = start; i < end && progress->running; i++)
	{
		for (j = start; j < end && progress->running; j++)
			if (DbColumn1_get(self, poses->ptrs[j]) > DbColumn1_get(self, poses->ptrs[i]))
				StdBigs_swap(poses, i, j);

		progress->done = ((float)i - start) / (end - start);
	}
	if (!ascending && i == end && progress->running)
		StdBigs_reversEx(poses, start, end);
}

//Binary Insertion Sort
void DbColumn1_qshortFlt(const DbColumn1* self, StdBigs* poses, const BIG start, const BIG end, const BOOL ascending)
{
	volatile StdProgress* progress = DbRoot_getProgress();

	BIG i;
	for (i = start + 1; i < end && progress->running; i++)
	{
		BIG lo = start;
		BIG hi = i;
		BIG m = start + (i - start) / 2;

		do
		{
			if (DbColumn1_get(self, poses->ptrs[i]) > DbColumn1_get(self, poses->ptrs[m]))
				lo = m + 1;
			else if (DbColumn1_get(self, poses->ptrs[i]) < DbColumn1_get(self, poses->ptrs[m]))
				hi = m;
			else
				break;
			m = lo + ((hi - lo) / 2);
		} while (lo < hi);

		if (m < i)
			StdBigs_rotate(poses, m, i);

		progress->done = ((float)i - start) / (end - start);
	}

	if (!ascending && i == end && progress->running)
		StdBigs_reversEx(poses, start, end);
}

void DbColumn1_shortFile(const DbColumn1* self, StdBigs* poses, const BIG start, const BIG end, const BOOL ascending, BOOL shortSizes)
{
	volatile StdProgress* progress = DbRoot_getProgress();

	//slow, very slow!
	FileHead headI;
	FileHead headJ;
	BIG i, j;
	for (i = start; i < end && progress->running; i++)
	{
		headI = DbColumn_fileGetHead(&self->base, poses->ptrs[i], 0);
		for (j = start; j < end && progress->running; j++)
		{
			headJ = DbColumn_fileGetHead(&self->base, poses->ptrs[j], 0);

			if ((shortSizes && headJ.size > headI.size) || (!shortSizes && !Std_cmpCHARascending((char*)headJ.ext, (char*)headI.ext)))
				StdBigs_swap(poses, i, j);
		}
	}

	if (!ascending && i == end && progress->running)
		StdBigs_reversEx(poses, start, end);
}

//Binary Insertion Sort
void DbColumn1_qshortFile(const DbColumn1* self, StdBigs* poses, const BIG start, const BIG end, const BOOL ascending, BOOL shortSizes)
{
	volatile StdProgress* progress = DbRoot_getProgress();

	BIG i;
	for (i = start + 1; i < end && progress->running; i++)
	{
		BIG lo = start;
		BIG hi = i;
		BIG m = start + (i - start) / 2;

		do
		{
			FileHead headI = DbColumn_fileGetHead(&self->base, poses->ptrs[i], 0);
			FileHead headM = DbColumn_fileGetHead(&self->base, poses->ptrs[m], 0);

			if ((shortSizes && headI.size > headM.size) || (!shortSizes && !Std_cmpCHARascending((char*)headI.ext, (char*)headM.ext)))
				lo = m + 1;
			else if ((shortSizes && headI.size < headM.size) || (!shortSizes && Std_cmpCHARascending((char*)headI.ext, (char*)headM.ext)))
				hi = m;
			else
				break;
			m = lo + ((hi - lo) / 2);
		} while (lo < hi);

		if (m < i)
			StdBigs_rotate(poses, m, i);

		progress->done = ((float)i - start) / (end - start);
	}

	if (!ascending && i == end && progress->running)
		StdBigs_reversEx(poses, start, end);
}

void DbColumn1_short(const DbColumn1* self, StdBigs* poses, const BIG start, const BIG end, const BOOL ascending)
{
	if (DbColumnFormat_findColumn(&self->base) == DbFormat_FILE_1)
		DbColumn1_shortFile(self, poses, start, end, ascending, DbColumn_cmpOption(&self->base, "shortSizes", _UNI32("1")));
	else
		DbColumn1_shortFlt(self, poses, start, end, ascending);
}

void DbColumn1_qshort(const DbColumn1* self, StdBigs* poses, const BIG start, const BIG end, const BOOL ascending)
{
	if (DbColumnFormat_findColumn(&self->base) == DbFormat_FILE_1)
		DbColumn1_qshortFile(self, poses, start, end, ascending, DbColumn_cmpOption(&self->base, "shortSizes", _UNI32("1")));
	else
		DbColumn1_qshortFlt(self, poses, start, end, ascending);
}

/*BIG DbColumn1_search(DbColumn1* self, double value)
{
	const UBIG NUM_ROWS = DbColumn_numRows(&self->base);
	UBIG i;
	for (i = 0; i < NUM_ROWS; i++)
		if (DbColumn1_get(self, i) == value)
			return i;
	return -1;
}*/

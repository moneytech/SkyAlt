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

void DbColumn1_getArrayPoses(const DbColumn1* self, UBIG row, StdBigs* out)
{
	StdBigs_clear(out);

	BIG r = DbColumn1_getLink(self, row);
	if (r >= 0)
		StdBigs_add(out, r);
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

BIG DbColumn1_findRowScroll(const DbColumn1* self, BIG r)
{
	const UBIG NUM_ROWS = DbColumn_numRows(&self->base);
	UBIG pos = 0;
	UBIG i;
	for (i = 0; i < NUM_ROWS; i++)
	{
		FileRow row = DbColumn1_getFileId(self, i);
		if(FileRow_is(row))
		{
			if (FileRow_isRow(row, r))
				return pos;

			pos++;	//only for valid rows
		}
	}
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

double DbColumn1_min(DbColumn1* self)
{
	DbColumn1* active = DbColumn_getActive(&self->base);
	const UBIG NUM_ROWS = DbColumn_numRows(&self->base);
	double minV = 0;

	if (NUM_ROWS > 0)
	{
		BOOL first = TRUE;
		UBIG i;
		for (i = 0; i < NUM_ROWS; i++)
		{
			if (DbColumn1_isValid(active, i))
			{
				if (first)
					minV = DbColumn1_get(self, i), first = FALSE;
				else
					minV = Std_dmin(DbColumn1_get(self, i), minV);
			}
		}
	}
	return minV;
}
double DbColumn1_max(DbColumn1* self)
{
	DbColumn1* active = DbColumn_getActive(&self->base);
	const UBIG NUM_ROWS = DbColumn_numRows(&self->base);
	double maxV = 0;

	if (NUM_ROWS > 0)
	{
		BOOL first = TRUE;
		UBIG i;
		for (i = 0; i < NUM_ROWS; i++)
		{
			if (DbColumn1_isValid(active, i))
			{
				if (first)
					maxV = DbColumn1_get(self, i), first = FALSE;
				else
					maxV = Std_dmax(DbColumn1_get(self, i), maxV);
			}
		}
	}
	return maxV;
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
	const UBIG NUM_ROWS = DbColumn_numRows(&self->base);
	UBIG i;
	for (i = 0; i < NUM_ROWS && StdProgress_is(); i++)
	{
		TableItems_get(&dst->data, i)->string = Std_newNumber(TableItems_getConst(&self->base.data, i)->flt);
		StdProgress_setEx("SHORTING", i, NUM_ROWS);
	}
}

int _DbColumn1_cmpFlt(const void* context, const void* a, const void* b)
{
	const DbColumn1* self = context;

	double fa = DbColumn1_get(self, *(BIG*)a);
	double fb = DbColumn1_get(self, *(BIG*)b);
	return (fa > fb) - (fa < fb);
}

void DbColumn1_qshortFlt(const DbColumn1* self, StdBigs* poses, const BIG start, const BIG end, const BOOL ascending)
{
	Os_qsort(&poses->ptrs[start], end - start, sizeof(BIG), _DbColumn1_cmpFlt, (void*)self);

	if (!ascending && StdProgress_is())
		StdBigs_reversEx(poses, start, end);
}

int _DbColumn1_cmpFilesSizes(const void* context, const void* a, const void* b)
{
	const DbColumn* self = context;

	FileHead fa = DbColumn_fileGetHead(self, *(BIG*)a, 0);
	FileHead fb = DbColumn_fileGetHead(self, *(BIG*)b, 0);

	return (fa.size > fb.size) - (fa.size < fb.size);
}
int _DbColumn1_cmpFilesNoSizes(const void* context, const void* a, const void* b)
{
	const DbColumn* self = context;

	FileHead fa = DbColumn_fileGetHead(self, *(BIG*)a, 0);
	FileHead fb = DbColumn_fileGetHead(self, *(BIG*)b, 0);

	return Std_cmpCHARascending((char*)fa.ext, (char*)fb.ext);
}

void DbColumn1_qshortFile(const DbColumn1* self, StdBigs* poses, const BIG start, const BIG end, const BOOL ascending, BOOL shortSizes)
{
	if (shortSizes)
		Os_qsort(&poses->ptrs[start], end - start, sizeof(BIG), _DbColumn1_cmpFilesSizes, (void*)self);
	else
		Os_qsort(&poses->ptrs[start], end - start, sizeof(BIG), _DbColumn1_cmpFilesNoSizes, (void*)self);

	if (!ascending && StdProgress_is())
		StdBigs_reversEx(poses, start, end);
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

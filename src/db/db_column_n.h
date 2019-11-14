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

UBIG DbColumnN_array_size(const double* link)
{
	return link ? link[0] : 0;
}

UBIG DbColumnN_array_bytes(const double* link)
{
	const UBIG N = DbColumnN_array_size(link);
	return N ? (N + 1) * sizeof(BIG) : 0;
}

BIG DbColumnN_array_get(const double* link, UBIG i)
{
	return link[1 + i];
}

UBIG DbColumnN_array_sizeActive(const double* link, DbTable* btable)
{
	const UBIG N = DbColumnN_array_size(link);
	UBIG num = 0;
	UBIG i;
	for (i = 0; i < N; i++)
	{
		BIG it = DbColumnN_array_get(link, i);
		if (DbTable_isRowActive(btable, it))
			num++;
	}
	return num;
}

double* DbColumnN_array_copy(const double* src)
{
	double* ret = 0;
	const UBIG src_num = DbColumnN_array_size(src);
	if (src_num)
	{
		const UBIG bytes = (src_num + 1) * sizeof(double);
		ret = Os_malloc(bytes);
		Os_memcpy(ret, src, bytes);
	}
	return ret;
}
BIG DbColumnN_array_findID(const double* link, const double r)	//return index
{
	if (link)
	{
		const UBIG N = DbColumnN_array_size(link);
		link++;
		BIG i;
		for (i = 0; i < N; i++)
		{
			if (link[i] == r)
				return i;
		}
	}
	return -1;
}

BIG DbColumnN_array_findIDactive_pos(const double* link, const UBIG r, DbTable* btable)	//return pos(invalid are skiped)
{
	if (link)
	{
		BIG pos = 0;
		const UBIG N = DbColumnN_array_size(link);
		link++;
		UBIG i;
		for (i = 0; i < N; i++)
		{
			if (link[i] == r)
				return pos;

			if (DbTable_isRowActive(btable, link[i]))
				pos++;
		}
	}
	return -1;
}

BOOL DbColumnN_array_resize(double** link, const UBIG N)
{
	UBIG oldN = *link ? (*link)[0] : 0;

	if (N == oldN)
		return FALSE;

	double* l = 0;
	if (N)
	{
		l = Os_malloc((1 + N) * sizeof(double));
		if (oldN)
			Os_memcpy(l, *link, (1 + oldN) * sizeof(double));
		l[0] = N;

		//reset
		BIG i;
		for (i = oldN; i < N; i++)
			l[1 + i] = -1;
	}

	Os_free(*link, oldN * sizeof(double));
	*link = l;

	return TRUE;
}

BOOL DbColumnN_array_reversOrder(double* link)
{
	const UBIG N = DbColumnN_array_size(link);

	link++;
	UBIG i;
	for (i = 0; i < N / 2; i++)
	{
		double t = link[i];
		link[i] = link[N - i - 1];
		link[N - i - 1] = t;
	}

	return N > 1;
}

BOOL DbColumnN_array_addID(double** link, const double r)
{
	if (DbColumnN_array_findID(*link, r) < 0)
	{
		const UBIG N = DbColumnN_array_size(*link);
		DbColumnN_array_resize(link, N + 1);
		(*link)[1 + N] = r;
		return TRUE;
	}
	return FALSE;
}

BOOL DbColumnN_array_setID(double** link, const double r)
{
	BOOL changed = FALSE;
	if (!*link || (*link)[0] != 1)
		changed |= DbColumnN_array_resize(link, 1);

	changed |= ((*link)[1] != r);
	(*link)[1] = r;

	return changed;
}

BOOL DbColumnN_array_addAllTable(double** link, DbTable* table)
{
	DbColumnN_array_resize(link, DbTable_numRows(table));

	UBIG i = 0;
	UBIG r = 0;
	while (DbTable_jumpRows(table, &r, 1) >= 0)
	{
		(*link)[1 + i] = r;
		r++;
		i++;
	}
	return TRUE;
}

BOOL DbColumnN_array_replaceID(double** link, const double rOld, const double rNew)
{
	BIG i = DbColumnN_array_findID(*link, rOld);
	if (i >= 0)
	{
		(*link)[1 + i] = rNew;
		return TRUE;
	}
	return FALSE;
}

BOOL DbColumnN_array_swap(double* link, UBIG i, UBIG j)
{
	const UBIG N = DbColumnN_array_size(link);
	BOOL ok = (i >= 0 && j >= 0 && i < N && j < N);

	if (ok)
	{
		double t = link[1 + i];
		link[1 + i] = link[1 + j];
		link[1 + j] = t;
	}
	return ok;
}

BOOL DbColumnN_array_removeID(double** link, const double r, BOOL realRemove)
{
	BIG find = DbColumnN_array_findID(*link, r);
	if (find >= 0)
	{
		if (realRemove)
		{
			Os_memcpy(&(*link)[1 + find], &(*link)[1 + find + 1], ((*link)[0] - find - 1) * sizeof(double));
			(*link)[0]--;
		}
		else
			(*link)[1 + find] = -1;

		return TRUE;
	}
	return FALSE;
}

BOOL DbColumnN_array_insertID(double** link, const double r, UBIG i)
{
	//remove old one
	BIG rPos = DbColumnN_array_findID(*link, r);
	if (rPos >= 0)
		(*link)[1 + rPos] = -1;

	const UBIG N = DbColumnN_array_size(*link);
	if (i >= 0 && i <= N)
	{
		DbColumnN_array_resize(link, N + 1);

		Os_memmove(&(*link)[1 + i + 1], &(*link)[1 + i], (N - i) * sizeof(double));
		(*link)[1 + i] = r;
		return TRUE;
	}
	return FALSE;
}

BOOL DbColumnN_array_union(double** dst, const double* a, const double* b)
{
	BOOL changed = DbColumnN_array_resize(dst, 0);
	UBIG i;
	for (i = 0; i < DbColumnN_array_size(a); i++)
		changed &= DbColumnN_array_addID(dst, a[1 + i]);
	for (i = 0; i < DbColumnN_array_size(b); i++)
		changed &= DbColumnN_array_addID(dst, b[1 + i]);
	return changed;
}
BOOL DbColumnN_array_minus(double** dst, const double* a, const double* b)
{
	BOOL changed = DbColumnN_array_resize(dst, 0);
	UBIG i;
	for (i = 0; i < DbColumnN_array_size(a); i++)
	{
		if (!DbColumnN_array_findID(b, a[1 + i]))
			changed &= DbColumnN_array_addID(dst, a[1 + i]);
	}
	return changed;
}
BOOL DbColumnN_array_intersect(double** dst, const double* a, const double* b)
{
	BOOL changed = DbColumnN_array_resize(dst, 0);
	UBIG i;
	for (i = 0; i < DbColumnN_array_size(a); i++)
	{
		if (DbColumnN_array_findID(b, a[1 + i]))
			changed &= DbColumnN_array_addID(dst, a[1 + i]);
	}
	return changed;
}
BOOL DbColumnN_array_surounding(double** dst, const double* a, const double* b)
{
	BOOL changed = DbColumnN_array_resize(dst, 0);
	changed &= DbColumnN_array_minus(dst, a, b);
	changed &= DbColumnN_array_minus(dst, b, a);
	return changed;
}

BOOL DbColumnN_array_cmp_order(const double* a, const double* b)
{
	if (DbColumnN_array_size(a) != DbColumnN_array_size(b))
		return FALSE;

	UBIG i;
	for (i = 0; i < DbColumnN_array_size(a); i++)
	{
		if (a[1 + i] != b[1 + i])
			return FALSE;
	}

	return TRUE;
}

BOOL DbColumnN_array_cmpActive_order(const double* a, const double* b, DbTable* btable)
{
	UBIG aPos = 0;
	UBIG i;
	for (i = 0; i < DbColumnN_array_size(a); i++)
	{
		BIG r = a[1 + i];
		if (DbTable_isRowActive(btable, r))
		{
			if (DbColumnN_array_findIDactive_pos(b, r, btable) != aPos)
				return FALSE;
			aPos++;
		}
	}

	return aPos == DbColumnN_array_sizeActive(b, btable);
}

BOOL DbColumnN_array_cmp_no_order(const double* a, const double* b, DbTable* btable)
{
	UBIG pos = 0;
	UBIG i;
	for (i = 0; i < DbColumnN_array_size(a); i++)
	{
		BIG r = a[1 + i];
		if (DbTable_isRowActive(btable, r))
		{
			if (DbColumnN_array_findID(b, r) < 0)
				return FALSE;
			pos++;
		}
	}
	return pos == DbColumnN_array_sizeActive(b, btable);
}

void DbColumnN_array_maintenanceSmall(double* self, DbTable* btable)
{
	UBIG i;
	UBIG N = DbColumnN_array_size(self);
	double* links = &self[1];

	UBIG move = 0;
	for (i = 0; i < N; i++)
	{
		links[i - move] = links[i];

		if (DbTable_isRowActive(btable, links[i]))
			move++;
	}
}

void DbColumnN_array_maintenance(double** self, BIG* tempMoves, BOOL resize)
{
	UBIG i;
	UBIG N = DbColumnN_array_size(*self);
	double* links = &(*self)[1];

	for (i = 0; i < N; i++)
	{
		BIG move = 0;
		if (links[i] >= 0 && (!tempMoves || (move = tempMoves[(BIG)links[i]]) >= 0))	//link is valid && row is valid
		{
			links[i] -= move;	//change link to moved record
		}
		else
			if (resize)
			{
				Os_memmove(&links[i], &links[i + 1], (N - i - 1) * sizeof(UBIG));	//remove
				N--;	//decrease size
				i--;
			}
			else
				links[i] = -1;
	}

	DbColumnN_array_resize(self, N);	//realloc down
}

typedef struct DbColumnN_s
{
	DbColumn base;
	DbTable* btable;
	BOOL isConverted;
}DbColumnN;

DbColumnN* DbColumnN_new(DbColumns* parent, DbFormatTYPE format, DbTable* btable)
{
	DbColumnN* self = Os_malloc(sizeof(DbColumnN));
	self->base = DbColumn_init(DbColumn_N, format, parent, 0);
	self->btable = btable;
	self->isConverted = TRUE;
	return self;
}

void DbColumnN_clear(DbColumnN* self)
{
	const UBIG NUM_ROWS = DbColumn_numRows(&self->base);
	UBIG i;
	for (i = 0; i < NUM_ROWS; i++)
	{
		double* link = DbColumn_getItem(&self->base, i)->fltArr;
		Os_free(link, DbColumnN_array_size(link));
	}
	DbColumn_clearItems(&self->base);
}
void DbColumnN_delete(DbColumnN* self)
{
	DbColumnN_clear(self);
	DbColumn_free((DbColumn*)self);
	Os_free(self, sizeof(DbColumnN));
}

double* DbColumnN_getLinks(const DbColumnN* self, const UBIG r)
{
	return DbColumn_getItemConst(&self->base, r)->fltArr;
}

DbTable* DbColumnN_getBTable(const DbColumnN* self)
{
	return self->btable;
}

void DbColumnN_setBTable(DbColumnN* self, DbTable* btable)
{
	self->btable = btable;
	self->base.data.defValue = 0;
}

UBIG DbColumnN_sizeHard(const DbColumnN* self, const UBIG r)
{
	return DbColumnN_array_size(DbColumn_getItemConst(&self->base, r)->fltArr);
}
UBIG DbColumnN_sizeActive(const DbColumnN* self, UBIG r)
{
	return self->btable ? DbColumnN_array_sizeActive(DbColumn_getItemConst(&self->base, r)->fltArr, self->btable) : DbColumnN_sizeHard(self, r);
}

double* DbColumnN_getFirst(const DbColumnN* self, UBIG r)
{
	const TableItem* item = DbColumn_getItemConst(&self->base, r);
	return item->fltArr ? &item->fltArr[1] : 0;
}

double* DbColumnN_getArray(DbColumnN* self, UBIG r)
{
	return DbColumn_getItem(&self->base, r)->fltArr;
}

BIG DbColumnN_jump(const DbColumnN* self, UBIG r, UBIG* pos, BIG jumps)
{
	return DbTable_jumpLinks(self->btable, DbColumnN_getLinks(self, r), pos, jumps);
}

BIG DbColumnN_getFirstRow(const DbColumnN* self, UBIG r)
{
	return DbTable_jumpLinksFirst(self->btable, DbColumnN_getLinks(self, r));
}

void DbColumnN_resize(DbColumnN* self, const UBIG rSrc, const UBIG N)
{
	if (DbColumnN_array_resize(&DbColumn_getItem(&self->base, rSrc)->fltArr, N))
		DbColumn_setChange(&self->base, rSrc);
}

double DbColumnN_getIndex(DbColumnN* self, UBIG r, UBIG i)
{
	if (self->btable)
	{
		//active
		UBIG pos = 0;
		return DbColumnN_jump(self, r, &pos, i + 1);
	}
	else
	{
		const UBIG N = DbColumnN_sizeHard(self, r);
		double* ids = DbColumnN_getFirst(self, r);
		return (i < N) ? ids[i] : -1;
	}
}

void DbColumnN_setIndex(DbColumnN* self, UBIG r, UBIG index, double value)
{
	const UBIG N = DbColumnN_sizeHard(self, r);

	if (index >= N)
		DbColumnN_resize(self, r, index + 1);

	double* ids = DbColumnN_getFirst(self, r);
	if (ids[index] != value)
		DbColumn_setChange(&self->base, r);
	ids[index] = value;
}

void DbColumnN_setFileId(DbColumnN* self, const UBIG r, UBIG index, FileRow value)
{
	DbColumnN_setIndex(self, r, index, FileRow_getDouble(value));
}

FileRow DbColumnN_getFileId(const DbColumnN* self, const UBIG r, UBIG i)
{
	const UBIG N = DbColumnN_sizeHard(self, r);
	FileRow* ids = (FileRow*)DbColumnN_getFirst(self, r);
	return (i < N) ? ids[i] : FileRow_initEmpty();
}

StdBigs DbColumnN_copyActiveBigs(DbColumnN* self, UBIG r)
{
	StdBigs arr = StdBigs_init();

	UBIG i = 0;
	while (DbColumnN_jump(self, r, &i, 1) >= 0)
	{
		StdBigs_add(&arr, DbColumnN_getIndex(self, r, i));
		i++;
	}

	return arr;
}

double* DbColumnN_exportRowArray(DbColumnN* self, UBIG r, double* out, BIG* out_maxBytes)
{
	UBIG N = DbColumnN_sizeHard(self, r);
	UBIG bytes = (1 + N) * sizeof(BIG);
	if (bytes > * out_maxBytes)
	{
		out = Os_realloc(out, bytes);
		*out_maxBytes = bytes;
	}

	UBIG num = 1;
	double* links = DbColumnN_getFirst(self, r);
	UBIG i;
	for (i = 0; i < N; i++)
		if (links[i] >= 0)
		{
			FileRow fr = DbColumn1_getFileId(DbTable_getColumnRows(self->btable), links[i]);
			out[num++] = *(double*)&fr;
		}

	if (*out_maxBytes)
		out[0] = num - 1;

	return out;
}
void DbColumnN_convertToPos(DbColumnN* self)
{
	if (!self->isConverted)
	{
		StdBIndex* index = DbColumn1_createIndex(DbTable_getColumnRows(self->btable));

		const UBIG NUM_ROWS = DbColumn_numRows(&self->base);
		UBIG r;
		for (r = 0; r < NUM_ROWS; r++)
		{
			UBIG N = DbColumnN_sizeHard(self, r);
			double* links = DbColumnN_getFirst(self, r);

			UBIG i;
			for (i = 0; i < N; i++)
			{
				FileRow fr = *(FileRow*)&links[i];
				if (FileRow_is(fr))
					links[i] = StdBIndex_search(index, FileRow_getBIG(fr));
			}
		}
		self->isConverted = TRUE;

		StdBIndex_delete(index);
	}
}

void DbColumnN_reversOrder(DbColumnN* self, const UBIG rSrc)
{
	if (DbColumnN_array_reversOrder(DbColumn_getItem(&self->base, rSrc)->fltArr))
		DbColumn_setChange(&self->base, rSrc);
}

void DbColumnN_clearRow(DbColumnN* self, const UBIG rSrc)
{
	if (DbColumnN_array_resize(&DbColumn_getItem(&self->base, rSrc)->fltArr, 0))
		DbColumn_setChange(&self->base, rSrc);
}

void DbColumnN_deleteRowData(DbColumnN* self, const UBIG rSrc)
{
	DbColumnN_array_resize(&DbColumn_getItem(&self->base, rSrc)->fltArr, 0);
	DbColumn_setChange(&self->base, rSrc);
}

void DbColumnN_setArray(DbColumnN* self, const UBIG r, double* bigArr)
{
	DbColumnN_clearRow(self, r);
	DbColumn_getItem(&self->base, r)->fltArr = bigArr;
	DbColumn_setChange(&self->base, r);
}

void DbColumnN_maintenanceContent(DbColumnN* self)
{
	if (!self->btable)
		return;

	const UBIG NUM_ROWS = DbColumn_numRows(&self->base);
	UBIG i;
	for (i = 0; i < NUM_ROWS; i++)
		DbColumnN_array_maintenance(&DbColumn_getItem(&self->base, i)->fltArr, DbTable_getTempMoves(self->btable), TRUE);
}

void DbColumnN_maintenance(DbColumnN* self)
{
	DbColumn1* active = DbColumn_getActive(&self->base);
	const UBIG NUM_ROWS = DbColumn_numRows(&self->base);

	UBIG i;
	for (i = 0; i < NUM_ROWS; i++)
	{
		if (!DbColumn1_isValid(active, i))
			DbColumnN_clearRow(self, i);	//removes all link
	}
}

BIG DbColumnN_findBig(const DbColumnN* self, const UBIG r, const double ID)
{
	return DbColumnN_array_findID(DbColumn_getItemConst(&self->base, r)->fltArr, ID);
}

void DbColumnN_copyRow(DbColumnN* self, const UBIG rDst, DbColumnN* copy, const UBIG rCopy)
{
	TableItem* dst = DbColumn_getItem(&self->base, rDst);
	TableItem* src = DbColumn_getItem(&copy->base, rCopy);

	DbColumnN_array_resize(&dst->fltArr, 0);
	dst->fltArr = DbColumnN_array_copy(src->fltArr);

	DbColumn_setChange(&self->base, rDst);
}

void DbColumnN_setOne(DbColumnN* self, const UBIG rSrc, const UBIG rAdd)
{
	TableItem* src = DbColumn_getItem(&self->base, rSrc);

	DbColumnN_array_resize(&src->fltArr, 0);
	DbColumnN_array_addID(&src->fltArr, rAdd);

	DbColumn_setChange(&self->base, rSrc);
}

void DbColumnN_add(DbColumnN* self, const UBIG rSrc, const UBIG rDst)
{
	if (DbColumnN_array_addID(&DbColumn_getItem(&self->base, rSrc)->fltArr, rDst))
		DbColumn_setChange(&self->base, rSrc);
}

void DbColumnN_set(DbColumnN* self, const UBIG rSrc, const BIG rDst)
{
	if (DbColumnN_array_setID(&DbColumn_getItem(&self->base, rSrc)->fltArr, rDst))
		DbColumn_setChange(&self->base, rSrc);
}

void DbColumnN_addAllTable(DbColumnN* self, const UBIG rSrc)
{
	if (DbColumnN_array_addAllTable(&DbColumn_getItem(&self->base, rSrc)->fltArr, self->btable))
		DbColumn_setChange(&self->base, rSrc);
}

void DbColumnN_insert(DbColumnN* self, const UBIG rSrc, const UBIG rDst, UBIG i)
{
	if (DbColumnN_array_insertID(&DbColumn_getItem(&self->base, rSrc)->fltArr, rDst, i))
		DbColumn_setChange(&self->base, rSrc);
}
void DbColumnN_insert_before(DbColumnN* self, const UBIG rSrc, const UBIG rDst, const UBIG rDstAfter)
{
	DbColumnN_insert(self, rSrc, rDst, DbColumnN_findBig(self, rSrc, rDstAfter));
}
void DbColumnN_insert_after(DbColumnN* self, const UBIG rSrc, const UBIG rDst, const UBIG rDstAfter)
{
	DbColumnN_insert(self, rSrc, rDst, DbColumnN_findBig(self, rSrc, rDstAfter) + 1);
}

void DbColumnN_replace(DbColumnN* self, const UBIG rSrc, const UBIG rOld, const UBIG rNew)
{
	if (DbColumnN_array_replaceID(&DbColumn_getItem(&self->base, rSrc)->fltArr, rOld, rNew))
		DbColumn_setChange(&self->base, rSrc);
}

void DbColumnN_remove(DbColumnN* self, const UBIG rSrc, const double rDst)
{
	if (DbColumnN_array_removeID(&DbColumn_getItem(&self->base, rSrc)->fltArr, rDst, self->btable == 0))
		DbColumn_setChange(&self->base, rSrc);
}

BOOL DbColumnN_swap(DbColumnN* self, const UBIG rSrc, BIG i, BIG j)
{
	BOOL changed = DbColumnN_array_swap(DbColumn_getItem(&self->base, rSrc)->fltArr, i, j);
	if (changed)
		DbColumn_setChange(&self->base, rSrc);
	return changed;
}

void DbColumnN_union(DbColumnN* self, const UBIG rSrc, DbColumnN* a, const UBIG rA, DbColumnN* b, const UBIG rB)
{
	if (DbColumnN_array_union(&DbColumn_getItem(&self->base, rSrc)->fltArr, DbColumn_getItem(&a->base, rA)->fltArr, DbColumn_getItem(&b->base, rB)->fltArr))
		DbColumn_setChange(&self->base, rSrc);
}
void DbColumnN_minus(DbColumnN* self, const UBIG rSrc, DbColumnN* a, const UBIG rA, DbColumnN* b, const UBIG rB)
{
	if (DbColumnN_array_minus(&DbColumn_getItem(&self->base, rSrc)->fltArr, DbColumn_getItem(&a->base, rA)->fltArr, DbColumn_getItem(&b->base, rB)->fltArr))
		DbColumn_setChange(&self->base, rSrc);
}
void DbColumnN_intersect(DbColumnN* self, const UBIG rSrc, DbColumnN* a, const UBIG rA, DbColumnN* b, const UBIG rB)
{
	if (DbColumnN_array_intersect(&DbColumn_getItem(&self->base, rSrc)->fltArr, DbColumn_getItem(&a->base, rA)->fltArr, DbColumn_getItem(&b->base, rB)->fltArr))
		DbColumn_setChange(&self->base, rSrc);
}
void DbColumnN_surounding(DbColumnN* self, const UBIG rSrc, DbColumnN* a, const UBIG rA, DbColumnN* b, const UBIG rB)
{
	if (DbColumnN_array_surounding(&DbColumn_getItem(&self->base, rSrc)->fltArr, DbColumn_getItem(&a->base, rA)->fltArr, DbColumn_getItem(&b->base, rB)->fltArr))
		DbColumn_setChange(&self->base, rSrc);
}

BIG DbColumnN_findBigPos(DbColumnN* self, const UBIG rSrc, const UBIG rDst)
{
	return DbColumnN_array_findIDactive_pos(DbColumn_getItem(&self->base, rSrc)->fltArr, rDst, self->btable);
}

void DbColumnN_moveBigActive(DbColumnN* self, const UBIG rSrc, const UBIG rDst, BIG move)
{
	double* link = DbColumn_getItem(&self->base, rSrc)->fltArr;

	DbColumnN_array_maintenanceSmall(link, self->btable);

	BIG pos = DbColumnN_array_findIDactive_pos(link, rDst, self->btable);

	const int one = (move > 0) ? 1 : -1;
	move = Std_abs(move);

	if (move)
		DbColumn_setChange(&self->base, rSrc);

	while (move > 0)
	{
		DbColumnN_array_swap(link, pos, pos + one);
		pos += one;
		move--;
	}
}

UBIG DbColumnN_bytes(DbColumnN* self)
{
	const UBIG NUM_ROWS = DbColumn_numRows(&self->base);
	UBIG sum = 0;
	UBIG i;
	for (i = 0; i < NUM_ROWS; i++)
		sum += DbColumnN_array_bytes(DbColumn_getItem(&self->base, i)->fltArr);

	return sum + TableItems_bytes(&self->base.data, NUM_ROWS) + sizeof(DbColumnN);
}

DbColumn* DbColumnN_firstBDbColumn(DbColumnN* self)
{
	DbColumns* columns = DbTable_getColumns(self->btable);
	return DbColumns_num(columns) ? DbColumns_get(columns, 0) : 0;
}

BOOL DbColumnN_cmpBigRow(DbColumnN* a, DbColumnN* b, UBIG rA, UBIG rB)
{
	if (a->btable)	//active
		return DbColumnN_array_cmpActive_order(DbColumn_getItem(&a->base, rA)->fltArr, DbColumn_getItem(&b->base, rB)->fltArr, a->btable);
	else
		return DbColumnN_array_cmp_order(DbColumn_getItem(&a->base, rA)->fltArr, DbColumn_getItem(&b->base, rB)->fltArr);
}

BOOL DbColumnN_cmpBigActive(DbColumnN* a, DbColumnN* b)
{
	const UBIG NUM_ROWS = DbColumn_numRows(&a->base);
	UBIG i;
	for (i = 0; i < NUM_ROWS; i++)
		if (!DbColumnN_array_cmpActive_order(DbColumn_getItem(&a->base, i)->fltArr, DbColumn_getItem(&b->base, i)->fltArr, a->btable))
			return FALSE;
	return TRUE;
}

void DbColumnN_short(const DbColumnN* self, StdBigs* poses, const BIG start, const BIG end, const BOOL ascending)
{
	volatile StdProgress* progress = DbRoot_getProgress();

	if (self->btable)	//active
	{
		//slow, very slow!
		BIG i, j;
		for (i = start; i < end && progress->running; i++)
			for (j = start; j < end && progress->running; j++)
				if (DbColumnN_sizeActive(self, poses->ptrs[j]) > DbColumnN_sizeActive(self, poses->ptrs[i]))	//compares lenghts
					StdBigs_swap(poses, i, j);

		if (!ascending && i == end && progress->running)
			StdBigs_reversEx(poses, start, end);
	}
	else
	{
		//slow, very slow!
		BIG i, j;
		for (i = start; i < end && progress->running; i++)
			for (j = start; j < end && progress->running; j++)
				if (DbColumnN_sizeHard(self, poses->ptrs[j]) > DbColumnN_sizeHard(self, poses->ptrs[i]))	//compares lenghts
					StdBigs_swap(poses, i, j);

		if (!ascending && i == end && progress->running)
			StdBigs_reversEx(poses, start, end);
	}
}

void DbColumnN_qshort(const DbColumnN* self, StdBigs* poses, const BIG start, const BIG end, const BOOL ascending)
{
	volatile StdProgress* progress = DbRoot_getProgress();

	BIG i;
	if (self->btable)	//active
	{
		for (i = start + 1; i < end && progress->running; i++)
		{
			BIG lo = start;
			BIG hi = i;
			BIG m = start + (i - start) / 2;

			do
			{
				if (DbColumnN_sizeActive(self, poses->ptrs[i]) > DbColumnN_sizeActive(self, poses->ptrs[m]))
					lo = m + 1;
				else if (DbColumnN_sizeActive(self, poses->ptrs[i]) < DbColumnN_sizeActive(self, poses->ptrs[m]))
					hi = m;
				else
					break;
				m = lo + ((hi - lo) / 2);
			} while (lo < hi);

			if (m < i)
				StdBigs_rotate(poses, m, i);

			progress->done = ((float)i - start) / (end - start);
		}
	}
	else
	{
		for (i = start + 1; i < end && progress->running; i++)
		{
			BIG lo = 0;
			BIG hi = i;
			BIG m = i / 2;

			do
			{
				if (DbColumnN_sizeHard(self, poses->ptrs[i]) > DbColumnN_sizeHard(self, poses->ptrs[m]))
					lo = m + 1;
				else if (DbColumnN_sizeHard(self, poses->ptrs[i]) < DbColumnN_sizeHard(self, poses->ptrs[m]))
					hi = m;
				else
					break;
				m = lo + ((hi - lo) / 2);
			} while (lo < hi);

			if (m < i)
				StdBigs_rotate(poses, m, i);

			progress->done = ((float)i - start) / (end - start);
		}
	}

	if (!ascending && i == end && progress->running)
		StdBigs_reversEx(poses, start, end);
}




BIG DbColumnN_search(const DbColumnN* self, double value)
{
	DbColumn1* active = DbColumn_getActive(&self->base);
	const UBIG NUM_ROWS = DbColumn_numRows(&self->base);

	UBIG i;
	for (i = 0; i < NUM_ROWS; i++)
	{
		if (DbColumn1_isValid(active, i))
		{
			if (DbColumnN_findBig(self, i, value) >= 0)
				return i;
		}
	}
	return -1;
}






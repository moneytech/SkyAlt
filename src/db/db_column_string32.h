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

typedef struct DbColumnString32_s
{
	DbColumn base;
} DbColumnString32;

void DbColumnString32_setEq(DbColumnString32* self, const UBIG r, UNI* str);
const UNI* DbColumnString32_get(const DbColumnString32* self, const UBIG r);

DbColumnString32* DbColumnString32_new(DbColumns* parent, DbFormatTYPE format)
{
	DbColumnString32* self = Os_malloc(sizeof(DbColumnString32));
	self->base = DbColumn_init(DbColumn_STRING_32, format, parent, 0);
	//self->def = 0;
	return self;
}
void DbColumnString32_clear(DbColumnString32* self)
{
	const UBIG NUM_ROWS = DbColumn_numRows(&self->base);
	UBIG i;
	for (i = 0; i < NUM_ROWS; i++)
		Std_deleteUNI(DbColumn_getItemConst(&self->base, i)->string); //free string

	DbColumn_clearItems(&self->base);
}

void DbColumnString32_delete(DbColumnString32* self)
{
	DbColumnString32_clear(self);

	DbColumn_free((DbColumn*)self);
	Os_free(self, sizeof(DbColumnString32));
}

const UNI* DbColumnString32_get(const DbColumnString32* self, const UBIG r)
{
	return DbColumn_getItemConst(&self->base, r)->string;
}

void DbColumnString32_setEq(DbColumnString32* self, const UBIG r, UNI* value)
{
	UNI** v = &DbColumn_getItem(&self->base, r)->string;

	//Std_printlnUNI(*v);
	//Std_printlnUNI(value);

	if (!Std_cmpUNI(*v, value))
		DbColumn_setChange(&self->base, r);
	*v = value;
}

void DbColumnString32_setEqFree(DbColumnString32* self, const UBIG r, UNI* value)
{
	UNI* orig = DbColumn_getItem(&self->base, r)->string;
	DbColumnString32_setEq(self, r, value);

	Std_deleteUNI(orig);
}

void DbColumnString32_setCopy(DbColumnString32* self, const UBIG r, const UNI* value)
{
	UNI* orig = DbColumn_getItem(&self->base, r)->string;
	if (!Std_cmpUNI(orig, value))
	{
		const UBIG NB = Std_bytesUNI(value);
		if (NB && orig && Std_bytesUNI(orig) >= NB)
		{
			DbColumn_setChange(&self->base, r);
			Os_memcpy(orig, value, NB);
		}
		else
			DbColumnString32_setEqFree(self, r, Std_newUNI(value));
	}
}

void DbColumnString32_copyRow(DbColumnString32* self, const UBIG rDst, DbColumnString32* copy, const UBIG rSrc)
{
	DbColumnString32_setCopy(self, rDst, DbColumnString32_get(copy, rSrc));
}

void DbColumnString32_deleteRowData(DbColumnString32* self, const UBIG r)
{
	Std_deleteUNI(DbColumn_getItem(&self->base, r)->string);
	DbColumn_getItem(&self->base, r)->string = 0;
	DbColumn_setChange(&self->base, r);
}

void DbColumnString32_maintenance(DbColumnString32* self)
{
	const UBIG NUM_ROWS = DbColumn_numRows(&self->base);
	DbColumn1* active = DbColumn_getActive(&self->base);

	UBIG i;
	for (i = 0; i < NUM_ROWS; i++)
		if (!DbColumn1_isValid(active, i))
			Std_deleteUNI(DbColumn_getItemConst(&self->base, i)->string);
}

UBIG DbColumnString32_bytes(DbColumnString32* self)
{
	const UBIG NUM_ROWS = DbColumn_numRows(&self->base);
	UBIG sum = 0;
	UBIG i;
	for (i = 0; i < NUM_ROWS; i++)
		sum += Std_sizeUNI(DbColumnString32_get(self, i)) * sizeof(UNI);
	return sum + TableItems_bytes(&self->base.data, NUM_ROWS) + sizeof(DbColumnString32);
}

BOOL DbColumnString32_cmpRow(DbColumnString32* a, DbColumnString32* b, UBIG ai, UBIG bi)
{
	return Std_cmpUNI(DbColumnString32_get(a, ai), DbColumnString32_get(b, bi));
}

BOOL DbColumnString32_cmp(DbColumnString32* a, DbColumnString32* b)
{
	const UBIG NUM_ROWS = DbColumn_numRows(&a->base);
	UBIG i;
	for (i = 0; i < NUM_ROWS; i++)
	{
		if (!Std_cmpUNI(DbColumnString32_get(a, i), DbColumnString32_get(b, i)))
			return FALSE;
	}
	return TRUE;
}

double DbColumnString32_getNumber(const DbColumnString32* self, UBIG r)
{
	return Std_getNumberFromUNI(DbColumnString32_get(self, r));
}

void DbColumnString32_toNumber(const DbColumnString32* self, DbColumn* dst)
{
	const UBIG NUM_ROWS = DbColumn_numRows(&self->base);
	UBIG i;
	for (i = 0; i < NUM_ROWS && StdProgress_is(); i++)
	{
		TableItems_get(&dst->data, i)->flt = Std_getNumberFromUNI(TableItems_getConst(&self->base.data, i)->string);
		StdProgress_setEx("CONVERTING", i, NUM_ROWS);
	}
}

int _DbColumnString32_cmp(const void* context, const void* a, const void* b)
{
	const DbColumnString32* self = context;

	const UNI* fa = DbColumnString32_get(self, *(BIG*)a);
	const UNI* fb = DbColumnString32_get(self, *(BIG*)b);

	return Std_cmpUNIascending(fa, fb);
}

void DbColumnString32_qshort(const DbColumnString32* self, StdBigs* poses, const BIG start, const BIG end, const BOOL ascending)
{
	Os_qsort(&poses->ptrs[start], end - start, sizeof(BIG), _DbColumnString32_cmp, (void*)self);

	if (!ascending && StdProgress_is())
		StdBigs_reversEx(poses, start, end);
}

BIG DbColumnString32_searchString(DbColumnString32* self, const UNI* value)
{
	const UBIG NUM_ROWS = DbColumn_numRows(&self->base);
	UBIG i;
	for (i = 0; i < NUM_ROWS; i++)
		if (Std_cmpUNI(DbColumnString32_get(self, i), value))
			return i;
	return -1;
}

BIG DbColumnString32_searchNumber(DbColumnString32* self, double value)
{
	const UBIG NUM_ROWS = DbColumn_numRows(&self->base);
	UBIG i;
	for (i = 0; i < NUM_ROWS; i++)
		if (DbColumnString32_getNumber(self, i) == value)
			return i;
	return -1;
}

#define DbColumnString32_DIV_START 1000000	//previous was ':'
#define DbColumnString32_DIV_END 1000001	//previous was ';'

static BOOL _DbColumnString32_getOptionPosition(const UNI* orig, const char* name, UBIG maxOutSize, BIG* startName, BIG* startValue, BIG* endValue)
{
	const BIG N = Std_sizeUNI(orig);
	const BIG Nname = Std_sizeCHAR(name);

	BIG pos = 0;
	while (pos >= 0)
	{
		BIG p = Std_subUNI_small_char(&orig[pos], name);
		if (p >= 0)
		{
			pos += p;
			*startName = pos;
			pos += Nname;
			if (pos < N && orig[pos] == DbColumnString32_DIV_START)
			{
				pos++;	//jump over div

				BIG start = pos;
				BIG end = Std_findUNI(&orig[start], DbColumnString32_DIV_END);
				if (end < 0)
					end = N;
				else
					end += start;
				if (end - start > maxOutSize)
					end = start + maxOutSize;

				*startValue = start;
				*endValue = end;
				break;
			}
		}
		else
			pos = -1;
	}

	return pos >= 0;
}

static BOOL _DbColumnString32_getOptionValue(const UNI* orig, const char* name, UNI* out, UBIG maxOutSize)
{
	*out = 0;

	BIG startName;
	BIG startValue;
	BIG endValue;
	if (_DbColumnString32_getOptionPosition(orig, name, maxOutSize, &startName, &startValue, &endValue))
	{
		//copy
		BIG i;
		for (i = startValue; i < endValue; i++)
			out[i - startValue] = orig[i];
		out[endValue - startValue] = 0;
		return TRUE;
	}

	return FALSE;
}

UNI* _DbColumnString32_addOptionValue(const UNI* orig, const char* name, const UNI* value)
{
	UNI* ret = Std_newUNI(orig);

	if (Std_findUNI(value, DbColumnString32_DIV_START) >= 0 || Std_findUNI(value, DbColumnString32_DIV_END) >= 0)
	{
		//printf("Warning: Option ignore because Value contains <div>\n");
		return ret;
	}

	BIG startName;
	BIG startValue;
	BIG endValue;
	if (_DbColumnString32_getOptionPosition(orig, name, 1000000, &startName, &startValue, &endValue))
	{
		const UBIG Ndst = Std_sizeUNI(value);
		BIG Nsrc = endValue - startValue;

		//create space
		while (Nsrc < Ndst)
		{
			ret = Std_insertUNI(ret, _UNI32(" "), startValue);
			Nsrc++;
		}
		while (Nsrc > Ndst)
		{
			ret = Std_removeUNI(ret, startValue);
			Nsrc--;
		}
		//replace
		BIG i;
		for (i = 0; i < Nsrc; i++)
			ret[startValue + i] = value[i];
	}
	else
	{
		UNI st[] = { DbColumnString32_DIV_START, 0 };
		UNI en[] = { DbColumnString32_DIV_END, 0 };

		//add new
		//if (Std_sizeUNI(ret) && ret[Std_sizeUNI(ret) - 1] != DbColumnString32_DIV_END)
		//	ret = Std_addAfterUNI(ret, en);

		ret = Std_addAfterUNI_char(ret, name);
		ret = Std_addAfterUNI(ret, st);
		ret = Std_addAfterUNI(ret, value);
		ret = Std_addAfterUNI(ret, en);
	}

	return ret;
}

UNI* _DbColumnString32_addOptionValueNumber(const UNI* orig, const char* name, const double value)
{
	UNI str[64];
	Std_buildNumberUNI(value, -1, str);
	return _DbColumnString32_addOptionValue(orig, name, str);
}

void DbColumnString32_setOption(DbColumnString32* self, BIG row, const char* name, const UNI* value)
{
	if (row < 0)
		return;

	const UNI* orig = DbColumnString32_get(self, row);
	UNI* fnl = _DbColumnString32_addOptionValue(orig, name, value);
	DbColumnString32_setEqFree(self, row, fnl);
}

void DbColumnString32_getOption(DbColumnString32* self, BIG row, const char* name, const UNI* defValue, UNI* out, UBIG outMaxSize)
{
	if (outMaxSize)
		*out = 0;

	if (row < 0)
		return;

	const UNI* orig = DbColumnString32_get(self, row);
	if (!_DbColumnString32_getOptionValue(orig, name, out, outMaxSize - 1))
	{
		if (defValue)
		{
			//Std_printlnUNI(orig);

			DbColumnString32_setOption(self, row, name, defValue);	//add default one
			Std_copyUNI(out, outMaxSize, defValue);	//copy def to out
		}
	}
}

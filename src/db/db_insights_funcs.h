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

typedef void DbInsightTempCallback(DbInsightItem* self, DbInsight* orig, BIG row);

void DbInsightItem_executeLink(DbInsightItem* self, DbInsightTempCallback* fn, DbInsight* orig, BIG row)
{
	if (self->next)
	{
		DbColumn* link = self->column;
		if (link->type == DbColumn_1)
		{
			BIG it = DbColumn1_getLink((DbColumn1*)link, row);
			if (it >= 0)
				DbInsightItem_executeLink(self->next, fn, orig, it);
		}
		else
			if (link->type == DbColumn_N)
			{
				UBIG i = 0;
				BIG it;
				while ((it = DbColumnN_jump((DbColumnN*)link, row, &i, 1)) >= 0)
				{
					DbInsightItem_executeLink(self->next, fn, orig, it);
					i++;
				}
			}
	}
	else
		fn(self, orig, row);
}

void DbInsightItem_execute(DbInsight* self, DbInsightTempCallback* fn, BIG row)
{
	if (row >= 0)
		DbInsightItem_executeLink(self->items, fn, self, row);
	else
	{
		BIG i;
		if (self->srcFilter)
		{
			const StdBigs* rows = DbFilter_getRows(self->srcFilter);
			for (i = 0; i < rows->num; i++)
				DbInsightItem_executeLink(self->items, fn, self, rows->ptrs[i]);
		}
		else
		{
			const UBIG NUM_ROWS = DbTable_numRows(self->srcTable);
			BIG i;
			for (i = 0; i < NUM_ROWS; i++)
			{
				if (DbTable_isRowActive(self->srcTable, i))
					DbInsightItem_executeLink(self->items, fn, self, i);
			}
		}
	}
}

void DbInsight_prepareResult(DbInsight* self)
{
	self->temp_sum = 0;
	self->temp_num = 0;
	self->temp_mean = 0;
	self->temp_min = Std_maxDouble();
	self->temp_max = Std_minDouble();

	self->result_number = 0;
	StdString_empty(&self->result_string);

	self->arr = StdBigs_init();
}
void DbInsight_saveNumber(DbInsight* self, BIG row)
{
	if (self->result_column)
		DbColumn_setFlt(self->result_column, row, 0, self->result_number);
	else
		StdString_addNumber(&self->result_string, self->precision, self->result_number);
}

void DbInsightTemp_sum(DbInsightItem* self, DbInsight* orig, BIG row)
{
	orig->temp_sum += DbColumn_getFlt(self->column, row, 0);
}
void DbInsightFunc_sum(DbInsight* self, BIG row)
{
	DbInsight_prepareResult(self);

	DbInsightItem_execute(self, &DbInsightTemp_sum, row);

	self->result_number = self->temp_sum;
	DbInsight_saveNumber(self, row);
}

void DbInsightTemp_avg(DbInsightItem* self, DbInsight* orig, BIG row)
{
	orig->temp_sum += DbColumn_getFlt(self->column, row, 0);
	orig->temp_num++;
}
void DbInsightFunc_avg(DbInsight* self, BIG row)
{
	DbInsight_prepareResult(self);

	DbInsightItem_execute(self, &DbInsightTemp_avg, row);

	self->result_number = self->temp_sum / self->temp_num;
	DbInsight_saveNumber(self, row);
}

void DbInsightTemp_count(DbInsightItem* self, DbInsight* orig, BIG row)
{
	orig->temp_num++;
}
void DbInsightFunc_count(DbInsight* self, BIG row)
{
	DbInsight_prepareResult(self);

	DbInsightItem_execute(self, &DbInsightTemp_count, row);

	self->result_number = self->temp_num;
	DbInsight_saveNumber(self, row);
}

void DbInsightTemp_min(DbInsightItem* self, DbInsight* orig, BIG row)
{
	orig->temp_min = Std_dmin(orig->temp_min, DbColumn_getFlt(self->column, row, 0));
}
void DbInsightFunc_min(DbInsight* self, BIG row)
{
	DbInsight_prepareResult(self);

	DbInsightItem_execute(self, &DbInsightTemp_min, row);

	self->result_number = self->temp_min;
	DbInsight_saveNumber(self, row);
}

void DbInsightTemp_max(DbInsightItem* self, DbInsight* orig, BIG row)
{
	orig->temp_max = Std_dmax(orig->temp_max, DbColumn_getFlt(self->column, row, 0));
}
void DbInsightFunc_max(DbInsight* self, BIG row)
{
	DbInsight_prepareResult(self);

	DbInsightItem_execute(self, &DbInsightTemp_max, row);

	self->result_number = self->temp_max;
	DbInsight_saveNumber(self, row);
}

void DbInsightTemp_range(DbInsightItem* self, DbInsight* orig, BIG row)
{
	double v = DbColumn_getFlt(self->column, row, 0);
	orig->temp_min = Std_dmin(orig->temp_min, v);
	orig->temp_max = Std_dmax(orig->temp_max, v);
}
void DbInsightFunc_range(DbInsight* self, BIG row)
{
	DbInsight_prepareResult(self);

	DbInsightItem_execute(self, &DbInsightTemp_range, row);

	self->result_number = (self->temp_max - self->temp_min);
	DbInsight_saveNumber(self, row);
}

void DbInsightTemp_stdDeviation(DbInsightItem* self, DbInsight* orig, BIG row)
{
	double dif = DbColumn_getFlt(self->column, row, 0) - orig->temp_mean;
	orig->temp_sum += dif * dif;
	orig->temp_num++;
}
void DbInsightFunc_stdDeviation(DbInsight* self, BIG row)
{
	DbInsight_prepareResult(self);

	DbInsightItem_execute(self, &DbInsightTemp_avg, row);

	double avg = self->temp_sum / self->temp_num;
	DbInsight_prepareResult(self);
	self->temp_mean = avg;
	DbInsightItem_execute(self, &DbInsightTemp_stdDeviation, row);

	if (self->temp_num > 1)
		self->result_number = Os_sqrt(self->temp_sum / (self->temp_num - 1));

	DbInsight_saveNumber(self, row);
}

void DbInsightTemp_addArrayRow(DbInsightItem* self, DbInsight* orig, BIG row)
{
	StdBigs_add(&orig->arr, row);
}
void DbInsightFunc_median(DbInsight* self, BIG row)
{
	DbInsight_prepareResult(self);

	DbInsightItem_execute(self, &DbInsightTemp_addArrayRow, row);

	DbColumn* column = DbInsight_getFinalColumn(self);
	DbColumn_short(column, &self->arr, 0, self->arr.num, TRUE);

	self->result_number = self->arr.num ? DbColumn_getFlt(column, self->arr.ptrs[self->arr.num / 2], 0) : 0;
	DbInsight_saveNumber(self, row);

	StdBigs_free(&self->arr);
}

void DbInsightFunc_unique(DbInsight* self, BIG row)
{
	DbInsight_prepareResult(self);

	DbInsightItem_execute(self, &DbInsightTemp_addArrayRow, row);

	DbColumn* column = DbInsight_getFinalColumn(self);
	DbColumn_short(column, &self->arr, 0, self->arr.num, TRUE);

	UBIG unique = 0;
	BIG i;
	double last_value = 0;
	for (i = 0; i < self->arr.num; i++)
	{
		double v = DbColumn_getFlt(column, self->arr.ptrs[i], 0);

		if (i == 0 || v != last_value)
		{
			unique++;
			last_value = v;
		}
	}

	self->result_number = unique;
	DbInsight_saveNumber(self, row);

	StdBigs_free(&self->arr);
}

const DbInsightFunc g_insigns_funcs[] = {
	{ "INSIGHT_SUM", &DbInsightFunc_sum },
	{ "INSIGHT_AVERAGE", &DbInsightFunc_avg },
	{ "INSIGHT_MEDIAN", &DbInsightFunc_median },
	{ "INSIGHT_COUNT", &DbInsightFunc_count },
	{ "INSIGHT_UNIQUE", &DbInsightFunc_unique },
	{ "INSIGHT_MIN", &DbInsightFunc_min },
	{ "INSIGHT_MAX", &DbInsightFunc_max },
	{ "INSIGHT_RANGE", &DbInsightFunc_range },
	{ "INSIGHT_STANDARD_DEVIATION", &DbInsightFunc_stdDeviation },
};

UBIG DbInsightSelectFunc_num(void)
{
	return sizeof(g_insigns_funcs) / sizeof(DbInsightFunc);
}

const DbInsightFunc* DbInsightSelectFunc_get(BIG index)
{
	return &g_insigns_funcs[Std_clamp(index, 0, DbInsightSelectFunc_num() - 1)];
}

const char* DbInsightSelectFunc_getName(BIG index)
{
	const DbInsightFunc* fn = DbInsightSelectFunc_get(index);
	return fn ? fn->name : 0;
}

const DbInsightFunc* DbInsightSelectFunc_find(DbFormatTYPE srcFormat, const char* name)
{
	const UBIG N = DbInsightSelectFunc_num();
	BIG i;
	for (i = 0; i < N; i++)
	{
		if (Std_cmpCHAR(g_insigns_funcs[i].name, name))
			return &g_insigns_funcs[i];
	}
	return 0;
}

UNI* DbInsightSelectFunc_getList(void)
{
	UNI* str = 0;

	const UBIG N = DbInsightSelectFunc_num();
	BIG i;
	for (i = 0; i < N; i++)
	{
		const DbInsightFunc* fn = DbInsightSelectFunc_get(i);

		if (Lang_is(fn->name))
			str = Std_addAfterUNI(str, Lang_find(fn->name));
		else
			str = Std_addAfterUNI_char(str, fn->name);

		if (i + 1 < N)
			str = Std_addAfterUNI_char(str, "/");
	}
	return str;
}

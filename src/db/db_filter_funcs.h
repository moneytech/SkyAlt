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

static void _DbFilterSelect_setMark(BOOL value, BOOL andd, BOOL neg, UCHAR* marks, BIG i)
{
	if (andd)
		marks[i] &= value ^ neg;
	else
		marks[i] |= value ^ neg;
}

#define DbFilterSelectFunc_number1_LOOP(NAME, OPERATOR)\
void NAME(const DbColumn* column, const UNI* value, double valueEx, const BOOL andd, StdBigs* poses, UCHAR* marks)\
{\
	const double v = Std_getNumberFromUNI(value);\
	BIG i;\
	for (i = 0; i < poses->num && StdProgress_is(); i++)\
	{\
		_DbFilterSelect_setMark(DbColumn1_get((DbColumn1*)column, poses->ptrs[i]) OPERATOR v, andd, FALSE, marks, i);\
		StdProgress_setEx("FILTERING", i, poses->num);\
	}\
}
DbFilterSelectFunc_number1_LOOP(DbFilterSelectFunc_number1_eq, == )
DbFilterSelectFunc_number1_LOOP(DbFilterSelectFunc_number1_neq, != )
DbFilterSelectFunc_number1_LOOP(DbFilterSelectFunc_number1_less, < )
	DbFilterSelectFunc_number1_LOOP(DbFilterSelectFunc_number1_greater, > )
	DbFilterSelectFunc_number1_LOOP(DbFilterSelectFunc_number1_eqLess, <= )
	DbFilterSelectFunc_number1_LOOP(DbFilterSelectFunc_number1_eqGreater, >= )
	void DbFilterSelectFunc_number1_subEx(const BOOL neg, const DbColumn* column, const UNI* value, double valueEx, const BOOL andd, StdBigs* poses, UCHAR* marks)
{
	BIG i;
	UNI str[64];
	for (i = 0; i < poses->num && StdProgress_is(); i++)
	{
		Std_buildNumberUNI(DbColumn1_get((DbColumn1*)column, poses->ptrs[i]), -1, str);
		_DbFilterSelect_setMark(Std_subUNI_small(str, value) >= 0, andd, neg, marks, i);
		StdProgress_setEx("FILTERING", i, poses->num);
	}
}
void DbFilterSelectFunc_number1_sub(const DbColumn* column, const UNI* value, double valueEx, const BOOL andd, StdBigs* poses, UCHAR* marks)
{
	DbFilterSelectFunc_number1_subEx(FALSE, column, value, 0, andd, poses, marks);
}
void DbFilterSelectFunc_number1_nsub(const DbColumn* column, const UNI* value, double valueEx, const BOOL andd, StdBigs* poses, UCHAR* marks)
{
	DbFilterSelectFunc_number1_subEx(TRUE, column, value, 0, andd, poses, marks);
}

#define DbFilterSelectFunc_date_LOOP(NAME, OPERATOR)\
void NAME(const DbColumn* column, const UNI* value, double valueEx, const BOOL andd, StdBigs* poses, UCHAR* marks)\
{\
	const OsDateTimeTYPE timeFormat = _DbRoot_getOptionNumber(DbColumn_getRow(column), "timeFormat", 0);\
	const double v = OsDate_roundNumber(Std_getNumberFromUNI(value), timeFormat);\
	BIG i;\
	for (i = 0; i < poses->num && StdProgress_is(); i++)\
	{\
		_DbFilterSelect_setMark(OsDate_roundNumber(DbColumn1_get((DbColumn1*)column, poses->ptrs[i]), timeFormat) OPERATOR v, andd, FALSE, marks, i);\
		StdProgress_setEx("FILTERING", i, poses->num);\
	}\
}
DbFilterSelectFunc_date_LOOP(DbFilterSelectFunc_date_eq, == )
DbFilterSelectFunc_date_LOOP(DbFilterSelectFunc_date_neq, != )
DbFilterSelectFunc_date_LOOP(DbFilterSelectFunc_date_less, < )
	DbFilterSelectFunc_date_LOOP(DbFilterSelectFunc_date_greater, > )
	DbFilterSelectFunc_date_LOOP(DbFilterSelectFunc_date_eqLess, <= )
	DbFilterSelectFunc_date_LOOP(DbFilterSelectFunc_date_eqGreater, >= )

#define DbFilterSelectFunc_date_LOOP_EX(NAME, OPERATOR)\
void NAME(const DbColumn* column, const UNI* value, double valueEx, const BOOL andd, StdBigs* poses, UCHAR* marks)\
{\
	OsDate today = OsDate_initActual();\
	double add = Std_getNumberFromUNI(value);\
	switch ((UINT)valueEx)\
	{\
		case 0:	today.m_year += add;	break;\
		case 1:	today.m_month += add;	break;\
		case 2:	today.m_day += add;		break;\
		case 3:	today.m_hour += add;	break;\
		case 4:	today.m_min += add;		break;\
		case 5:	today.m_sec += add;		break;\
	}\
	const OsDateTimeTYPE timeFormat = _DbRoot_getOptionNumber(DbColumn_getRow(column), "timeFormat", 0);\
	const double v = OsDate_asNumber(&today);\
	BIG i;\
	for (i = 0; i < poses->num && StdProgress_is(); i++)\
	{\
		_DbFilterSelect_setMark(OsDate_roundNumber(DbColumn1_get((DbColumn1*)column, poses->ptrs[i]), timeFormat) OPERATOR v, andd, FALSE, marks, i);\
		StdProgress_setEx("FILTERING", i, poses->num);\
	}\
}
	DbFilterSelectFunc_date_LOOP_EX(DbFilterSelectFunc_date_todayLess, < )
	DbFilterSelectFunc_date_LOOP_EX(DbFilterSelectFunc_date_todayGreater, > )

	void DbFilterSelectFunc_string32_eqEx(const BOOL neg, const DbColumn* column, const UNI* value, double valueEx, const BOOL andd, StdBigs* poses, UCHAR* marks)
{
	BIG i;
	for (i = 0; i < poses->num && StdProgress_is(); i++)
	{
		_DbFilterSelect_setMark(Std_cmpUNI(DbColumnString32_get((DbColumnString32*)column, poses->ptrs[i]), value), andd, neg, marks, i);
		StdProgress_setEx("FILTERING", i, poses->num);
	}
}
void DbFilterSelectFunc_string32_eq(const DbColumn* column, const UNI* value, double valueEx, const BOOL andd, StdBigs* poses, UCHAR* marks)
{
	DbFilterSelectFunc_string32_eqEx(FALSE, column, value, 0, andd, poses, marks);
}
void DbFilterSelectFunc_string32_neq(const DbColumn* column, const UNI* value, double valueEx, const BOOL andd, StdBigs* poses, UCHAR* marks)
{
	DbFilterSelectFunc_string32_eqEx(TRUE, column, value, 0, andd, poses, marks);
}

void DbFilterSelectFunc_string32_emptyEx(const BOOL neg, const DbColumn* column, const UNI* value, double valueEx, const BOOL andd, StdBigs* poses, UCHAR* marks)
{
	BIG i;
	for (i = 0; i < poses->num && StdProgress_is(); i++)
	{
		_DbFilterSelect_setMark(Std_sizeUNI(DbColumnString32_get((DbColumnString32*)column, poses->ptrs[i])) == 0, andd, neg, marks, i);
		StdProgress_setEx("FILTERING", i, poses->num);
	}
}
void DbFilterSelectFunc_string32_empty(const DbColumn* column, const UNI* value, double valueEx, const BOOL andd, StdBigs* poses, UCHAR* marks)
{
	DbFilterSelectFunc_string32_emptyEx(FALSE, column, value, 0, andd, poses, marks);
}
void DbFilterSelectFunc_string32_nempty(const DbColumn* column, const UNI* value, double valueEx, const BOOL andd, StdBigs* poses, UCHAR* marks)
{
	DbFilterSelectFunc_string32_emptyEx(TRUE, column, value, 0, andd, poses, marks);
}

void DbFilterSelectFunc_string32_subEx(const BOOL neg, const DbColumn* column, const UNI* value, double valueEx, const BOOL andd, StdBigs* poses, UCHAR* marks)
{
	BIG i;
	for (i = 0; i < poses->num && StdProgress_is(); i++)
	{
		_DbFilterSelect_setMark(Std_subUNI_small(DbColumnString32_get((DbColumnString32*)column, poses->ptrs[i]), value) >= 0, andd, neg, marks, i);
		StdProgress_setEx("FILTERING", i, poses->num);
	}
}
void DbFilterSelectFunc_string32_sub(const DbColumn* column, const UNI* value, double valueEx, const BOOL andd, StdBigs* poses, UCHAR* marks)
{
	DbFilterSelectFunc_string32_subEx(FALSE, column, value, 0, andd, poses, marks);
}
void DbFilterSelectFunc_string32_nsub(const DbColumn* column, const UNI* value, double valueEx, const BOOL andd, StdBigs* poses, UCHAR* marks)
{
	DbFilterSelectFunc_string32_subEx(TRUE, column, value, 0, andd, poses, marks);
}

void DbFilterSelectFunc_tags_eqEx(const BOOL neg, const DbColumn* column, const UNI* value, double valueEx, const BOOL andd, StdBigs* poses, UCHAR* marks)
{
	double v = Std_getNumberFromUNI(value);
	BIG i;
	for (i = 0; i < poses->num && StdProgress_is(); i++)
	{
		const UBIG n = DbColumn_sizeHard(column, poses->ptrs[i]);
		BOOL found = FALSE;
		if (n == 0)
			found = (v < 0);
		else
			found = (n == 1 && DbColumn_getIndex(column, poses->ptrs[i], 0) == v);

		_DbFilterSelect_setMark(found, andd, neg, marks, i);
		StdProgress_setEx("FILTERING", i, poses->num);
	}
}
void DbFilterSelectFunc_tags_eq(const DbColumn* column, const UNI* value, double valueEx, const BOOL andd, StdBigs* poses, UCHAR* marks)
{
	DbFilterSelectFunc_tags_eqEx(FALSE, column, value, 0, andd, poses, marks);
}
void DbFilterSelectFunc_tags_neq(const DbColumn* column, const UNI* value, double valueEx, const BOOL andd, StdBigs* poses, UCHAR* marks)
{
	DbFilterSelectFunc_tags_eqEx(TRUE, column, value, 0, andd, poses, marks);
}

void DbFilterSelectFunc_tags_subEx(const BOOL neg, const DbColumn* column, const UNI* value, double valueEx, const BOOL andd, StdBigs* poses, UCHAR* marks)
{
	double v = Std_getNumberFromUNI(value);
	BIG i, ii;
	for (i = 0; i < poses->num && StdProgress_is(); i++)
	{
		const UBIG n = DbColumn_sizeHard(column, poses->ptrs[i]);
		BOOL found = FALSE;
		for (ii = 0; ii < n && StdProgress_is(); ii++)
			found |= (DbColumn_getIndex(column, poses->ptrs[i], ii) == v);

		_DbFilterSelect_setMark(found, andd, neg, marks, i);
		StdProgress_setEx("FILTERING", i, poses->num);
	}
}
void DbFilterSelectFunc_tags_sub(const DbColumn* column, const UNI* value, double valueEx, const BOOL andd, StdBigs* poses, UCHAR* marks)
{
	DbFilterSelectFunc_tags_subEx(FALSE, column, value, 0, andd, poses, marks);
}
void DbFilterSelectFunc_tags_nsub(const DbColumn* column, const UNI* value, double valueEx, const BOOL andd, StdBigs* poses, UCHAR* marks)
{
	DbFilterSelectFunc_tags_subEx(TRUE, column, value, 0, andd, poses, marks);
}

void DbFilterSelectFunc_tags_emptyEx(const BOOL neg, const DbColumn* column, const UNI* value, double valueEx, const BOOL andd, StdBigs* poses, UCHAR* marks)
{
	BIG i;
	for (i = 0; i < poses->num && StdProgress_is(); i++)
	{
		BOOL found = DbColumn_getFirstRow(column, poses->ptrs[i]) < 0;
		_DbFilterSelect_setMark(found, andd, neg, marks, i);
		StdProgress_setEx("FILTERING", i, poses->num);
	}
}
void DbFilterSelectFunc_tags_empty(const DbColumn* column, const UNI* value, double valueEx, const BOOL andd, StdBigs* poses, UCHAR* marks)
{
	DbFilterSelectFunc_tags_emptyEx(FALSE, column, value, 0, andd, poses, marks);
}
void DbFilterSelectFunc_tags_nempty(const DbColumn* column, const UNI* value, double valueEx, const BOOL andd, StdBigs* poses, UCHAR* marks)
{
	DbFilterSelectFunc_tags_emptyEx(TRUE, column, value, 0, andd, poses, marks);
}

typedef void DbFilterSelectCallback(const DbColumn* column, const UNI* value, double valueEx, const BOOL andd, StdBigs* poses, UCHAR* marks);
typedef struct DbFilterSelectFunc_s
{
	DbFormatTYPE srcFormat;
	const char* name;
	DbFilterSelectCallback* func;
} DbFilterSelectFunc;

const DbFilterSelectFunc g_filter_selects[] = {
	{ DbFormat_NUMBER_1, "=", &DbFilterSelectFunc_number1_eq },
	{ DbFormat_NUMBER_1, "<>", &DbFilterSelectFunc_number1_neq },
	{ DbFormat_NUMBER_1, "<", &DbFilterSelectFunc_number1_less },
	{ DbFormat_NUMBER_1, ">", &DbFilterSelectFunc_number1_greater },
	{ DbFormat_NUMBER_1, "<=", &DbFilterSelectFunc_number1_eqLess },
	{ DbFormat_NUMBER_1, ">=", &DbFilterSelectFunc_number1_eqGreater },
	{ DbFormat_NUMBER_1, "FILTER_FUNC_CONTAIN", &DbFilterSelectFunc_number1_sub },
	{ DbFormat_NUMBER_1, "FILTER_FUNC_NOT_CONTAIN", &DbFilterSelectFunc_number1_nsub },

	{ DbFormat_CURRENCY, "=", &DbFilterSelectFunc_number1_eq },
	{ DbFormat_CURRENCY, "<>", &DbFilterSelectFunc_number1_neq },
	{ DbFormat_CURRENCY, "<", &DbFilterSelectFunc_number1_less },
	{ DbFormat_CURRENCY, ">", &DbFilterSelectFunc_number1_greater },
	{ DbFormat_CURRENCY, "<=", &DbFilterSelectFunc_number1_eqLess },
	{ DbFormat_CURRENCY, ">=", &DbFilterSelectFunc_number1_eqGreater },

	{ DbFormat_PERCENTAGE, "=", &DbFilterSelectFunc_number1_eq },
	{ DbFormat_PERCENTAGE, "<>", &DbFilterSelectFunc_number1_neq },
	{ DbFormat_PERCENTAGE, "<", &DbFilterSelectFunc_number1_less },
	{ DbFormat_PERCENTAGE, ">", &DbFilterSelectFunc_number1_greater },
	{ DbFormat_PERCENTAGE, "<=", &DbFilterSelectFunc_number1_eqLess },
	{ DbFormat_PERCENTAGE, ">=", &DbFilterSelectFunc_number1_eqGreater },

	{ DbFormat_RATING, "=", &DbFilterSelectFunc_number1_eq },
	{ DbFormat_RATING, "<>", &DbFilterSelectFunc_number1_neq },
	{ DbFormat_RATING, "<", &DbFilterSelectFunc_number1_less },
	{ DbFormat_RATING, ">", &DbFilterSelectFunc_number1_greater },
	{ DbFormat_RATING, "<=", &DbFilterSelectFunc_number1_eqLess },
	{ DbFormat_RATING, ">=", &DbFilterSelectFunc_number1_eqGreater },

	{ DbFormat_SLIDER, "=", &DbFilterSelectFunc_number1_eq },
	{ DbFormat_SLIDER, "<>", &DbFilterSelectFunc_number1_neq },
	{ DbFormat_SLIDER, "<", &DbFilterSelectFunc_number1_less },
	{ DbFormat_SLIDER, ">", &DbFilterSelectFunc_number1_greater },
	{ DbFormat_SLIDER, "<=", &DbFilterSelectFunc_number1_eqLess },
	{ DbFormat_SLIDER, ">=", &DbFilterSelectFunc_number1_eqGreater },

	{ DbFormat_SUMMARY, "=", &DbFilterSelectFunc_number1_eq },
	{ DbFormat_SUMMARY, "<>", &DbFilterSelectFunc_number1_neq },
	{ DbFormat_SUMMARY, "<", &DbFilterSelectFunc_number1_less },
	{ DbFormat_SUMMARY, ">", &DbFilterSelectFunc_number1_greater },
	{ DbFormat_SUMMARY, "<=", &DbFilterSelectFunc_number1_eqLess },
	{ DbFormat_SUMMARY, ">=", &DbFilterSelectFunc_number1_eqGreater },

	{ DbFormat_CHECK, "=", &DbFilterSelectFunc_number1_eq },

	{ DbFormat_DATE, "=", &DbFilterSelectFunc_date_eq },
	{ DbFormat_DATE, "<>", &DbFilterSelectFunc_date_neq },
	{ DbFormat_DATE, "<", &DbFilterSelectFunc_date_less },
	{ DbFormat_DATE, ">", &DbFilterSelectFunc_date_greater },
	{ DbFormat_DATE, "<=", &DbFilterSelectFunc_date_eqLess },
	{ DbFormat_DATE, ">=", &DbFilterSelectFunc_date_eqGreater },
	{ DbFormat_DATE, "FILTER_FUNC_TODAY_LESS", &DbFilterSelectFunc_date_todayLess },
	{ DbFormat_DATE, "FILTER_FUNC_TODAY_GREATER", &DbFilterSelectFunc_date_todayGreater },

	{ DbFormat_LINK_1, "FILTER_FUNC_EMPTY", &DbFilterSelectFunc_tags_empty },
	{ DbFormat_LINK_1, "FILTER_FUNC_NOT_EMPTY", &DbFilterSelectFunc_tags_nempty },
	//...

	{ DbFormat_LINK_N, "FILTER_FUNC_EMPTY", &DbFilterSelectFunc_tags_empty },
	{ DbFormat_LINK_N, "FILTER_FUNC_NOT_EMPTY", &DbFilterSelectFunc_tags_nempty },
	//...

	{ DbFormat_LINK_MIRRORED, "FILTER_FUNC_EMPTY", &DbFilterSelectFunc_tags_empty },
	{ DbFormat_LINK_MIRRORED, "FILTER_FUNC_NOT_EMPTY", &DbFilterSelectFunc_tags_nempty },

	{ DbFormat_LINK_JOINTED, "FILTER_FUNC_EMPTY", &DbFilterSelectFunc_tags_empty },
	{ DbFormat_LINK_JOINTED, "FILTER_FUNC_NOT_EMPTY", &DbFilterSelectFunc_tags_nempty },

	{ DbFormat_LINK_FILTERED, "FILTER_FUNC_EMPTY", &DbFilterSelectFunc_tags_empty },
	{ DbFormat_LINK_FILTERED, "FILTER_FUNC_NOT_EMPTY", &DbFilterSelectFunc_tags_nempty },

	{ DbFormat_TEXT, "=", &DbFilterSelectFunc_string32_eq },
	{ DbFormat_TEXT, "<>", &DbFilterSelectFunc_string32_neq },
	{ DbFormat_TEXT, "FILTER_FUNC_CONTAIN", &DbFilterSelectFunc_string32_sub },
	{ DbFormat_TEXT, "FILTER_FUNC_NOT_CONTAIN", &DbFilterSelectFunc_string32_nsub },
	{ DbFormat_TEXT, "FILTER_FUNC_EMPTY", &DbFilterSelectFunc_string32_empty },
	{ DbFormat_TEXT, "FILTER_FUNC_NOT_EMPTY", &DbFilterSelectFunc_string32_nempty },

	{ DbFormat_PHONE, "=", &DbFilterSelectFunc_string32_eq },
	{ DbFormat_PHONE, "<>", &DbFilterSelectFunc_string32_neq },
	{ DbFormat_PHONE, "FILTER_FUNC_CONTAIN", &DbFilterSelectFunc_string32_sub },
	{ DbFormat_PHONE, "FILTER_FUNC_NOT_CONTAIN", &DbFilterSelectFunc_string32_nsub },
	{ DbFormat_PHONE, "FILTER_FUNC_EMPTY", &DbFilterSelectFunc_string32_empty },
	{ DbFormat_PHONE, "FILTER_FUNC_NOT_EMPTY", &DbFilterSelectFunc_string32_nempty },

	{ DbFormat_URL, "=", &DbFilterSelectFunc_string32_eq },
	{ DbFormat_URL, "<>", &DbFilterSelectFunc_string32_neq },
	{ DbFormat_URL, "FILTER_FUNC_CONTAIN", &DbFilterSelectFunc_string32_sub },
	{ DbFormat_URL, "FILTER_FUNC_NOT_CONTAIN", &DbFilterSelectFunc_string32_nsub },
	{ DbFormat_URL, "FILTER_FUNC_EMPTY", &DbFilterSelectFunc_string32_empty },
	{ DbFormat_URL, "FILTER_FUNC_NOT_EMPTY", &DbFilterSelectFunc_string32_nempty },

	{ DbFormat_EMAIL, "=", &DbFilterSelectFunc_string32_eq },
	{ DbFormat_EMAIL, "<>", &DbFilterSelectFunc_string32_neq },
	{ DbFormat_EMAIL, "FILTER_FUNC_CONTAIN", &DbFilterSelectFunc_string32_sub },
	{ DbFormat_EMAIL, "FILTER_FUNC_NOT_CONTAIN", &DbFilterSelectFunc_string32_nsub },
	{ DbFormat_EMAIL, "FILTER_FUNC_EMPTY", &DbFilterSelectFunc_string32_empty },
	{ DbFormat_EMAIL, "FILTER_FUNC_NOT_EMPTY", &DbFilterSelectFunc_string32_nempty },

	{ DbFormat_LOCATION, "=", &DbFilterSelectFunc_string32_eq },
	{ DbFormat_LOCATION, "<>", &DbFilterSelectFunc_string32_neq },
	{ DbFormat_LOCATION, "FILTER_FUNC_CONTAIN", &DbFilterSelectFunc_string32_sub },
	{ DbFormat_LOCATION, "FILTER_FUNC_NOT_CONTAIN", &DbFilterSelectFunc_string32_nsub },
	{ DbFormat_LOCATION, "FILTER_FUNC_EMPTY", &DbFilterSelectFunc_string32_empty },
	{ DbFormat_LOCATION, "FILTER_FUNC_NOT_EMPTY", &DbFilterSelectFunc_string32_nempty },

	{ DbFormat_MENU, "=", &DbFilterSelectFunc_tags_eq },
	{ DbFormat_MENU, "<>", &DbFilterSelectFunc_tags_neq },
	{ DbFormat_MENU, "FILTER_FUNC_CONTAIN", &DbFilterSelectFunc_tags_sub },
	{ DbFormat_MENU, "FILTER_FUNC_NOT_CONTAIN", &DbFilterSelectFunc_tags_nsub },
	{ DbFormat_MENU, "FILTER_FUNC_EMPTY", &DbFilterSelectFunc_tags_empty },
	{ DbFormat_MENU, "FILTER_FUNC_NOT_EMPTY", &DbFilterSelectFunc_tags_nempty },

	{ DbFormat_TAGS, "=", &DbFilterSelectFunc_tags_eq },
	{ DbFormat_TAGS, "<>", &DbFilterSelectFunc_tags_neq },
	{ DbFormat_TAGS, "FILTER_FUNC_CONTAIN", &DbFilterSelectFunc_tags_sub },
	{ DbFormat_TAGS, "FILTER_FUNC_NOT_CONTAIN", &DbFilterSelectFunc_tags_nsub },
	{ DbFormat_TAGS, "FILTER_FUNC_EMPTY", &DbFilterSelectFunc_tags_empty },
	{ DbFormat_TAGS, "FILTER_FUNC_NOT_EMPTY", &DbFilterSelectFunc_tags_nempty },
};

static UBIG _DbFilterSelectFunc_numAll(void)
{
	return sizeof(g_filter_selects) / sizeof(DbFilterSelectFunc);
}

UBIG DbFilterSelectFunc_num(DbFormatTYPE srcFormat)
{
	UBIG n = 0;
	BIG i;
	for (i = 0; i < _DbFilterSelectFunc_numAll(); i++)
		if (g_filter_selects[i].srcFormat == srcFormat)
			n++;
	return n;
}
const DbFilterSelectFunc* DbFilterSelectFunc_get(DbFormatTYPE srcFormat, BIG index)
{
	BIG i;
	for (i = 0; i < _DbFilterSelectFunc_numAll(); i++)
	{
		if (g_filter_selects[i].srcFormat == srcFormat)
		{
			if (index == 0)
				return &g_filter_selects[i];
			index--;
		}
	}
	return 0;
}

const char* DbFilterSelectFunc_getName(DbFormatTYPE srcFormat, BIG index)
{
	const DbFilterSelectFunc* fn = DbFilterSelectFunc_get(srcFormat, index);
	return fn ? fn->name : 0;
}

const DbFilterSelectFunc* DbFilterSelectFunc_find(DbFormatTYPE srcFormat, const char* name)
{
	BIG i;
	for (i = 0; i < _DbFilterSelectFunc_numAll(); i++)
	{
		if (g_filter_selects[i].srcFormat == srcFormat && Std_cmpCHAR(g_filter_selects[i].name, name))
			return &g_filter_selects[i];
	}
	return 0;
}

UNI* DbFilterSelectFunc_getList(DbFormatTYPE srcFormat)
{
	UNI* str = 0;

	const UBIG N = DbFilterSelectFunc_num(srcFormat);
	BIG i;
	for (i = 0; i < N; i++)
	{
		const DbFilterSelectFunc* fn = DbFilterSelectFunc_get(srcFormat, i);

		if (Lang_is(fn->name))
			str = Std_addAfterUNI(str, Lang_find(fn->name));
		else
			str = Std_addAfterUNI_char(str, fn->name);

		if (i + 1 < N)
			str = Std_addAfterUNI_char(str, "/");
	}
	return str;
}

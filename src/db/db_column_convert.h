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

void DbColumn_setDefaultOptions(BIG row, DbFormatTYPE type)
{
	UNI str[64];
	switch (type)
	{
		case DbFormat_NUMBER_1:
		case DbFormat_NUMBER_N:
		case DbFormat_PERCENTAGE:
		_DbRoot_getOptionNumber(row, "precision", 2);	//rewrite only If doesn't exist!
		_DbRoot_getOptionNumber(row, "units", 0);
		_DbRoot_getOptionNumber(row, "mult100", 0);
		break;

		case DbFormat_RATING:
		_DbRoot_getOptionNumber(row, "numStars", 5);
		break;

		case DbFormat_CURRENCY:
		_DbRoot_getOptionString(row, "currency_before", _UNI32("$"), str, 64);
		_DbRoot_getOptionString(row, "currency_after", _UNI32(" USD"), str, 64);
		_DbRoot_getOptionNumber(row, "precision", 1);
		_DbRoot_getOptionNumber(row, "units", 0);
		_DbRoot_getOptionNumber(row, "before", 1);
		break;

		case DbFormat_SLIDER:
		_DbRoot_getOptionNumber(row, "min", 0);
		_DbRoot_getOptionNumber(row, "max", 10);
		_DbRoot_getOptionNumber(row, "jump", 1);
		break;

		case DbFormat_TEXT:
		_DbRoot_getOptionNumber(row, "password", 0);
		break;

		case DbFormat_DATE:
		_DbRoot_getOptionNumber(row, "timeFormat", 0);
		break;

		case DbFormat_FILE_1:
		case DbFormat_FILE_N:
		_DbRoot_getOptionNumber(row, "preview", 1);
		break;

		case DbFormat_LINK_1:
		case DbFormat_LINK_N:
		case DbFormat_LINK_MIRRORED:
		case DbFormat_LINK_JOINTED:
		case DbFormat_LINK_FILTERED:
		_DbRoot_getOptionNumber(row, "numColumnPreviews", 1);
		break;

		default:
		break;
	}
}

static DbColumn* _DbColumn_replaceRow(DbColumn* srcColumn, DbColumn* dstColumn, const UNI* format)
{
	BIG srcRow = DbColumn_getRow(srcColumn);
	BIG dstRow = DbColumn_getRow(dstColumn);

	//backup
	UNI type[32];
	DbColumnString32_getOption(DbRoot_getColumnOptions(), dstRow, "type", 0, type, 32);

	//copy options
	const UNI* oldOptions = DbColumnString32_get(DbRoot_getColumnOptions(), srcRow);
	DbColumnString32_setCopy(DbRoot_getColumnOptions(), dstRow, oldOptions);

	//restore backup
	DbColumnString32_setOption(DbRoot_getColumnOptions(), dstRow, "type", type);

	//new format(line below needed)
	DbColumnString32_setOption(DbRoot_getColumnOptions(), dstRow, "format", format);

	DbRoot_replaceAndRemoveRow(srcRow, dstRow);
	return dstColumn;
}

DbColumn* DbColumnConvert_convert(const DbColumnConvert* self, DbColumnConvertPrm prm)
{
	DbColumn* dstColumn = prm.srcColumn;

	if (self->func)
		dstColumn = self->func(self, prm);	//convert

	BIG dstRow = DbColumn_getRow(dstColumn);
	UNI* format = Std_newUNI_char(DbColumnFormat_findColumnName(self->dstType));

	//SetFormat and Replace
	if (prm.srcColumn != dstColumn)
		_DbColumn_replaceRow(prm.srcColumn, dstColumn, format);
	else
		DbColumnString32_setOption(DbRoot_getColumnOptions(), dstRow, "format", format);		//new format

	//set options
	DbColumn_setDefaultOptions(dstRow, self->dstType);

	Std_deleteUNI(format);
	return dstColumn;
}

DbColumn* DbColumnConvert_convertEx(DbColumn* srcColumn, DbFormatTYPE dstType, DbColumn* btableColumn)
{
	DbColumn* dstColumn = 0;

	const DbColumnConvert* self = DbColumnConvert_find(DbColumnFormat_findColumn(srcColumn), dstType);
	if (self)
	{
		DbColumnConvertPrm prm;
		prm.srcColumn = srcColumn;
		prm.bTableColumn = btableColumn;
		dstColumn = DbColumnConvert_convert(self, prm);
	}

	return dstColumn;
}

DbColumn* DbColumn_convert_1_N(const DbColumnConvert* self, DbColumnConvertPrm prm)
{
	UNI name[64];
	DbColumn* dstColumn = (DbColumn*)DbColumns_createColumnN(prm.srcColumn->parent, DbColumn_getName(prm.srcColumn, name, 64), ((DbColumn1*)prm.srcColumn)->btable);

	const UBIG NUM_ROWS = DbColumn_numRows(prm.srcColumn);
	UBIG i;
	for (i = 0; i < NUM_ROWS; i++)
		DbColumn_setFlt(dstColumn, i, 0, DbColumn_getFlt(prm.srcColumn, i, 0));

	return dstColumn;
}

DbColumn* DbColumn_convert_N_1(const DbColumnConvert* self, DbColumnConvertPrm prm)
{
	UNI name[64];
	DbColumn* dstColumn = (DbColumn*)DbColumns_createColumn1(prm.srcColumn->parent, DbColumn_getName(prm.srcColumn, name, 64), ((DbColumnN*)prm.srcColumn)->btable);

	const UBIG NUM_ROWS = DbColumn_numRows(prm.srcColumn);
	UBIG i;
	for (i = 0; i < NUM_ROWS; i++)
		DbColumn_setFlt(dstColumn, i, 0, DbColumn_getFlt(prm.srcColumn, i, 0));

	return dstColumn;
}

DbColumn* DbColumn_convert_to_string(const DbColumnConvert* self, DbColumnConvertPrm prm)
{
	UNI name[64];
	DbColumn* dstColumn = (DbColumn*)DbColumns_createColumnString32(prm.srcColumn->parent, DbColumn_getName(prm.srcColumn, name, 64));

	const UBIG NUM_ROWS = DbColumn_numRows(prm.srcColumn);
	UBIG i;
	for (i = 0; i < NUM_ROWS; i++)
	{
		StdString str = StdString_init();
		DbColumnString32_setEqFree((DbColumnString32*)dstColumn, i, DbColumn_getStringCopyLong(prm.srcColumn, i, &str));
		StdString_freeIgnore(&str);
	}

	return dstColumn;
}

DbColumn* DbColumn_convert_string_to_number1(const DbColumnConvert* self, DbColumnConvertPrm prm)
{
	UNI name[64];
	DbColumn* dstColumn = (DbColumn*)DbColumns_createColumn1(prm.srcColumn->parent, DbColumn_getName(prm.srcColumn, name, 64), 0);

	const UBIG NUM_ROWS = DbColumn_numRows(prm.srcColumn);
	UBIG i;
	for (i = 0; i < NUM_ROWS; i++)
		DbColumn1_set((DbColumn1*)dstColumn, i, Std_getNumberFromUNI(DbColumnString32_get((DbColumnString32*)prm.srcColumn, i)));

	return dstColumn;
}

DbColumn* DbColumn_convert_string_to_link(const DbColumnConvert* self, DbColumnConvertPrm prm)
{
	UNI name[64];
	DbColumn* dstColumn;

	if (self->dstType == DbFormat_LINK_1)	dstColumn = (DbColumn*)DbColumns_createColumn1(prm.srcColumn->parent, DbColumn_getName(prm.srcColumn, name, 64), prm.bTableColumn->parent->parent);
	else									dstColumn = (DbColumn*)DbColumns_createColumnN(prm.srcColumn->parent, DbColumn_getName(prm.srcColumn, name, 64), prm.bTableColumn->parent->parent);

	const UBIG NUM_ROWS = DbColumn_numRows(prm.srcColumn);
	UBIG i;
	for (i = 0; i < NUM_ROWS; i++)
	{
		const UNI* str = DbColumnString32_get((DbColumnString32*)prm.srcColumn, i);
		BIG row = DbColumn_searchString(prm.bTableColumn, str);
		if (row < 0)
		{
			//create new
			row = DbTable_addRow(prm.bTableColumn->parent->parent);
			DbColumn_setStringCopy(prm.bTableColumn, row, 0, str);
		}

		//if (row >= 0)
		DbColumn_setFlt(dstColumn, i, 0, row);
	}

	return dstColumn;
}

DbColumn* DbColumn_convert_number_to_link(const DbColumnConvert* self, DbColumnConvertPrm prm)
{
	UNI name[64];
	DbColumn* dstColumn;

	if (self->dstType == DbFormat_LINK_1)	dstColumn = (DbColumn*)DbColumns_createColumn1(prm.srcColumn->parent, DbColumn_getName(prm.srcColumn, name, 64), prm.bTableColumn->parent->parent);
	else									dstColumn = (DbColumn*)DbColumns_createColumnN(prm.srcColumn->parent, DbColumn_getName(prm.srcColumn, name, 64), prm.bTableColumn->parent->parent);

	const UBIG NUM_ROWS = DbColumn_numRows(prm.srcColumn);
	UBIG i;
	for (i = 0; i < NUM_ROWS; i++)
	{
		BIG row = DbColumn_searchNumber(prm.bTableColumn, DbColumn_getFlt(prm.srcColumn, i, 0));
		if (row >= 0)
			DbColumn_setFlt(dstColumn, i, 0, row);
	}

	return dstColumn;
}

static BIG _DbColumn_addOption(StdArr* names, const UNI* str, BIG optionsRow, StdBigs* rows)
{
	//find or add name
	BIG ii;
	for (ii = 0; ii < names->num; ii++)
	{
		if (Std_cmpUNI(names->ptrs[ii], str))
			break;
	}

	if (ii == names->num)	//not found
	{
		UBIG r = DbTable_addRow(DbRoot_getInfoTable());
		_DbRoot_setOptionString(r, "name", str);
		DbColumnN_add(DbRoot_subs(), optionsRow, r);

		StdBigs_add(rows, r);
		StdArr_add(names, Std_newUNI(str));
	}

	return ii;
}

DbColumn* DbColumn_convert_to_menu(const DbColumnConvert* self, DbColumnConvertPrm prm)
{
	UNI name[64];
	DbColumn* dstColumn;
	if (self->dstType == DbFormat_MENU)	dstColumn = (DbColumn*)DbColumns_createColumn1(prm.srcColumn->parent, DbColumn_getName(prm.srcColumn, name, 64), DbRoot_getInfoTable());
	else								dstColumn = (DbColumn*)DbColumns_createColumnN(prm.srcColumn->parent, DbColumn_getName(prm.srcColumn, name, 64), DbRoot_getInfoTable());

	BIG optionsRow = DbRows_findSubType(DbColumn_getRow(dstColumn), "options");
	StdArr names = StdArr_init();
	StdBigs rows = StdBigs_init();

	const UBIG NUM_ROWS = DbColumn_numRows(prm.srcColumn);
	UBIG i;
	StdString str = StdString_init();
	for (i = 0; i < NUM_ROWS; i++)
	{
		StdString_empty(&str);
		DbColumn_getStringCopyLong(prm.srcColumn, i, &str);

		UBIG strN = Std_sizeUNI(str.str);
		if (strN)
		{
			BIG ii = _DbColumn_addOption(&names, str.str, optionsRow, &rows);
			DbColumn_add(dstColumn, i, rows.ptrs[ii]);
		}
	}

	StdString_free(&str);
	StdBigs_free(&rows);
	StdArr_freeFn(&names, (StdArrFREE)&Std_deleteUNI);

	return dstColumn;
}

DbColumn* DbColumn_convert_menu_to_string(const DbColumnConvert* self, DbColumnConvertPrm prm)
{
	UNI name[64];
	DbColumn* dstColumn = (DbColumn*)DbColumns_createColumnString32(prm.srcColumn->parent, DbColumn_getName(prm.srcColumn, name, 64));

	const UBIG NUM_ROWS = DbColumn_numRows(prm.srcColumn);
	UBIG i, ii;
	for (i = 0; i < NUM_ROWS; i++)
	{
		UNI* str = 0;

		const UBIG N = DbColumn_sizeActive(prm.srcColumn, i);
		for (ii = 0; ii < N; ii++)
		{
			BIG r = DbColumn_getIndex(prm.srcColumn, i, ii);

			UNI name[64];
			_DbRoot_getOptionString(r, "name", 0, name, 64);
			str = Std_addAfterUNI(str, name);
			if (ii + 1 < N)
				str = Std_addAfterUNI_char(str, ";");
		}

		DbColumnString32_setEqFree((DbColumnString32*)dstColumn, i, str);
	}

	return dstColumn;
}

DbColumn* DbColumn_convert_to_tags(const DbColumnConvert* self, DbColumnConvertPrm prm)
{
	UNI name[64];
	DbColumn* dstColumn = (DbColumn*)DbColumns_createColumnN(prm.srcColumn->parent, DbColumn_getName(prm.srcColumn, name, 64), DbRoot_getInfoTable());

	BIG optionsRow = DbRows_findSubType(DbColumn_getRow(dstColumn), "options");
	StdArr names = StdArr_init();
	StdBigs rows = StdBigs_init();

	const UBIG NUM_ROWS = DbColumn_numRows(prm.srcColumn);
	UBIG i;
	StdString str = StdString_init();
	StdString strTemp = StdString_init();

	for (i = 0; i < NUM_ROWS; i++)
	{
		StdString_empty(&str);
		DbColumn_getStringCopyLong(prm.srcColumn, i, &str);

		UNI* s = str.str;
		BIG n;
		while ((n = Std_separFind(s, _UNI32("/\\|,;:."))) >= 0)
		{
			StdString_setUNI_n(&strTemp, s, n);

			if (n)
			{
				BIG ii = _DbColumn_addOption(&names, strTemp.str, optionsRow, &rows);
				DbColumn_add(dstColumn, i, rows.ptrs[ii]);
			}

			if (s[n] == 0)
				break;

			s += (n + 1);
		}
	}

	StdString_free(&strTemp);
	StdString_free(&str);
	StdBigs_free(&rows);
	StdArr_freeFn(&names, (StdArrFREE)&Std_deleteUNI);

	return dstColumn;
}

DbColumn* DbColumn_convert_string_to_date(const DbColumnConvert* self, DbColumnConvertPrm prm)
{
	UNI name[64];
	DbColumn* dstColumn = (DbColumn*)DbColumns_createColumn1(prm.srcColumn->parent, DbColumn_getName(prm.srcColumn, name, 64), 0);

	const UBIG NUM_ROWS = DbColumn_numRows(prm.srcColumn);
	UBIG i;
	StdString str = StdString_init();
	for (i = 0; i < NUM_ROWS; i++)
	{
		StdString_empty(&str);
		DbColumn_getStringCopyLong(prm.srcColumn, i, &str);

		OsDate date = OsDate_initFromString(str.str, prm.dateType);
		DbColumn1_set((DbColumn1*)dstColumn, i, OsDate_asNumber(&date));
	}

	return dstColumn;
}

DbColumn* DbColumn_convert_date_to_string(const DbColumnConvert* self, DbColumnConvertPrm prm)
{
	UNI name[64];
	DbColumn* dstColumn = (DbColumn*)DbColumns_createColumnString32(prm.srcColumn->parent, DbColumn_getName(prm.srcColumn, name, 64));

	const UBIG NUM_ROWS = DbColumn_numRows(prm.srcColumn);
	UBIG i;
	for (i = 0; i < NUM_ROWS; i++)
	{
		OsDateTYPE formatDate = UiIniSettings_getDateFormat();
		OsDateTimeTYPE formatTime = DbColumn_getOptionNumber(prm.srcColumn, "timeFormat");

		UNI* str = 0;
		OsDate date = OsDate_initFromNumber(DbColumn_getFlt(prm.srcColumn, i, 0));
		if (OsDate_is(&date))
		{
			char* monthStr = (formatDate == OsDate_TEXT) ? Std_newCHAR_uni(Lang_find_month(date.m_month)) : 0;
			str = OsDate_getStringDateTimeUNI(&date, formatDate, formatTime, monthStr);
			Std_deleteCHAR(monthStr);
		}

		DbColumnString32_setEqFree((DbColumnString32*)dstColumn, i, str);
	}

	return dstColumn;
}

const DbColumnConvert g_column_converts[] = {
	//{ DbFormat_NUMBER_1, DbFormat_NUMBER_N, &DbColumn_convert_1_N },
	{ DbFormat_NUMBER_1, DbFormat_SLIDER, 0 },
	{ DbFormat_NUMBER_1, DbFormat_CHECK, 0 },
	{ DbFormat_NUMBER_1, DbFormat_CURRENCY, 0 },
	{ DbFormat_NUMBER_1, DbFormat_PERCENTAGE, 0 },
	{ DbFormat_NUMBER_1, DbFormat_RATING, 0 },
	{ DbFormat_NUMBER_1, DbFormat_MENU, &DbColumn_convert_to_menu },
	{ DbFormat_NUMBER_1, DbFormat_TAGS, &DbColumn_convert_to_menu },

	{ DbFormat_NUMBER_1, DbFormat_TEXT, &DbColumn_convert_to_string },
	{ DbFormat_NUMBER_1, DbFormat_URL, &DbColumn_convert_to_string },
	{ DbFormat_NUMBER_1, DbFormat_PHONE, &DbColumn_convert_to_string },
	{ DbFormat_NUMBER_1, DbFormat_EMAIL, &DbColumn_convert_to_string },
	{ DbFormat_NUMBER_1, DbFormat_LOCATION, &DbColumn_convert_to_string },

	{ DbFormat_NUMBER_1, DbFormat_LINK_1, &DbColumn_convert_number_to_link },
	{ DbFormat_NUMBER_1, DbFormat_LINK_N, &DbColumn_convert_number_to_link },

	/*{ DbFormat_NUMBER_N, DbFormat_NUMBER_1, &DbColumn_convert_N_1 },
	{ DbFormat_NUMBER_N, DbFormat_CURRENCY, &DbColumn_convert_N_1 },
	{ DbFormat_NUMBER_N, DbFormat_PERCENTAGE, &DbColumn_convert_N_1 },
	{ DbFormat_NUMBER_N, DbFormat_RATING, &DbColumn_convert_N_1 },
	{ DbFormat_NUMBER_N, DbFormat_MENU, &DbColumn_convert_to_menu },
	{ DbFormat_NUMBER_N, DbFormat_TAGS, &DbColumn_convert_to_menu },

	{ DbFormat_NUMBER_N, DbFormat_TEXT, &DbColumn_convert_to_string },
	{ DbFormat_NUMBER_N, DbFormat_URL, &DbColumn_convert_to_string },
	{ DbFormat_NUMBER_N, DbFormat_PHONE, &DbColumn_convert_to_string },
	{ DbFormat_NUMBER_N, DbFormat_EMAIL, &DbColumn_convert_to_string },
	{ DbFormat_NUMBER_N, DbFormat_LOCATION, &DbColumn_convert_to_string },

	{ DbFormat_NUMBER_N, DbFormat_LINK_1, &DbColumn_convert_number_to_link },
	{ DbFormat_NUMBER_N, DbFormat_LINK_N, &DbColumn_convert_number_to_link },*/

	{ DbFormat_FILE_1, DbFormat_FILE_N, &DbColumn_convert_1_N },
	{ DbFormat_FILE_N, DbFormat_FILE_1, &DbColumn_convert_N_1 },

	{ DbFormat_LINK_1, DbFormat_LINK_N, &DbColumn_convert_1_N },
	{ DbFormat_LINK_1, DbFormat_TEXT, &DbColumn_convert_to_string },

	{ DbFormat_LINK_N, DbFormat_LINK_1, &DbColumn_convert_N_1 },
	{ DbFormat_LINK_N, DbFormat_TEXT, &DbColumn_convert_to_string },

	{ DbFormat_LINK_MIRRORED, DbFormat_LINK_N, 0},
	{ DbFormat_LINK_JOINTED, DbFormat_LINK_N, 0},
	{ DbFormat_LINK_FILTERED, DbFormat_LINK_N, 0},

	{ DbFormat_MENU, DbFormat_TAGS, 0 },
	{ DbFormat_MENU, DbFormat_TEXT, &DbColumn_convert_menu_to_string },
	{ DbFormat_MENU, DbFormat_URL, &DbColumn_convert_menu_to_string },
	{ DbFormat_MENU, DbFormat_PHONE, &DbColumn_convert_menu_to_string },
	{ DbFormat_MENU, DbFormat_EMAIL, &DbColumn_convert_menu_to_string },
	{ DbFormat_MENU, DbFormat_LOCATION, &DbColumn_convert_menu_to_string },

	{ DbFormat_TAGS, DbFormat_MENU, 0 },
	{ DbFormat_TAGS, DbFormat_TEXT, &DbColumn_convert_menu_to_string },
	{ DbFormat_TAGS, DbFormat_URL, &DbColumn_convert_menu_to_string },
	{ DbFormat_TAGS, DbFormat_PHONE, &DbColumn_convert_menu_to_string },
	{ DbFormat_TAGS, DbFormat_EMAIL, &DbColumn_convert_menu_to_string },
	{ DbFormat_TAGS, DbFormat_LOCATION, &DbColumn_convert_menu_to_string },

	{ DbFormat_SLIDER, DbFormat_NUMBER_1, 0 },
	//{ DbFormat_SLIDER, DbFormat_NUMBER_N, &DbColumn_convert_1_N },
	{ DbFormat_SLIDER, DbFormat_MENU, &DbColumn_convert_to_menu },
	{ DbFormat_SLIDER, DbFormat_TAGS, &DbColumn_convert_to_menu },

	{ DbFormat_SLIDER, DbFormat_TEXT, &DbColumn_convert_to_string },
	{ DbFormat_SLIDER, DbFormat_URL, &DbColumn_convert_to_string },
	{ DbFormat_SLIDER, DbFormat_PHONE, &DbColumn_convert_to_string },
	{ DbFormat_SLIDER, DbFormat_EMAIL, &DbColumn_convert_to_string },
	{ DbFormat_SLIDER, DbFormat_LOCATION, &DbColumn_convert_to_string },

	{ DbFormat_CHECK, DbFormat_NUMBER_1, 0 },
	//{ DbFormat_CHECK, DbFormat_NUMBER_N, &DbColumn_convert_1_N },
	{ DbFormat_CHECK, DbFormat_MENU, &DbColumn_convert_to_menu },
	{ DbFormat_CHECK, DbFormat_TAGS, &DbColumn_convert_to_menu },

	{ DbFormat_CHECK, DbFormat_TEXT, &DbColumn_convert_to_string },
	{ DbFormat_CHECK, DbFormat_URL, &DbColumn_convert_to_string },
	{ DbFormat_CHECK, DbFormat_PHONE, &DbColumn_convert_to_string },
	{ DbFormat_CHECK, DbFormat_EMAIL, &DbColumn_convert_to_string },
	{ DbFormat_CHECK, DbFormat_LOCATION, &DbColumn_convert_to_string },

	{ DbFormat_CURRENCY, DbFormat_NUMBER_1, 0 },
	//{ DbFormat_CURRENCY, DbFormat_NUMBER_N, &DbColumn_convert_1_N },
	{ DbFormat_CURRENCY, DbFormat_MENU, &DbColumn_convert_to_menu },
	{ DbFormat_CURRENCY, DbFormat_TAGS, &DbColumn_convert_to_menu },

	{ DbFormat_CURRENCY, DbFormat_TEXT, &DbColumn_convert_to_string },
	{ DbFormat_CURRENCY, DbFormat_URL, &DbColumn_convert_to_string },
	{ DbFormat_CURRENCY, DbFormat_PHONE, &DbColumn_convert_to_string },
	{ DbFormat_CURRENCY, DbFormat_EMAIL, &DbColumn_convert_to_string },
	{ DbFormat_CURRENCY, DbFormat_LOCATION, &DbColumn_convert_to_string },

	{ DbFormat_PERCENTAGE, DbFormat_NUMBER_1, 0 },
	//{ DbFormat_PERCENTAGE, DbFormat_NUMBER_N, &DbColumn_convert_1_N },
	{ DbFormat_PERCENTAGE, DbFormat_MENU, &DbColumn_convert_to_menu},
	{ DbFormat_PERCENTAGE, DbFormat_TAGS, &DbColumn_convert_to_menu },

	{ DbFormat_PERCENTAGE, DbFormat_TEXT, &DbColumn_convert_to_string },
	{ DbFormat_PERCENTAGE, DbFormat_URL, &DbColumn_convert_to_string },
	{ DbFormat_PERCENTAGE, DbFormat_PHONE, &DbColumn_convert_to_string },
	{ DbFormat_PERCENTAGE, DbFormat_EMAIL, &DbColumn_convert_to_string },
	{ DbFormat_PERCENTAGE, DbFormat_LOCATION, &DbColumn_convert_to_string },

	{ DbFormat_RATING, DbFormat_NUMBER_1, 0 },
	//{ DbFormat_RATING, DbFormat_NUMBER_N, &DbColumn_convert_1_N },
	{ DbFormat_RATING, DbFormat_MENU, &DbColumn_convert_to_menu },
	{ DbFormat_RATING, DbFormat_TAGS, &DbColumn_convert_to_menu },
	{ DbFormat_RATING, DbFormat_TEXT, &DbColumn_convert_to_string },
	{ DbFormat_RATING, DbFormat_URL, &DbColumn_convert_to_string },
	{ DbFormat_RATING, DbFormat_PHONE, &DbColumn_convert_to_string },
	{ DbFormat_RATING, DbFormat_EMAIL, &DbColumn_convert_to_string },
	{ DbFormat_RATING, DbFormat_LOCATION, &DbColumn_convert_to_string },

	{ DbFormat_DATE, DbFormat_TEXT, &DbColumn_convert_date_to_string },

	{ DbFormat_TEXT, DbFormat_URL, 0 },
	{ DbFormat_TEXT, DbFormat_PHONE, 0 },
	{ DbFormat_TEXT, DbFormat_EMAIL, 0 },
	{ DbFormat_TEXT, DbFormat_LOCATION, 0 },
	{ DbFormat_TEXT, DbFormat_NUMBER_1, &DbColumn_convert_string_to_number1 },
	{ DbFormat_TEXT, DbFormat_CURRENCY, &DbColumn_convert_string_to_number1 },
	{ DbFormat_TEXT, DbFormat_PERCENTAGE, &DbColumn_convert_string_to_number1 },
	{ DbFormat_TEXT, DbFormat_RATING, &DbColumn_convert_string_to_number1 },
	{ DbFormat_TEXT, DbFormat_SLIDER, &DbColumn_convert_string_to_number1 },
	{ DbFormat_TEXT, DbFormat_CHECK, &DbColumn_convert_string_to_number1 },
	{ DbFormat_TEXT, DbFormat_MENU, &DbColumn_convert_to_menu },
	{ DbFormat_TEXT, DbFormat_TAGS, &DbColumn_convert_to_tags },
	{ DbFormat_TEXT, DbFormat_NUMBER_1, &DbColumn_convert_string_to_number1 },
	{ DbFormat_TEXT, DbFormat_DATE, &DbColumn_convert_string_to_date },
	{ DbFormat_TEXT, DbFormat_LINK_1, &DbColumn_convert_string_to_link },
	{ DbFormat_TEXT, DbFormat_LINK_N, &DbColumn_convert_string_to_link },

	{ DbFormat_URL, DbFormat_TEXT, 0 },
	{ DbFormat_URL, DbFormat_PHONE, 0 },
	{ DbFormat_URL, DbFormat_EMAIL, 0 },
	{ DbFormat_URL, DbFormat_LOCATION, 0 },

	{ DbFormat_PHONE, DbFormat_TEXT, 0 },
	{ DbFormat_PHONE, DbFormat_URL, 0 },
	{ DbFormat_PHONE, DbFormat_EMAIL, 0 },
	{ DbFormat_PHONE, DbFormat_LOCATION, 0 },

	{ DbFormat_EMAIL, DbFormat_TEXT, 0 },
	{ DbFormat_EMAIL, DbFormat_URL, 0 },
	{ DbFormat_EMAIL, DbFormat_PHONE, 0 },
	{ DbFormat_EMAIL, DbFormat_LOCATION, 0 },

	{ DbFormat_LOCATION, DbFormat_TEXT, 0 },
	{ DbFormat_LOCATION, DbFormat_URL, 0 },
	{ DbFormat_LOCATION, DbFormat_PHONE, 0 },
	{ DbFormat_LOCATION, DbFormat_EMAIL, 0 },

	{ DbFormat_SUMMARY, DbFormat_NUMBER_1, 0 },
};

static UBIG _DbColumnConvert_numAll(void)
{
	return sizeof(g_column_converts) / sizeof(DbColumnConvert);
}

UBIG DbColumnConvert_num(DbFormatTYPE srcType, BOOL sameColType)
{
	DbColumnTYPE colType = DbColumnFormat_findColumnType(srcType);

	UBIG n = 0;
	BIG i;
	for (i = 0; i < _DbColumnConvert_numAll(); i++)
		if (g_column_converts[i].srcType == srcType && (!sameColType || DbColumnFormat_findColumnType(g_column_converts[i].dstType) == colType))
			n++;
	return n;
}
const DbColumnConvert* DbColumnConvert_get(DbFormatTYPE srcType, BIG index, BOOL sameColType)
{
	DbColumnTYPE colType = DbColumnFormat_findColumnType(srcType);

	BIG i;
	for (i = 0; i < _DbColumnConvert_numAll(); i++)
	{
		if (g_column_converts[i].srcType == srcType && (!sameColType || DbColumnFormat_findColumnType(g_column_converts[i].dstType) == colType))
		{
			if (index == 0)
				return &g_column_converts[i];
			index--;
		}
	}
	return 0;
}

const DbColumnConvert* DbColumnConvert_find(DbFormatTYPE srcType, DbFormatTYPE dstType)
{
	BIG i;
	for (i = 0; i < _DbColumnConvert_numAll(); i++)
	{
		if (g_column_converts[i].srcType == srcType && g_column_converts[i].dstType == dstType)
			return &g_column_converts[i];
	}
	return 0;
}

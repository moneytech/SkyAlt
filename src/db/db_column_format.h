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

typedef struct DbColumnFormat_s
{
	DbFormatTYPE formatType;
	DbColumnTYPE columnType;
	const char* lang;
	const char* name;
}DbColumnFormat;

const DbColumnFormat g_column_formats[] = {
	{ DbFormat_NUMBER_1, DbColumn_1, "COLUMN_NUMBER_1", "number" },
	{ DbFormat_NUMBER_N, DbColumn_N, "COLUMN_NUMBER_N", "number" },

	{ DbFormat_CURRENCY, DbColumn_1, "COLUMN_CURRENCY", "currency" },
	{ DbFormat_PERCENTAGE, DbColumn_1, "COLUMN_PERCENTAGE", "percentage" },
	{ DbFormat_RATING, DbColumn_1, "COLUMN_RATING", "rating" },
	{ DbFormat_SLIDER, DbColumn_1, "COLUMN_SLIDER", "slider" },
	{ DbFormat_CHECK, DbColumn_1,"COLUMN_CHECK",  "check" },

	{ DbFormat_MENU, DbColumn_1, "COLUMN_MENU", "menu" },
	{ DbFormat_TAGS, DbColumn_N, "COLUMN_TAGS", "tags" },
	{ DbFormat_LINK_1, DbColumn_1, "COLUMN_LINK_1", "link" },
	{ DbFormat_LINK_N, DbColumn_N, "COLUMN_LINK_N", "link" },

	{ DbFormat_FILE_1, DbColumn_1, "COLUMN_FILE_1", "file" },
	{ DbFormat_FILE_N, DbColumn_N, "COLUMN_FILE_N", "file" },

	{ DbFormat_TEXT, DbColumn_STRING_32, "COLUMN_TEXT", "text" },
	{ DbFormat_PHONE, DbColumn_STRING_32, "COLUMN_PHONE", "phone" },
	{ DbFormat_URL, DbColumn_STRING_32, "COLUMN_URL", "url" },
	{ DbFormat_EMAIL, DbColumn_STRING_32, "COLUMN_EMAIL", "email" },
	{ DbFormat_LOCATION, DbColumn_STRING_32, "COLUMN_LOCATION", "location" },

	{ DbFormat_DATE, DbColumn_1, "COLUMN_DATE", "date" },
};

static UBIG _DbColumnFormat_numAll(void)
{
	return sizeof(g_column_formats) / sizeof(DbColumnFormat);
}

const char* DbColumnFormat_findColumnLang(DbFormatTYPE format)
{
	BIG i;
	for (i = 0; i < _DbColumnFormat_numAll(); i++)
	{
		if (g_column_formats[i].formatType == format)
			return g_column_formats[i].lang;
	}
	return 0;
}

DbColumnTYPE DbColumnFormat_findColumnType(DbFormatTYPE format)
{
	BIG i;
	for (i = 0; i < _DbColumnFormat_numAll(); i++)
	{
		if (g_column_formats[i].formatType == format)
			return g_column_formats[i].columnType;
	}
	return 0;
}
const char* DbColumnFormat_findColumnName(DbFormatTYPE format)
{
	BIG i;
	for (i = 0; i < _DbColumnFormat_numAll(); i++)
	{
		if (g_column_formats[i].formatType == format)
			return g_column_formats[i].name;
	}
	return 0;
}

DbFormatTYPE DbColumnFormat_findColumn(const DbColumn* column)
{
	if (DbTable_getIdsColumn(column->parent->parent) == column)
		return DbFormat_ROW;

	return column->format;
}
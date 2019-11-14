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

typedef struct DbRoot_s
{
	DbTable* info;
	DbColumnString32* info_options;
	DbColumnN* info_subs;

	DbTables* tables;
	DbTables* tables_notSaved;

	UBIG numChanges;

	StdArr filters;
} DbRoot;

DbRoot* g_DbRoot = 0;
BOOL g_DbRoot_loaded = FALSE;
volatile StdProgress g_DbRoot_progress;

void DbRoot_initProgress(void)
{
	g_DbRoot_progress = StdProgress_init();
}
void DbRoot_freeProgress(void)
{
	StdProgress_free((StdProgress*)&g_DbRoot_progress);
}

volatile StdProgress* DbRoot_getProgress(void)
{
	return &g_DbRoot_progress;
}

BOOL DbRoot_is(void)
{
	return g_DbRoot_loaded;
}

BIG DbRoot_getRootRow(void)
{
	return 0;
}

DbTables* DbRoot_getTables(void)
{
	return g_DbRoot->tables;
}

DbTable* DbRoot_getInfoTable(void)
{
	return g_DbRoot->info;
}

DbColumnString32* DbRoot_getColumnOptions(void)
{
	return g_DbRoot->info_options;
}
DbColumnN* DbRoot_getColumnSubs(void)
{
	return g_DbRoot->info_subs;
}

FileRow DbRoot_getFileId(BIG row)
{
	return DbColumn1_getFileId(DbTable_getColumnRows(g_DbRoot->info), row);
}

UNI* _DbRoot_getOptionString(const BIG row, const char* name, const UNI* defValue, UNI* out, const UBIG outMaxSize)
{
	DbColumnString32_getOption(g_DbRoot->info_options, row, name, defValue, out, outMaxSize);
	return out;
}

double _DbRoot_getOptionNumber(const BIG row, const char* name, double defValue)
{
	UNI def[64];
	Std_buildNumberUNI(defValue, -1, def);

	UNI result[64];
	_DbRoot_getOptionString(row, name, def, result, 64);
	return Std_getNumberFromUNI(result);
}

void _DbRoot_setOptionString(const BIG row, const char* name, const UNI* value)
{
	if (row >= 0)
		DbColumnString32_setOption(g_DbRoot->info_options, row, name, value);
}

void _DbRoot_setOptionNumber(const BIG row, const char* name, double value)
{
	UNI str[64];
	Std_buildNumberUNI(value, -1, str);
	_DbRoot_setOptionString(row, name, str);
}

BOOL _DbRoot_cmpOptionString(const BIG row, const char* name, const UNI* defValue, const UNI* compare)
{
	UNI tmp[64];
	_DbRoot_getOptionString(row, name, defValue, tmp, 64);
	return Std_cmpUNI(tmp, compare);
}
BOOL _DbRoot_cmpOptionStringCHAR(const BIG row, const char* name, const UNI* defValue, const char* compare)
{
	UNI tmp[64];
	_DbRoot_getOptionString(row, name, defValue, tmp, 64);
	return Std_cmpUNI_CHAR(tmp, compare);
}

BOOL DbRoot_cmpName(BIG row, const UNI* compare)
{
	return _DbRoot_cmpOptionString(row, "name", 0, compare);
}
UNI* DbRoot_getName(BIG row, UNI* out, const UBIG outMaxSize)
{
	return _DbRoot_getOptionString(row, "name", 0, out, outMaxSize);
}
void DbRoot_setName(BIG row, const UNI* value)
{
	_DbRoot_setOptionString(row, "name", value);
}

static BOOL _DbRoot_cmpType(BIG row, const char* compare)
{
	return _DbRoot_cmpOptionStringCHAR(row, "type", 0, compare);
}
static void _DbRoot_setType(BIG row, const char* type)
{
	UNI* t = Std_newUNI_char(type);
	_DbRoot_setOptionString(row, "type", t);
	Std_deleteUNI(t);
}

static BOOL _DbRoot_isEnable(BIG row)
{
	return _DbRoot_getOptionNumber(row, "enable", 1) > 0;
}

static BOOL _DbRoot_isAscending(BIG row)
{
	return _DbRoot_getOptionNumber(row, "ascending", 0) == 0;
}
static BOOL _DbRoot_isAND(BIG row)
{
	return _DbRoot_getOptionNumber(row, "ascending", 0) == 0;
}

BIG DbRoot_getPanelLeft(void)
{
	UBIG i = 0;
	while (DbTable_jumpRows(g_DbRoot->info, &i, 1) >= 0)
	{
		if (_DbRoot_cmpOptionString(i, "panel_left", 0, _UNI32("1")))
			return i;
		i++;
	}
	return -1;
}
BIG DbRoot_getPanelRight(void)
{
	UBIG i = 0;
	while (DbTable_jumpRows(g_DbRoot->info, &i, 1) >= 0)
	{
		if (_DbRoot_cmpOptionString(i, "panel_right", 0, _UNI32("1")))
			return i;
		i++;
	}
	return -1;
}

void DbRoot_setPanelLeft(BIG row)
{
	BIG oldRow = DbRoot_getPanelLeft();
	if (oldRow != row)
	{
		_DbRoot_setOptionNumber(oldRow, "panel_left", 0);
		_DbRoot_setOptionNumber(row, "panel_left", 1);
	}
}
void DbRoot_setPanelRight(BIG row)
{
	BIG oldRow = DbRoot_getPanelRight();
	if (oldRow != row)
	{
		_DbRoot_setOptionNumber(oldRow, "panel_right", 0);
		_DbRoot_setOptionNumber(row, "panel_right", 1);
	}
}

void DbRoot_swapPanelRight(void)
{
	BIG viewLeft = DbRoot_getPanelLeft();
	BIG viewRight = DbRoot_getPanelRight();

	double scroolLeft = _DbRoot_getOptionNumber(viewLeft, "scrollL", 0);
	double scroolLRight = _DbRoot_getOptionNumber(viewLeft, "scrollR", 0);

	DbRoot_setPanelLeft(viewRight);
	DbRoot_setPanelRight(viewLeft);

	_DbRoot_setOptionNumber(viewLeft, "scrollL", scroolLRight);
	_DbRoot_setOptionNumber(viewLeft, "scrollR", scroolLeft);
}

BOOL DbRoot_isType_root(const BIG row)
{
	return DbRoot_getRootRow() == row || _DbRoot_cmpType(row, "root");
}
BOOL DbRoot_isType_folder(const BIG row)
{
	return _DbRoot_cmpType(row, "folder");
}
BOOL DbRoot_isType_table(const BIG row)
{
	return _DbRoot_cmpType(row, "table");
}
BOOL DbRoot_isType_page(const BIG row)
{
	return _DbRoot_cmpType(row, "page");
}

BOOL DbRoot_isTypeView_filter(const BIG row)
{
	return _DbRoot_cmpType(row, "filter");
}
BOOL DbRoot_isTypeView_cards(const BIG row)
{
	return _DbRoot_cmpType(row, "cards");
}
BOOL DbRoot_isTypeView_group(const BIG row)
{
	return _DbRoot_cmpType(row, "groups");
}
BOOL DbRoot_isTypeView_kanban(const BIG row)
{
	return _DbRoot_cmpType(row, "kanban");
}
BOOL DbRoot_isTypeView_calendar(const BIG row)
{
	return _DbRoot_cmpType(row, "calendar");
}
BOOL DbRoot_isTypeView_timeline(const BIG row)
{
	return _DbRoot_cmpType(row, "timeline");
}
BOOL DbRoot_isTypeView_chart(const BIG row)
{
	return _DbRoot_cmpType(row, "chart");
}
BOOL DbRoot_isTypeView_map(const BIG row)
{
	return _DbRoot_cmpType(row, "map");
}
BOOL DbRoot_isTypeView(const BIG row)
{
	return DbRoot_isTypeView_filter(row) ||
		DbRoot_isTypeView_cards(row) ||
		DbRoot_isTypeView_group(row) ||
		DbRoot_isTypeView_kanban(row) ||
		DbRoot_isTypeView_calendar(row) ||
		DbRoot_isTypeView_timeline(row) ||
		DbRoot_isTypeView_chart(row) ||
		DbRoot_isTypeView_map(row);
}

BOOL DbRoot_isType_column(const BIG row)
{
	BOOL is = FALSE;
	BIG parent = DbRoot_findParent(row);
	if (parent >= 0)
	{
		BIG parentTable = DbRoot_findParent(parent);
		if (parentTable >= 0 && DbRoot_isType_table(parentTable))
			is = _DbRoot_cmpType(parent, "columns");
	}
	return is;
}

static BIG _DbRoot_createView(const BIG parentRow, const char* type)
{
	UBIG r = DbTable_addRow(g_DbRoot->info);
	_DbRoot_setType(r, type);
	DbColumnN_add(g_DbRoot->info_subs, parentRow, r);
	return r;
}

BIG DbRoot_createView_filter(const BIG parentRow)
{
	BIG r = _DbRoot_createView(parentRow, "filter");

	//subs
	{
		UBIG columnsRow = DbTable_addRow(g_DbRoot->info);
		UBIG viewsRow = DbTable_addRow(g_DbRoot->info);

		_DbRoot_setType(columnsRow, "columns");
		_DbRoot_setType(viewsRow, "views");

		DbColumnN_add(g_DbRoot->info_subs, r, columnsRow);
		DbColumnN_add(g_DbRoot->info_subs, r, viewsRow);
	}

	//filters
	{
		UBIG shortRow = DbTable_addRow(g_DbRoot->info);
		_DbRoot_setType(shortRow, "short");
		DbColumnN_add(g_DbRoot->info_subs, r, shortRow);

		UBIG selectRow = DbTable_addRow(g_DbRoot->info);
		_DbRoot_setType(selectRow, "select");
		DbColumnN_add(g_DbRoot->info_subs, r, selectRow);
	}

	//_DbRoot_updateTables();

	return r;
}
BIG DbRoot_createView_cards(const BIG parentRow)
{
	BIG r = _DbRoot_createView(parentRow, "cards");

	//columns
	{
		UBIG columnsRow = DbTable_addRow(g_DbRoot->info);
		_DbRoot_setType(columnsRow, "columns");
		DbColumnN_add(g_DbRoot->info_subs, r, columnsRow);
	}

	//_DbRoot_updateTables();
	return r;
}
BIG DbRoot_createView_group(const BIG parentRow)
{
	BIG r = _DbRoot_createView(parentRow, "groups");

	//lanes
	{
		UBIG lanesRow = DbTable_addRow(g_DbRoot->info);
		_DbRoot_setType(lanesRow, "group");
		DbColumnN_add(g_DbRoot->info_subs, r, lanesRow);
	}

	//_DbRoot_updateTables();
	return r;
}
BIG DbRoot_createView_kanban(const BIG parentRow)
{
	BIG r = _DbRoot_createView(parentRow, "kanban");
	return r;
}
BIG DbRoot_createView_calendar(const BIG parentRow)
{
	BIG r = _DbRoot_createView(parentRow, "calendar");
	return r;
}
BIG DbRoot_createView_timeline(const BIG parentRow)
{
	BIG r = _DbRoot_createView(parentRow, "timeline");
	return r;
}
BIG DbRoot_createView_chart(const BIG parentRow)
{
	BIG r = _DbRoot_createView(parentRow, "chart");
	return r;
}
BIG DbRoot_createView_map(const BIG parentRow)
{
	BIG r = _DbRoot_createView(parentRow, "map");
	return r;
}

static const char* DbRoot_getColumnTypeName(DbColumnTYPE type)
{
	switch (type)
	{
		case DbColumn_1:			return "column_1";
		case DbColumn_N:			return "column_n";
		case DbColumn_STRING_32:	return "string_32";
	}

	return 0;
}

static BIG DbRoot_getColumnType(BIG r)
{
	if (_DbRoot_cmpType(r, "column_1"))		return DbColumn_1;
	if (_DbRoot_cmpType(r, "column_n"))		return DbColumn_N;
	if (_DbRoot_cmpType(r, "string_32"))	return DbColumn_STRING_32;

	return -1;
}

BIG DbRoot_findParent(UBIG findRow)
{
	UBIG i = 0;
	while (DbTable_jumpRows(g_DbRoot->info, &i, 1) >= 0)
	{
		if (DbColumnN_findBig(g_DbRoot->info_subs, i, findRow) >= 0)
			return i;
		i++;
	}
	return -1;
}

BOOL DbRoot_isParentRow(BIG row, BIG findParentRow)
{
	if (row == findParentRow)
		return TRUE;

	row = DbRoot_findParent(row);
	if (row >= 0)
		return DbRoot_isParentRow(row, findParentRow);

	return FALSE;
}

DbTable* DbRoot_findParentTableNoLoad(BIG row)
{
	if (row < 0)
		return 0;

	DbTable* table = DbTables_find(g_DbRoot->tables, DbRoot_getFileId(row));
	if (table)
		return table;
	return DbRoot_findParentTableNoLoad(DbRoot_findParent(row));
}

DbTable* DbRoot_findParentTable(BIG row)
{
	DbTable* table = DbRoot_findParentTableNoLoad(row);
	if (table)
		DbTable_loadLast(table);
	return table;
}

BIG DbRoot_findParentTableRow(BIG row)	//doesn't load table
{
	if (row < 0)
		return -1;

	DbTable* table = DbTables_find(g_DbRoot->tables, DbRoot_getFileId(row));
	if (table)
		return DbTable_getRow(table);

	return DbRoot_findParentTableRow(DbRoot_findParent(row));
}

BIG DbRoot_findParentType(UBIG row, const char* typeValue)
{
	if (_DbRoot_cmpType(row, typeValue))
		return row;

	BIG r = DbRoot_findParent(row);
	if (r >= 0)
		return DbRoot_findParentType(r, typeValue);

	return -1;
}

BIG DbRoot_findChildType(UBIG row, const char* typeValue)
{
	if (_DbRoot_cmpType(row, typeValue))
		return row;

	BIG it;
	UBIG i = 0;
	while ((it = DbColumnN_jump(g_DbRoot->info_subs, row, &i, 1)) >= 0)
	{
		if (_DbRoot_cmpType(it, typeValue))
			return it;
		i++;
	}

	return -1;
}

UBIG DbRoot_findOrCreateChildType(UBIG row, const char* typeValue)
{
	BIG sub = DbRoot_findChildType(row, typeValue);
	if (sub < 0)
	{
		//add
		sub = DbTable_addRow(g_DbRoot->info);
		_DbRoot_setType(sub, typeValue);
		DbColumnN_add(g_DbRoot->info_subs, row, sub);
	}
	return sub;
}

static void _DbRoot_createColumnRow(DbColumn* column)
{
	DbTable* parentTable = DbColumn_getTable(column);
	if (!parentTable || parentTable->parent == g_DbRoot->tables_notSaved)
		return;

	if (!parentTable->rows)	//not row column
		return;

	//new row
	UBIG r = DbTable_addRow(g_DbRoot->info);
	column->fileId = DbRoot_getFileId(r);
	_DbRoot_setType(r, DbRoot_getColumnTypeName(column->type));

	//for menu or tags
	UBIG optionsRow = DbTable_addRow(g_DbRoot->info);
	_DbRoot_setType(optionsRow, "options");
	DbColumnN_add(g_DbRoot->info_subs, r, optionsRow);

	//set btable
	if ((column->type == DbColumn_1 || column->type == DbColumn_N) && DbColumn_getBTable(column))
	{
		UBIG btableSetRow = DbTable_addRow(g_DbRoot->info);
		_DbRoot_setType(btableSetRow, "btable");
		DbColumnN_add(g_DbRoot->info_subs, r, btableSetRow);

		DbColumnN_add(g_DbRoot->info_subs, btableSetRow, DbTable_getRow(DbColumn_getBTable(column)));
	}

	//add to parent Table
	DbColumnN_add(g_DbRoot->info_subs, DbRoot_getRow(parentTable->columns->fileId), r);
}

static void _DbRoot_createTableRow(DbTable* table, BIG parentRow)
{
	if (!table || table->parent == g_DbRoot->tables_notSaved)
		return;

	UBIG r = DbTable_addRow(g_DbRoot->info);
	table->fileId = DbRoot_getFileId(r);
	_DbRoot_setType(r, "table");

	//columns(views)
	{
		UBIG columnsRow = DbTable_addRow(g_DbRoot->info);
		UBIG viewsRow = DbTable_addRow(g_DbRoot->info);

		_DbRoot_setType(columnsRow, "columns");
		_DbRoot_setType(viewsRow, "views");

		DbColumnN_add(g_DbRoot->info_subs, r, columnsRow);
		DbColumnN_add(g_DbRoot->info_subs, r, viewsRow);

		table->columns->fileId = DbRoot_getFileId(columnsRow);
	}

	//short
	{
		UBIG shortRow = DbTable_addRow(g_DbRoot->info);
		_DbRoot_setType(shortRow, "short");
		DbColumnN_add(g_DbRoot->info_subs, r, shortRow);
	}

	parentRow = DbRoot_findParentType(parentRow, "folder");

	//add to parent Folder
	DbColumnN_add(g_DbRoot->info_subs, parentRow, r);
}

BIG DbRoot_createPageRow(BIG parentRow)
{
	UBIG r = DbTable_addRow(g_DbRoot->info);
	_DbRoot_setType(r, "page");

	//add to parent Folder
	parentRow = DbRoot_findParentType(parentRow, "folder");

	DbColumnN_add(g_DbRoot->info_subs, parentRow, r);
	return r;
}

BIG DbRoot_createFolderRow(void)
{
	UBIG r = DbTable_addRow(g_DbRoot->info);
	_DbRoot_setType(r, "folder");
	DbRoot_setName(r, Lang_find("FOLDER"));

	//add to parent Root
	DbColumnN_add(g_DbRoot->info_subs, DbRoot_getRootRow(), r);

	return r;
}

DbTable* DbRoot_addTableNotSave(void)
{
	return DbTables_add(g_DbRoot->tables_notSaved, FileRow_initEmpty());
}

static void _DbRoot_createInfo(void)
{
	g_DbRoot->info = 0;	//note on next row, DbTable_new(), checks if info exist!
	g_DbRoot->info = DbTable_new(FileRow_initEmpty(), 0);
	g_DbRoot->info->fileId = FileRow_init(1);

	g_DbRoot->info_options = DbColumns_addColumnString32(g_DbRoot->info->columns);
	g_DbRoot->info_subs = DbColumns_addColumnN(g_DbRoot->info->columns, g_DbRoot->info);

	g_DbRoot->info_subs->base.fileId = FileRow_init(1);
	g_DbRoot->info_options->base.fileId = FileRow_init(2);
}

void DbRoot_delete(void)
{
	g_DbRoot_loaded = FALSE;

	if (g_DbRoot)
	{
		StdArr_freeFn(&g_DbRoot->filters, (StdArrFREE)&DbFilter_delete);

		DbTables_delete(g_DbRoot->tables);
		DbTables_delete(g_DbRoot->tables_notSaved);
		DbTable_delete(g_DbRoot->info);

		Os_free(g_DbRoot, sizeof(DbRoot));
		g_DbRoot = 0;
	}
}

BOOL DbRoot_new(void)
{
	if (g_DbRoot)
		DbRoot_delete();

	g_DbRoot = Os_malloc(sizeof(DbRoot));

	g_DbRoot->numChanges = 0;

	g_DbRoot->tables = DbTables_new();
	g_DbRoot->tables_notSaved = DbTables_new();

	g_DbRoot->filters = StdArr_init();

	_DbRoot_createInfo();
	return TRUE;
}

static void _DbRoot_updateView(BIG viewsRow)
{
	DbTable* table = DbRoot_findParentTableNoLoad(viewsRow);

	BIG viewRow;
	UBIG i3 = 0;
	while ((viewRow = DbColumnN_jump(g_DbRoot->info_subs, viewsRow, &i3, 1)) >= 0)
	{
		BIG viewSubRow;
		UBIG i4 = 0;
		while ((viewSubRow = DbColumnN_jump(g_DbRoot->info_subs, viewRow, &i4, 1)) >= 0)
		{
			if (_DbRoot_cmpType(viewSubRow, "views"))
			{
				_DbRoot_updateView(viewSubRow);
			}
			else
				if (_DbRoot_cmpType(viewSubRow, "columns"))
				{
					BIG ii;
					for (ii = 0; ii < DbColumns_num(table->columns); ii++)
					{
						DbColumn* col = DbColumns_get(table->columns, ii);
						if (col == &table->rows->base)
							continue;

						BIG crow = DbColumn_getRow(col);

						BOOL found = FALSE;
						BIG columnRow;
						UBIG i5 = 0;
						while ((columnRow = DbColumnN_jump(g_DbRoot->info_subs, viewSubRow, &i5, 1)) >= 0)
						{
							BIG bColumnRow = DbColumnN_getFirstRow(g_DbRoot->info_subs, columnRow);
							if (bColumnRow < 0)
								_DbRoot_removeRowInner(columnRow);
							else
								if (bColumnRow == crow)
									found = TRUE;
							i5++;
						}

						if (!found)
						{
							//new row
							UBIG r = DbTable_addRow(g_DbRoot->info);
							_DbRoot_setOptionNumber(r, "width", _DbRoot_getOptionNumber(crow, "width", 8));
							DbColumnN_add(g_DbRoot->info_subs, r, crow);

							DbColumnN_add(g_DbRoot->info_subs, viewSubRow, r);
						}
					}
				}

			i4++;
		}
		i3++;
	}
}

DbFormatTYPE _DbRoot_getFormat(BIG columnRow)
{
	BIG columnType = DbRoot_getColumnType(columnRow);

	BIG i;
	for (i = 0; i < _DbColumnFormat_numAll(); i++)
	{
		UNI str[32];
		_DbRoot_getOptionString(columnRow, "format", 0, str, 32);

		if (g_column_formats[i].columnType == columnType && Std_cmpUNI_CHAR(str, g_column_formats[i].name))
			return g_column_formats[i].formatType;
	}

	return DbFormat_ROW;
}

void DbRoot_updateTables(void)
{
	BIG rootRow = DbTable_firstRow(g_DbRoot->info);

	//update columns
	BIG folderRow;
	UBIG i0 = 0;
	while ((folderRow = DbColumnN_jump(g_DbRoot->info_subs, rootRow, &i0, 1)) >= 0)
	{
		if (_DbRoot_cmpType(folderRow, "folder"))
		{
			BIG tableRow;
			UBIG i1 = 0;
			while ((tableRow = DbColumnN_jump(g_DbRoot->info_subs, folderRow, &i1, 1)) >= 0)
			{
				if (_DbRoot_cmpType(tableRow, "table"))
				{
					DbTable* table = DbTables_findOrAdd(g_DbRoot->tables, DbRoot_getFileId(tableRow));

					BIG tableSubRow;
					UBIG i2 = 0;
					while ((tableSubRow = DbColumnN_jump(g_DbRoot->info_subs, tableRow, &i2, 1)) >= 0)
					{
						if (_DbRoot_cmpType(tableSubRow, "columns"))
						{
							table->columns->fileId = DbRoot_getFileId(tableSubRow);

							BIG columnRow;
							UBIG i3 = 0;
							while ((columnRow = DbColumnN_jump(g_DbRoot->info_subs, tableSubRow, &i3, 1)) >= 0)
							{
								BIG columnType = DbRoot_getColumnType(columnRow);
								if (columnType >= 0)
								{
									DbColumns_findOrAddUnloadType(table->columns, DbRoot_getFileId(columnRow), columnType, _DbRoot_getFormat(columnRow), 0);
								}
								else
									printf("error: Unknown Column type\n");
								i3++;
							}
						}
						else
							if (_DbRoot_cmpType(tableSubRow, "views"))
							{
								_DbRoot_updateView(tableSubRow);
							}
						i2++;
					}
				}
				else
					printf("error: Should be Table\n");
				i1++;
			}
		}
		else
			printf("error: Should be Folder\n");
		i0++;
	}

	//update views columns
	i0 = 0;
	while ((folderRow = DbColumnN_jump(g_DbRoot->info_subs, rootRow, &i0, 1)) >= 0)
	{
		if (_DbRoot_cmpType(folderRow, "folder"))
		{
			BIG tableRow;
			UBIG i1 = 0;
			while ((tableRow = DbColumnN_jump(g_DbRoot->info_subs, folderRow, &i1, 1)) >= 0)
			{
				if (_DbRoot_cmpType(tableRow, "table"))
				{
					DbTables_findOrAdd(g_DbRoot->tables, DbRoot_getFileId(tableRow));

					BIG tableSubRow;
					UBIG i2 = 0;
					while ((tableSubRow = DbColumnN_jump(g_DbRoot->info_subs, tableRow, &i2, 1)) >= 0)
					{
						if (_DbRoot_cmpType(tableSubRow, "views"))
							_DbRoot_updateView(tableSubRow);
						i2++;
					}
				}
				else
					printf("error: Should be Table\n");
				i1++;
			}
		}
		else
			printf("error: Should be Folder\n");
		i0++;
	}

	//update btables(after all are loaded)
	UBIG i, j;
	for (i = 0; i < DbTables_num(g_DbRoot->tables); i++)
	{
		DbTable* table = DbTables_get(g_DbRoot->tables, i);
		for (j = 0; j < DbColumns_num(table->columns); j++)
		{
			DbColumn* column = DbColumns_get(table->columns, j);
			if (column && (column->type == DbColumn_1 || column->type == DbColumn_N))
			{
				BIG columnRow = DbColumn_getRow(column);
				if (columnRow >= 0)
				{
					BIG bTableSetRow = DbRows_findSubType(columnRow, "btable");
					if (bTableSetRow >= 0)
					{
						BIG btableRow = DbColumnN_getFirstRow(g_DbRoot->info_subs, bTableSetRow);
						if (btableRow >= 0)
							DbColumn_setBTable(column, DbTables_find(g_DbRoot->tables, DbRoot_getFileId(btableRow)));
					}
				}

				DbFormatTYPE format = DbColumnFormat_findColumn(column);
				if (format == DbFormat_MENU || format == DbFormat_TAGS)
					DbColumn_setBTable(column, g_DbRoot->info);
			}
		}
	}

	//remove unused tables
	DbTable_checkExists(g_DbRoot->tables);

	//copy tables/columns
	for (i = 0; i < DbTables_num(g_DbRoot->tables); i++)
		DbTable_checkForCopyAsk(DbTables_get(g_DbRoot->tables, i));
}

BOOL DbRoot_newOpen(void)
{
	DbRoot_new();

	DbTable_unloadHard(g_DbRoot->info);
	DbTable_load(g_DbRoot->info, 0);

	if (DbTable_numRowsReal(g_DbRoot->info) == 0)
		return FALSE;

	DbRoot_updateTables();

	g_DbRoot_loaded = TRUE;
	return TRUE;
}

DbTable* DbRoot_createTable(BIG folderRow)
{
	folderRow = DbRoot_findParentType(folderRow, "folder");
	DbTable* table = DbTables_create(g_DbRoot->tables, folderRow);
	DbTable_setName(table, Lang_find("TABLE"));
	return table;
}

DbTable* DbRoot_createTableExample(BIG folderRow)
{
	DbTable* table = DbRoot_createTable(folderRow);

	//example data
	DbColumn* column = DbTable_createColumnFormat(table, DbFormat_NUMBER_1, 0, 0);
	int i;
	for (i = 0; i < 5; i++)
		DbColumn_setFlt(column, DbTable_addRow(table), 0, i);

	return table;
}

void DbRoot_createPage(BIG parent)
{
	//...
}

BOOL DbRoot_newCreate(void)
{
	DbRoot_new();

	BIG rootRow = DbTable_addRow(g_DbRoot->info);	//root
	_DbRoot_setType(rootRow, "root");

	BIG folderRow = DbRoot_createFolderRow();
	DbTable* table = DbRoot_createTableExample(folderRow);

	DbRoot_setPanelLeft(DbTable_getRow(table));

	g_DbRoot_loaded = TRUE;
	return TRUE;
}

DbColumn* DbRoot_findColumn(BIG row)
{
	FileRow id = DbRoot_getFileId(row);
	return DbTables_findColumn(g_DbRoot->tables, id);
}

DbTable* DbRoot_findTable(BIG row)
{
	FileRow id = DbRoot_getFileId(row);
	return DbTables_find(g_DbRoot->tables, id);
}

BIG DbRoot_getRow(FileRow fileId)
{
	return FileRow_is(fileId) ? DbTable_findRow(g_DbRoot->info, fileId) : -1;
}

StdBigs DbRoot_getTableLinks(void)
{
	return DbTables_getLinks(g_DbRoot->tables);
}

void DbRoot_maintenance(void)
{
	DbTables_maintenance(g_DbRoot->tables);
}

static void _DbRoot_removeRowInner(BIG row)
{
	if (row < 0)
		return;

	UBIG i = 0;
	BIG subRow;
	while ((subRow = DbColumnN_jump(g_DbRoot->info_subs, row, &i, 1)) >= 0)
	{
		if (subRow > row)	//only older
			_DbRoot_removeRowInner(subRow);
		i++;
	}
	DbTable_removeRow(g_DbRoot->info, row);
}

void DbRoot_removeRow(BIG row)
{
	_DbRoot_removeRowInner(row);
	//_DbRoot_updateTables();
}

void DbRoot_replaceAndRemoveRow(BIG oldRow, BIG newRow)
{
	UBIG i = 0;
	while (DbTable_jumpRows(g_DbRoot->info, &i, 1) >= 0)
	{
		DbColumnN_remove(g_DbRoot->info_subs, i, newRow);

		DbColumnN_replace(g_DbRoot->info_subs, i, oldRow, newRow);
		i++;
	}

	_DbRoot_removeRowInner(oldRow);
	//_DbRoot_updateTables();
}

BOOL DbRoot_removeTable(DbTable* table)
{
	if (DbTables_findBTable(g_DbRoot->tables, table) || DbTables_findBTable(g_DbRoot->tables_notSaved, table))
		return FALSE;	//can't be remove because link to table still exists

	BOOL removed = FALSE;
	removed |= DbTables_removeRow(g_DbRoot->tables, table);
	removed |= DbTables_removeRow(g_DbRoot->tables_notSaved, table);

	return removed;
}

UBIG _DbRoot_duplicateRowInner(BIG srcRow, BIG parentRow)
{
	UBIG dstRow = DbTable_addRow(g_DbRoot->info);
	DbTable_copyRow(g_DbRoot->info, dstRow, srcRow);
	DbColumnN_deleteRowData(g_DbRoot->info_subs, dstRow);
	DbColumnN_add(g_DbRoot->info_subs, parentRow, dstRow);

	//make Table copy later
	if (DbRoot_isType_table(dstRow))
	{
		DbTable* srcTable = DbTables_findOrAdd(g_DbRoot->tables, DbRoot_getFileId(srcRow));
		DbTable* dstTable = DbTables_findOrAdd(g_DbRoot->tables, DbRoot_getFileId(dstRow));
		dstTable->copyTableAsk = srcTable;
	}

	//make Column copy later
	if (DbRoot_isType_column(dstRow))
	{
		DbColumn* srcColumn = DbRoot_findColumn(srcRow);

		DbTable* dstTable = DbRoot_findParentTableNoLoad(dstRow);
		if (dstTable)
		{
			DbColumn* dstColumn = DbColumns_findOrAddUnloadType(dstTable->columns, DbRoot_getFileId(dstRow), DbRoot_getColumnType(dstRow), _DbRoot_getFormat(dstRow), 0);
			dstColumn->copyColumnAsk = srcColumn;
		}
	}

	UBIG i = 0;
	BIG subRow;
	while ((subRow = DbColumnN_jump(g_DbRoot->info_subs, srcRow, &i, 1)) >= 0)
	{
		if (subRow > srcRow)	//only older
			_DbRoot_duplicateRowInner(subRow, dstRow);
		else
			DbColumnN_add(g_DbRoot->info_subs, dstRow, subRow);
		i++;
	}

	return dstRow;
}

UBIG DbRoot_duplicateRow(BIG srcRow, BIG parentRow)
{
	UBIG newRow = _DbRoot_duplicateRowInner(srcRow, parentRow);
	//_DbRoot_updateTables();
	return newRow;
}

BOOL DbRoot_hasConnectionBTables(BIG row)
{
	if (DbRoot_isType_table(row))
	{
		DbTable* table = DbTables_find(g_DbRoot->tables, DbRoot_getFileId(row));
		if (table)
		{
			if (DbTables_findBTable(g_DbRoot->tables, table))// || DbTables_findBTable(g_DbRoot->tables_notSaved, table))
				return TRUE;
		}
	}

	UBIG i = 0;
	BIG subRow;
	while ((subRow = DbColumnN_jump(g_DbRoot->info_subs, row, &i, 1)) >= 0)
	{
		if (subRow > row)	//only older
			if (DbRoot_hasConnectionBTables(subRow))
				return TRUE;
		i++;
	}
	return FALSE;
}

UBIG DbRoot_bytes(BOOL all)
{
	UBIG sum = 0;
	sum += DbTables_bytes(g_DbRoot->tables);
	if (all)
		sum += DbTables_bytes(g_DbRoot->tables_notSaved);
	return sum;
}

const UBIG DbRoot_numEx(BOOL all)
{
	UBIG sum = 0;
	sum += DbTables_num(g_DbRoot->tables);
	if (all)
		sum += DbTables_num(g_DbRoot->tables_notSaved);
	return sum;
}

const UBIG DbRoot_numColumns(BOOL all)
{
	UBIG n = 0;
	n += DbTables_numColumns(g_DbRoot->tables);
	if (all)
		n += DbTables_numColumns(g_DbRoot->tables_notSaved);
	return n;
}

const UBIG DbRoot_numRecords(BOOL all)
{
	UBIG n = 0;
	n += DbTables_numRecords(g_DbRoot->tables);
	if (all)
		n += DbTables_numRecords(g_DbRoot->tables_notSaved);
	return n;
}

const UBIG DbRoot_numCells(BOOL all, BOOL realRows)
{
	UBIG n = 0;
	n += DbTables_numCells(g_DbRoot->tables, realRows);
	if (all)
		n += DbTables_numCells(g_DbRoot->tables_notSaved, realRows);
	return n;
}

UBIG DbRoot_numInfoChanges(void)
{
	return g_DbRoot ? DbTable_numChanges(g_DbRoot->info) : 0;
}

UBIG DbRoot_numAllChanges(void)
{
	if (!g_DbRoot)
		return 0;

	UBIG n = DbRoot_numInfoChanges();
	n += DbTables_numChanges(g_DbRoot->tables);
	return n;
}

BOOL DbRoot_isChangedSave(void)
{
	if (DbTable_isChangedSave(g_DbRoot->info))
		return TRUE;
	return DbTables_isChangedSave(g_DbRoot->tables);
}

BOOL DbRoot_isChangedExe(void)
{
	if (DbTable_isChangedExe(g_DbRoot->info))
		return TRUE;
	return DbTables_isChangedExe(g_DbRoot->tables);
}

void DbRoot_save(void)
{
	UBIG doneCells = 0;
	UBIG numAllCells = DbTables_numCells(g_DbRoot->tables, FALSE);

	DbTable_save(g_DbRoot->info, &doneCells, numAllCells);
	DbTables_save(g_DbRoot->tables);

	//unload not needed tables ...
	StdArr keepTables = StdArr_init();

	DbTable* l = DbRoot_findParentTable(DbRoot_getPanelLeft());
	DbTable* r = DbRoot_findParentTable(DbRoot_getPanelRight());

	if (l)	DbTable_addBTablesList(l, &keepTables);
	if (r)	DbTable_addBTablesList(r, &keepTables);

	DbTables_unloadUnlisted(g_DbRoot->tables, &keepTables);

	StdArr_freeBase(&keepTables);
}

static void _DbRoot_maintenanceFilter(void)
{
	int i;
	for (i = g_DbRoot->filters.num - 1; i >= 0; i--)
	{
		DbFilter* fi = g_DbRoot->filters.ptrs[i];

		if (fi->refs == 0 && (Os_time() - fi->ref_time) > 10)	//10 seconds
		{
			DbFilter_delete(fi);
			StdArr_remove(&g_DbRoot->filters, i);
		}
		else
		{
			//pokud smažu Table/Column[Table_getRow() / Column_getRow()] tak musím smazat i filter ...
		}
	}
}

DbFilter* DbRoot_addFilter(DbFilter* filter)
{
	if (!filter)
		return 0;

	int i;
	for (i = 0; i < g_DbRoot->filters.num; i++)
	{
		DbFilter* fi = g_DbRoot->filters.ptrs[i];

		if (DbFilter_cmp(fi, filter))
		{
			DbFilter_delete(filter);
			fi->refs++;
			return fi;
		}
	}

	StdArr_add(&g_DbRoot->filters, filter);
	filter->refs++;
	return filter;
}

void DbRoot_removeFilter(DbFilter* filter)
{
	if (!filter)
		return;

	filter->refs--;
	filter->ref_time = Os_time();

	_DbRoot_maintenanceFilter();
}

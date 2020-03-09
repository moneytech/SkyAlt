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

typedef struct DbRoot_s
{
	//info tble
	DbTable* info;
	DbColumnString32* info_options;
	DbColumnN* info_subs;
	DbColumn1* info_ref;

	//map table
	DbTable* map;
	DbColumnString32* map_name;
	DbColumn1* map_lat;
	DbColumn1* map_long;

	DbTables* tables;
	DbTables* tables_notSaved;

	UBIG numChanges;

	StdBigs parentRows;
	BIG  parent_subs_changes;

	StdArr filters;
} DbRoot;

DbRoot* g_DbRoot = 0;
BOOL g_DbRoot_loaded = FALSE;

BOOL DbRoot_is(void)
{
	return g_DbRoot_loaded && FileProject_is();
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

DbColumnN* DbRoot_subs(void)
{
	return g_DbRoot->info_subs;
}
BIG DbRoot_subs_row(BIG row)
{
	return DbColumnN_getFirstRow(DbRoot_subs(), row);
}

DbColumn1* DbRoot_ref(void)
{
	return g_DbRoot->info_ref;
}
BIG DbRoot_ref_row(BIG row)
{
	return DbColumn1_getLink(DbRoot_ref(), row);
}
DbColumn* DbRoot_ref_column(BIG row)
{
	return DbRoot_findColumn(DbRoot_ref_row(row));
}
DbTable* DbRoot_ref_table(BIG row)
{
	return DbRoot_findTable(DbRoot_ref_row(row));
}

void DbRoot_ref_set(BIG src, BIG ref)
{
	DbColumn1_set(DbRoot_ref(), src, ref);
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
void DbRoot_setEnable(BIG row, BOOL enable)
{
	_DbRoot_setOptionNumber(row, "enable", enable);
}

static BOOL _DbRoot_isAscending(BIG row)
{
	return _DbRoot_getOptionNumber(row, "ascending", 0) == 0;
}
static BOOL _DbRoot_isAND(BIG row)
{
	return _DbRoot_getOptionNumber(row, "ascending", 0) == 0;
}

BIG DbRoot_isType_panel(BIG row)
{
	return DbRoot_findOrCreateChildType(DbRoot_getRootRow(), "panel_left") == row || DbRoot_findOrCreateChildType(DbRoot_getRootRow(), "panel_right") == row;
}

BIG DbRoot_getPanelLeft(void)
{
	return DbRoot_ref_row(DbRoot_findOrCreateChildType(DbRoot_getRootRow(), "panel_left"));
}
BIG DbRoot_getPanelRight(void)
{
	return DbRoot_ref_row(DbRoot_findOrCreateChildType(DbRoot_getRootRow(), "panel_right"));
}

void DbRoot_setPanelLeft(BIG row)
{
	DbRoot_ref_set(DbRoot_findOrCreateChildType(DbRoot_getRootRow(), "panel_left"), row);
}
void DbRoot_setPanelRight(BIG row)
{
	DbRoot_ref_set(DbRoot_findOrCreateChildType(DbRoot_getRootRow(), "panel_right"), row);
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
BOOL DbRoot_isType_remote(const BIG row)
{
	return _DbRoot_cmpType(row, "remote");
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
BOOL DbRoot_isTypeView_summary(const BIG row)
{
	return _DbRoot_cmpType(row, "summary");
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
		DbRoot_isTypeView_summary(row) ||
		DbRoot_isTypeView_cards(row) ||
		DbRoot_isTypeView_group(row) ||
		DbRoot_isTypeView_kanban(row) ||
		DbRoot_isTypeView_calendar(row) ||
		DbRoot_isTypeView_timeline(row) ||
		DbRoot_isTypeView_chart(row) ||
		DbRoot_isTypeView_map(row);
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

static void _DbRoot_updateParentsInner(BIG row)
{
	UBIG i = 0;
	BIG it;
	while ((it = DbColumnN_jump(DbRoot_subs(), row, &i, 1)) >= 0)
	{
		g_DbRoot->parentRows.ptrs[it] = row;
		_DbRoot_updateParentsInner(it);
		i++;
	}
}
static void _DbRoot_updateParents()
{
	const UBIG num_rows = DbTable_numRows(g_DbRoot->info);
	const UBIG numChanges = g_DbRoot->info_subs->base.numChanges;

	if (g_DbRoot->parentRows.num != num_rows || g_DbRoot->parent_subs_changes != numChanges)
	{
		BIG rootRow = DbTable_firstRow(g_DbRoot->info);

		StdBigs_resize(&g_DbRoot->parentRows, num_rows);
		StdBigs_setAll(&g_DbRoot->parentRows, -1);
		_DbRoot_updateParentsInner(rootRow);

		g_DbRoot->parent_subs_changes = numChanges;
	}
}

static BOOL _DbRoot_checkRowsForDuplicityInner(UBIG row, StdBigs* used)
{
	BOOL ok = TRUE;

	if (used->ptrs[row])
	{
		ok = FALSE;
		printf("Error: Row(%lld) has duplicity\n", row);
	}
	used->ptrs[row]++;

	UBIG i = 0;
	BIG it;
	while ((it = DbColumnN_jump(DbRoot_subs(), row, &i, 1)) >= 0)
	{
		ok &= _DbRoot_checkRowsForDuplicityInner(it, used);
		i++;
	}

	return ok;
}
static BOOL _DbRoot_checkRowsForDuplicity(void)
{
	StdBigs used = StdBigs_init();
	StdBigs_resize(&used, DbTable_numRows(g_DbRoot->info));
	StdBigs_setAll(&used, 0);

	BIG rootRow = DbTable_firstRow(g_DbRoot->info);
	BOOL ok = _DbRoot_checkRowsForDuplicityInner(rootRow, &used);

	StdBigs_free(&used);

	return ok;
}

DbFilter* DbRoot_findFilter(BIG row)
{
	int i;
	for (i = 0; i < g_DbRoot->filters.num; i++)
	{
		DbFilter* fi = g_DbRoot->filters.ptrs[i];
		if (fi->row == row)
			return fi;
	}
	return 0;
}
static DbFilter* _DbRoot_findFilterParent(BIG row)
{
	DbFilter* filter = DbRoot_findFilter(row);
	if (!filter)
	{
		row = DbRoot_findParent(row);
		if (row >= 0)
			filter = _DbRoot_findFilterParent(row);
	}

	return filter;
}

BIG DbRoot_addSubRow(const BIG parentRow, BIG pos)
{
	UBIG r = DbTable_addRow(g_DbRoot->info);
	DbColumnN_insert(DbRoot_subs(), parentRow, r, pos);
	return r;
}

static BIG _DbRoot_createView(const BIG parentRow, const char* type)
{
	UBIG r = DbTable_addRow(g_DbRoot->info);
	_DbRoot_setType(r, type);
	DbColumnN_add(DbRoot_subs(), parentRow, r);
	return r;
}

BIG DbRoot_createViewReference(const BIG parentRow, BIG origViewRow)
{
	UBIG r = DbTable_addRow(g_DbRoot->info);

	_DbRoot_setOptionNumber(r, "reference", 1);
	//DbColumnN_add(DbRoot_subs(), r, origViewRow);

	//ref
	{
		UBIG referenceRow = DbTable_addRow(g_DbRoot->info);
		_DbRoot_setType(referenceRow, "reference");
		DbColumn1_set(DbRoot_ref(), referenceRow, origViewRow);

		DbColumnN_add(DbRoot_subs(), r, referenceRow);
	}

	DbColumnN_add(DbRoot_subs(), parentRow, r);
	return r;
}

void DbRoot_pernamentReference(const BIG row)
{
	_DbRoot_setOptionNumber(row, "reference", 0);
}

BIG DbRoot_getOrigReference(const BIG row)
{
	BIG refRow = DbRoot_findChildType(row, "reference");
	return refRow ? DbRoot_ref_row(refRow) : -1;
	//return DbColumn1_getLink(DbRoot_ref(), row);
}
BOOL DbRoot_isReference(const BIG row)
{
	return _DbRoot_getOptionNumber(row, "reference", 0) == 1;
	//return _DbRoot_cmpType(row, "reference");
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

		DbColumnN_add(DbRoot_subs(), r, columnsRow);
		DbColumnN_add(DbRoot_subs(), r, viewsRow);
	}

	//filters
	{
		UBIG shortRow = DbTable_addRow(g_DbRoot->info);
		_DbRoot_setType(shortRow, "short");
		DbColumnN_add(DbRoot_subs(), r, shortRow);

		UBIG selectRow = DbTable_addRow(g_DbRoot->info);
		_DbRoot_setType(selectRow, "select");
		DbColumnN_add(DbRoot_subs(), r, selectRow);
	}

	return r;
}
BIG DbRoot_createView_cards(const BIG parentRow)
{
	BIG r = _DbRoot_createView(parentRow, "cards");

	//columns
	{
		UBIG columnsRow = DbTable_addRow(g_DbRoot->info);
		_DbRoot_setType(columnsRow, "columns");
		DbColumnN_add(DbRoot_subs(), r, columnsRow);
	}

	//group/quickFilter
	{
		UBIG groupRow = DbTable_addRow(g_DbRoot->info);
		_DbRoot_setType(groupRow, "group");
		DbColumnN_add(DbRoot_subs(), r, groupRow);
		DbRoot_setEnable(groupRow, FALSE);	//default = not visible

		//add one
		UBIG oneRow = DbTable_addRow(g_DbRoot->info);
		DbColumnN_add(DbRoot_subs(), groupRow, oneRow);
	}

	return r;
}
BIG DbRoot_createView_group(const BIG parentRow)
{
	BIG r = _DbRoot_createView(parentRow, "groups");

	//columns
	{
		UBIG columnsRow = DbTable_addRow(g_DbRoot->info);
		_DbRoot_setType(columnsRow, "columns");
		DbColumnN_add(DbRoot_subs(), r, columnsRow);
	}

	//group
	{
		UBIG groupRow = DbTable_addRow(g_DbRoot->info);
		_DbRoot_setType(groupRow, "group");
		DbColumnN_add(DbRoot_subs(), r, groupRow);
	}

	return r;
}

BIG DbRoot_createView_summary(const BIG parentRow)
{
	BIG r = _DbRoot_createView(parentRow, "summary");

	//subs
	{
		UBIG columnsRow = DbTable_addRow(g_DbRoot->info);
		UBIG viewsRow = DbTable_addRow(g_DbRoot->info);

		_DbRoot_setType(columnsRow, "columns");
		_DbRoot_setType(viewsRow, "views");

		DbColumnN_add(DbRoot_subs(), r, columnsRow);
		DbColumnN_add(DbRoot_subs(), r, viewsRow);

		//new Link column
		{
			DbTable* origTable = DbRoot_findParentTable(parentRow);

			UBIG cRow = DbTable_addRow(g_DbRoot->info);
			UNI name[64];
			_DbRoot_setOptionString(cRow, "name", DbTable_getName(origTable, name, 64));
			_DbRoot_setType(cRow, DbRoot_getColumnTypeName(DbColumn_N));
			_DbRoot_setOptionNumber(cRow, "lead", 1);
			_DbRoot_setOptionString(cRow, "format", _UNI32("link"));
			_DbRoot_setOptionNumber(cRow, "width", 8);

			//btable
			{
				UBIG bRow = DbTable_addRow(g_DbRoot->info);
				_DbRoot_setType(bRow, "btable");
				DbColumn1_set(DbRoot_ref(), bRow, DbTable_getRow(origTable));
				DbColumnN_add(DbRoot_subs(), cRow, bRow);
			}
			DbColumnN_add(DbRoot_subs(), columnsRow, cRow);
		}
	}

	//short
	{
		UBIG shortRow = DbTable_addRow(g_DbRoot->info);
		_DbRoot_setType(shortRow, "short");
		DbColumnN_add(DbRoot_subs(), r, shortRow);
	}

	//group
	{
		UBIG groupRow = DbTable_addRow(g_DbRoot->info);
		_DbRoot_setType(groupRow, "group");
		DbColumnN_add(DbRoot_subs(), r, groupRow);

		//add one
		UBIG oneRow = DbTable_addRow(g_DbRoot->info);
		DbColumnN_add(DbRoot_subs(), groupRow, oneRow);
	}

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

	//columns - skin
	{
		UBIG columnsRow = DbTable_addRow(g_DbRoot->info);
		_DbRoot_setType(columnsRow, "columns");
		DbColumnN_add(DbRoot_subs(), r, columnsRow);
	}

	//group/quickFilter
	{
		UBIG groupRow = DbTable_addRow(g_DbRoot->info);
		_DbRoot_setType(groupRow, "group");
		DbColumnN_add(DbRoot_subs(), r, groupRow);
		DbRoot_setEnable(groupRow, FALSE);	//default = not visible

		//add one
		UBIG oneRow = DbTable_addRow(g_DbRoot->info);
		DbColumnN_add(DbRoot_subs(), groupRow, oneRow);
	}

	//chart columns
	{
		UBIG columnsRow = DbTable_addRow(g_DbRoot->info);
		_DbRoot_setType(columnsRow, "chart_columns");
		DbColumnN_add(DbRoot_subs(), r, columnsRow);
	}

	//chart rows
	{
		UBIG rowsRow = DbTable_addRow(g_DbRoot->info);
		_DbRoot_setType(rowsRow, "chart_rows");
		DbColumnN_add(DbRoot_subs(), r, rowsRow);
	}

	//chart groups
	{
		UBIG groupRow = DbTable_addRow(g_DbRoot->info);
		_DbRoot_setType(groupRow, "chart_groups");
		DbColumnN_add(DbRoot_subs(), r, groupRow);
	}

	return r;
}
BIG DbRoot_createView_map(const BIG parentRow)
{
	BIG r = _DbRoot_createView(parentRow, "map");

	_DbRoot_setOptionNumber(r, "map_type", 0);
	_DbRoot_setOptionNumber(r, "map_render", 0);

	//columns
	{
		UBIG columnsRow = DbTable_addRow(g_DbRoot->info);
		_DbRoot_setType(columnsRow, "columns");
		DbColumnN_add(DbRoot_subs(), r, columnsRow);
	}

	//group/quickFilter
	{
		UBIG groupRow = DbTable_addRow(g_DbRoot->info);
		_DbRoot_setType(groupRow, "group");
		DbColumnN_add(DbRoot_subs(), r, groupRow);
		DbRoot_setEnable(groupRow, FALSE);	//default = not visible

		//add one
		UBIG oneRow = DbTable_addRow(g_DbRoot->info);
		DbColumnN_add(DbRoot_subs(), groupRow, oneRow);
	}

	//labels
	{
		UBIG columnsRow = DbTable_addRow(g_DbRoot->info);
		//_DbRoot_setEnable(columnsRow, TRUE);
		_DbRoot_setType(columnsRow, "label_columns");
		DbColumnN_add(DbRoot_subs(), r, columnsRow);
	}

	//advanced
	{
		UBIG f;
		f = DbTable_addRow(g_DbRoot->info);
		_DbRoot_setType(f, "location");
		DbColumnN_add(DbRoot_subs(), r, f);

		f = DbTable_addRow(g_DbRoot->info);
		_DbRoot_setType(f, "radius");
		DbColumnN_add(DbRoot_subs(), r, f);
		_DbRoot_setOptionNumber(f, "multiplier", 1);

		f = DbTable_addRow(g_DbRoot->info);
		_DbRoot_setType(f, "selection");
		DbColumnN_add(DbRoot_subs(), r, f);
	}

	return r;
}

BIG DbRoot_findParent(UBIG findRow)
{
	_DbRoot_updateParents();
	return StdBigs_getNeg(&g_DbRoot->parentRows, findRow);
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

DbTable* DbRoot_findParentTable(BIG row)
{
	if (row < 0)
		return 0;

	//_DbRoot_updateParents();

	//if(DbRoot_isType_table(row))
	{
		DbTable* table = DbTables_find(g_DbRoot->tables, DbRoot_getFileId(row));
		if (table)
			return table;
	}
	return DbRoot_findParentTable(DbRoot_findParent(row));
}

BIG DbRoot_findParentTableRow(BIG row)
{
	if (row < 0)
		return -1;

	//_DbRoot_updateParents();

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

BIG DbRoot_findParentFolderRow(UBIG row)
{
	return DbRoot_findParentType(row, "folder");
}

BIG DbRoot_findChildType(UBIG row, const char* typeValue)
{
	if (_DbRoot_cmpType(row, typeValue))
		return row;

	BIG it;
	UBIG i = 0;
	while ((it = DbColumnN_jump(DbRoot_subs(), row, &i, 1)) >= 0)
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
		DbColumnN_add(DbRoot_subs(), row, sub);
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
	DbColumnN_add(DbRoot_subs(), r, optionsRow);

	//set btable
	if ((column->type == DbColumn_1 || column->type == DbColumn_N) && DbColumn_getBTable(column))
	{
		UBIG btableSetRow = DbTable_addRow(g_DbRoot->info);
		_DbRoot_setType(btableSetRow, "btable");
		DbColumnN_add(DbRoot_subs(), r, btableSetRow);

		DbColumn1_set(DbRoot_ref(), btableSetRow, DbTable_getRow(DbColumn_getBTable(column)));
	}

	//add to parent Table
	DbColumnN_add(DbRoot_subs(), DbRoot_getRow(parentTable->columns->fileId), r);
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

		DbColumnN_add(DbRoot_subs(), r, columnsRow);
		DbColumnN_add(DbRoot_subs(), r, viewsRow);

		table->columns->fileId = DbRoot_getFileId(columnsRow);
	}

	//short
	{
		UBIG shortRow = DbTable_addRow(g_DbRoot->info);
		_DbRoot_setType(shortRow, "short");
		DbColumnN_add(DbRoot_subs(), r, shortRow);
	}

	//add to parent Folder
	DbColumnN_add(DbRoot_subs(), parentRow, r);
}

BIG DbRoot_createPageRow(BIG parentRow)
{
	UBIG r = DbTable_addRow(g_DbRoot->info);
	_DbRoot_setType(r, "page");

	//add to parent Folder
	parentRow = DbRoot_findParentType(parentRow, "folder");

	DbColumnN_add(DbRoot_subs(), parentRow, r);
	return r;
}

BIG DbRoot_createFolderRow(void)
{
	UBIG r = DbTable_addRow(g_DbRoot->info);
	_DbRoot_setType(r, "folder");
	DbRoot_setName(r, Lang_find("FOLDER"));

	//add to parent Root
	DbColumnN_add(DbRoot_subs(), DbRoot_getRootRow(), r);

	return r;
}

BIG DbRoot_createRemoteRow(void)
{
	UBIG r = DbTable_addRow(g_DbRoot->info);
	_DbRoot_setType(r, "remote");
	DbRoot_setName(r, Lang_find("REMOTE"));

	//add to parent Root
	DbColumnN_add(DbRoot_subs(), DbRoot_getRootRow(), r);

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
	g_DbRoot->info_ref = DbColumns_addColumn1(g_DbRoot->info->columns, g_DbRoot->info);

	g_DbRoot->info_subs->base.fileId = FileRow_init(1);		//first columnRow created by user is 7 !!!
	g_DbRoot->info_ref->base.fileId = FileRow_init(2);
	g_DbRoot->info_options->base.fileId = FileRow_init(3);
}

static void _DbRoot_createMap(void)
{
	g_DbRoot->map = 0;	//note on next row, DbTable_new(), checks if info exist!
	g_DbRoot->map = DbTable_new(FileRow_initEmpty(), 0);
	g_DbRoot->map->fileId = FileRow_init(4);

	g_DbRoot->map_name = DbColumns_addColumnString32(g_DbRoot->map->columns);
	g_DbRoot->map_lat = DbColumns_addColumn1(g_DbRoot->map->columns, 0);
	g_DbRoot->map_long = DbColumns_addColumn1(g_DbRoot->map->columns, 0);

	g_DbRoot->map_name->base.fileId = FileRow_init(4);	//first columnRow created by user is 7 !!!
	g_DbRoot->map_lat->base.fileId = FileRow_init(5);
	g_DbRoot->map_long->base.fileId = FileRow_init(6);
}

void DbRoot_delete(void)
{
	g_DbRoot_loaded = FALSE;

	if (g_DbRoot)
	{
		UBIG doneCells = 0;
		DbTable_save(g_DbRoot->map, &doneCells, DbTable_numCells(g_DbRoot->map, FALSE));

		DbTables_freeFilterAndInsight(g_DbRoot->tables);
		DbTables_freeFilterAndInsight(g_DbRoot->tables_notSaved);

		StdArr_freeFn(&g_DbRoot->filters, (StdArrFREE)&DbFilter_delete);
		//StdArr_freeFn(&g_DbRoot->insights, (StdArrFREE)&DbInsight_delete);
		//StdBigs_free(&g_DbRoot->activeRows);

		DbTables_delete(g_DbRoot->tables);
		DbTables_delete(g_DbRoot->tables_notSaved);

		DbTable_delete(g_DbRoot->info);
		DbTable_delete(g_DbRoot->map);

		StdBigs_free(&g_DbRoot->parentRows);

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

	g_DbRoot->parentRows = StdBigs_init();
	g_DbRoot->parent_subs_changes = -1;
	//g_DbRoot->insights = StdArr_init();

	//g_DbRoot->activeRows = StdBigs_init();

	_DbRoot_createInfo();
	_DbRoot_createMap();
	return TRUE;
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

static BIG _DbRoot_getGroupColumnRow(const BIG row)
{
	BIG propRow = DbRows_findSubType(row, "group");
	if (propRow >= 0)
	{
		BIG lineRow = DbColumnN_getFirstRow(DbRoot_subs(), propRow);
		if (lineRow >= 0)
		{
			return DbColumn1_getLink(DbRoot_ref(), lineRow);
		}
	}
	return -1;
}


BIG DbRoot_findSubLineRefRow(BIG row, BIG findRow)
{
	UBIG i = 0;
	BIG it;
	while ((it = DbColumnN_jump(DbRoot_subs(), row, &i, 1)) >= 0)
	{
		if (DbRoot_ref_row(it) == findRow)
			return it;
		i++;
	}
	return -1;
}


static void _DbRoot_updateColumns(BIG tableSubRow)
{
	DbTable* table = DbRoot_findParentTable(tableSubRow);

	table->columns->fileId = DbRoot_getFileId(tableSubRow);

	BIG columnRow;
	UBIG i3 = 0;
	while ((columnRow = DbColumnN_jump(DbRoot_subs(), tableSubRow, &i3, 1)) >= 0)
	{
		BIG columnType = DbRoot_getColumnType(columnRow);
		if (columnType >= 0)
		{
			DbColumn* column = DbColumns_findOrAddUnloadType(table->columns, DbRoot_getFileId(columnRow), columnType, _DbRoot_getFormat(columnRow), 0);
			DbColumn_setRemote(column, _DbRoot_getOptionNumber(columnRow, "remote", 0));
		}
		else
			printf("error: Unknown Column type\n");
		i3++;
	}
}

static DbInsight* _DbRoot_createColumnInsight(DbColumn* column)
{
	const BOOL errEnable = (column->summary_origColumn == 0);

	DbTable* table = DbColumn_getTable(column);
	BIG crowT = DbColumn_getRow(column);
	BIG propRow = DbRows_findOrCreateSubType(crowT, "summary");

	//check path
	{
		DbRows rows = DbRows_initSubLink(crowT, "summary");
		DbTable* btable = table;
		BIG it;
		UBIG i = 0;
		BOOL removeActive = FALSE;
		while ((it = DbColumnN_jump(DbRoot_subs(), propRow, &i, 1)) >= 0)
		{
			if (!removeActive)
			{
				UBIG ii = i + 1;
				BOOL last = (DbColumnN_jump(DbRoot_subs(), propRow, &ii, 1) < 0);

				DbColumn* column = DbRoot_ref_column(it);
				if (column)
				{
					if (DbColumn_getTable(column) != btable)
						removeActive = TRUE;
				}
				else
					if (!last)
						removeActive = TRUE;

				btable = column ? DbColumn_getBTable(column) : 0;
			}

			if (removeActive)
				DbRows_removeRow(&rows, it);

			i++;
		}

		if (!removeActive && btable)
			DbRows_addNewRow(&rows);
		else
			if (DbColumnN_sizeActive(DbRoot_subs(), propRow) == 0)
				DbRows_addNewRow(&rows);

		DbRows_free(&rows);
	}

	//create summary
	DbInsight* insight;
	{
		DbFilter* filter = DbRoot_findFilter(DbTable_getRow(table));
		//build func
		const DbInsightFunc* funcInsight = DbInsightSelectFunc_get(DbValue_getOptionNumber(crowT, "insight_func", 0));
		insight = DbInsight_new(funcInsight, table, filter, column);

		DbTable* btable = table;
		BIG it;
		UBIG i = 0;
		while (btable && (it = DbColumnN_jump(DbRoot_subs(), propRow, &i, 1)) >= 0)
		{
			DbColumn* column = DbRoot_ref_column(it);
			if (column)
				DbInsight_addItem(insight, column);
			i++;
		}

		column->err = (insight ? DbInsight_hasError(insight) : TRUE) && errEnable;
	}
	return insight;
}

static DbFilter* _DbRoot_createFilterSelect(BIG row, BIG realRow, DbFilter* filter, DbTable* table)
{
	BIG selectRow = DbRows_findSubType(row, "select");

	if (selectRow < 0 || !_DbRoot_isEnable(selectRow))
		return filter;

	BIG maxRecords = _DbRoot_getOptionNumber(selectRow, "maxRecords", 0);
	if (maxRecords > 0)
	{
		filter = filter ? filter : DbFilter_new(realRow);
		DbFilter_setMaxRecords(filter, maxRecords);
	}

	//select
	UBIG i = 0;
	BIG it;
	while ((it = DbColumnN_jump(DbRoot_subs(), selectRow, &i, 1)) >= 0)
	{
		BIG colRow = DbRoot_findOrCreateChildType(it, "column");
		DbColumn* column = DbRoot_ref_column(colRow);	//DbColumn* column = DbColumns_find(table->columns, DbRoot_getFileId(DbColumnN_getFirstRow(subs, colRow)));
		if (column && _DbRoot_isEnable(it))
		{
			DbFormatTYPE format = DbColumnFormat_findColumn(column);

			UNI value[32];
			if (format == DbFormat_MENU || format == DbFormat_TAGS)
			{
				BIG row = DbRoot_subs_row(DbRoot_findOrCreateChildType(it, "option"));
				Std_buildNumberUNI(row, 0, value);
			}
			else
				_DbRoot_getOptionString(it, "value", 0, value, 32);

			filter = filter ? filter : DbFilter_new(realRow);
			UINT funcIndex = _DbRoot_getOptionNumber(it, "func", 0);
			DbFilter_addSelect(filter, _DbRoot_isAND(it), column, DbFilterSelectFunc_get(format, funcIndex), value, _DbRoot_getOptionNumber(it, "valueEx", 0));
		}
		i++;
	}

	return filter;
}

static DbFilter* _DbRoot_createFilter(BIG row, BIG realRow)
{
	DbFilter* filter = 0;

	BIG selectRow = DbRows_findSubType(row, "select");
	BIG groupRow = DbRows_findSubType(row, "group");
	BIG shortRow = DbRows_findSubType(row, "short");

	BOOL selectOk = (selectRow >= 0 && _DbRoot_isEnable(selectRow));
	BOOL groupOk = (groupRow >= 0 && _DbRoot_isEnable(groupRow));
	BOOL shortOk = (shortRow >= 0 && _DbRoot_isEnable(shortRow));

	if (selectOk || groupOk || shortOk)
	{
		const BOOL isSummaryTable = DbRoot_isTypeView_summary(row);
		DbTable* table = DbRoot_findParentTable(isSummaryTable ? DbRoot_findParent(row) : row);

		UBIG i;
		BIG it;
		if (shortOk)
		{
			i = 0;
			while ((it = DbColumnN_jump(DbRoot_subs(), shortRow, &i, 1)) >= 0)
			{
				//DbColumn* column = DbColumns_find(table->columns, DbRoot_getFileId(DbColumnN_getFirstRow(subs, it)));
				DbColumn* column = DbRoot_ref_column(it);
				if (column && _DbRoot_isEnable(it))
				{
					filter = filter ? filter : DbFilter_new(realRow);
					DbFilter_addShort(filter, column, _DbRoot_isAscending(it));
				}
				i++;
			}
		}

		if (groupOk)
		{
			//group
			i = 0;
			while ((it = DbColumnN_jump(DbRoot_subs(), groupRow, &i, 1)) >= 0)
			{
				//DbColumn* column = DbColumns_find(table->columns, DbRoot_getFileId(DbColumnN_getFirstRow(subs, it)));
				DbColumn* column = DbRoot_ref_column(it);
				if (column && _DbRoot_isEnable(it))
				{
					filter = filter ? filter : DbFilter_new(realRow);
					DbFilter_addGroup(filter, column, _DbRoot_isAscending(it), FALSE);
				}
				i++;
			}
		}

		if (selectOk)
		{
			filter = _DbRoot_createFilterSelect(row, realRow, filter, table);
		}
	}

	return filter;
}

static void _DbRoot_updateFilter(BIG row)
{
	DbFilter* filterNew = _DbRoot_createFilter(row, row);

	if (filterNew)
	{
		if (DbRoot_isTypeView_summary(row))
			_DbFilter_buildSummaryTable(filterNew);
	}

	if (filterNew)
		filterNew->parent = _DbRoot_findFilterParent(DbRoot_findParent(row));

	DbFilter* filterOld = DbRoot_findFilter(row);
	if (filterOld && filterNew)
	{
		if (DbFilter_cmp(filterOld, filterNew))
		{
			DbFilter_delete(filterNew);
		}
		else
		{
			DbFilter_delete(filterOld);
			StdArr_replace(&g_DbRoot->filters, filterOld, filterNew);
		}
	}
	else
		if (filterOld && !filterNew)
		{
			DbFilter_delete(filterOld);
			StdArr_removeFind(&g_DbRoot->filters, filterOld);
		}
		else
			if (!filterOld && filterNew)
			{
				StdArr_add(&g_DbRoot->filters, filterNew);
			}
}

static void _DbRoot_updateViewSummary(BIG viewsRow, BIG viewRow)
{
	BIG columnsRow = DbRoot_findChildType(viewRow, "columns");

	BIG groupRow = _DbRoot_getGroupColumnRow(viewRow);

	//update settings(names, precision, etc.)
	BIG crow;
	UBIG i4 = 0;
	while ((crow = DbColumnN_jump(DbRoot_subs(), columnsRow, &i4, 1)) >= 0)
	{
		if (_DbRoot_getOptionNumber(crow, "lead", 0) == 0)
			//if(origColumn >= 0)	//is column from origTable
		{
			//copy
			BIG origColumnRow = DbColumn1_getLink(DbRoot_ref(), crow);

			//backup
			UNI name[64];
			DbColumn_getName(DbRoot_findColumnExisted(origColumnRow), name, 64);
			const UNI* funcName = Lang_find(DbInsightSelectFunc_getName(_DbRoot_getOptionNumber(crow, "insight_func", 0)));
			UNI* str = Std_newUNI(name);
			if (origColumnRow != groupRow)
			{
				str = Std_addAfterUNI_char(str, "(");
				str = Std_addAfterUNI(str, funcName);
				str = Std_addAfterUNI_char(str, ")");
			}

			const BOOL enable = _DbRoot_isEnable(crow);
			const double width = _DbRoot_getOptionNumber(crow, "width", 8);
			const int func = _DbRoot_getOptionNumber(crow, "insight_func", 0);

			//copy options
			const UNI* orig = DbColumnString32_get(DbRoot_getColumnOptions(), origColumnRow);
			UNI* fnl = _DbColumnString32_addOptionValueNumber(orig, "insight_func", func);
			UNI* dst = _DbColumnString32_addOptionValueNumber(fnl, "width", width);
			Std_deleteUNI(fnl);
			fnl = _DbColumnString32_addOptionValueNumber(dst, "lead", 0);
			Std_deleteUNI(dst);
			dst = _DbColumnString32_addOptionValueNumber(fnl, "enable", enable);
			Std_deleteUNI(fnl);
			fnl = _DbColumnString32_addOptionValue(dst, "name", str);
			Std_deleteUNI(dst);

			DbColumnString32_setCopy(DbRoot_getColumnOptions(), crow, fnl);
			Std_deleteUNI(fnl);

			Std_deleteUNI(str);
		}
		else
		{
			//update Btable name for LinkColumn
			DbTable* btable = DbRoot_findParentTable(viewsRow);
			UNI name[64];
			DbRoot_setName(crow, DbTable_getName(btable, name, 64));
		}
		i4++;
	}

	//recreate columns
	DbTable* summaryTable = DbTables_findOrAdd(g_DbRoot->tables, DbRoot_getFileId(viewRow));
	summaryTable->summary = TRUE;
	_DbRoot_updateColumns(columnsRow);

	//update columns
	{
		//BIG groupRow = _DbRoot_getGroupColumnRow(viewRow);
		i4 = 0;
		while ((crow = DbColumnN_jump(DbRoot_subs(), columnsRow, &i4, 1)) >= 0)
		{
			DbColumn* col = DbRoot_findColumn(crow);

			BIG origColumn = DbColumn1_getLink(DbRoot_ref(), crow);
			col->summary_origColumn = DbRoot_findColumn(origColumn);

			col->summary_links = (_DbRoot_getOptionNumber(crow, "lead", 0) == 1);	//is column from origTable
			col->summary_group = (origColumn == groupRow);

			i4++;
		}
	}

	//build insights
	{
	}
}

void DbRoot_updateColumnsIds(BIG row)
{
	const DbTable* table = DbRoot_findParentTable(row);
	const BIG idColumnsRow = DbRows_findOrCreateSubType(row, "id_columns");

	const BIG orig_idColumnsRow = DbRows_findOrCreateSubType(DbTable_getRow(table), "id_columns");

	BIG i;
	for (i = 0; i < DbColumns_num(table->columns); i++)
	{
		DbColumn* col = DbColumns_get(table->columns, i);
		if (col == &table->rows->base)
			continue;

		BOOL found = FALSE;
		BIG it;
		UBIG i5 = 0;
		while ((it = DbColumnN_jump(DbRoot_subs(), idColumnsRow, &i5, 1)) >= 0)
		{
			if (DbRoot_ref_column(it) < 0)
				DbRoot_removeRow(it);
			else
				if (DbRoot_ref_column(it) == col)
				{
					found = TRUE;
					break;
				}
			i5++;
		}

		if (!found)
		{
			//new row
			UBIG r = DbTable_addRow(g_DbRoot->info);

			const BIG colRow = DbColumn_getRow(col);

			BIG origLaneRow = DbRoot_findSubLineRefRow(orig_idColumnsRow, colRow);
			if(origLaneRow >= 0)
				DbTable_copyRow(g_DbRoot->info, r, origLaneRow);
			else
			{
				DbRoot_setEnable(r, FALSE);
				DbColumn1_set(DbRoot_ref(), r, colRow);
			}

			//BIG origColumnRow = DbColumn_getRow(col);
			//DbColumn1_set(DbRoot_ref(), r, origColumnRow);

			

			//DbRoot_setEnable(r, _DbRoot_isEnable(origLaneRow));
			//_DbRoot_setOptionNumber(r, "enable", 0);

			DbColumnN_add(DbRoot_subs(), idColumnsRow, r);
		}
	}
}

static void _DbRoot_updateView(BIG viewsRow)
{
	DbTable* table = DbRoot_findParentTable(viewsRow);

	BIG viewRow;
	UBIG i3 = 0;
	while ((viewRow = DbColumnN_jump(DbRoot_subs(), viewsRow, &i3, 1)) >= 0)
	{
		if (_DbRoot_getOptionNumber(viewRow, "reference", 0) == 1)
		{
			BIG ref = DbRoot_getOrigReference(viewRow);
			if (ref >= 0)
			{
				//copy base
				//BOOL lPanel = _DbRoot_getOptionNumber(viewRow, "panel_left", 0);
				//BOOL rPanel = _DbRoot_getOptionNumber(viewRow, "panel_right", 0);
				DbColumnString32_setCopy(DbRoot_getColumnOptions(), viewRow, DbColumnString32_get(DbRoot_getColumnOptions(), ref));
				_DbRoot_setOptionNumber(viewRow, "reference", 1);
				//_DbRoot_setOptionNumber(viewRow, "panel_left", lPanel);
				//_DbRoot_setOptionNumber(viewRow, "panel_right", rPanel);

				BIG it;
				UBIG i9;

				//remove old one
				BIG keep = DbRoot_findChildType(viewRow, "reference");
				i9 = 0;
				while ((it = DbColumnN_jump(DbRoot_subs(), viewRow, &i9, 1)) >= 0)
				{
					if (it != keep)
						DbRoot_removeRow(it);
					i9++;
				}

				//duplicate ref
				i9 = 0;
				while ((it = DbColumnN_jump(DbRoot_subs(), ref, &i9, 1)) >= 0)
				{
					DbRoot_duplicateRow(it, viewRow);
					i9++;
				}
			}
			else
				_DbRoot_setOptionNumber(viewRow, "reference", 0);
		}

		const BOOL isSummaryTable = DbRoot_isTypeView_summary(viewRow);
		//const BOOL isMap = DbRoot_isTypeView_map(viewRow);
		//DbTable* table2 = DbRoot_findParentTable(viewRow);

		if (isSummaryTable)
			_DbRoot_updateViewSummary(viewsRow, viewRow);

		_DbRoot_updateFilter(viewRow);

		BIG viewSubRow;
		UBIG i4 = 0;
		while ((viewSubRow = DbColumnN_jump(DbRoot_subs(), viewRow, &i4, 1)) >= 0)
		{
			if (_DbRoot_cmpType(viewSubRow, "views"))
			{
				_DbRoot_updateView(viewSubRow);
			}
			else
				if (_DbRoot_cmpType(viewSubRow, "columns"))
				{
					//remove old/un-used rows when moving View out/in Summary
					BIG columnRow;
					UBIG i6 = 0;
					while ((columnRow = DbColumnN_jump(DbRoot_subs(), viewSubRow, &i6, 1)) >= 0)
					{
						BIG bColumnRow = DbColumn1_get(DbRoot_ref(), columnRow);

						DbColumn* cl = DbRoot_findColumnExisted(bColumnRow);
						if (cl && DbColumn_getTable(cl) != table)
							DbRoot_removeRow(columnRow);

						i6++;
					}

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
						while ((columnRow = DbColumnN_jump(DbRoot_subs(), viewSubRow, &i5, 1)) >= 0)
						{
							BIG bColumnRow = DbColumn1_get(DbRoot_ref(), columnRow);
							if (bColumnRow < 0 && (!isSummaryTable || _DbRoot_getOptionNumber(columnRow, "lead", 0) == 0))	//dont remove Link table in Summary table
							{
								DbRoot_removeRow(columnRow);
								found = TRUE;
							}
							else
								if (bColumnRow == crow)
									found = TRUE;
							i5++;
						}

						if (!found)
						{
							//new row
							UBIG r = DbTable_addRow(g_DbRoot->info);

							//copy old
							_DbRoot_setOptionNumber(r, "enable", _DbRoot_getOptionNumber(crow, "enable", 1));
							_DbRoot_setOptionNumber(r, "width", _DbRoot_getOptionNumber(crow, "width", 8));

							DbColumn1_set(DbRoot_ref(), r, crow);
							DbColumnN_add(DbRoot_subs(), viewSubRow, r);
						}
					}
				}
			i4++;
		}

		if (DbRoot_isTypeView_filter(viewRow) || isSummaryTable)
			DbRoot_updateColumnsIds(viewRow);

		i3++;
	}
}

void DbRoot_updateTable(BIG folderRow, BOOL remote)
{
	//OsDate timeNext = remote ? DbRoot_getRemoteRefreshNextTime(folderRow) : OsDate_initEmpty();

	BIG tableRow;
	UBIG i1 = 0;
	while ((tableRow = DbColumnN_jump(DbRoot_subs(), folderRow, &i1, 1)) >= 0)
	{
		if (_DbRoot_cmpType(tableRow, "table"))
		{
			DbTable* table = DbTables_findOrAdd(g_DbRoot->tables, DbRoot_getFileId(tableRow));
			if (remote)
				DbTable_setRemote(table, folderRow);//, timeNext);

			BIG tableSubRow;
			UBIG i2 = 0;
			while ((tableSubRow = DbColumnN_jump(DbRoot_subs(), tableRow, &i2, 1)) >= 0)
			{
				if (_DbRoot_cmpType(tableSubRow, "columns"))
					_DbRoot_updateColumns(tableSubRow);

				i2++;
			}

			_DbRoot_updateFilter(tableRow);

			i2 = 0;
			while ((tableSubRow = DbColumnN_jump(DbRoot_subs(), tableRow, &i2, 1)) >= 0)
			{
				if (_DbRoot_cmpType(tableSubRow, "views"))
					_DbRoot_updateView(tableSubRow);

				i2++;
			}

			DbRoot_updateColumnsIds(tableRow);
		}
		else
			if (_DbRoot_cmpType(tableRow, "page"))
			{
				//...
			}
			else
				printf("error: Should be Table\n");
		i1++;
	}
}

void DbRoot_updateTables(void)
{
	BIG rootRow = DbTable_firstRow(g_DbRoot->info);

	//upate parents
	//_DbRoot_updateParents();
	//printf("DbRoot_updateTables()\n");

	//update root items
	BIG folderRow;
	UBIG i0 = 0;
	while ((folderRow = DbColumnN_jump(DbRoot_subs(), rootRow, &i0, 1)) >= 0)
	{
		if (_DbRoot_cmpType(folderRow, "folder"))
		{
			DbRoot_updateTable(folderRow, FALSE);
		}
		else
			if (_DbRoot_cmpType(folderRow, "remote"))
			{
				DbRoot_updateTable(folderRow, TRUE);
				DbRoot_updateRemote(folderRow);
			}
		//else
		//	printf("error: Should be Folder\n");
		i0++;
	}

	//update views columns
	i0 = 0;
	while ((folderRow = DbColumnN_jump(DbRoot_subs(), rootRow, &i0, 1)) >= 0)
	{
		if (_DbRoot_cmpType(folderRow, "folder") || _DbRoot_cmpType(folderRow, "remote"))
		{
			BIG tableRow;
			UBIG i1 = 0;
			while ((tableRow = DbColumnN_jump(DbRoot_subs(), folderRow, &i1, 1)) >= 0)
			{
				if (_DbRoot_cmpType(tableRow, "table"))
				{
					DbTables_findOrAdd(g_DbRoot->tables, DbRoot_getFileId(tableRow));

					BIG tableSubRow;
					UBIG i2 = 0;
					while ((tableSubRow = DbColumnN_jump(DbRoot_subs(), tableRow, &i2, 1)) >= 0)
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
			if (_DbRoot_cmpType(folderRow, "remote"))
			{
			}
		//else
		//	printf("error: Should be Folder\n");
		i0++;
	}

	//remove unused tables / columns
	DbTable_checkExists(g_DbRoot->tables);

	//repair column refs(in/out summary)
	UBIG i = 0;
	while (DbTable_jumpRows(g_DbRoot->info, &i, 1) >= 0)
	{
		BIG p = DbRoot_findParent(i);
		p = (p >= 0) ? DbRoot_findParent(p) : -1;

		if (!DbRoot_isTypeView_summary(p))	//exclude summary view
		{
			BIG ref = DbRoot_ref_row(i);
			if (ref >= 0)
			{
				const DbTable* treeTable = DbRoot_findParentTable(i);

				const DbColumn* refColumn = DbRoot_findColumnExisted(ref);
				const DbTable* refTable = refColumn ? DbColumn_getTable(refColumn) : 0;

				if (treeTable && refTable && treeTable != refTable)
				{
					DbColumn* base = DbColumn_findColumnBase(refColumn);
					if (treeTable->summary || refTable->summary)
					{
						DbColumn* dst = DbColumns_findColumnFromBase(treeTable->columns, base);

						BIG dstRow = dst ? DbColumn_getRow(dst) : -1;
						DbRoot_ref_set(i, dstRow);
						//DbColumn1_set(DbRoot_ref(), i, dstRow);
					}
				}
			}
		}
		i++;
	}

	//update btables(after all are loaded) & link_mirror
	UBIG j;
	for (i = 0; i < DbTables_num(g_DbRoot->tables); i++)
	{
		DbTable* table = DbTables_get(g_DbRoot->tables, i);
		for (j = 0; j < DbColumns_num(table->columns); j++)
		{
			DbColumn* column = DbColumns_get(table->columns, j);
			const BOOL errEnable = (column->summary_origColumn == 0);	//turn off for summary table columns

			if (column && (column->type == DbColumn_1 || column->type == DbColumn_N))
			{
				DbTable* btable = 0;

				DbFormatTYPE format = DbColumnFormat_findColumn(column);

				BIG columnRow = DbColumn_getRow(column);

				if (columnRow >= 0)
				{
					BIG bTableSetRow = DbRows_findOrCreateSubType(columnRow, "btable");

					DbColumn* bcolumn = column;
					if (format == DbFormat_LINK_MIRRORED)
					{
						//set btable
						bcolumn = DbRoot_ref_column(DbRows_findOrCreateSubType(columnRow, "mirror"));
						if (bcolumn)
						{
							DbColumn1_set(DbRoot_ref(), bTableSetRow, DbTable_getRow(bcolumn->parent->parent));
							column->links_mirrored = bcolumn;
						}
					}
					else
						if (format == DbFormat_LINK_FILTERED)
						{
							//set btable
							bcolumn = DbRoot_ref_column(DbRows_findOrCreateSubType(columnRow, "source_column"));
							if (bcolumn)
							{
								DbColumn1_set(DbRoot_ref(), bTableSetRow, DbTable_getRow(bcolumn->parent->parent));	//btable = table, in this case
								column->links_filtered = bcolumn;
							}
						}
						else
							if (format == DbFormat_LINK_JOINTED)
							{
								//set btable
								bcolumn = DbRoot_ref_column(DbRows_findOrCreateSubType(columnRow, "out"));
								if (bcolumn)
								{
									DbColumn1_set(DbRoot_ref(), bTableSetRow, DbTable_getRow(bcolumn->parent->parent));
								}
							}

					column->err = (bcolumn == 0 && errEnable);

					BIG btableRow = DbColumn1_getLink(DbRoot_ref(), bTableSetRow);
					if (btableRow >= 0)
						btable = DbTables_find(g_DbRoot->tables, DbRoot_getFileId(btableRow));
				}

				if (format == DbFormat_MENU || format == DbFormat_TAGS)
					btable = g_DbRoot->info;

				DbColumn_setBTable(column, btable);
			}
		}
	}

	//update summary columns links
	for (i = 0; i < DbTables_num(g_DbRoot->tables); i++)
	{
		DbTable* table = DbTables_get(g_DbRoot->tables, i);
		for (j = 0; j < DbColumns_num(table->columns); j++)
		{
			DbColumn* column = DbColumns_get(table->columns, j);
			const BOOL errEnable = (column->summary_origColumn == 0);	//turn off for summary table columns

			DbFormatTYPE format = DbColumnFormat_findColumn(column);
			if (format == DbFormat_SUMMARY)
			{
				DbColumn_setInsight(column, _DbRoot_createColumnInsight(column));
			}
			else
				if (format == DbFormat_LINK_FILTERED)
				{
					BIG row = DbColumn_getRow(column);
					DbFilter* links_filter = _DbRoot_createFilterSelect(row, row, 0, DbColumn_getBTable(column->links_filtered));

					if (!links_filter)
						links_filter = DbFilter_new(row);	//empty

					DbColumn_setLinkFiltered(column, links_filter);
				}
				else
					if (format == DbFormat_LINK_JOINTED)
					{
						column->err = !DbJointed_check((DbColumnN*)column) && errEnable;
						if (!column->err)
						{
							DbJointed* jointed = DbJointed_build((DbColumnN*)column);
							DbColumn_setLinkJointed(column, jointed);
						}
					}
		}
	}

	//copy tables/columns
	for (i = 0; i < DbTables_num(g_DbRoot->tables); i++)
		DbTable_checkForCopyAsk(DbTables_get(g_DbRoot->tables, i));
}

BOOL DbRoot_newOpen(void)
{
	DbRoot_new();

	DbTable_unloadHard(g_DbRoot->info);
	DbTable_loadLast(g_DbRoot->info);

	DbTable_unloadHard(g_DbRoot->map);
	DbTable_loadLast(g_DbRoot->map);

	if (DbTable_numRowsReal(g_DbRoot->info) == 0)
		return FALSE;

	_DbRoot_checkRowsForDuplicity();

	DbRoot_updateTables();

	//DbRoot_print(DbTable_firstRow(g_DbRoot->info), 0);

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

DbColumn* DbRoot_findColumnExisted(BIG row)
{
	return DbTables_findColumn(g_DbRoot->tables, DbRoot_getFileId(row));
}

DbColumn* DbRoot_findColumn(BIG row)
{
	const DbTable* table = DbRoot_findParentTable(row);
	if (table)
	{
		const FileRow id = DbRoot_getFileId(row);
		return DbColumns_findEx(DbTable_getColumns(table), id, id);
	}

	return 0;
}

DbTable* DbRoot_findTable(BIG row)
{
	return DbTables_find(g_DbRoot->tables, DbRoot_getFileId(row));
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

void DbRoot_removeRow(BIG row)
{
	if (row < 0)
		return;

	UBIG i = 0;
	BIG it;
	while ((it = DbColumnN_jump(DbRoot_subs(), row, &i, 1)) >= 0)
	{
		DbRoot_removeRow(it);
		i++;
	}

	DbTable_removeRow(g_DbRoot->info, row);
}

void DbRoot_replaceAndRemoveRow(BIG oldRow, BIG newRow)
{
	UBIG i = 0;
	while (DbTable_jumpRows(g_DbRoot->info, &i, 1) >= 0)
	{
		DbColumnN_remove(DbRoot_subs(), i, newRow);
		DbColumnN_replace(DbRoot_subs(), i, oldRow, newRow);
		i++;
	}

	DbRoot_removeRow(oldRow);
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

static UBIG _DbRoot_duplicateRow_subs(BIG srcRow, BIG parentRow, StdBigs* srcRows, StdBigs* dstRows)
{
	UBIG dstRow = DbTable_addRow(g_DbRoot->info);
	DbTable_copyRow(g_DbRoot->info, dstRow, srcRow);
	DbColumnN_deleteRowData(DbRoot_subs(), dstRow);		//it will re-create them later
	DbColumnN_add(DbRoot_subs(), parentRow, dstRow);

	StdBigs_add(srcRows, srcRow);
	StdBigs_add(dstRows, dstRow);

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

		DbTable* dstTable = DbRoot_findParentTable(dstRow);
		if (dstTable)
		{
			DbColumn* dstColumn = DbColumns_findOrAddUnloadType(dstTable->columns, DbRoot_getFileId(dstRow), DbRoot_getColumnType(dstRow), _DbRoot_getFormat(dstRow), 0);
			dstColumn->copyColumnAsk = srcColumn;
		}
	}

	UBIG i = 0;
	BIG subRow;
	while ((subRow = DbColumnN_jump(DbRoot_subs(), srcRow, &i, 1)) >= 0)
	{
		_DbRoot_duplicateRow_subs(subRow, dstRow, srcRows, dstRows);
		i++;
	}

	return dstRow;
}

static void _DbRoot_duplicateRow_indexed(BIG row, StdBigs* srcRows, StdBigs* dstRows)
{
	BIG pos = StdBigs_find(srcRows, DbRoot_ref_row(row));
	if (pos >= 0)
		DbColumn1_set(DbRoot_ref(), row, dstRows->ptrs[pos]);

	UBIG i = 0;
	BIG subRow;
	while ((subRow = DbColumnN_jump(DbRoot_subs(), row, &i, 1)) >= 0)
	{
		_DbRoot_duplicateRow_indexed(subRow, srcRows, dstRows);
		i++;
	}
}

UBIG DbRoot_duplicateRow(BIG srcRow, BIG parentRow)
{
	StdBigs srcRows = StdBigs_init();
	StdBigs dstRows = StdBigs_init();

	BIG dst = _DbRoot_duplicateRow_subs(srcRow, parentRow, &srcRows, &dstRows);
	_DbRoot_duplicateRow_indexed(dst, &srcRows, &dstRows);	//redirect refs

	StdBigs_free(&srcRows);
	StdBigs_free(&dstRows);

	return dst;
}

void DbRoot_moveSubs(BIG dst, BIG src, BIG afterRow)
{
	UBIG i = 0;
	BIG it;
	while ((it = DbColumnN_jump(DbRoot_subs(), src, &i, 1)) >= 0)
	{
		DbColumnN_insert_after(DbRoot_subs(), dst, it, afterRow);
		afterRow = it;
		i++;
	}
	DbColumnN_deleteRowData(DbRoot_subs(), src);
}

BOOL DbRoot_hasConnectionBTables(BIG row)
{
	BIG r = DbRoot_ref_row(row);
	if (DbRoot_isType_table(r))
	{
		DbTable* table = DbTables_find(g_DbRoot->tables, DbRoot_getFileId(r));
		if (table)
		{
			if (DbTables_findBTable(g_DbRoot->tables, table))// || DbTables_findBTable(g_DbRoot->tables_notSaved, table))
				return TRUE;
		}
	}

	UBIG i = 0;
	BIG subRow;
	while ((subRow = DbColumnN_jump(DbRoot_subs(), row, &i, 1)) >= 0)
	{
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
	{
		sum += DbTables_bytes(g_DbRoot->tables_notSaved);
	}

	return sum;
}

const UBIG DbRoot_numEx(BOOL all)
{
	UBIG sum = 0;
	sum += DbTables_num(g_DbRoot->tables);
	if (all)
	{
		sum += DbTables_num(g_DbRoot->tables_notSaved);
	}
	return sum;
}

const UBIG DbRoot_numColumns(BOOL all)
{
	UBIG n = 0;
	n += DbTables_numColumns(g_DbRoot->tables);
	if (all)
	{
		n += DbTables_numColumns(g_DbRoot->tables_notSaved);
	}
	return n;
}

const UBIG DbRoot_numRecords(BOOL all)
{
	UBIG n = 0;
	n += DbTables_numRecords(g_DbRoot->tables);
	if (all)
	{
		n += DbTables_numRecords(g_DbRoot->tables_notSaved);
	}
	return n;
}

const UBIG DbRoot_numCells(BOOL all, BOOL realRows)
{
	UBIG n = 0;
	n += DbTables_numCells(g_DbRoot->tables, realRows);
	if (all)
	{
		n += DbTables_numCells(g_DbRoot->tables_notSaved, realRows);
	}
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
	n += DbTable_numChanges(g_DbRoot->map);
	n += DbTables_numChanges(g_DbRoot->tables);
	return n;
}

BOOL DbRoot_isChangedSave(void)
{
	if (DbTable_isChangedSave(g_DbRoot->info))
		return TRUE;
	if (DbTable_isChangedSave(g_DbRoot->map))
		return TRUE;
	return DbTables_isChangedSave(g_DbRoot->tables);
}

BOOL DbRoot_isChangedExe(void)
{
	if (DbTable_isChangedExe(g_DbRoot->info))
		return TRUE;
	if (DbTable_isChangedExe(g_DbRoot->map))
		return TRUE;
	return DbTables_isChangedExe(g_DbRoot->tables);
}

void DbRoot_save(void)
{
	UBIG doneCells = 0;
	UBIG numAllCells = DbTables_numCells(g_DbRoot->tables, FALSE);

	DbTable_save(g_DbRoot->info, &doneCells, numAllCells);
	DbTable_save(g_DbRoot->map, &doneCells, numAllCells);

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

void _DbRoot_refreshTables(BIG row)
{
	if (row >= 0)
	{
		BIG parentRow = DbRoot_findParent(row);
		_DbRoot_refreshTables(parentRow);	//deepest first

		DbTable* table = DbRoot_findTable(row);
		if (table)
		{
			DbTable_loadLast(table);
		}

		row = parentRow;
	}
}

void _DbRoot_refreshFilters(BIG row)
{
	DbFilter* filter = _DbRoot_findFilterParent(row);
	if (filter)
		DbFilter_execute(filter);
}

void _DbRoot_refreshColumns(BIG row)
{
	if (row >= 0)
	{
		BIG parentRow = DbRoot_findParent(row);
		_DbRoot_refreshColumns(parentRow);	//deepest first

		DbTable* table = DbRoot_findTable(row);
		if (table)
		{
			UBIG N_COLS = DbColumns_num(DbTable_getColumns(table));
			BIG i;
			for (i = 0; i < N_COLS; i++)
			{
				DbColumn* column = DbColumns_get(DbTable_getColumns(table), i);
				if (column)
				{
					DbColumn_refreshInsight(column);
					DbColumn_refreshLinkMirrored(column);
					DbColumn_refreshLinkFiltered(column);
					DbColumn_refreshLinkJointed(column);
				}
			}
		}

		row = parentRow;
	}
}

void DbRoot_refresh(void)
{
	BIG leftRow = DbRoot_getPanelLeft();
	BIG rightRow = DbRoot_getPanelRight();

	_DbRoot_refreshTables(leftRow);
	_DbRoot_refreshTables(rightRow);

	_DbRoot_refreshColumns(leftRow);
	_DbRoot_refreshColumns(rightRow);

	_DbRoot_refreshFilters(leftRow);
	_DbRoot_refreshFilters(rightRow);

	//clear un-used tables(time-out)? ...
}

BOOL DbRoot_searchMapLocation(const UNI* name, Vec2f* out_pos, const MapPolyIndex** out_poly)
{
	BOOL found = FALSE;
	*out_pos = Vec2f_init();

	*out_poly = Map_findPoly(name);
	if (*out_poly)
	{
		*out_pos = (*out_poly)->pos;
		found = TRUE;
	}
	else
	{
		BIG r = DbColumnString32_searchString(g_DbRoot->map_name, name);	//optimalize with hashes(directly in DbColumnString32) ...
		if (r >= 0)
		{
			*out_pos = Vec2f_init2(DbColumn1_get(g_DbRoot->map_lat, r), DbColumn1_get(g_DbRoot->map_long, r));
			found = TRUE;
		}
	}

	if (!found)
	{
		found = Map_downloadPos(name, out_pos);
		if (found)
			DbRoot_addMapLocation(name, *out_pos);
	}

	return found;
}

void DbRoot_addMapLocation(const UNI* name, Vec2f latLong)
{
	BIG r = DbTable_addRow(g_DbRoot->map);

	DbColumnString32_setCopy(g_DbRoot->map_name, r, name);
	DbColumn1_set(g_DbRoot->map_lat, r, latLong.x);
	DbColumn1_set(g_DbRoot->map_long, r, latLong.y);
}

UBIG DbRoot_mapBytes(void)
{
	return DbTable_bytes(g_DbRoot->map);
}

void DbRoot_mapDeleteGeolocation(void)
{
	//delete files
	FileProject_removeMyColumn(g_DbRoot->map_name->base.fileId);
	FileProject_removeMyColumn(g_DbRoot->map_lat->base.fileId);
	FileProject_removeMyColumn(g_DbRoot->map_long->base.fileId);

	//remove table
	DbTable_delete(g_DbRoot->map);

	//create new table
	_DbRoot_createMap();
}

OsODBC* DbRoot_connectRemote(BIG row, BOOL logErrors)
{
	UNI str[128];

	char* connection = Std_newCHAR_uni(DbValue_getOptionString(row, "connection", str, 128));
	char* server = Std_newCHAR_uni(DbValue_getOptionString(row, "server", str, 128));
	USHORT port = DbValue_getOptionNumber(row, "port", 0);
	char* user = Std_newCHAR_uni(DbValue_getOptionString(row, "user", str, 128));
	char* password = Std_newCHAR_uni(DbValue_getOptionString(row, "password", str, 128));
	char* driver = Std_newCHAR_uni(DbValue_getOptionString(row, "driver", str, 128));

	OsODBC* odbc = OsODBC_new(connection, server, port, user, password, driver);

	if (logErrors && !odbc)
		Logs_addError("ERR_REMOTE_CANT_CONNECT");

	Std_deleteCHAR(connection);
	Std_deleteCHAR(server);
	Std_deleteCHAR(user);
	Std_deleteCHAR(password);
	Std_deleteCHAR(driver);

	return odbc;
}

double _DbRoot_getRemoteRefreshSeconds(BIG row)
{
	double val = DbValue_getOptionNumber(row, "refresh", 10);
	if (val <= 0)
	{
		val = 10;
		DbValue_setOptionNumber(row, "refresh", val);
	}

	double refreshSeconds = val * (DbValue_getOptionNumber(row, "refresh_type", 0) == 0 ? 60 : 3600);
	return refreshSeconds;
}

void DbRoot_updateRemoteRefreshTime(BIG row)
{
	OsDate now = OsDate_initActual();
	DbValue_setOptionNumber(row, "refresh_time", OsDate_asNumber(&now));
}

void DbRoot_refreshRemoteNow(BIG row)
{
	OsDate now = OsDate_initActual();
	double last = OsDate_asNumber(&now) - _DbRoot_getRemoteRefreshSeconds(row);
	DbValue_setOptionNumber(row, "refresh_time", last);
}

OsDate DbRoot_getRemoteRefreshNextTime(BIG row)
{
	double last = DbValue_getOptionNumber(row, "refresh_time", 0);
	if (last == 0)
	{
		OsDate now = OsDate_initActual();
		last = OsDate_asNumber(&now);
	}
	last += _DbRoot_getRemoteRefreshSeconds(row);

	OsDate next = OsDate_initFromNumber(last);
	return next;
}

BIG DbRoot_getRemoteRefreshTimeRemain(BIG row)
{
	OsDate nextDate = DbRoot_getRemoteRefreshNextTime(row);
	OsDate nowDate = OsDate_initActual();

	BIG remainSeconds = OsDate_differenceSeconds(&nextDate, &nowDate);
	return remainSeconds;
}

BOOL DbRoot_isRemoteSaveItIntoFile(BIG row)
{
	return DbValue_getOptionNumber(row, "save_file", 0);
}

DbTable* DbRoot_findTableNameRemote(const UNI* name, BIG remoteRow)
{
	DbTable* table = DbTables_findNameRemote(g_DbRoot->tables, name, remoteRow);
	//if (!table)
	//	DbTables_findNameRemote(g_DbRoot->tables_notSaved, name, remoteRow);
	return table;
}

BOOL DbRoot_updateRemote(BIG remoteRow)
{
	if (DbColumnN_sizeActive(DbRoot_subs(), remoteRow) > 0)
	{
		if (DbRoot_getRemoteRefreshTimeRemain(remoteRow) > 0)
			return FALSE;	//no need to refresh
	}

	OsODBC* odbc = DbRoot_connectRemote(remoteRow, FALSE);
	if (odbc)
	{
		//tables
		StdArr tableNames;
		tableNames.num = OsODBC_getTablesList(odbc, (UNI***)&tableNames.ptrs);
		BIG i;
		for (i = 0; i < tableNames.num; i++)
		{
			const UNI* tableName = tableNames.ptrs[i];

			DbTable* table = DbRoot_findTableNameRemote(tableName, remoteRow);
			if (!table)
				table = DbTables_create(g_DbRoot->tables, remoteRow);

			DbTable_setName(table, tableName);
			DbTable_setRemote(table, remoteRow);
		}

		//columns
		for (i = 0; i < tableNames.num; i++)
		{
			DbTable* table = DbRoot_findTableNameRemote(tableNames.ptrs[i], remoteRow);

			//primary key
			StdArr primaryColumnNames;
			primaryColumnNames.num = OsODBC_getPrimaryColumnList(odbc, tableNames.ptrs[i], (UNI***)&primaryColumnNames.ptrs);

			//foreign keys
			StdArr fkSrcColumnNames;
			StdArr fkDstTableNames;
			fkSrcColumnNames.num = fkDstTableNames.num = OsODBC_getForeignColumnList(odbc, tableNames.ptrs[i], (UNI***)&fkSrcColumnNames.ptrs, (UNI***)&fkDstTableNames.ptrs);

			//names
			StdArr columnNames;
			StdBigs columnTypes;
			columnNames.num = OsODBC_getColumnsList(odbc, tableNames.ptrs[i], (UNI***)&columnNames.ptrs, &columnTypes.ptrs);
			StdBigs_setAlloc(&columnTypes, columnNames.num);

			BIG ii;
			for (ii = 0; ii < columnTypes.num; ii++)
			{
				const UNI* colName = columnNames.ptrs[ii];

				//primary
				//if (primaryColumnNames.num && Std_cmpUNI(colName, (UNI*)primaryColumnNames.ptrs[0]))
				//	continue;

				DbColumn* column = DbColumns_findName(table->columns, colName);
				if (!column)
				{
					OsODBCType type = columnTypes.ptrs[ii];
					if (type == OsODBC_UNKNOWN)
						continue;

					DbFormatTYPE format = OsODBC_STRING;
					if (type == OsODBC_NUMBER)
						format = DbFormat_NUMBER_1;
					if (type == OsODBC_STRING)
						format = DbFormat_TEXT;
					if (type == OsODBC_DATE)
						format = DbFormat_DATE;

					//if (btable)
					//	format = DbFormat_LINK_1;

					column = DbTable_createColumnFormat(table, format, colName, 0/*btable*/);
					column->loaded = FALSE;
					column->remote = TRUE;
					DbColumn_setOptionNumber(column, "remote", 1);
				}

				//foreign
				//DbTable* btable = 0;
				BIG jj;
				for (jj = 0; jj < fkSrcColumnNames.num; jj++)
				{
					if (Std_cmpUNI(fkSrcColumnNames.ptrs[jj], colName))
						column->remoteForeign = DbRoot_findTableNameRemote(fkDstTableNames.ptrs[jj], remoteRow);
				}

				//primary
				//if (DbColumn_isType1(column) && Std_cmpUNI(colName, primaryColumnName))
				//	table->rows = (DbColumn1*)column;
			}

			//remove un-used Columns
			BIG columnsRow = DbRoot_findChildType(DbTable_getRow(table), "columns");
			BIG columnRow;
			UBIG i1 = 0;
			while ((columnRow = DbColumnN_jump(DbRoot_subs(), columnsRow, &i1, 1)) >= 0)
			{
				DbColumn* column = DbRoot_findColumn(columnRow);
				if (column->remote)
				{
					UNI name[64];
					DbColumn_getName(column, name, 64);

					BOOL found = FALSE;
					BIG ii;
					for (ii = 0; ii < columnNames.num; ii++)
					{
						if (Std_cmpUNI(name, columnNames.ptrs[ii]))
							found = TRUE;
					}
					if (!found)
						DbRoot_removeRow(columnRow);
				}
				i1++;
			}

			StdArr_freeFn(&columnNames, (StdArrFREE)&Std_deleteUNI);
			StdBigs_free(&columnTypes);

			StdArr_freeFn(&primaryColumnNames, (StdArrFREE)&Std_deleteUNI);
			StdArr_freeFn(&fkSrcColumnNames, (StdArrFREE)&Std_deleteUNI);
			StdArr_freeFn(&fkDstTableNames, (StdArrFREE)&Std_deleteUNI);
		}

		//remove un-used Tables
		BIG tableRow;
		UBIG i1 = 0;
		while ((tableRow = DbColumnN_jump(DbRoot_subs(), remoteRow, &i1, 1)) >= 0)
		{
			if (_DbRoot_cmpType(tableRow, "table"))
			{
				UNI name[64];
				DbRoot_getName(tableRow, name, 64);

				BOOL found = FALSE;
				BIG ii;
				for (ii = 0; ii < tableNames.num; ii++)
				{
					if (Std_cmpUNI(name, tableNames.ptrs[ii]))
						found = TRUE;
				}
				if (!found)
					DbRoot_removeRow(tableRow);
			}
			i1++;
		}

		StdArr_freeFn(&tableNames, (StdArrFREE)&Std_deleteUNI);

		OsODBC_delete(odbc);
	}
	else
	{
		//errors nezobrazuji, jenom ikona Remote folder je \E8erven\E1 ...
	}

	return TRUE;
}

BOOL DbRoot_tryRefreshRemote(void)
{
	BOOL refreshed = FALSE;

	BIG rootRow = DbTable_firstRow(g_DbRoot->info);

	BIG folderRow;
	UBIG i0 = 0;
	while ((folderRow = DbColumnN_jump(DbRoot_subs(), rootRow, &i0, 1)) >= 0)
	{
		if (_DbRoot_cmpType(folderRow, "remote"))
		{
			if (DbRoot_getRemoteRefreshTimeRemain(folderRow) < 0)
			{
				DbTables_refreshRemote(g_DbRoot->tables, folderRow);

				DbRoot_updateRemoteRefreshTime(folderRow);
				refreshed = TRUE;
			}
		}

		i0++;
	}

	return refreshed;
}

BOOL DbRoot_tryUpdateFileIndexes(void)
{
	return FileProject_tryUpdateIndex();
}

void DbRoot_print(BIG row, UINT deep)
{
	printf("%lld ", row);

	int t;
	for (t = 0; t < deep; t++)
		printf("\t");

	Std_printUNI(DbColumnString32_get(DbRoot_getColumnOptions(), row));
	printf(" %lld\n", DbRoot_ref_row(row));

	UBIG i = 0;
	BIG it;
	while ((it = DbColumnN_jump(DbRoot_subs(), row, &i, 1)) >= 0)
	{
		DbRoot_print(it, deep + 1);
		i++;
	}
}
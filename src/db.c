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

 //collaborate
#include "os.h"
#include "std.h"
#include "language.h"
#include "log.h"
#include "file.h"
#include "map.h"

//header
#include "db.h"

OsDateTYPE UiIniSettings_getDateFormat(void);

typedef struct DbTables_s DbTables;
typedef struct DbJointed_s DbJointed;

typedef enum
{
	DbColumn_1,
	DbColumn_N,
	DbColumn_STRING_32,	    //Text:		utf-32
	//DbColumn_STRING_8,	//Text:		utf-8
} DbColumnTYPE;

DbTable* DbRoot_getInfoTable(void);
DbTable* DbRoot_addTableNotSave(void);
DbTables* DbRoot_getTables(void);
BIG DbRoot_getRow(FileRow fileId);
UNI* _DbRoot_getOptionString(const BIG row, const char* name, const UNI* defValue, UNI* out, const UBIG outMaxSize);
double _DbRoot_getOptionNumber(const BIG row, const char* name, double defValue);
void _DbRoot_setOptionString(const BIG row, const char* name, const UNI* value);
void _DbRoot_setOptionNumber(const BIG row, const char* name, double value);
BOOL _DbRoot_cmpOptionString(const BIG row, const char* name, const UNI* defValue, const UNI* compare);
static void _DbRoot_createColumnRow(DbColumn* column);
static void _DbRoot_createTableRow(DbTable* table, BIG tablesRow);
BOOL DbRoot_removeTable(DbTable* table);
void DbRoot_replaceAndRemoveRow(BIG oldRow, BIG newRow);
BOOL DbRoot_updateRemote(BIG row);
BOOL DbRoot_isRemoteSaveItIntoFile(BIG row);
DbColumn* DbRoot_findColumnExisted(BIG row);

UBIG DbTables_num(const DbTables* self);
DbTable* DbTables_get(const DbTables* self, UBIG i);

DbColumns* DbTable_getColumns(const DbTable* self);
BOOL DbTable_isRowActive(const DbTable* self, const BIG r);
UBIG DbTable_numRows(const DbTable* self);
BIG DbTable_jumpRows(DbTable* self, UBIG* pos, BIG jumps);
BIG DbTable_jumpLinks(DbTable* self, const double* links, UBIG* pos, BIG jumps);
BIG DbTable_jumpLinksFirst(DbTable* self, const double* links);
BIG DbTable_findRow(const DbTable* self, FileRow row);
DbColumn1* DbTable_getColumnRows(const DbTable* self);
BIG* DbTable_getTempMoves(DbTable* self);
UBIG DbTable_addRowEx(DbTable* self, FileRow row);
void DbTable_removeRow(DbTable* self, UBIG row);
void DbTable_setMaxRow(DbTable* self, FileRow lastRow);
BOOL DbTable_isLoaded(DbTable* self);
//BIG DbTable_getRemoteRow(const DbTable* self);

BOOL DbTable_isRemoteSaveItIntoFile(const DbTable* self);

const UBIG DbColumns_num(const DbColumns* self);
DbColumn* DbColumns_get(const DbColumns* self, const UINT i);
DbTable* DbColumns_getTable(const DbColumns* self);

DbFilter* DbFilter_newCopy(const DbFilter* src);
void DbFilter_delete(DbFilter* self);
static BOOL _DbFilter_needUpdate(DbFilter* self);
static void _DbFilter_executeEx(DbFilter* self, StdBigs* poses);
const StdBigs* DbFilter_getRows(const DbFilter* self);
BOOL DbFilter_cmp(const DbFilter* a, const DbFilter* b);

FileHead DbColumn_fileGetHead(const DbColumn* self, const UBIG r, const UBIG ri);

DbColumnTYPE DbColumnFormat_findColumnType(DbFormatTYPE format);
const char* DbColumnFormat_findColumnName(DbFormatTYPE format);

DbColumn* DbInsight_getItemColumn(const DbInsight* self, BIG i);
void DbInsight_delete(DbInsight* self);
BOOL DbInsight_cmp(const DbInsight* a, const DbInsight* b);
BOOL DbInsight_execute(DbInsight* self);

void DbJointed_delete(DbJointed* self);
BOOL DbJointed_needUpdate(DbJointed* self);
BOOL DbJointed_execute(DbJointed* self);
BOOL DbJointed_cmp(const DbJointed* a, const DbJointed* b);

//database
#include "db/db_column_hald.h"
#include "db/db_column.h"

#include "db/db_column_1.h"
#include "db/db_column_n.h"
#include "db/db_column_string32.h"

#include "db/db_column_switch.h"
#include "db/db_columns.h"
#include "db/db_column_convert.h"
#include "db/db_column_format.h"

#include "db/db_table.h"

#include "db/db_insights.h"
#include "db/db_insights_funcs.h"

#include "db/db_filter_funcs.h"
#include "db/db_filter.h"

#include "db/db_jointed.h"

#include "db/db_root.h"

#include "db/db_value.h"
#include "db/db_rows.h"
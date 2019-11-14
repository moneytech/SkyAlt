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

typedef struct DbRoot_s DbRoot;
typedef struct DbTable_s DbTable;
typedef struct DbColumns_s DbColumns;
typedef struct DbColumn_s DbColumn;
typedef struct DbColumn1_s DbColumn1;
typedef struct DbColumnN_s DbColumnN;
typedef struct DbColumnString32_s DbColumnString32;
typedef struct DbFilter_s DbFilter;

typedef enum
{
	DbFormat_NUMBER_1,
	DbFormat_NUMBER_N,
	DbFormat_CURRENCY,
	DbFormat_PERCENTAGE,
	DbFormat_RATING,
	DbFormat_SLIDER,
	DbFormat_CHECK,

	DbFormat_MENU,
	DbFormat_TAGS,
	DbFormat_LINK_1,
	DbFormat_LINK_N,

	DbFormat_FILE_1,
	DbFormat_FILE_N,

	DbFormat_TEXT,
	DbFormat_PHONE,
	DbFormat_URL,
	DbFormat_EMAIL,
	DbFormat_LOCATION,

	DbFormat_DATE,

	DbFormat_ROW,
}DbFormatTYPE;

const char* DbColumnFormat_findColumnLang(DbFormatTYPE format);
DbFormatTYPE DbColumnFormat_findColumn(const DbColumn* column);

typedef struct DbColumnConvertPrm_s
{
	DbColumn* srcColumn;
	DbColumn* bTableColumn;
	OsDateTYPE dateType;
} DbColumnConvertPrm;

typedef struct DbColumnConvert_s DbColumnConvert;
typedef DbColumn* DbConvertCallback(const DbColumnConvert* self, DbColumnConvertPrm prm);

typedef struct DbColumnConvert_s
{
	DbFormatTYPE srcType;
	DbFormatTYPE dstType;

	DbConvertCallback* func;
} DbColumnConvert;

DbColumn* DbColumnConvert_convert(const DbColumnConvert* self, DbColumnConvertPrm prm);
DbColumn* DbColumnConvert_convertEx(DbColumn* srcColumn, DbFormatTYPE dstType, DbColumn* btableColumn);
UBIG DbColumnConvert_num(DbFormatTYPE srcType);
const DbColumnConvert* DbColumnConvert_get(DbFormatTYPE srcType, BIG index);
const DbColumnConvert* DbColumnConvert_find(DbFormatTYPE srcType, DbFormatTYPE dstType);

UBIG DbFilterSelectFunc_num(DbFormatTYPE srcFormat);
const char* DbFilterSelectFunc_getName(DbFormatTYPE srcFormat, BIG index);
UNI* DbFilterSelectFunc_getList(DbFormatTYPE srcFormat);



void DbRoot_initProgress(void);
void DbRoot_freeProgress(void);
volatile StdProgress* DbRoot_getProgress(void);

BOOL DbRoot_is(void);
BOOL DbRoot_new(void);
BOOL DbRoot_newOpen(void);
BOOL DbRoot_newCreate(void);
void DbRoot_delete(void);

void DbRoot_updateTables(void);

DbColumnString32* DbRoot_getColumnOptions(void);
DbColumnN* DbRoot_getColumnSubs(void);

void DbRoot_save(void);
UBIG DbRoot_bytes(BOOL all);
const UBIG DbRoot_numEx(BOOL all);
const UBIG DbRoot_numColumns(BOOL all);
const UBIG DbRoot_numRecords(BOOL all);
const UBIG DbRoot_numCells(BOOL all, BOOL realRows);
UBIG DbRoot_numInfoChanges(void);
UBIG DbRoot_numAllChanges(void);

DbTable* DbRoot_createTable(BIG folderRow);
DbTable* DbRoot_createTableExample(BIG folderRow);
BIG DbRoot_createFolderRow(void);
void DbRoot_createPage(BIG parent);

BIG DbRoot_getRootRow(void);

void DbRoot_removeRow(BIG row);
UBIG DbRoot_duplicateRow(BIG srcRow, BIG parentRow);

BIG DbRoot_findParent(UBIG findRow);
BOOL DbRoot_isParentRow(BIG row, BIG findParentRow);
DbTable* DbRoot_findParentTable(BIG row);
BIG DbRoot_findParentTableRow(BIG row);
BIG DbRoot_findParentType(UBIG row, const char* typeValue);
BIG DbRoot_findChildType(UBIG row, const char* typeValue);
UBIG DbRoot_findOrCreateChildType(UBIG row, const char* typeValue);

DbColumn* DbRoot_findColumn(BIG row);
DbTable* DbRoot_findTable(BIG row);

BOOL DbRoot_hasConnectionBTables(BIG row);

BIG DbRoot_getPanelLeft(void);
BIG DbRoot_getPanelRight(void);
void DbRoot_setPanelLeft(BIG row);
void DbRoot_setPanelRight(BIG row);
void DbRoot_swapPanelRight(void);

BOOL DbRoot_cmpName(BIG row, const UNI* compare);
UNI* DbRoot_getName(BIG row, UNI* out, const UBIG outMaxSize);
void DbRoot_setName(BIG row, const UNI* value);

BOOL DbRoot_isType_root(const BIG row);
BOOL DbRoot_isType_folder(const BIG row);
BOOL DbRoot_isType_table(const BIG row);
BOOL DbRoot_isType_page(const BIG row);
BOOL DbRoot_isType_column(const BIG row);
BOOL DbRoot_isTypeView_filter(const BIG row);
BOOL DbRoot_isTypeView_cards(const BIG row);
BOOL DbRoot_isTypeView_group(const BIG row);
BOOL DbRoot_isTypeView_kanban(const BIG row);
BOOL DbRoot_isTypeView_calendar(const BIG row);
BOOL DbRoot_isTypeView_timeline(const BIG row);
BOOL DbRoot_isTypeView_chart(const BIG row);
BOOL DbRoot_isTypeView_map(const BIG row);
BOOL DbRoot_isTypeView(const BIG row);

BIG DbRoot_createView_cards(const BIG parentRow);
BIG DbRoot_createView_filter(const BIG parentRow);
BIG DbRoot_createView_group(const BIG parentRow);
BIG DbRoot_createView_kanban(const BIG parentRow);
BIG DbRoot_createView_calendar(const BIG parentRow);
BIG DbRoot_createView_timeline(const BIG parentRow);
BIG DbRoot_createView_chart(const BIG parentRow);
BIG DbRoot_createView_map(const BIG parentRow);

DbColumn* DbTable_getIdsColumn(const DbTable* self);
BOOL DbTable_isIdsColumn(const DbTable* self, const DbColumn* column);
UBIG DbTable_addRow(DbTable* self);
BIG DbTable_getRow(const DbTable* self);
void DbTable_loadLast(DbTable* self);
const UNI* DbTable_getName(const DbTable* self, UNI* out, const UBIG outMaxSize);
DbColumns* DbTable_getColumns(const DbTable* self);
DbColumn* DbTable_createColumnFormat(DbTable* self, DbFormatTYPE format, const UNI* name, DbTable* btable);

const UBIG DbColumns_num(const DbColumns* self);
DbColumn* DbColumns_get(const DbColumns* self, const UINT i);

BIG DbColumn_getRow(const DbColumn* self);
DbTable* DbColumn_getBTable(const DbColumn* self);
const UNI* DbColumn_getName(const DbColumn* self, UNI* out, const UBIG outMaxSize);
DbTable* DbColumn_getTable(const DbColumn* self);
UNI* DbColumn_getStringCopyLong(const DbColumn* self, const UBIG r, StdString* out);
UNI* DbColumn_getStringCopyWithFormatLong(const DbColumn* self, const UBIG r, StdString* out);
void DbColumn_setStringCopy(DbColumn* self, const UBIG r, UBIG index, const UNI* value);
double DbColumn_getFlt(const DbColumn* self, const UBIG r, UBIG index);
void DbColumn_setFlt(DbColumn* self, const UBIG r, UBIG index, double value);

void DbColumn_insert_before(DbColumn* self, const UBIG rSrc, const UBIG rDst, const UBIG rDstAfter);
void DbColumn_insert_after(DbColumn* self, const UBIG rSrc, const UBIG rDst, const UBIG rDstAfter);

double DbColumn_getOptionNumber(const DbColumn* self, const char* name);
UNI* DbColumn_getOption(const DbColumn* self, const char* name, UNI* out, const UBIG outMaxSize);
BOOL DbColumn_cmpOption(const DbColumn* self, const char* name, const UNI* compare);

BOOL DbColumn_isType1(const DbColumn* self);

DbColumn* DbColumn_moveToTable(DbColumn* self);



BIG DbColumnN_jump(const DbColumnN* self, UBIG r, UBIG* pos, BIG jumps);
DbTable* DbColumnN_getBTable(const DbColumnN* self);
void DbColumnN_clearRow(DbColumnN* self, const UBIG rSrc);
void DbColumnN_addAllTable(DbColumnN* self, const UBIG rSrc);




DbColumnN* DbFilter_getColumnGroupSubs(const DbFilter* self);
DbColumnN* DbFilter_getColumnGroupRows(const DbFilter* self);
DbColumn1* DbFilter_getColumnGroupCount(const DbFilter* self);
void DbFilter_clearShortsAndGroups(DbFilter* self);
void DbFilter_addGroup(DbFilter* self, DbColumn* column, BOOL ascending, BOOL addOnlyOneRow);

typedef struct DbValue_s
{
	DbColumn* column;
	BIG row;
	UBIG index;

	const char* lang_id;
	const char* option;

	UNI* staticText;

	StdString result;
	StdString resultCmp;
	StdString resultTemp;

	BOOL changed;

	UNI* staticPre;
	UNI* staticPost;

	BOOL formated;
	DbFormatTYPE optionFormat;

	BOOL ignoreUpdateResult;
} DbValue;

DbValue DbValue_initEmpty(void);
DbValue DbValue_initCopy(const DbValue* src);

DbValue DbValue_initStaticCopy(const UNI* value);
DbValue DbValue_initStaticCopyUnderline(const UNI* value);
DbValue DbValue_initStatic(UNI* value);
DbValue DbValue_initStaticCopyCHAR(const char* value);
DbValue DbValue_initNumber(double value);
DbValue DbValue_initGET(DbColumn* column, BIG row);
DbValue DbValue_initLang(const char* lang_id);
DbValue DbValue_initOptionEx(DbColumnString32* column, BIG row, const char* option, const UNI* defValue);
DbValue DbValue_initOption(BIG row, const char* option, const UNI* defValue);
DbValue DbValue_initOptionEnable(BIG row);
void DbValue_free(DbValue* self);

const UNI* DbValue_result(const DbValue* self);
BIG DbValue_getRow(const DbValue* self);
void DbValue_setRow(DbValue* self, BIG row, UBIG index);

BOOL DbValue_isRowEnable(const DbValue* self);

BOOL DbValue_isType1(const DbValue* self);
BOOL DbValue_isTypeN(const DbValue* self);
BOOL DbValue_isTypeString32(const DbValue* self);
BOOL DbValue_isTypeLink(const DbValue* self);
BOOL DbValue_isTypeFile(const DbValue* self);

UBIG DbValue_getN(const DbValue* self);
double DbValue_getNumber(const DbValue* self);
void DbValue_setNumber(DbValue* self, double value);

void DbValue_setFormated(DbValue* self, BOOL formated);
BOOL DbValue_hasChanged(DbValue* self);
void DbValue_setTextCopy(DbValue* self, const UNI* value);
void DbValue_setText(DbValue* self, UNI* value);

void DbValue_setCd(DbValue* self, Rgba cd);
Rgba DbValue_getCd(const DbValue* self);

void DbValue_setDate(DbValue* self, OsDate date);
OsDate DbValue_getDate(const DbValue* self);

FileRow DbValue_getFileId(const DbValue* self);
void DbValue_getFileExt(const DbValue* self, const UBIG index, UNI ext[8]);
void DbValue_getFileExt_char(const DbValue* self, const UBIG index, char ext[8]);
UBIG DbValue_getFileSize(const DbValue* self, const UBIG index);
UBIG DbValue_readFileCache(const DbValue* self, const UBIG index, UBIG pos, UBIG size, UCHAR* buff);
void DbValue_importFile(DbValue* self, OsFile* srcFile, const UBIG index, UNI ext[8], volatile StdProgress* progress);
void DbValue_importData(DbValue* self, UBIG size, UCHAR* data, const UBIG index, UNI ext[8], volatile StdProgress* progress);
void DbValue_exportFile(const DbValue* self, OsFile* dstFile, const UBIG index, volatile StdProgress* progress);

BOOL DbValue_getMapPosition(const DbValue* self, Vec2f* out);

DbFormatTYPE DbValue_getFormat(const DbValue* self);
BOOL DbValue_isFormatUnderline(const DbValue* self);

const UNI* DbValue_now_getText(DbValue* self, BIG row);
double DbValue_getOptionNumber(BIG row, const char* name, double defValue);
void DbValue_setOptionNumber(BIG row, const char* name, double value);


typedef struct DbValues_s
{
	DbValue* values;
	UBIG num;
} DbValues;

DbValues DbValues_init(void);
DbValues DbValues_initCopy(const DbValues* src);
void DbValues_free(DbValues* self);
DbValue* DbValues_add(DbValues* self, DbValue value);
DbValue* DbValues_findRow(const DbValues* self, BIG row);
void DbValues_updateText(DbValues* self);

typedef struct DbRows_s
{
	DbTable* table;
	DbColumn* column;
	BIG row;

	StdBigs array;
	BOOL arrayStatic;

	DbFilter* filter;
	DbValue filterSearch;
} DbRows;

DbRows DbRows_initCopy(const DbRows* src);
void DbRows_free(DbRows* self);
DbRows DbRows_initEmpty(void);
DbRows DbRows_initTable(DbTable* table);
DbRows DbRows_initLink(DbColumnN* column, BIG row);
DbRows DbRows_initArray(DbTable* table, StdBigs array);

void DbRows_setBaseRow(DbRows* self, BIG row);
BIG DbRows_getBaseRow(const DbRows* self);
DbTable* DbRows_getTable(const DbRows* self);

BOOL DbRows_isColumnValid(const DbRows* self);
BOOL DbRows_is(const DbRows* self);

FileRow DbRows_getFileId(const DbRows* self, UBIG pos);
BIG DbRows_getRow(const DbRows* self, UBIG pos);
BOOL DbRows_isRow(const DbRows* self, UBIG pos);
UBIG DbRows_getSize(const DbRows* self);
BOOL DbRows_isInside(const DbRows* self, UBIG pos);
BIG DbRows_findSubType(BIG row, const char* subType);
DbRows DbRows_initSubs(DbTable* table, const char* subType, BOOL onlyEnable);
DbValues DbRows_getSubs(BIG row, const char* subType, BOOL onlyEnable, int maxN);
DbValue DbRows_getSubsCell(BIG row, const char* subType, BOOL onlyEnable, BIG index, BIG cellRow);
DbValue DbRows_getSubsOption(BIG row, const char* subType, BOOL onlyEnable, BIG index, const char* optionName, BOOL columnDirect);
DbColumn* DbRows_getSubsColumn(BIG row, const char* subType, BOOL onlyEnable, BIG index);
BIG DbRows_getSubsRow(BIG row, const char* subType, BOOL onlyEnable, BIG index);
UBIG DbRows_getSubsNum(BIG row, const char* subType, BOOL onlyEnable);
DbValues DbRows_getOptions(BIG row, const char* subType, const char* valueType, BOOL onlyEnable);
DbRows DbRows_getSubsArray(BIG row, const char* subType);
BIG DbRows_getAddSubsLine(BIG row, const char* subType);

BOOL DbRows_hasChanged(DbRows* self);

DbRows DbRows_initTables(void);
DbRows DbRows_initFilter(BIG row);
void DbRows_forceEmptyFilter(DbRows* self);
BOOL DbRows_hasFilterSubActive(BIG row, const char* subName);
BOOL DbRows_hasColumnsSubDeactive(BIG row, const char* columnsName);

BIG DbRows_addNewRow(DbRows* self);
void DbRows_removeRow(DbRows* self, BIG row);
void DbRows_removeFile(DbRows* self, FileRow fileRow);
void DbRows_addLinkRow(DbRows* self, BIG row);
void DbRows_setLinkRow(DbRows* self, BIG row);
void DbRows_insertIDbefore(DbRows* self, BIG row, BIG findRow);
void DbRows_insertIDafter(DbRows* self, BIG row, BIG findRow);
BOOL DbRows_isSubChild(DbRows* self, BIG origRow, BIG findRow);
BIG DbRows_findLinkPos(DbRows* self, BIG findRow);

BOOL DbRows_getColumnMinMax(DbRows* self, DbColumn1* column, double* mn, double* mx);
BOOL DbRows_getColumnsMinMax(DbRows* self, DbColumn1** columns, double* mn, double* mx);




void DbColumn_addLinksToColumn(DbColumn* dst, DbColumn* src, DbRows* rows);
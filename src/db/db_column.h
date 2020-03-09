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

typedef struct DbColumn_s
{
	FileRow fileId;

	DbColumns* parent;

	TableItems data;
	TableItems changes;

	DbColumnTYPE type;
	DbFormatTYPE format;
	BOOL loaded;

	UBIG numChanges;
	DbColumn* copyColumnAsk;

	DbColumn* summary_origColumn;
	BOOL summary_links;	//nahradit tyto nastavením(a mnoho ostatních) pøímém ètení z DbRoot pøes row .......
	BOOL summary_group;

	BOOL remote;
	DbTable* remoteForeign;
	//BOOL remotePrimaryKey;
	//OsODBCType remoteType;

	DbInsight* insight;
	DbInsight* insightTable;

	BIG links_mirror_changes;
	DbColumn* links_mirrored;	//original table

	DbColumn* links_filtered;
	DbFilter* links_filtered_filter;

	DbJointed* links_jointed;

	BOOL err;

	char* extra_name;


	double default_number;
	UNI* default_text;

} DbColumn;

DbColumn DbColumn_init(DbColumnTYPE type, DbFormatTYPE format, DbColumns* parent, double defValue)
{
	DbColumn self;

	self.fileId = FileRow_initEmpty();
	self.parent = parent;
	self.data = TableItems_init(defValue);
	self.changes = TableItems_init(0);
	self.type = type;
	self.format = format;
	self.loaded = TRUE;
	self.numChanges = 0;
	self.copyColumnAsk = 0;

	self.summary_origColumn = 0;
	self.summary_links = FALSE;
	self.summary_group = FALSE;

	self.remote = FALSE;
	self.remoteForeign = 0;

	self.insight = 0;
	self.insightTable = 0;

	self.links_mirror_changes = -1;
	self.links_mirrored = 0;

	self.links_filtered = 0;
	self.links_filtered_filter = 0;

	self.links_jointed = 0;

	self.err = FALSE;

	self.extra_name = 0;


	self.default_number = 0;
	self.default_text = 0;

	return self;
}

const char* DbColumn_getExtraName(const DbColumn* self)
{
	return self->extra_name;
}
void DbColumn_setExtraName(DbColumn* self, const char* name)
{
	Std_deleteCHAR(self->extra_name);
	self->extra_name = Std_newCHAR(name);
}



BOOL DbColumn_isErr(const DbColumn* self)
{
	return self->err;
}

DbColumn* DbColumn_findColumnBase(const DbColumn* self)
{
	return self->summary_origColumn ? DbColumn_findColumnBase(self->summary_origColumn) : (DbColumn*)self;
}

DbColumn* DbColumn_getSummaryOrigColumn(const DbColumn* self)
{
	return self->summary_origColumn;
}

BOOL DbColumn_isSummaryLinks(const DbColumn* self)
{
	return self->summary_links;
}
BOOL DbColumn_isSummaryGroup(const DbColumn* self)
{
	return self->summary_group;
}

BIG DbColumn_getRow(const DbColumn* self)
{
	return DbRoot_getRow(self->fileId);
}

double DbColumn_getDefValue(const DbColumn* self)
{
	return self->data.defValue;
}

const UNI* DbColumn_getName(const DbColumn* self, UNI* out, const UBIG outMaxSize)
{
	return DbRoot_getName(DbColumn_getRow(self), out, outMaxSize);
}

const UNI* DbColumn_getPath(const DbColumn* self, UNI* out, const UBIG outMaxSize)
{
	UNI* orig = out;

	DbRoot_getName(DbTable_getRow(DbColumn_getTable(self)), out, outMaxSize);
	UBIG N = Std_sizeUNI(out);
	if (N < outMaxSize - 5)
	{
		out += N;
		out = Std_copyUNI_char(out, outMaxSize - N - 1, " : ");

		DbRoot_getName(DbColumn_getRow(self), out, outMaxSize - N - 3);
	}

	return orig;
}

void DbColumn_setName(DbColumn* self, const UNI* name)
{
	DbRoot_setName(DbColumn_getRow(self), name);
}

void DbColumn_setRemote(DbColumn* self, BOOL remote)
{
	self->remote = remote;
}
BOOL DbColumn_isRemote(const DbColumn* self)
{
	return self->remote;
}

OsODBCType DbColumn_getRemoteType(const DbColumn* self)
{
	if (self->type == DbColumn_STRING_32)
		return OsODBC_STRING;

	if (self->format == DbFormat_DATE)
		return OsODBC_DATE;

	return OsODBC_NUMBER;
}

void DbColumn_setInsight(DbColumn* self, DbInsight* insight)
{
	if (self->insight)
	{
		if (insight && DbInsight_cmp(self->insight, insight))
		{
			DbInsight_delete(insight);
		}
		else
		{
			DbInsight_delete(self->insight);
			self->insight = insight;
		}
	}
	else
		self->insight = insight;
}

void DbColumn_setInsightTable(DbColumn* self, DbInsight* insightTable)
{
	if (self->insightTable)
	{
		if (insightTable && DbInsight_cmp(self->insightTable, insightTable))
		{
			DbInsight_delete(insightTable);
		}
		else
		{
			DbInsight_delete(self->insightTable);
			self->insightTable = insightTable;
		}
	}
	else
		self->insightTable = insightTable;
}

void DbColumn_setLinkFiltered(DbColumn* self, DbFilter* filter)
{
	if (self->links_filtered_filter)
	{
		if (filter && DbFilter_cmp(self->links_filtered_filter, filter))
		{
			DbFilter_delete(filter);
		}
		else
		{
			DbFilter_delete(self->links_filtered_filter);
			self->links_filtered_filter = filter;
		}
	}
	else
		self->links_filtered_filter = filter;
}

void DbColumn_setLinkJointed(DbColumn* self, DbJointed* links_jointed)
{
	if (self->links_jointed)
	{
		if (links_jointed && DbJointed_cmp(self->links_jointed, links_jointed))
		{
			DbJointed_delete(links_jointed);
		}
		else
		{
			DbJointed_delete(self->links_jointed);
			self->links_jointed = links_jointed;
		}
	}
	else
		self->links_jointed = links_jointed;
}

void DbColumn_freeFilterAndInsight(DbColumn* self)
{
	DbColumn_setInsight(self, 0);
	DbColumn_setInsightTable(self, 0);
	DbColumn_setLinkFiltered(self, 0);
	DbColumn_setLinkJointed(self, 0);
}



void DbColumn_setDefaultString32(DbColumn* self, const UNI* text)
{
	Std_deleteUNI(self->default_text);
	self->default_number = 0;
	self->default_text = Std_newUNI(text);
}

void DbColumn_setDefaultNumber(DbColumn* self, double number)
{
	DbColumn_setDefaultString32(self, 0);

	DbColumnTYPE columnType = DbColumnFormat_findColumnType(self->type);
	switch (columnType)
	{
		case DbColumn_1:	self->default_number = number;	break;
		case DbColumn_N:	self->default_number = number;	break;
		case DbColumn_STRING_32: self->default_text = Std_newNumberPrecision(number, -1);	break;
	}
}



void DbColumn_free(DbColumn* self)
{
	DbColumn_setDefaultString32(self, 0);

	TableItems_free(&self->data);
	TableItems_free(&self->changes);

	Std_deleteCHAR(self->extra_name);
}

void DbColumn_clearItems(DbColumn* self)
{
	TableItems_clear(&self->data);
	TableItems_clear(&self->changes);
}

TableItems* DbColumn_getHalds(DbColumn* self)
{
	return &self->data;
}

TableItem* DbColumn_getItem(DbColumn* self, const UBIG r)
{
	return TableItems_get(&self->data, r);
}
const TableItem* DbColumn_getItemConst(const DbColumn* self, const UBIG r)
{
	return TableItems_getConst(&self->data, r);
}

DbTable* DbColumn_getTable(const DbColumn* self)
{
	return self->parent ? DbColumns_getTable(self->parent) : 0;
}

DbColumnTYPE DbColumn_getType(const DbColumn* self)
{
	return self->type;
}

UBIG DbColumn_numRows(const DbColumn* self)
{
	DbTable* table = DbColumn_getTable(self);
	return table ? DbTable_numRows(table) : 0;
}

DbColumn1* DbColumn_getActive(const DbColumn* self)
{
	DbTable* table = DbColumn_getTable(self);
	return table ? DbTable_getColumnRows(table) : 0;
}

void DbColumn_setChange(DbColumn* self, UBIG i)
{
	TableItems_setBitTo1(&self->changes, i);
	self->numChanges++;
}
void DbColumn_setChangeAll(DbColumn* self)
{
	TableItems_resetAllTo1(&self->changes);
	self->numChanges++;
}

BOOL DbColumn_isChange(DbColumn* self, UBIG i)
{
	return TableItems_getBit(&self->changes, i);
}

void DbColumn_reset_save(DbColumn* self)
{
	TableItems_resetAllTo0(&self->changes);
	self->numChanges = 0;
}

void DbColumn_updateNumberOfRows(DbColumn* self, const UBIG old_num_rows)
{
	const UBIG NUM_ROWS = DbColumn_numRows(self);
	TableItems_add(&self->data, NUM_ROWS);

	TableItems_addBits(&self->changes, NUM_ROWS);

	//set new-ones to 1
	BIG i;
	for (i = old_num_rows; i < NUM_ROWS; i++)
		DbColumn_setChange(self, i);

	if (old_num_rows != NUM_ROWS)
		self->numChanges++;
}

/*void DbColumn_resetChanges(DbColumn* self)
{
	TableItems_resetAllTo1(&self->changes, 0);
	self->numChanges++;
}*/

BOOL DbColumn_isChangedSave(const DbColumn* self)
{
	return TableItems_isChangedSave(&self->changes);
}
BOOL DbColumn_isChangedExe(DbColumn* self)
{
	return TableItems_isChangedExe(&self->changes);
}

UBIG DbColumn_numChanges(const DbColumn* self)
{
	return (self->insight || self->insightTable || self->links_mirrored || self->links_filtered || self->links_jointed) ? 0 : self->numChanges;
}

double DbColumn_getOptionNumber(const DbColumn* self, const char* name)
{
	return _DbRoot_getOptionNumber(DbColumn_getRow(self), name, 0);
}
UNI* DbColumn_getOption(const DbColumn* self, const char* name, UNI* out, const UBIG outMaxSize)
{
	return _DbRoot_getOptionString(DbColumn_getRow(self), name, 0, out, outMaxSize);
}
void DbColumn_setOptionString(DbColumn* self, const char* name, const UNI* value)
{
	_DbRoot_setOptionString(DbColumn_getRow(self), name, value);
}
void DbColumn_setOptionNumber(DbColumn* self, const char* name, double value)
{
	_DbRoot_setOptionNumber(DbColumn_getRow(self), name, value);
}
BOOL DbColumn_cmpOption(const DbColumn* self, const char* name, const UNI* compare)
{
	return _DbRoot_cmpOptionString(DbColumn_getRow(self), name, 0, compare);
}

BOOL DbColumn_isType1(const DbColumn* self)
{
	return self->type == DbColumn_1;
}

BOOL DbColumn_isFindAndReplace(const DbColumn* self)
{
	if (self->type == DbColumn_STRING_32)
		return TRUE;

	if (self->type == DbColumn_1)
		return self->format == DbFormat_NUMBER_1 || self->format == DbFormat_CURRENCY || self->format == DbFormat_PERCENTAGE;

	return FALSE;
}


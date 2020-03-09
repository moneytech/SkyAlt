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

DbValue DbValue_initEmpty(void)
{
	DbValue self;
	self.column = 0;
	self.row = -1;
	self.lang_id = 0;
	self.option = 0;
	self.staticText = 0;
	self.result = StdString_init();
	self.resultCmp = StdString_init();
	self.resultTemp = StdString_init();
	//self.resultNumber = 0;
	self.staticPre = 0;
	self.staticPost = 0;
	self.formated = TRUE;
	self.index = 0;
	self.ignoreUpdateResult = FALSE;
	self.optionFormat = DbFormat_TEXT;
	self.changed = FALSE;
	self.insight = 0;
	return self;
}

DbValue DbValue_initCopy(const DbValue* src)
{
	DbValue self = DbValue_initEmpty();
	self = *src;
	self.staticText = Std_newUNI(src->staticText);
	self.result = StdString_initCopy(&src->result);
	self.resultCmp = StdString_initCopy(&src->resultCmp);
	self.resultTemp = StdString_initCopy(&src->resultTemp);
	self.staticPre = Std_newUNI(src->staticPre);
	self.staticPost = Std_newUNI(src->staticPost);

	if (src->insight)
		self.insight = DbInsight_newCopy(src->insight);

	self.changed = TRUE;
	return self;
}

DbValue DbValue_initStaticCopy(const UNI* value)
{
	DbValue self = DbValue_initEmpty();
	self.staticText = Std_newUNI(value);
	return self;
}

DbValue DbValue_initStatic(UNI* value)
{
	DbValue self = DbValue_initEmpty();
	self.staticText = value;
	return self;
}

DbValue DbValue_initStaticCopyCHAR(const char* value)
{
	DbValue self = DbValue_initEmpty();
	self.staticText = Std_newUNI_char(value);
	return self;
}

DbValue DbValue_initNumber(double value)
{
	DbValue self = DbValue_initEmpty();
	self.staticText = Std_newNumber(value);
	return self;
}

DbValue DbValue_initGET(DbColumn* column, BIG row)
{
	DbValue self = DbValue_initEmpty();
	self.column = column;
	self.row = row;
	return self;
}

DbValue DbValue_initLang(const char* lang_id)
{
	DbValue self = DbValue_initEmpty();
	self.lang_id = lang_id;
	return self;
}

DbValue DbValue_initOptionEx(DbColumnString32* column, BIG row, const char* option, const UNI* defValue)
{
	DbValue self = DbValue_initEmpty();
	self.column = &column->base;
	self.row = row;
	self.option = option;
	self.staticText = Std_newUNI(defValue);
	return self;
}
DbValue DbValue_initOption(BIG row, const char* option, const UNI* defValue)
{
	return DbValue_initOptionEx(DbRoot_getColumnOptions(), row, option, defValue);
}

DbValue DbValue_initOptionEnable(BIG row)
{
	return DbValue_initOption(row, "enable", _UNI32("1"));
}

DbValue DbValue_initInsight(UINT funcType, const DbRows* filter, DbColumn* column)
{
	DbValue self = DbValue_initEmpty();

	self.insight = DbInsight_new(DbInsightSelectFunc_get(funcType), filter->table, filter->filter, 0);
	DbInsight_addItem(self.insight, column);

	return self;
}

void DbValue_free(DbValue* self)
{
	Std_deleteUNI(self->staticText);
	StdString_free(&self->result);
	StdString_free(&self->resultCmp);
	StdString_free(&self->resultTemp);
	Std_deleteUNI(self->staticPre);
	Std_deleteUNI(self->staticPost);

	if (self->insight)
		DbInsight_delete(self->insight);

	Os_memset(self, sizeof(DbValue));
}

const UNI* DbValue_result(const DbValue* self)
{
	return self->result.str;
}
const UNI* DbValue_resultNoPost(const DbValue* self)
{
	return self->resultCmp.str;
}

BIG DbValue_getRow(const DbValue* self)
{
	return self->row;
}

void DbValue_setRow(DbValue* self, BIG row, UBIG index)
{
	self->row = row;
	self->index = index;
}

BOOL DbValue_isRowEnable(const DbValue* self)
{
	return (self->row >= 0) ? _DbRoot_isEnable(self->row) : FALSE;
}

BOOL DbValue_is(const DbValue* self)
{
	return (self->column && self->row >= 0);
}

BOOL DbValue_isType1(const DbValue* self)
{
	return self->row >= 0 && self->column && self->column->type == DbColumn_1;
}
BOOL DbValue_isTypeN(const DbValue* self)
{
	return self->row >= 0 && self->column && self->column->type == DbColumn_N;
}
BOOL DbValue_isTypeString32(const DbValue* self)
{
	return self->row >= 0 && self->column && self->column->type == DbColumn_STRING_32;
}
BOOL DbValue_isTypeLink(const DbValue* self)
{
	return self->row >= 0 && self->column && self->column->type == DbColumn_N && DbColumn_getBTable(self->column);
}
BOOL DbValue_isTypeFile(const DbValue* self)
{
	DbFormatTYPE type = (self->row >= 0 && self->column) ? DbColumnFormat_findColumn(self->column) : DbFormat_TEXT;
	return type == DbFormat_FILE_1 || type == DbFormat_FILE_N;
}

static void _DbValue_updateCmp(DbValue* self, const UNI* src)
{
	self->changed |= (!Std_cmpUNI(self->resultCmp.str, src));

	StdString_setUNI(&self->resultCmp, src);

	StdString_setUNI(&self->result, self->staticPre);
	StdString_addUNI(&self->result, src);
	StdString_addUNI(&self->result, self->staticPost);
}

static void _DbValue_updateResult(DbValue* self)
{
	if (self->ignoreUpdateResult)
		return;

	if (self->insight)
	{
		_DbValue_updateCmp(self, self->insight->result_string.str);
	}
	else
		if (self->lang_id)
		{
			_DbValue_updateCmp(self, Lang_find(self->lang_id));
		}
		else
			if (self->column && self->row >= 0)
			{
				if (self->option)
				{
					//if(!DbColumnString32_getOption(...))	//not 64, but StdString ...

					UNI str[64];
					DbColumnString32_getOption((DbColumnString32*)self->column, self->row, self->option, self->staticText, str, 64);

					if (self->optionFormat == DbFormat_DATE)
					{
						OsDate date = OsDate_initFromNumber(Std_getNumberFromUNI(str));
						if (OsDate_is(&date))
						{
							UNI formatTime = _DbRoot_getOptionNumber(self->row, "timeFormat", 0);

							char month[32];
							Std_copyCHAR_uni(month, 32, Lang_find_month(date.m_month));
							char time[64];
							OsDate_getStringDateTime(&date, UiIniSettings_getDateFormat(), formatTime, month, time);
							Std_copyUNI_char(str, 64, time);
						}
					}

					_DbValue_updateCmp(self, str);
				}
				else
				{
					const UNI* str = self->formated ? DbColumn_getStringCopyWithFormatLong(self->column, self->row, &self->resultTemp) : DbColumn_getStringCopyLong(self->column, self->row, &self->resultTemp);
					_DbValue_updateCmp(self, str);
				}
			}
			else
				_DbValue_updateCmp(self, self->staticText);
}

BOOL DbValue_hasChanged(DbValue* self)
{
	if (self->insight)
		DbInsight_execute(self->insight);

	_DbValue_updateResult(self);

	BOOL changed = self->changed;
	self->changed = FALSE;
	return changed;
}

UBIG DbValue_getN(const DbValue* self)
{
	if (self->column && self->row >= 0)
		return DbColumn_sizeHard(self->column, self->row);
	return 0;
}

double DbValue_getNumber(const DbValue* self)
{
	double ret = 0;

	if (self->insight)
	{
		ret = self->insight->result_number;
	}
	else
		if (self->lang_id)
		{
			ret = Std_getNumberFromUNI(Lang_find(self->lang_id));
		}
		else
			if (self->column && self->row >= 0)
			{
				if (self->option)
				{
					UNI str[64];
					DbColumnString32_getOption((DbColumnString32*)self->column, self->row, self->option, self->staticText, str, 64);
					ret = Std_getNumberFromUNI(str);
				}
				else
				{
					ret = DbColumn_getFlt(self->column, self->row, self->index);
				}
			}
			else
				ret = Std_getNumberFromUNI(self->staticText);

	return ret;
}

void DbValue_setNumber(DbValue* self, double value)
{
	if (self->column && self->row >= 0)
	{
		if (self->option)
		{
			UNI str[64];
			Std_buildNumberUNI(value, -1, str);
			DbColumnString32_setOption((DbColumnString32*)self->column, self->row, self->option, str);
		}
		else
		{
			DbColumn_setFlt(self->column, self->row, self->index, value);
		}
	}
	else
	{
		Std_deleteUNI(self->staticText);
		self->staticText = Std_newNumber(value);
	}

	_DbValue_updateResult(self);
}

void DbValue_setFormated(DbValue* self, BOOL formated)
{
	BOOL changed = (self->formated != formated);

	self->formated = formated;

	if (changed)
		_DbValue_updateResult(self);
}

void DbValue_setTextCopy(DbValue* self, const UNI* value)
{
	if (self->column && self->row >= 0)
	{
		if (self->option)
		{
			DbColumnString32_setOption((DbColumnString32*)self->column, self->row, self->option, value);
		}
		else
		{
			DbColumn_setStringCopy(self->column, self->row, self->index, value);
		}
	}
	else
	{
		Std_deleteUNI(self->staticText);
		self->staticText = Std_newUNI(value);
	}

	_DbValue_updateResult(self);
}

void DbValue_setText(DbValue* self, UNI* value)
{
	if (self->column && self->row >= 0)
	{
		if (self->option)
		{
			DbColumnString32_setOption((DbColumnString32*)self->column, self->row, self->option, value);
			Std_deleteUNI(value);
		}
		else
		{
			DbColumn_setString(self->column, self->row, self->index, value);
		}
	}
	else
	{
		Std_deleteUNI(self->staticText);
		self->staticText = value;
	}

	_DbValue_updateResult(self);
}

void DbValue_setCd(DbValue* self, Rgba cd)
{
	DbValue_setNumber(self, Rgba_asNumber(cd));
	_DbValue_updateResult(self);
}
Rgba DbValue_getCd(const DbValue* self)
{
	return Rgba_initFromNumber(DbValue_getNumber(self));
}

void DbValue_setDate(DbValue* self, OsDate date)
{
	DbValue_setNumber(self, OsDate_asNumber(&date));
}
OsDate DbValue_getDate(const DbValue* self)
{
	double date = DbValue_getNumber(self);
	return OsDate_initFromNumber(date);
}

FileRow DbValue_getFileId(const DbValue* self)
{
	return DbColumn_fileGetPos(self->column, self->row, self->index);
}
void DbValue_getFileExt(const DbValue* self, const UBIG index, UNI ext[8])
{
	DbColumn_fileGetExt(self->column, self->row, index, ext);
}
void DbValue_getFileExt_char(const DbValue* self, const UBIG index, char ext[8])
{
	UNI extUni[8];
	DbValue_getFileExt(self, index, extUni);
	Std_setCHAR_uni(ext, extUni, 8);
}
UBIG DbValue_getFileSize(const DbValue* self, const UBIG index)
{
	return DbColumn_fileGetSize(self->column, self->row, index);
}
UBIG DbValue_readFileCache(const DbValue* self, const UBIG index, UBIG pos, UBIG size, UCHAR* buff)
{
	return DbColumn_fileGetDataEx(self->column, self->row, index, pos, size, buff);
}
void DbValue_importFile(DbValue* self, OsFile* srcFile, const UBIG index, UNI ext[8])
{
	DbColumn_fileImport(self->column, self->row, index, srcFile, ext);
}
void DbValue_importData(DbValue* self, UBIG size, UCHAR* data, const UBIG index, UNI ext[8])
{
	DbColumn_fileImportData(self->column, self->row, index, ext, size, data);
}

void DbValue_exportFile(const DbValue* self, OsFile* dstFile, const UBIG index)
{
	DbColumn_fileExport(self->column, self->row, index, dstFile);
}

DbFormatTYPE DbValue_getFormat(const DbValue* self)
{
	return self->column ? DbColumnFormat_findColumn(self->column) : DbFormat_TEXT;
}

BOOL DbValue_isFormatUnderline(const DbValue* self)
{
	DbFormatTYPE format = DbValue_getFormat(self);
	return (format == DbFormat_PHONE || format == DbFormat_EMAIL || format == DbFormat_URL || format == DbFormat_LOCATION);
}

const UNI* DbValue_now_getText(DbValue* self, BIG row)
{
	self->row = row;
	_DbValue_updateResult(self);
	return self->result.str;
}

double DbValue_getOptionNumber(BIG row, const char* name, double defValue)
{
	return _DbRoot_getOptionNumber(row, name, defValue);
}
void DbValue_setOptionNumber(BIG row, const char* name, double value)
{
	_DbRoot_setOptionNumber(row, name, value);
}
UNI* DbValue_getOptionString(BIG row, const char* name, UNI* out, const UBIG outMaxSize)
{
	return _DbRoot_getOptionString(row, name, 0, out, outMaxSize);
}
void DbValue_setOptionString(BIG row, const char* name, const UNI* value)
{
	_DbRoot_setOptionString(row, name, value);
}

DbValues DbValues_init(void)
{
	DbValues self;
	self.values = 0;
	self.num = 0;
	return self;
}
DbValues DbValues_initCopy(const DbValues* src)
{
	DbValues self;
	self.num = src->num;
	self.values = Os_malloc(self.num * sizeof(DbValue));

	BIG i;
	for (i = 0; i < self.num; i++)
		self.values[i] = DbValue_initCopy(&src->values[i]);

	return self;
}

void DbValues_free(DbValues* self)
{
	BIG i;
	for (i = 0; i < self->num; i++)
		DbValue_free(&self->values[i]);
	Os_free(self->values, self->num * sizeof(DbValue));

	Os_memset(self, sizeof(DbValues));
}
DbValue* DbValues_add(DbValues* self, DbValue value)
{
	self->num++;
	self->values = Os_realloc(self->values, self->num * sizeof(DbValue));
	self->values[self->num - 1] = value;

	return &self->values[self->num - 1];
}

DbValue* DbValues_findRow(const DbValues* self, BIG row)
{
	BIG i;
	for (i = 0; i < self->num; i++)
		if (self->values[i].row == row)
			return &self->values[i];
	return 0;
}

void DbValues_updateText(DbValues* self)
{
	BIG i;
	for (i = 0; i < self->num; i++)
		_DbValue_updateResult(&self->values[i]);
}

void DbValues_setRow(DbValues* self, BIG row)
{
	BIG i;
	for (i = 0; i < self->num; i++)
		self->values[i].row = row;
}
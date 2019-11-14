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

void DbColumn_clear(DbColumn* self)
{
	switch (self->type)
	{
		case DbColumn_1:
		DbColumn1_clear((DbColumn1*)self);
		break;

		case DbColumn_N:
		DbColumnN_clear((DbColumnN*)self);
		break;

		case DbColumn_STRING_32:
		DbColumnString32_clear((DbColumnString32*)self);
		break;
	}
}

void DbColumn_delete(DbColumn* self)
{
	switch (self->type)
	{
		case DbColumn_1:
		DbColumn1_delete((DbColumn1*)self);
		break;

		case DbColumn_N:
		DbColumnN_delete((DbColumnN*)self);
		break;

		case DbColumn_STRING_32:
		DbColumnString32_delete((DbColumnString32*)self);
		break;
	}
}

void DbColumn_deleteRowData(DbColumn* self, const UBIG r)
{
	//note: file is deleted during save(test which files need to be kept)

	switch (self->type)
	{
		case DbColumn_1:
		DbColumn1_deleteRowData((DbColumn1*)self, r);
		break;

		case DbColumn_N:
		DbColumnN_deleteRowData((DbColumnN*)self, r);
		break;

		case DbColumn_STRING_32:
		DbColumnString32_deleteRowData((DbColumnString32*)self, r);
		break;
	}
}

void DbColumn_maintenance(DbColumn* self)
{
	switch (self->type)
	{
		case DbColumn_1:
		break;

		case DbColumn_N:
		DbColumnN_maintenance((DbColumnN*)self);
		break;

		case DbColumn_STRING_32:
		DbColumnString32_maintenance((DbColumnString32*)self);
		break;
	}

	DbColumn1* active = DbColumn_getActive(self);
	if (active)
	{
		const UBIG NUM_ROWS = DbColumn_numRows(self);
		TableItems_maintenance(DbColumn_getHalds(self), NUM_ROWS, DbColumn_getHalds(&active->base));
	}
}

UBIG DbColumn_bytes(DbColumn* self)
{
	UBIG sum = 0;
	switch (self->type)
	{
		case DbColumn_1:
		sum = DbColumn1_bytes((DbColumn1*)self);
		break;

		case DbColumn_N:
		sum = DbColumnN_bytes((DbColumnN*)self);
		break;

		case DbColumn_STRING_32:
		sum = DbColumnString32_bytes((DbColumnString32*)self);
		break;
	}

	return sum;
}

double DbColumn_getFlt(const DbColumn* self, const UBIG r, UBIG index)
{
	switch (self->type)
	{
		case DbColumn_1:
		return DbColumn1_get((DbColumn1*)self, r);

		case DbColumn_STRING_32:
		return DbColumnString32_getNumber((DbColumnString32*)self, r);

		case DbColumn_N:
		return DbColumnN_getIndex((DbColumnN*)self, r, index);
	}
	return 0;
}

void DbColumn_setFlt(DbColumn* self, const UBIG r, UBIG index, double value)
{
	switch (self->type)
	{
		case DbColumn_1:
		DbColumn1_set((DbColumn1*)self, r, value);
		break;

		case DbColumn_STRING_32:
		DbColumnString32_setEqFree((DbColumnString32*)self, r, Std_newNumber(value));
		break;

		case DbColumn_N:
		DbColumnN_setIndex((DbColumnN*)self, r, index, value);
		break;
	}
}

void DbColumn_setStringCopy(DbColumn* self, const UBIG r, UBIG index, const UNI* value)
{
	switch (self->type)
	{
		case DbColumn_1:
		{
			DbColumn1_set((DbColumn1*)self, r, Std_getNumberFromUNI(value));
			break;
		}
		case DbColumn_STRING_32:
		{
			DbColumnString32_setCopy((DbColumnString32*)self, r, value);
			break;
		}
		case DbColumn_N:
		{
			DbColumnN_setIndex((DbColumnN*)self, r, index, Std_getNumberFromUNI(value));
			break;
		}
	}
}

void DbColumn_setString(DbColumn* self, const UBIG r, UBIG index, UNI* value)
{
	switch (self->type)
	{
		case DbColumn_1:
		{
			DbColumn1_set((DbColumn1*)self, r, Std_getNumberFromUNI(value));
			Std_deleteUNI(value);
			break;
		}
		case DbColumn_STRING_32:
		{
			DbColumnString32_setEqFree((DbColumnString32*)self, r, value);
			break;
		}
		case DbColumn_N:
		{
			DbColumnN_setIndex((DbColumnN*)self, r, index, Std_getNumberFromUNI(value));
			Std_deleteUNI(value);
			break;
		}
	}
}

FileRow DbColumn_fileGetPos(const DbColumn* self, const UBIG r, const UBIG ri)
{
	return (self->type == DbColumn_1) ? DbColumn1_getFileId((DbColumn1*)self, r) : DbColumnN_getFileId((DbColumnN*)self, r, ri);
}
UCHAR DbColumn_fileGetData(DbColumn* self, const UBIG r, const UBIG ri, UBIG i)
{
	FileRow filePos = DbColumn_fileGetPos(self, r, ri);

	UBIG move = i % 16;
	i -= move;

	UCHAR buff[16];
	FileCache_readData(filePos, i, 16, buff);
	return buff[move];
}

UBIG DbColumn_fileGetDataEx(DbColumn* self, const UBIG r, const UBIG ri, UBIG pos, UBIG size, UCHAR* buff)
{
	FileRow filePos = DbColumn_fileGetPos(self, r, ri);
	return FileCache_readData(filePos, pos, size, buff);
}

FileHead DbColumn_fileGetHead(const DbColumn* self, const UBIG r, const UBIG ri)
{
	FileRow filePos = DbColumn_fileGetPos(self, r, ri);

	FileHead head = FileHead_initEmpty();
	if (FileRow_is(filePos))
		FileCache_readHead(filePos, &head);
	return head;
}

UBIG DbColumn_fileGetSize(const DbColumn* self, const UBIG r, const UBIG ri)
{
	FileHead head = DbColumn_fileGetHead(self, r, ri);
	UBIG size = head.size;
	FileHead_free(&head);
	return size;
}
void DbColumn_fileGetExt(const DbColumn* self, const UBIG r, const UBIG ri, UNI ext[8])
{
	FileHead head = DbColumn_fileGetHead(self, r, ri);
	Std_setUNI_char(ext, (char*)head.ext, 8);
	FileHead_free(&head);
}

void DbColumn_setFileId(DbColumn* self, const UBIG r, const UBIG ri, FileRow value)
{
	if (self->type == DbColumn_1)
		DbColumn1_setFileId((DbColumn1*)self, r, value);
	else
		DbColumnN_setFileId((DbColumnN*)self, r, ri, value);
}

BOOL DbColumn_fileImportData(DbColumn* self, const UBIG r, const UBIG ri, UNI ext[8], UBIG size, UCHAR* data, volatile StdProgress* progress)
{
	BOOL ok = FALSE;
	FileFile* file = FileUser_createFile(FileProject_getUserMe());
	if (file)
	{
		ok = FileFile_importData(file, data, size, ext, progress);
		if (ok)
			DbColumn_setFileId(self, r, ri, FileFile_getId(file));

		FileFile_delete(file);
	}
	return ok;
}

BOOL DbColumn_fileImport(DbColumn* self, const UBIG r, const UBIG ri, OsFile* srcFile, UNI ext[8], volatile StdProgress* progress)
{
	BOOL ok = FALSE;
	FileFile* file = FileUser_createFile(FileProject_getUserMe());
	if (file)
	{
		ok = FileFile_import(file, srcFile, ext, progress);
		if (ok)
			DbColumn_setFileId(self, r, ri, FileFile_getId(file));

		FileFile_delete(file);
	}
	return ok;
}

BOOL DbColumn_fileExport(DbColumn* self, const UBIG r, const UBIG ri, OsFile* dstFile, volatile StdProgress* progress)
{
	BOOL ok = FALSE;
	FileRow filePos = DbColumn_fileGetPos(self, r, ri);

	if (FileRow_is(filePos))
	{
		FileFile* file = FileProject_openFile(filePos, FALSE);
		if (file)
		{
			ok = (FileFile_export(file, dstFile, progress) > 0);
			FileFile_delete(file);
		}
	}
	return ok;
}

DbTable* DbColumn_getBTable(const DbColumn* self)
{
	if (self->type == DbColumn_1)
		return ((DbColumn1*)self)->btable;
	if (self->type == DbColumn_N)
		return ((DbColumnN*)self)->btable;
	return 0;
}

double DbColumn_getIndex(const DbColumn* self, UBIG r, UBIG i)
{
	return (self->type == DbColumn_1) ? DbColumn1_getLink((DbColumn1*)self, r) : DbColumnN_getIndex((DbColumnN*)self, r, i);
}

UBIG DbColumn_sizeActive(const DbColumn* self, UBIG r)
{
	return (self->type == DbColumn_1) ? DbColumn1_sizeActive((DbColumn1*)self, r) : DbColumnN_sizeActive((DbColumnN*)self, r);
}
UBIG DbColumn_sizeHard(const DbColumn* self, UBIG r)
{
	return (self->type == DbColumn_1) ? 1 : DbColumnN_sizeHard((DbColumnN*)self, r);
}


void DbColumn_add(DbColumn* self, const UBIG rSrc, const UBIG rDst)
{
	if (self->type == DbColumn_1)
		DbColumn1_addLink((DbColumn1*)self, rSrc, rDst);
	else
		DbColumnN_add((DbColumnN*)self, rSrc, rDst);
}
void DbColumn_set(DbColumn* self, const UBIG rSrc, const UBIG rDst)
{
	if (self->type == DbColumn_1)
		DbColumn1_set((DbColumn1*)self, rSrc, rDst);
	else
		DbColumnN_set((DbColumnN*)self, rSrc, rDst);
}

void DbColumn_remove(DbColumn* self, const UBIG rSrc, const double rDst)
{
	if (self->type == DbColumn_1)
		DbColumn1_removeLink((DbColumn1*)self, rSrc, rDst);
	else
		DbColumnN_remove((DbColumnN*)self, rSrc, rDst);
}

void DbColumn_insert_before(DbColumn* self, const UBIG rSrc, const UBIG rDst, const UBIG rDstAfter)
{
	if (self->type == DbColumn_1)
		DbColumn1_set((DbColumn1*)self, rSrc, rDst);
	else
		DbColumnN_insert_before((DbColumnN*)self, rSrc, rDst, rDstAfter);
}

void DbColumn_insert_after(DbColumn* self, const UBIG rSrc, const UBIG rDst, const UBIG rDstAfter)
{
	if (self->type == DbColumn_1)
		DbColumn1_set((DbColumn1*)self, rSrc, rDst);
	else
		DbColumnN_insert_after((DbColumnN*)self, rSrc, rDst, rDstAfter);
}

BIG DbColumn_getFirstRow(const DbColumn* self, const UBIG rSrc)
{
	if (self->type == DbColumn_1)
		return DbColumn1_getLink((DbColumn1*)self, rSrc);
	else
		return DbColumnN_getFirstRow((DbColumnN*)self, rSrc);
}

void DbColumn_setBTable(DbColumn* self, DbTable* btable)
{
	if (self->type == DbColumn_1)
		DbColumn1_setBTable((DbColumn1*)self, btable);
	else
		DbColumnN_setBTable((DbColumnN*)self, btable);
}

BIG DbColumn_jump(DbColumn* self, UBIG r, UBIG* pos, BIG jumps)
{
	return (self->type == DbColumn_1) ? DbColumn1_jump((DbColumn1*)self, r, pos, jumps) : DbColumnN_jump((DbColumnN*)self, r, pos, jumps);
}

BIG DbColumn_searchNumber(DbColumn* self, const double value)
{
	if (self->type == DbColumn_1)
		return DbColumn1_search((DbColumn1*)self, value);
	else
		if (self->type == DbColumn_N)
			return DbColumnN_search((DbColumnN*)self, value);
		else
			if (self->type == DbColumn_STRING_32)
				return DbColumnString32_searchNumber((DbColumnString32*)self, value);

	return -1;
}

BIG DbColumn_searchString(DbColumn* self, const UNI* value)
{
	if (self->type == DbColumn_1)
		return DbColumn1_search((DbColumn1*)self, Std_getNumberFromUNI(value));
	else
		if (self->type == DbColumn_N)
			return DbColumnN_search((DbColumnN*)self, Std_getNumberFromUNI(value));
		else
			if (self->type == DbColumn_STRING_32)
				return DbColumnString32_searchString((DbColumnString32*)self, value);

	return -1;
}

void _DbColumn_addSepar(StdString* out, BIG i, BIG N, const UNI* separ)
{
	if (i + 1 < N)
		StdString_addUNI(out, separ ? separ : _UNI32(";"));
}
void _DbColumn_addSeparEx(StdString* out, const UNI* addStr, BIG i, BIG N, const UNI* separ)
{
	StdString_addUNI(out, addStr);
	_DbColumn_addSepar(out, i, N, separ);
}

UNI* DbColumn_getStringCopyLong(const DbColumn* self, const UBIG r, StdString* out)
{
	StdString_empty(out);

	switch (self->type)
	{
		case DbColumn_1:
		{
			UNI nmbr[64];
			const UBIG N = DbColumn_sizeActive(self, r);
			BIG i;
			for (i = 0; i < N; i++)
				_DbColumn_addSeparEx(out, Std_buildNumberUNI(DbColumn_getFlt(self, r, i), -1, nmbr), i, N, 0);
			break;
		}

		case DbColumn_STRING_32:
		{
			StdString_addUNI(out, DbColumnString32_get((DbColumnString32*)self, r));
			break;
		}

		case DbColumn_N:
		{
			UNI nmbr[64];
			const UBIG N = DbColumn_sizeActive(self, r);
			BIG i;
			for (i = 0; i < N; i++)
				_DbColumn_addSeparEx(out, Std_buildNumberUNI(DbColumn_getIndex(self, r, i), -1, nmbr), i, N, 0);
			break;
		}
	}

	return out->str;
}

UNI* DbColumn_getStringCopyWithFormatLong(const DbColumn* self, const UBIG r, StdString* out)
{
	StdString_empty(out);

	DbFormatTYPE type = DbColumnFormat_findColumn(self);

	switch (type)
	{
		case DbFormat_NUMBER_1:
		case DbFormat_NUMBER_N:
		case DbFormat_SLIDER:
		case DbFormat_CHECK:
		{
			const UNI precision = DbColumn_getOptionNumber(self, "precision");
			UNI nmbr[64];
			const UBIG N = DbColumn_sizeActive(self, r);
			BIG i;
			for (i = 0; i < N; i++)
			{
				Std_buildNumberUNI(DbColumn_getFlt(self, r, i), precision, nmbr);
				if (type == DbFormat_NUMBER_1 || type == DbFormat_NUMBER_N)
					Std_separNumberThousands(nmbr, ' ');

				_DbColumn_addSeparEx(out, nmbr, i, N, 0);
			}

			break;
		}

		case DbFormat_TEXT:
		{
			BOOL password = DbColumn_getOptionNumber(self, "password");

			DbColumn_getStringCopyLong(self, r, out);
			if (password)
				Std_replaceCharacters(out->str, L'*');
			break;
		}

		case DbFormat_LINK_1:
		case DbFormat_LINK_N:
		{
			DbTable* btable = DbColumn_getBTable(self);
			const UBIG numColumns = Std_clamp(DbColumn_getOptionNumber(self, "numColumns"), 1, DbColumns_num(DbTable_getColumns(btable)) - 1);	//-1 because first is ColumnRowID

			const UBIG N = DbColumn_sizeActive(self, r);
			UBIG i, ii;
			for (i = 0; i < N; i++)
			{
				BIG bRow = DbColumn_getIndex(self, r, i);

				for (ii = 0; ii < numColumns; ii++)
				{
					StdString bStr = StdString_init();
					DbColumn_getStringCopyWithFormatLong(DbColumns_get(DbTable_getColumns(btable), ii + 1), bRow, &bStr);	//+1 skip first ColumnRowID
					_DbColumn_addSeparEx(out, bStr.str, !(i + 1 == N && ii + 1 == numColumns), 1, 0);
					StdString_free(&bStr);
				}
			}
			break;
		}
		case DbFormat_FILE_1:
		case DbFormat_FILE_N:
		{
			const UBIG N = DbColumn_sizeActive(self, r);
			BIG i;
			for (i = 0; i < N; i++)
			{
				FileHead head = DbColumn_fileGetHead(self, r, i);
				UNI* inf = FileHead_getInfoUNI(&head, TRUE);
				_DbColumn_addSeparEx(out, inf, i, N, 0);
				Std_deleteUNI(inf);
				FileHead_free(&head);
			}
			break;
		}

		case DbFormat_CURRENCY:
		{
			UNI precision = DbColumn_getOptionNumber(self, "precision");
			BOOL before = DbColumn_getOptionNumber(self, "before");
			UNI currency[32];
			DbColumn_getOption(self, "currency", currency, 32);

			UNI nmbr[64];
			const UBIG N = DbColumn_sizeActive(self, r);
			BIG i;
			for (i = 0; i < N; i++)
			{
				Std_buildNumberUNI(DbColumn_getFlt(self, r, i), precision, nmbr);
				Std_separNumberThousands(nmbr, ' ');

				if (before)
					StdString_addUNI(out, currency);

				StdString_addUNI(out, nmbr);

				if (!before)
					StdString_addUNI(out, currency);

				_DbColumn_addSepar(out, i, N, 0);
			}

			break;
		}

		case DbFormat_PERCENTAGE:
		{
			UNI precision = DbColumn_getOptionNumber(self, "precision");
			double mult = DbColumn_getOptionNumber(self, "mult100") ? 100 : 1;

			UNI nmbr[64];
			const UBIG N = DbColumn_sizeActive(self, r);
			BIG i;
			for (i = 0; i < N; i++)
			{
				Std_buildNumberUNI(DbColumn_getFlt(self, r, i) * mult, precision, nmbr);
				Std_separNumberThousands(nmbr, ' ');

				StdString_addUNI(out, nmbr);
				StdString_addUNI(out, _UNI32("%"));
				_DbColumn_addSepar(out, i, N, 0);
			}
			break;
		}

		case DbFormat_PHONE:
		{
			DbColumn_getStringCopyLong(self, r, out);
			break;
		}
		case DbFormat_URL:
		{
			DbColumn_getStringCopyLong(self, r, out);
			break;
		}
		case DbFormat_EMAIL:
		{
			DbColumn_getStringCopyLong(self, r, out);
			break;
		}
		case DbFormat_LOCATION:
		{
			DbColumn_getStringCopyLong(self, r, out);
			BIG pos = Std_findUNI_last(out->str, ';');
			if (pos >= 0)
				out->str[pos] = 0;
			break;
		}

		case DbFormat_RATING:
		{
			UNI nmbr[64];
			const UBIG N = DbColumn_sizeActive(self, r);
			BIG i;
			for (i = 0; i < N; i++)
			{
				Std_buildNumberUNI(DbColumn_getFlt(self, r, i), 0, nmbr);
				_DbColumn_addSeparEx(out, nmbr, i, N, 0);
			}
			break;
		}

		case DbFormat_DATE:
		{
			OsDateTimeTYPE formatTime = DbColumn_getOptionNumber(self, "timeFormat");

			OsDate date = OsDate_initFromNumber(DbColumn_getFlt(self, r, 0));

			if (OsDate_is(&date))
			{
				char month[32];
				Std_copyCHAR_uni(month, 32, Lang_find_month(date.m_month));
				char time[64];
				OsDate_getStringDateTime(&date, UiIniSettings_getDateFormat(), formatTime, month, time);
				StdString_addCHAR(out, time);
			}

			break;
		}

		case DbFormat_MENU:
		case DbFormat_TAGS:
		{
			UNI option[64];
			const UBIG N = DbColumn_sizeActive(self, r);
			BIG i;
			for (i = 0; i < N; i++)
			{
				BIG optionRow = DbColumn_getIndex(self, r, i);

				_DbRoot_getOptionString(optionRow, "name", 0, option, 64);
				_DbColumn_addSeparEx(out, option, i, N, _UNI32(" | "));

			}
			break;
		}

		case DbFormat_ROW:
		{
			UNI nmbr[64];
			const UBIG N = DbColumn_sizeActive(self, r);
			BIG i;
			for (i = 0; i < N; i++)
			{
				double flt = DbColumn_getFlt(self, r, i);

				FileRow* fr = (FileRow*)&flt;

				Std_buildNumberUNI(fr->row, 0, nmbr);
				Std_separNumberThousands(nmbr, ' ');

				StdString_addUNI(out, nmbr);
				_DbColumn_addSepar(out, i, N, 0);
			}
			break;
		}

		default:
		break;
	}

	return out->str;
}

void DbColumn_moveToTableEx(DbColumn* self, const DbColumn* src)
{
	DbTable* btable = DbColumn_getTable(self);

	const UBIG NUM_ROWS = DbColumn_numRows(src);
	UBIG i;
	for (i = 0; i < NUM_ROWS; i++)
	{
		const UBIG N = DbColumn_sizeActive(src, i);
		UBIG ii;
		for (ii = 0; ii < N; ii++)
		{
			BIG it = DbColumn_getIndex(src, i, ii);
			if (DbTable_isRowActive(btable, it))
				DbColumn_add(self, it, i);
		}
	}
}

DbColumn* DbColumn_moveToTable(DbColumn* self)
{
	DbColumn* dst = 0;

	DbTable* otable = DbColumn_getTable(self);
	DbTable* btable = DbColumn_getBTable(self);
	if (btable)
	{
		//new
		dst = DbTable_createColumnFormat(btable, DbColumnFormat_findColumn(self), 0, otable);

		//convert
		DbColumn_moveToTableEx(dst, self);

		//remove
		DbRoot_removeRow(DbColumn_getRow(self));
	}

	return dst;
}


void DbColumn_addLinksToColumn(DbColumn* dst, DbColumn* src, DbRows* rows)
{
	if (DbColumn_getTable(dst) != DbColumn_getTable(src))
		return;

	const double maxRows = DbRows_getSize(rows);

	BIG i, ii;
	for (i = 0; i < maxRows; i++)
	{
		BIG row = DbRows_getRow(rows, i);

		const UBIG size = DbColumn_sizeActive(src, row);
		for (ii = 0; ii < size; ii++)
		{
			DbColumn_add(dst, row, DbColumn_getIndex(src, row, ii));
		}
	}
}


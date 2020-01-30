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

static int _IOTsv_readItem(OsFile* f, char* buff, const int max_buff, BOOL* eol, BOOL* eof)
{
	int pos = 0;

	char ch;
	while (!(*eof = OsFile_read(f, &ch, 1) == 0) && !(*eol = (ch == '\0' || ch == '\r' || ch == '\n')) && ch != '\t' && pos < max_buff)
	{
		buff[pos++] = ch;
	}
	buff[pos] = 0;

	if (ch == '\r')
		*eof = OsFile_read(f, &ch, 1) == 0; //damn you, Windows ending

	if (*eof)
		*eol = *eof;

	return pos;
}

BOOL IOTsv_write(const char* path, BOOL firstRowColumnNames, DbRows* rows, DbValues* columns)
{
	const double maxRows = DbRows_getSize(rows);

	OsFile f;
	if (OsFile_init(&f, path, OsFile_W))
	{
		UBIG c, r;
		const UBIG N_COLS = columns->num;

		if (firstRowColumnNames)
		{
			UNI name[64];
			for (c = 0; c < N_COLS && StdProgress_is(); c++)
			{
				OsFile_writeUNI(&f, DbColumn_getName(columns->values[c].column, name, 64));
				if (c + 1 < N_COLS)
					OsFile_writeUNIch(&f, L'\t');
			}

			OsFile_writeUNIch(&f, L'\n');
		}

		for (r = 0; r < maxRows && StdProgress_is(); r++)
		{
			BIG row = DbRows_getRow(rows, r);

			for (c = 0; c < N_COLS && StdProgress_is(); c++)
			{
				const UNI* str = DbValue_now_getText(&columns->values[c], row);

				OsFile_writeUNI(&f, str);

				if (c + 1 < N_COLS)
					OsFile_writeUNIch(&f, L'\t');
			}

			StdProgress_setEx("EXPORTING", r, maxRows);
			OsFile_writeUNIch(&f, L'\n');
		}

		OsFile_free(&f);
		return TRUE;
	}
	else
		Logs_addError("ERR_INVALID_PATH");

	return FALSE;
}

BOOL IOTsv_read(const char* path, BOOL firstRowColumnNames, BOOL recognizeColumnType, DbRows* tableOut)
{
	const double fileBytes = OsFile_bytes(path);

	OsFile f;
	if (OsFile_init(&f, path, OsFile_R))
	{
		const int buff_max = 64 * 1024;
		char* buff = Os_malloc(buff_max);

		DbValues columns = DbValues_init();

		BOOL eof = FALSE;
		while (StdProgress_is() && !eof) //rows
		{
			UINT c = 0;
			BIG r = -1;

			BOOL eol = FALSE;
			while (!eol) //columns
			{
				int buff_num = _IOTsv_readItem(&f, buff, buff_max, &eol, &eof);
				if (!eol || buff_num > 0)
				{
					if (firstRowColumnNames)
					{
						UNI* name = Std_newUNI_char(buff);
						DbValues_add(&columns, DbValue_initGET(DbTable_createColumnFormat(tableOut->table, DbFormat_TEXT, name, 0), -1));
						Std_deleteUNI(name);
					}
					else
					{
						DbValue* column;
						if (c < columns.num)
							column = &columns.values[c];
						else
							column = DbValues_add(&columns, DbValue_initGET(DbTable_createColumnFormat(tableOut->table, DbFormat_TEXT, 0, 0), -1));
						column->ignoreUpdateResult = TRUE;

						if (recognizeColumnType && Std_countDigitsInRowCHAR(buff))
							column->column = DbColumnConvert_convertEx(column->column, DbFormat_NUMBER_1, 0);	//convert to number

						if (r < 0)
							r = DbRows_addNewRow(tableOut);
						DbValue_setRow(column, r, 0);
						if (DbValue_isType1(column))	DbValue_setNumber(column, Os_atof(buff));
						else							DbValue_setText(column, Std_newUNI_char(buff));
					}
				}
				c++;
			}

			if (!firstRowColumnNames)
				recognizeColumnType = FALSE;
			firstRowColumnNames = FALSE;

			StdProgress_setEx("IMPORTING", OsFile_getSeekPos(&f), fileBytes);
		}

		DbValues_free(&columns);

		Os_free(buff, buff_max);
		OsFile_free(&f);
		return TRUE;
	}
	else
		Logs_addError("ERR_INVALID_PATH");

	return FALSE;
}

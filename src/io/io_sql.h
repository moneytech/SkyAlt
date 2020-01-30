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

BOOL IOSql_write(const char* path, DbRows* rows, DbValues* columns)
{
	BOOL ok = FALSE;

	const double maxRows = DbRows_getSize(rows);

	char* folderPath;
	char* folderName;

	OsFile_getParts(path, &folderPath, &folderName);

	if (!folderName)
	{
		Std_deleteCHAR(folderPath);
		Logs_addError("ERR_INVALID_PATH");
		return FALSE;
	}

	if (folderPath)
		folderPath = Std_addAfterCHAR(Std_addAfterCHAR(folderPath, "/"), folderName);
	else
		folderPath = Std_newCHAR(folderName);

	OsFile f;
	if (OsFile_init(&f, path, OsFile_W))
	{
		UNI name[64];
		UBIG c, r;
		const int N_COLS = columns->num;

		//Begin Table
		char* tableName = Std_newCHAR_uni(DbTable_getName(rows->table, name, 64));
		fprintf(f.m_file, "CREATE TABLE `%s` (\n", tableName);

		//Primary Column "ID"
		OsFile_writeUNI(&f, _UNI32("`ID` INTEGER NOT NULL AUTO_INCREMENT"));

		//Columns names
		for (c = 0; c < N_COLS; c++)
		{
			OsFile_writeUNI(&f, _UNI32(",\n"));

			DbValue* column = &columns->values[c];
			char* columnName = Std_newCHAR_uni(DbColumn_getName(column->column, name, 64));

			if (DbValue_isType1(column))
				fprintf(f.m_file, "`%s` FLOAT", columnName);
			else
				if (DbValue_isTypeN(column))
					;	//...
				else
					if (DbValue_isTypeString32(column))
						fprintf(f.m_file, "`%s` TEXT", columnName);
					else
						if (DbValue_isTypeLink(column))
							;	//...
						else
							if (DbValue_isTypeFile(column))
								fprintf(f.m_file, "`%s` BINARY", columnName);

			Std_deleteCHAR(columnName);
		}

		//End Table
		fprintf(f.m_file, ",\nPRIMARY KEY (`ID`)\n);\n\n");

		//Data
		for (r = 0; r < maxRows; r++)
		{
			BIG row = DbRows_getRow(rows, r);
			fprintf(f.m_file, "INSERT INTO `%s` VALUES (", tableName);

			for (c = 0; c < N_COLS; c++)
			{
				DbValue* column = &columns->values[c];
				DbValue_setRow(column, row, 0);
				DbValue_hasChanged(column);

				if (DbValue_isType1(column))
					fprintf(f.m_file, "%f", DbValue_getNumber(column));
				else
					if (DbValue_isTypeN(column))
						;	//...
					else
						if (DbValue_isTypeString32(column))
						{
							OsFile_writeUNI(&f, _UNI32("`"));
							OsFile_writeUNI(&f, DbValue_result(column));
							OsFile_writeUNI(&f, _UNI32("`"));
						}
						else
							if (DbValue_isTypeLink(column))
								;	//...
							else
								if (DbValue_isTypeFile(column))
								{
									OsFile_writeUNI(&f, _UNI32("`"));
									DbValue_exportFile(column, &f, 0);
									OsFile_writeUNI(&f, _UNI32("`"));
								}

				if (c + 1 < N_COLS)
					fprintf(f.m_file, ",");
			}

			fprintf(f.m_file, ");\n");

			StdProgress_setEx("EXPORTING", r, maxRows);
		}

		Std_deleteCHAR(tableName);
		OsFile_free(&f);
		ok = TRUE;
	}

	Std_deleteCHAR(folderName);
	Std_deleteCHAR(folderPath);

	return ok;
}

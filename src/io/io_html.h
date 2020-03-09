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

const char* g_IOHtml_begin = "\
﻿<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\n\
<html xmlns=\"http://www.w3.org/1999/xhtml\" xml:lang=\"en\" lang=\"en\">\n\
<head>\n\
	<meta http-equiv=\"content-type\" content=\"text/html;charset=utf-8\" />\n\
	<title>%s</title>\n\
	<style type=\"text/css\">\n\
		*{	padding: 0;	margin: 0;	}\n\
		body {	width: 90%;	margin-left: auto;	margin-right: auto;	text-align: center;}\n\
		h2 {	padding: 30px 0px 30px 0px;	text-decoration: underline;}\n\
		table {    width:100%;}\n\
		table, th, td {    border: 1px solid black;    border-collapse: collapse;    padding: 5px 0px 5px 0px;}\n\
	</style>\n\
</head>\n\
<body>\n\
<h2>%s</h2>\n\
<table>\n";

const UNI* g_IOHtml_end = _UNI32("</table>\n</body>\n</html>");

BOOL IOHtml_isImage(const char* ext)
{
	return Std_cmpCHARsmall(ext, "jpg") ||
		Std_cmpCHARsmall(ext, "jpeg") ||
		Std_cmpCHARsmall(ext, "png") ||
		Std_cmpCHARsmall(ext, "apng") ||
		Std_cmpCHARsmall(ext, "gif") ||
		Std_cmpCHARsmall(ext, "svg") ||
		Std_cmpCHARsmall(ext, "bmp");
}

BOOL IOHtml_write(const char* path, BOOL renderImages, DbRows* rows, DbValues* columns)
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
		UBIG c, r, ri;

		//Begin
		{
			char* title = Std_newCHAR_uni(DbTable_getName(rows->table, name, 64));
			fprintf(f.m_file, g_IOHtml_begin, title, title);
			Std_deleteCHAR(title);
		}

		const int N_COLS = columns->num;

		//Columns names
		OsFile_writeUNI(&f, _UNI32("\t<tr>"));
		for (c = 0; c < N_COLS; c++)
		{
			DbValue* column = &columns->values[c];

			OsFile_writeUNI(&f, _UNI32("<th>"));
			OsFile_writeUNI(&f, DbColumn_getName(column->column, name, 64));
			OsFile_writeUNI(&f, _UNI32("</th>"));
		}
		OsFile_writeUNI(&f, _UNI32("</tr>\n"));

		//Data
		BOOL folderMade = FALSE;
		for (r = 0; r < maxRows; r++)
		{
			BIG row = DbRows_getRow(rows, r);

			OsFile_writeUNI(&f, _UNI32("\t<tr>"));
			for (c = 0; c < N_COLS; c++)
			{
				OsFile_writeUNI(&f, _UNI32("<td>"));

				DbValue* column = &columns->values[c];

				const UNI* str = DbValue_now_getText(column, row);
				if (DbValue_isTypeFile(column))
				{
					const UBIG N_indexes = DbValue_getN(column);
					for (ri = 0; ri < N_indexes; ri++)
					{
						if (DbValue_getFileSize(column, ri))
						{
							//create folder
							if (!folderMade)
							{
								OsFileDir_makeDir(folderPath);
								folderMade = TRUE;
							}

							//create/save file
							char number[64];
							Std_buildNumber(r, 0, number);
							char number_index[64];
							Std_buildNumber(ri, 0, number_index);

							UNI nameUni[64];
							DbColumn_getName(column->column, nameUni, 64);

							char ext[8];
							DbValue_getFileExt_char(column, ri, ext);

							char* ff_name = Std_addAfterCHAR(Std_addAfterCHAR(Std_addAfterCHAR(Std_addAfterCHAR(Std_addAfterCHAR(Std_addAfterCHAR(Std_newCHAR_uni(nameUni), "_"), number), "_"), number_index), "."), ext);
							char* ff_path_short = Std_addAfterCHAR(Std_addCHAR(folderName, "/"), ff_name);
							char* ff_path = Std_addAfterCHAR(Std_addCHAR(folderPath, "/"), ff_name);
							OsFile ff;
							if (OsFile_init(&ff, ff_path, OsFile_W))
							{
								DbValue_exportFile(column, &ff, ri);
								OsFile_free(&ff);
							}

							//create link(render)
							fprintf(f.m_file, "<a href=\"%s\">", ff_path_short);

							if (renderImages && IOHtml_isImage(ext))
								fprintf(f.m_file, "<img src=\"%s\" alt=\"%s\" height=\"100\">", ff_path_short, ff_name);
							else
								fprintf(f.m_file, "%s", ff_name);

							OsFile_writeUNI(&f, _UNI32("</a>\n"));

							Std_deleteCHAR(ff_name);
							Std_deleteCHAR(ff_path_short);
							Std_deleteCHAR(ff_path);
						}
					}
				}
				else
					OsFile_writeUNI(&f, str);

				OsFile_writeUNI(&f, _UNI32("</td>"));
			}
			OsFile_writeUNI(&f, _UNI32("</tr>\n"));

			StdProgress_setEx("EXPORTING", r, maxRows);
		}

		//End
		OsFile_writeUNI(&f, g_IOHtml_end);

		OsFile_free(&f);
		ok = TRUE;
	}

	Std_deleteCHAR(folderName);
	Std_deleteCHAR(folderPath);

	return ok;
}

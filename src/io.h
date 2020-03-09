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

BOOL IOFiles_write(const char* path, DbRows* rows, DbValue* data);
BOOL IOFiles_writeSingle(const char* path, DbValue* data);

UBIG IOFiles_readNet(const char* url, DbValue* file, int index);
void IOFiles_read(const char* paths, BOOL subDirs, DbRows* tableOut);
BIG IOFiles_readSingle(const char* path, DbValue* value, int index);

BOOL IOCsv_write(const char* path, BOOL firstRowColumnNames, DbRows* rows, DbValues* columns);
BOOL IOCsv_read(const char* path, BOOL firstRowColumnNames, BOOL recognizeColumnType, DbRows* tableOut);

BOOL IOTsv_write(const char* path, BOOL firstRowColumnNames, DbRows* rows, DbValues* columns);
BOOL IOTsv_read(const char* path, BOOL firstRowColumnNames, BOOL recognizeColumnType, DbRows* tableOut);

BOOL IOHtml_write(const char* path, BOOL renderImages, DbRows* rows, DbValues* columns);

BOOL IOSql_write(const char* path, DbRows* rows, DbValues* columns);

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

BOOL IOFiles_write(const char* path, DbRows* rows, DbValue* data, volatile StdProgress* progress);
BOOL IOFiles_writeSingle(const char* path, DbValue* data, volatile StdProgress* progress);

UBIG IOFiles_readNet(const char* url, DbValue* file, int index, volatile StdProgress* progress);
void IOFiles_read(const char* paths, BOOL subDirs, DbRows* tableOut, volatile StdProgress* progress);
BIG IOFiles_readSingle(const char* path, DbValue* value, int index, volatile StdProgress* progress);

BOOL IOCsv_write(const char* path, BOOL firstRowColumnNames, DbRows* rows, DbValues* columns, volatile StdProgress* progress);
BOOL IOCsv_read(const char* path, BOOL firstRowColumnNames, BOOL recognizeColumnType, DbRows* tableOut, volatile StdProgress* progress);

BOOL IOTsv_write(const char* path, BOOL firstRowColumnNames, DbRows* rows, DbValues* columns, volatile StdProgress* progress);
BOOL IOTsv_read(const char* path, BOOL firstRowColumnNames, BOOL recognizeColumnType, DbRows* tableOut, volatile StdProgress* progress);

BOOL IOHtml_write(const char* path, BOOL renderImages, DbRows* rows, DbValues* columns, volatile StdProgress* progress);

BOOL IOSql_write(const char* path, DbRows* rows, DbValues* columns, volatile StdProgress* progress);

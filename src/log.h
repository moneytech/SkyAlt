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

void Logs_initGlobal(void);
void Logs_freeGlobal(void);
void Logs_clear(void);

void Logs_addInfo(const char* id);
void Logs_addWarning(const char* id);
void Logs_addError(const char* id);

BOOL Logs_isChanged(void);

UBIG Logs_num(void);
UNI* Logs_dateStr(UBIG i);
const char* Logs_id(UBIG i);
const char* Logs_type(UBIG i);

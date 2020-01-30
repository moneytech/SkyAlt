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

BOOL Lang_initGlobal(const char* lang_selectName);
void Lang_freeGlobal(void);
void Lang_setPos(UINT i);
int Lang_getPos(void);
BOOL Lang_is(const char* id);
const UNI* Lang_find(const char* id);
const UNI* Lang_find_month(int index);
const UNI* Lang_find_shortday(int index);
int Lang_findLangNamePos(const char* language);
const char* Lang_getLangName(void);
UNI** Lang_getList(const UNI* find, BIG* out_size);

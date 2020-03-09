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

#define LICENSE_DAYS_WARNING 7

BOOL License_init(void);
void License_free(void);
UNI* License_getTitle(void);
const UNI* License_getCompany(void);
int License_getCount(void);
OsDate License_getExpiration(void);
float License_getRemainingDays(void);
BOOL License_exist(void);
BOOL License_isTimeValid(void);

void License_makeKey(void);
void License_makeFile(const UNI* company, const int num_license, const int num_months);
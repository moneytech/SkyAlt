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

 //collaborate
#include "os.h"
#include "std.h"
#include "language.h"
#include "log.h"
#include "file.h"
#include "map.h"

//header
#include "license.h"

const UINT g_license_bom = 0x0000feff;	//UTF-32

UCHAR g_License_pubKey[] = { 4,166,223,8,223,38,99,225,236,186,231,210,179,177,229,162,147,174,249,140,241,168,20,237,114,249,121,209,96,53,88,131,72,26,42,113,200,139,141,104,120,108,93,229,164,97,159,8,78,96,74,148,34,40,91,61,59,34,155,147,151,135,131,212,194,204,204,204,204,204,204,204,204,204,204,204,204,204,204,204,204,204,204,204,204,204,204,204,204,204,204,204,204,204,204,204 };
const OsCryptoECDSAPublic* License_get_pubKey(void)
{
	return (OsCryptoECDSAPublic*)g_License_pubKey;
}

typedef struct License_s
{
	UNI* company;
	int num_license;
	OsDate expiration;
}License;

License* g_License = 0;

void License_free(void)
{
	if (g_License)
	{
		Std_deleteUNI(g_License->company);
		Os_free(g_License, sizeof(License));
		g_License = 0;
	}
}

BOOL License_init()
{
	if (g_License)
		License_free();

	BOOL signOk = FALSE;

	UNI* company = 0;
	int num_license = 0;
	OsDate expiration;

	UBIG str_n;
	UNI* origFile = (UNI*)OsFile_initRead("license", &str_n, sizeof(UNI));

	UINT* bom = (UINT*)origFile;
	if (bom && *bom == g_license_bom)
	{
		UNI* origStr = (UNI*)(bom + 1);
		UNI* str = origStr;

		BIG f = Std_findUNI(str, '\n');
		if (f > 0)
		{
			company = Std_newUNI_copy(str, f);	//company
			str += f + 1;
			f = Std_findUNI(str, '\n');
			if (f > 0)
			{
				num_license = Std_getNumberFromUNI(str);	//#licenses
				str += f + 1;
				f = Std_findUNI(str, '\n');
				if (f > 0)
				{
					//#expiration date
					expiration = OsDate_initFromNumber(Std_getNumberFromUNI(str));//expiration

					str += f + 1;
					BIG msgSize = ((UBIG)str) - ((UBIG)origStr);

					f = Std_findUNI(str, '\n');
					if (f < 0)
						f = Std_sizeUNI(str);
					if (f > 0)
					{
						//check signiture
						OsCryptoECDSA ecdsa;
						if (OsCryptoECDSA_initFromPublic(&ecdsa, License_get_pubKey()))
						{
							if (f < sizeof(OsCryptoECDSASign) * 2 - 1)
							{
								int sign_size = f / 2;
								OsCryptoECDSASign sign;
								Std_getFromHEX_uni(str, f + 1, (UCHAR*)&sign);
								signOk = OsCryptoECDSA_verify(&ecdsa, (UCHAR*)origStr, msgSize, &sign, sign_size);
							}
						}
					}
				}
			}
		}
	}
	Std_deleteUNI(origFile);

	if (signOk)
	{
		g_License = Os_malloc(sizeof(License));
		g_License->company = company;
		g_License->num_license = num_license;
		g_License->expiration = expiration;
	}
	else
		Std_deleteUNI(company);

	return signOk;
}

const UNI* License_getCompany(void)
{
	return g_License ? g_License->company : 0;
}
int License_getCount(void)
{
	return g_License ? g_License->num_license : 0;
}
OsDate License_getExpiration(void)
{
	return g_License ? g_License->expiration : OsDate_initEmpty();
}

float License_getRemainingDays(void)
{
	float days = 0;
	if (g_License)
	{
		OsDate now = OsDate_initActual();
		OsDate exp = License_getExpiration();
		days = OsDate_differenceSeconds(&exp, &now) / 3600 / 24;
	}
	return days;
}

BOOL License_exist(void)
{
	return (g_License != 0);
}
BOOL License_isTimeValid(void)
{
	if (!License_exist())
		return FALSE;

	OsDate now = OsDate_initActual();
	return OsDate_differenceSeconds(&g_License->expiration, &now) > 0;
}

UNI* License_getTitle(void)
{
	if (!License_exist())
		return Std_newUNI(Lang_find("NON_COMMERCIAL_USE_ONLY"));

	UNI* str = Std_newUNI(g_License->company);

	str = Std_addAfterUNI_char(str, " - ");

	if (License_isTimeValid())
	{
		str = Std_addAfterUNI(str, Lang_find("COMMERCIAL_OK"));

		float remainDays = License_getRemainingDays();
		if (remainDays < LICENSE_DAYS_WARNING)
		{
			str = Std_addAfterUNI_char(str, " - ");

			char days[64];
			Std_buildNumber(remainDays, 1, days);
			str = Std_addAfterUNI_char(str, days);

			str = Std_addAfterUNI_char(str, " ");
			str = Std_addAfterUNI(str, Lang_find("DAYS"));
		}
	}
	else
		str = Std_addAfterUNI(str, Lang_find("COMMERCIAL_EXPIRED"));

	return str;
}

void License_makeKey(void)
{
	OsCryptoECDSA ecdsa;
	if (OsCryptoECDSA_initRandom(&ecdsa))
	{
		OsCryptoECDSAKey privKey;
		OsCryptoECDSAPublic pubKey;
		if (OsCryptoECDSA_exportKey(&ecdsa, &privKey), OsCryptoECDSA_exportPublic(&ecdsa, &pubKey))
		{
			printf("private: ");
			Std_array_print((UCHAR*)&privKey, sizeof(OsCryptoECDSAKey));

			printf("public: ");
			Std_array_print((UCHAR*)&pubKey, sizeof(OsCryptoECDSAKey));
		}
	}
}
/*#include "../signitures/license.h"
void License_makeFile(const UNI* company, const int num_license, const int num_months)
{
	UNI* str = 0;

	//company
	{
		str = Std_addAfterUNI(str, company);
		str = Std_addAfterUNI_char(str, "\n");
	}

	//#licenses
	{
		char nmbr[64];
		snprintf(nmbr, 64, "%d", num_license);
		str = Std_addAfterUNI_char(str, nmbr);
		str = Std_addAfterUNI_char(str, "\n");
	}

	//expiration date
	{
		OsDate date = OsDate_initActual();
		int i;
		for (i = 0; i < num_months; i++)
			OsDate_addMonth(&date);

		//round it to end of month
		OsDate_addMonth(&date);
		date.m_day = 0;
		date.m_hour = 0;
		date.m_min = 0;
		date.m_sec = 0;

		char nmbr[64];
		snprintf(nmbr, 32, "%lld", (UBIG)OsDate_asNumber(&date));
		str = Std_addAfterUNI_char(str, nmbr);
		str = Std_addAfterUNI_char(str, "\n");
	}

	//signiture
	{
		OsCryptoECDSA ecdsa;
		if (OsCryptoECDSA_initFromKey(&ecdsa, License_get_privKey()))
		{
			OsCryptoECDSASign sign;
			int sign_n = OsCryptoECDSA_sign(&ecdsa, (UCHAR*)str, Std_sizeUNI(str) * sizeof(UNI), &sign);
			char hex[sizeof(OsCryptoECDSASign) * 2 + 1];
			Std_setHEX_char(hex, (UCHAR*)&sign, sign_n);

			str = Std_addAfterUNI_char(str, hex);

			OsCryptoECDSA_free(&ecdsa);
		}
	}

	//write file
	{
		OsFile file;
		if (OsFile_init(&file, "license", OsFile_W))
		{
			OsFile_write(&file, &g_license_bom, sizeof(bom));

			OsFile_write(&file, str, Std_sizeUNI(str) * sizeof(UNI));
			OsFile_free(&file);
		}
	}

	Std_deleteUNI(str);

	if (License_init())
		printf("License makeFile() success\n");
	else
		printf("Error: License makeFile()\n");

	License_free();
}*/
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

//header
#include "language.h"

typedef struct LangItem_s
{
	char* id;
	UNI* translation;
}LangItem;
LangItem LangItem_init(char* id, UNI* translation)
{
	LangItem self;
	self.id = id;
	self.translation = translation;
	return self;
}
void LangItem_free(LangItem* self)
{
	Std_deleteCHAR(self->id);
	Std_deleteUNI(self->translation);
	Os_memset(self, sizeof(LangItem));
}

typedef struct Lang_s
{
	char* name;
	LangItem* items;
	int num;
}Lang;
Lang Lang_init(const char* name)
{
	Lang self;
	self.name = Std_newCHAR(name);
	self.items = 0;
	self.num = 0;
	return self;
}
void Lang_addItem(Lang* self, LangItem item)
{
	self->num++;
	self->items = Os_realloc(self->items, self->num * sizeof(LangItem));
	self->items[self->num - 1] = item;
}
void Lang_free(Lang* self)
{
	int i;
	for (i = 0; i < self->num; i++)
		LangItem_free(&self->items[i]);
	Os_free(self->items, self->num * sizeof(LangItem));

	Std_deleteCHAR(self->name);
	Os_memset(self, sizeof(Lang));
}

Lang* g_lang_langs = 0;
int g_lang_num = 0;
int g_lang_select = -1;
UNI* g_lang_list = 0;

void Lang_freeGlobal(void)
{
	int i;
	for (i = 0; i < g_lang_num; i++)
		Lang_free(&g_lang_langs[i]);

	Os_free(g_lang_langs, g_lang_num * sizeof(Lang));
	Std_deleteUNI(g_lang_list);

	g_lang_langs = 0;
	g_lang_num = 0;
	g_lang_select = -1;
	g_lang_list = 0;
}

const UNI* Lang_findEx(const char* id, int langPos);

BOOL Lang_initGlobal(const char* lang_selectName)
{
	Lang_freeGlobal();

	int i;
	int fi;

	char* currPath = OsFileDir_currentDir();

	//language files
	{
		char* dir = Std_addCHAR(currPath, "/languages");

		char** pathes;
		const BIG num_files = OsFileDir_getFileList(dir, TRUE, FALSE, TRUE, &pathes);

		for (fi = 0; fi < num_files; fi++)
		{
			g_lang_num++;
			g_lang_langs = Os_realloc(g_lang_langs, g_lang_num * sizeof(Lang));
			Lang* lang = &g_lang_langs[g_lang_num - 1];

			{
				char* folder;
				char* name;
				OsFile_getParts(pathes[fi], &folder, &name);

				*lang = Lang_init(name);

				Std_deleteCHAR(folder);
				Std_deleteCHAR(name);
			}

			UBIG N;
			UNI* strOrig = (UNI*)OsFile_initRead(pathes[fi], &N, 4);
			UNI* str = strOrig;
			if (str)	str++;	//skip BOM

			int line = 1;
			while (str && *str)
			{
				//get id
				UBIG n = 0;
				while (str[n] && str[n] != '\r' && str[n] != '\n' && str[n] != ' ')
					n++;
				char* id = Std_newCHAR_uni_n(str, n);
				str += n;

				//skip space
				while (*str && (*str == '\t' || *str == ' '))
					str++;

				//translation
				n = 0;
				while (str[n] && str[n] != '\r' && str[n] != '\n')
					n++;
				UNI* translation = Std_newUNI_copy(str, n);
				str += n;

				//add
				if (Std_sizeCHAR(id) && Std_sizeUNI(translation))
					Lang_addItem(lang, LangItem_init(id, translation));
				else
				{
					printf("Warning: Language file(%s) line(%d) is invalid(%s)\n", pathes[fi], line, id);
					Std_deleteCHAR(id);
					Std_deleteUNI(translation);
				}

				while (*str && (*str == '\r' || *str == '\n' || *str == '\t' || *str == ' '))	//skip empty lines
				{
					if (*str == '\n')		line++;
					str++;
				}
			}

			Os_free(strOrig, N);
			Std_deleteCHAR(pathes[fi]);
		}
		Os_free(pathes, num_files * sizeof(char*));
		Std_deleteCHAR(dir);
	}

	//license files
	{
		char* dir = Std_addCHAR(currPath, "/eula");
		char** pathes;
		const BIG num_files = OsFileDir_getFileList(dir, TRUE, FALSE, TRUE, &pathes);

		for (fi = 0; fi < num_files; fi++)
		{
			Lang* findLang = 0;

			{
				char* folder;
				char* name;
				OsFile_getParts(pathes[fi], &folder, &name);

				//find it
				for (i = 0; i < g_lang_num; i++)
				{
					if (Std_cmpCHARsmall(name, g_lang_langs[i].name))
						findLang = &g_lang_langs[i];
				}

				Std_deleteCHAR(folder);
				Std_deleteCHAR(name);
			}

			if (findLang)
			{
				UBIG N;
				UNI* str = (UNI*)OsFile_initRead(pathes[fi], &N, 4);
				if (str)
				{
					Std_removeLetterUNI(str, '\r');
					Lang_addItem(findLang, LangItem_init(Std_newCHAR("EULA_TEXT"), Std_newUNI(str + 1)));	//+1 skip BOM
				}
				Os_free(str, N);
			}

			Std_deleteCHAR(pathes[fi]);
		}
		Os_free(pathes, num_files * sizeof(char*));
		Std_deleteCHAR(dir);
	}

	//creates g_lang_list
	for (i = 0; i < g_lang_num; i++)
	{
		Lang* lang = &g_lang_langs[i];

		int ii;
		for (ii = 0; ii < lang->num; ii++)
		{
			if (Std_cmpCHARsmall("THIS_LANGUAGE", lang->items[ii].id))
			{
				g_lang_list = Std_addAfterUNI(g_lang_list, lang->items[ii].translation);
				if (i + 1 < g_lang_num)
					g_lang_list = Std_addAfterUNI(g_lang_list, _UNI32("/"));
				break;
			}
		}
	}

	//select language
	for (i = 0; i < g_lang_num; i++)
	{
		if (Std_cmpCHARsmall(lang_selectName, g_lang_langs[i].name))
			g_lang_select = i;

		if (g_lang_select < 0 && Std_cmpCHARsmall("en", g_lang_langs[i].name))
			g_lang_select = i;
	}

	//copy english EULA if there are not available in langauge
	BOOL eula_exist = FALSE;
	const UNI* eulaStr = Lang_findEx("EULA_TEXT", Lang_findLangNamePos("en"));
	for (i = 0; i < g_lang_num; i++)
	{
		if (Lang_is("EULA_TEXT"))
			eula_exist = TRUE;
		else
			Lang_addItem(&g_lang_langs[i], LangItem_init(Std_newCHAR("EULA_TEXT"), Std_newUNI(eulaStr)));
	}
	Std_deleteCHAR(currPath);


	if(g_lang_select < 0)
		printf("Error: No language translation available(folder 'languages' missing or it is empty)\n");
	else
	if(!eula_exist)
		printf("Error: Eula translation not found(folder 'eula' missing or it is empty)\n");

	return (eula_exist && g_lang_select >= 0);
}

void Lang_setPos(UINT i)
{
	g_lang_select = Std_clamp(i, 0, g_lang_num - 1);
}
int Lang_getPos(void)
{
	return g_lang_select;
}

BOOL Lang_is(const char* id)
{
	Lang* self = &g_lang_langs[g_lang_select];

	int i;
	for (i = 0; i < self->num; i++)
	{
		if (Std_cmpCHARsmall(id, self->items[i].id))
			return TRUE;
	}
	if (Std_cmpCHARsmall(id, "LANGUAGE_LIST"))
		return TRUE;

	return FALSE;
}

const UNI* Lang_findEx(const char* id, int langPos)
{
	Lang* self = &g_lang_langs[langPos];

	int i;
	for (i = 0; i < self->num; i++)
	{
		if (Std_cmpCHARsmall(id, self->items[i].id))
			return self->items[i].translation;
	}

	if (Std_cmpCHARsmall(id, "LANGUAGE_LIST"))
		return g_lang_list;

	//printf("Warning: Lang_find(%s) can't find translation\n", id);
	//Os_showConsole(TRUE);

	return 0;// _UNI32("--No translation--");
}
const UNI* Lang_find(const char* id)
{
	return Lang_findEx(id, g_lang_select);
}

static const char* _Lang_findId(const Lang* self, const UNI* translation)
{
	int i;
	for (i = 0; i < self->num; i++)
	{
		if (Std_cmpUNIsmall(translation, self->items[i].translation))
			return self->items[i].id;
	}
	return 0;
}

int Lang_findLangNamePos(const char* language)
{
	int i;
	for (i = 0; i < g_lang_num; i++)
	{
		if (Std_cmpCHARsmall(language, g_lang_langs[i].name))
			return i;
	}
	return -1;
}
const char* Lang_getLangName(void)
{
	return g_lang_langs[g_lang_select].name;
}

const char* g_lang_months[] =
{
	"MONTH_JAN",
	"MONTH_FEB",
	"MONTH_MAR",
	"MONTH_APR",
	"MONTH_MAY",
	"MONTH_JUN",
	"MONTH_JUL",
	"MONTH_AUG",
	"MONTH_SEP",
	"MONTH_OCT",
	"MONTH_NOV",
	"MONTH_DEC",
};
const char* g_lang_smonths[] =
{
	"SMONTH_JAN",
	"SMONTH_FEB",
	"SMONTH_MAR",
	"SMONTH_APR",
	"SMONTH_MAY",
	"SMONTH_JUN",
	"SMONTH_JUL",
	"SMONTH_AUG",
	"SMONTH_SEP"
	"SMONTH_OCT",
	"SMONTH_NOV",
	"SMONTH_DEC",
};

const char* g_lang_days[] =
{
	"DAY_MONDAY",
	"DAY_TUESDAY",
	"DAY_WEDNESDAY",
	"DAY_THURSDAY",
	"DAY_FRIDAY",
	"DAY_SATURDAY",
	"DAY_SUNDAY",
};
const char* g_lang_sdays[] =
{
	"SDAY_MONDAY",
	"SDAY_TUESDAY",
	"SDAY_WEDNESDAY",
	"SDAY_THURSDAY",
	"SDAY_FRIDAY",
	"SDAY_SATURDAY",
	"SDAY_SUNDAY",
};

const UNI* Lang_find_month(int index)
{
	return Lang_find(g_lang_months[Std_clamp(index, 0, 11)]);
}

const UNI* Lang_find_shortday(int index)
{
	return Lang_find(g_lang_sdays[Std_clamp(index, 0, 6)]);
}

static int _Lang_findIdInList(const char* id, const char** list, int listN)
{
	int i;
	for (i = 0; i < listN; i++)
		if (Std_cmpCHARsmall(id, list[i]))
			return i;
	return -1;
}

static UNI** _Lang_getSubList(const char* id, const char** list, int listN, BIG* out_size)
{
	UNI** ret = 0;
	int i = _Lang_findIdInList(id, list, listN);	//is 'id' in list
	if (i >= 0)
	{
		//create list
		ret = Os_malloc(listN * sizeof(UNI*));

		int p = 0;
		int ii;
		for (ii = i; ii < listN; ii++)
			ret[p++] = Std_newUNI(Lang_find(list[ii]));
		for (ii = 0; ii < i; ii++)
			ret[p++] = Std_newUNI(Lang_find(list[ii]));

		*out_size = listN;
	}
	return ret;
}

static UNI** _Lang_getList(const Lang* lang, const UNI* find, BIG* out_size)
{
	UNI** list = 0;

	const char* id = _Lang_findId(lang, find);
	if (id)
	{
		if (!list)	list = _Lang_getSubList(id, g_lang_days, sizeof(g_lang_days) / sizeof(char*), out_size);
		if (!list)	list = _Lang_getSubList(id, g_lang_sdays, sizeof(g_lang_sdays) / sizeof(char*), out_size);

		if (!list)	list = _Lang_getSubList(id, g_lang_months, sizeof(g_lang_months) / sizeof(char*), out_size);
		if (!list)	list = _Lang_getSubList(id, g_lang_smonths, sizeof(g_lang_smonths) / sizeof(char*), out_size);
	}

	return list;
}

UNI** Lang_getList(const UNI* find, BIG* out_size)
{
	UNI** list = _Lang_getList(&g_lang_langs[g_lang_select], find, out_size);

	int i;
	for (i = 0; i < g_lang_num; i++)
	{
		if (!list)
			list = _Lang_getList(&g_lang_langs[i], find, out_size);
	}

	return list;
}
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

typedef struct GuiItemCalendarSmall_s
{
	GuiItem base;

	OsDateTimeTYPE formatTime;
	DbValue value;

	OsDate currDate;
	OsDate pageDate;

	UBIG subs_month;
	UBIG subs_today;
	UBIG subs_days;

	BIG subs_man_days;
	BIG subs_man_time;
}GuiItemCalendarSmall;

GuiItem* GuiItemCalendarSmall_new(Quad2i grid, DbValue value, OsDateTimeTYPE formatTime, GuiItemCallback* call)
{
	GuiItemCalendarSmall* self = Os_malloc(sizeof(GuiItemCalendarSmall));
	self->base = GuiItem_init(GuiItem_CALENDAR_SMALL, grid);

	GuiItem_setCallClick(&self->base, call);

	self->value = value;
	self->formatTime = formatTime;

	self->currDate = OsDate_initEmpty();
	self->pageDate = OsDate_initEmpty();

	return (GuiItem*)self;
}

GuiItem* GuiItemCalendarSmall_newCopy(GuiItemCalendarSmall* src, BOOL copySub)
{
	GuiItemCalendarSmall* self = Os_malloc(sizeof(GuiItemCalendarSmall));
	*self = *src;
	GuiItem_initCopy(&self->base, &src->base, copySub);

	self->value = DbValue_initCopy(&src->value);

	return (GuiItem*)self;
}

void GuiItemCalendarSmall_delete(GuiItemCalendarSmall* self)
{
	DbValue_free(&self->value);

	GuiItem_free(&self->base);
	Os_free(self, sizeof(GuiItemCalendarSmall));
}

int GuiItemCalendarSmall_getSizeY(const GuiItemCalendarSmall* self)
{
	return self->formatTime > 0 ? 15 : 13;
}

BOOL GuiItemCalendarSmall_isUS(void)
{
	return UiIniSettings_getDateFormat() == OsDate_US;
}

static OsDate _GuiItemCalendarSmall_getValue(const GuiItemCalendarSmall* self)
{
	return DbValue_getDate(&self->value);
}
static void _GuiItemCalendarSmall_setValue(GuiItemCalendarSmall* self, OsDate date)
{
	DbValue_setDate(&self->value, date);
}

void GuiItemCalendarSmall_clickToday(GuiItem* self)
{
	GuiItemCalendarSmall* calendar = GuiItem_findParentType(self->parent, GuiItem_CALENDAR_SMALL);
	if (calendar)
	{
		_GuiItemCalendarSmall_setValue(calendar, OsDate_initActual());

		calendar->pageDate = OsDate_initActual();	//revert calendar

		GuiItem_callClick(&calendar->base);
	}
}

void GuiItemCalendarSmall_clickMonthBack(GuiItem* self)
{
	GuiItemCalendarSmall* calendar = GuiItem_findParentType(self->parent, GuiItem_CALENDAR_SMALL);
	if (calendar)
	{
		OsDate_subMonth(&calendar->pageDate);
	}
}
void GuiItemCalendarSmall_clickMonthForward(GuiItem* self)
{
	GuiItemCalendarSmall* calendar = GuiItem_findParentType(self->parent, GuiItem_CALENDAR_SMALL);
	if (calendar)
	{
		OsDate_addMonth(&calendar->pageDate);
	}
}

void GuiItemCalendarSmall_clickSDay(GuiItem* self)
{
	GuiItemCalendarSmall* calendar = GuiItem_findParentType(self->parent, GuiItem_CALENDAR_SMALL);
	if (calendar)
	{
		BIG v = GuiItem_findAttribute(self, "date");

		OsDate src = *(OsDate*)&v;

		OsDate dst = _GuiItemCalendarSmall_getValue(calendar);
		dst.m_year = src.m_year;
		dst.m_month = src.m_month;
		dst.m_day = src.m_day;
		_GuiItemCalendarSmall_setValue(calendar, dst);

		GuiItem_callClick(&calendar->base);
	}
}

void GuiItemCalendarSmall_clickYear(GuiItem* self)
{
	GuiItemCalendarSmall* calendar = GuiItem_findParentType(self->parent, GuiItem_CALENDAR_SMALL);
	if (calendar)
	{
		OsDate date = _GuiItemCalendarSmall_getValue(calendar);
		date.m_year = Std_max(DbValue_getNumber(&((GuiItemEdit*)self)->text), 0);
		_GuiItemCalendarSmall_setValue(calendar, date);
		calendar->pageDate = date;
	}
}
void GuiItemCalendarSmall_clickMonth(GuiItem* self)
{
	GuiItemCalendarSmall* calendar = GuiItem_findParentType(self->parent, GuiItem_CALENDAR_SMALL);
	if (calendar)
	{
		OsDate date = _GuiItemCalendarSmall_getValue(calendar);
		date.m_month = Std_clamp(DbValue_getNumber(&((GuiItemEdit*)self)->text), 0, 12) - 1;
		_GuiItemCalendarSmall_setValue(calendar, date);
		calendar->pageDate = date;
	}
}
void GuiItemCalendarSmall_clickDay(GuiItem* self)
{
	GuiItemCalendarSmall* calendar = GuiItem_findParentType(self->parent, GuiItem_CALENDAR_SMALL);
	if (calendar)
	{
		OsDate date = _GuiItemCalendarSmall_getValue(calendar);
		date.m_day = Std_clamp(DbValue_getNumber(&((GuiItemEdit*)self)->text), 0, 31) - 1;
		_GuiItemCalendarSmall_setValue(calendar, date);
		calendar->pageDate = date;
	}
}
void GuiItemCalendarSmall_clickHour(GuiItem* self)
{
	GuiItemCalendarSmall* calendar = GuiItem_findParentType(self->parent, GuiItem_CALENDAR_SMALL);
	if (calendar)
	{
		OsDate date = _GuiItemCalendarSmall_getValue(calendar);
		date.m_hour = Std_clamp(DbValue_getNumber(&((GuiItemEdit*)self)->text), 0, 23);
		_GuiItemCalendarSmall_setValue(calendar, date);
	}
}
void GuiItemCalendarSmall_clickMinute(GuiItem* self)
{
	GuiItemCalendarSmall* calendar = GuiItem_findParentType(self->parent, GuiItem_CALENDAR_SMALL);
	if (calendar)
	{
		OsDate date = _GuiItemCalendarSmall_getValue(calendar);
		date.m_min = Std_clamp(DbValue_getNumber(&((GuiItemEdit*)self)->text), 0, 59);
		_GuiItemCalendarSmall_setValue(calendar, date);
	}
}
void GuiItemCalendarSmall_clickSecond(GuiItem* self)
{
	GuiItemCalendarSmall* calendar = GuiItem_findParentType(self->parent, GuiItem_CALENDAR_SMALL);
	if (calendar)
	{
		OsDate date = _GuiItemCalendarSmall_getValue(calendar);
		date.m_sec = Std_clamp(DbValue_getNumber(&((GuiItemEdit*)self)->text), 0, 59);
		_GuiItemCalendarSmall_setValue(calendar, date);
	}
}

void GuiItemCalendarSmall_update(GuiItemCalendarSmall* self, Quad2i coord, Win* win)
{
	OsDate realToday = OsDate_initActual();

	self->currDate = _GuiItemCalendarSmall_getValue(self);

	if (self->pageDate.m_year == 0)
	{
		OsDate curr = _GuiItemCalendarSmall_getValue(self);
		if (curr.m_year == 0)
			curr = OsDate_initActual();
		self->pageDate = curr;
	}

	//update cells
	if (self->base.subs.num)
	{
		GuiItem* layout = GuiItem_getSub(&self->base, 0);

		//today
		{
			GuiItemButton* today = (GuiItemButton*)GuiItem_getSub(layout, self->subs_today);
			char* monthStr = Std_newCHAR_uni(Lang_find_month(realToday.m_month));
			UNI* str = OsDate_getStringDateTimeUNI(&realToday, UiIniSettings_getDateFormat(), self->formatTime, monthStr);
			DbValue_setTextCopy(&today->text, str);
			Std_deleteUNI(str);
			Std_deleteCHAR(monthStr);
		}

		//month
		GuiItemText* month = (GuiItemText*)GuiItem_getSub(layout, self->subs_month);
		UNI* yearStr = Std_newNumber(self->pageDate.m_year);
		UNI* monthStr = Std_newUNI(Lang_find_month(self->pageDate.m_month));
		monthStr = Std_addAfterUNI(monthStr, _UNI32(" "));
		monthStr = Std_addAfterUNI(monthStr, yearStr);
		Std_deleteUNI(yearStr);
		DbValue_setTextCopy(&month->text, monthStr);
		Std_deleteUNI(monthStr);

		//days
		OsDate pdate = OsDate_initDay(0, self->pageDate.m_month, self->pageDate.m_year);
		int week = OsDate_getWeekDay(&pdate);
		if (GuiItemCalendarSmall_isUS())
			week++;
		while (week > 0)	//loop!
		{
			OsDate_subDay(&pdate);
			week--;
		}

		int i;
		for (i = 0; i < 7 * 6; i++)
		{
			GuiItemButton* day = (GuiItemButton*)GuiItem_getSub(layout, self->subs_days + i);
			DbValue_setNumber(&day->text, pdate.m_day + 1);
			GuiItem_setAttribute((GuiItem*)day, "date", *(BIG*)&pdate);

			day->type = (OsDate_cmpOnlyDate(&pdate, &self->currDate)) ? GuiItemButton_BLACK : GuiItemButton_CLASSIC;
			day->circle = OsDate_cmpOnlyDate(&pdate, &realToday);

			GuiItem_setEnable(&day->base, pdate.m_month == self->pageDate.m_month);
			OsDate_addDay(&pdate);
		}

		//date
		DbValue_setNumber(&((GuiItemButton*)GuiItem_getSub(layout, self->subs_man_days + 0))->text, self->currDate.m_year);
		DbValue_setNumber(&((GuiItemButton*)GuiItem_getSub(layout, self->subs_man_days + 1))->text, self->currDate.m_month+1);
		DbValue_setNumber(&((GuiItemButton*)GuiItem_getSub(layout, self->subs_man_days + 2))->text, self->currDate.m_day+1);

		//time
		if (self->subs_man_time > 0)
		{
			DbValue_setNumber(&((GuiItemButton*)GuiItem_getSub(layout, self->subs_man_time + 0))->text, self->currDate.m_hour);
			DbValue_setNumber(&((GuiItemButton*)GuiItem_getSub(layout, self->subs_man_time + 1))->text, self->currDate.m_min);
			DbValue_setNumber(&((GuiItemButton*)GuiItem_getSub(layout, self->subs_man_time + 2))->text, self->currDate.m_sec);
		}
	}

	GuiItem_setRedraw(&self->base, DbValue_hasChanged(&self->value));
}

GuiItemLayout* GuiItemCalendarSmall_resize(GuiItemCalendarSmall* self, GuiItemLayout* layout, Win* win)
{
	if (!self->base.resize)
		return (GuiItemLayout*)GuiItem_getSub(&self->base, 0);

	int x, y;
	GuiItemCalendarSmall* calc = (GuiItemCalendarSmall*)self;
	GuiItem_freeSubs(&self->base);

	//layout
	layout = GuiItemLayout_newCoord(&self->base, FALSE, FALSE, win);
	GuiItem_addSubName(&self->base, "layout_main", &layout->base);
	for (x = 0; x < 7; x++)
		GuiItemLayout_addColumn(layout, x, 3);

	//Today
	calc->subs_today = layout->base.subs.num;
	GuiItem_addSubName(&layout->base, "today", GuiItemButton_newClassicEx(Quad2i_init4(0, 0, 7, 1), DbValue_initEmpty(), &GuiItemCalendarSmall_clickToday));

	//Week header
	GuiItem_addSubName(&layout->base, "<", GuiItemButton_newAlphaEx(Quad2i_init4(0, 2, 1, 1), DbValue_initStaticCopy(_UNI32("<")), &GuiItemCalendarSmall_clickMonthBack));
	calc->subs_month = layout->base.subs.num;
	GuiItem_addSubName(&layout->base, "space", GuiItemText_new(Quad2i_init4(1, 2, 5, 1), TRUE, DbValue_initEmpty(), DbValue_initEmpty()));
	GuiItem_addSubName(&layout->base, ">", GuiItemButton_newAlphaEx(Quad2i_init4(6, 2, 1, 1), DbValue_initStaticCopy(_UNI32(">")), &GuiItemCalendarSmall_clickMonthForward));

	//Days
	if (GuiItemCalendarSmall_isUS())
	{
		GuiItem_addSubName(&layout->base, "sun", GuiItemText_new(Quad2i_init4(0, 3, 1, 1), TRUE, DbValue_initStaticCopy(Lang_find_shortday(6)), DbValue_initEmpty()));	//sunday
		for (x = 0; x < 6; x++)
			GuiItem_addSubName(&layout->base, "mon-sat", GuiItemText_new(Quad2i_init4(x + 1, 3, 1, 1), TRUE, DbValue_initStaticCopy(Lang_find_shortday(x)), DbValue_initEmpty()));	//monday-saturday
	}
	else
	{
		for (x = 0; x < 7; x++)
			GuiItem_addSubName(&layout->base, "sday", GuiItemText_new(Quad2i_init4(x, 3, 1, 1), TRUE, DbValue_initStaticCopy(Lang_find_shortday(x)), DbValue_initEmpty()));
	}
	calc->subs_days = layout->base.subs.num;
	for (y = 0; y < 6; y++)
		for (x = 0; x < 7; x++)
			GuiItem_addSubName(&layout->base, "button", GuiItemButton_newAlphaEx(Quad2i_init4(x, 4 + y, 1, 1), DbValue_initEmpty(), &GuiItemCalendarSmall_clickSDay));

	calc->subs_man_days = layout->base.subs.num;
	GuiItem_addSubName(&layout->base, "year", GuiItemEdit_newEx(Quad2i_init4(0, 11, 3, 2), DbValue_initEmpty(), DbValue_initLang("YEAR"), &GuiItemCalendarSmall_clickYear));
	GuiItem_addSubName(&layout->base, "month", GuiItemEdit_newEx(Quad2i_init4(3, 11, 2, 2), DbValue_initEmpty(), DbValue_initLang("MONTH"), &GuiItemCalendarSmall_clickMonth));
	GuiItem_addSubName(&layout->base, "day", GuiItemEdit_newEx(Quad2i_init4(5, 11, 2, 2), DbValue_initEmpty(), DbValue_initLang("DAY"), &GuiItemCalendarSmall_clickDay));

	if (self->formatTime > 0)
	{
		calc->subs_man_time = layout->base.subs.num;
		GuiItem_addSubName(&layout->base, "hour", GuiItemEdit_newEx(Quad2i_init4(0, 13, 3, 2), DbValue_initEmpty(), DbValue_initLang("HOUR"), &GuiItemCalendarSmall_clickHour));
		GuiItem_addSubName(&layout->base, "minute", GuiItemEdit_newEx(Quad2i_init4(3, 13, 2, 2), DbValue_initEmpty(), DbValue_initLang("MINUTE"), &GuiItemCalendarSmall_clickMinute));
		GuiItem_addSubName(&layout->base, "second", GuiItemEdit_newEx(Quad2i_init4(5, 13, 2, 2), DbValue_initEmpty(), DbValue_initLang("SECOND"), &GuiItemCalendarSmall_clickSecond));
	}
	else
		calc->subs_man_time = -1;

	return layout;
}

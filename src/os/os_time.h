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

double Os_time(void)
{
#ifdef _WIN32
	LARGE_INTEGER frequency;
	LARGE_INTEGER time;
	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&time);
	return ((double)time.QuadPart) / frequency.QuadPart;
#elif __linux__
	struct timeval startTime;
	gettimeofday(&startTime, 0);
	return (double)startTime.tv_sec + ((double)startTime.tv_usec) / 1000000;
#endif
}

double Os_printTime(const char* text, double startTime)
{
	const double dt = Os_time() - startTime;;
	printf("%s %f\n", text, dt);
	return dt;
}

int Os_timeZone(void)
{
#ifdef _WIN32
	TIME_ZONE_INFORMATION tzi;
	DWORD dwRet = GetTimeZoneInformation(&tzi);
	return tzi.Bias / -60;	//hours(prague is +1)
#elif __linux__
	time_t rawtime;
	struct tm* newtime;
	time(&rawtime);
	newtime = localtime(&rawtime);
	return newtime->tm_gmtoff / 3600;
#endif
}

double Os_timeUTC(void)
{
	double t = (UINT)time(0);	//in seconds

	double td = Os_time();
	td -= (UBIG)td;

	t += td;
	return t;	//in seconds
}

OsDate OsDate_initEmpty(void)
{
	OsDate d;
	d.m_year = d.m_month = d.m_day = d.m_hour = d.m_min = d.m_sec = 0;
	return d;
}
OsDate OsDate_initDay(char day, char month, short year)
{
	OsDate d = OsDate_initEmpty();
	d.m_year = year;
	d.m_month = month;
	d.m_day = day;
	return d;
}

OsDate OsDate_initActual(void)
{
	OsDate self = OsDate_initEmpty();

	time_t rawtime;
	struct tm* newtime;
	time(&rawtime);
	newtime = localtime(&rawtime);
	self.m_year = newtime->tm_year + 1900;
	self.m_month = newtime->tm_mon;
	self.m_day = newtime->tm_mday - 1;
	self.m_hour = newtime->tm_hour;
	self.m_min = newtime->tm_min;
	self.m_sec = newtime->tm_sec;
	return self;
}

BOOL OsDate_is(const OsDate* self)
{
	return self->m_year > 0;
}

BOOL OsDate_cmpOnlyDate(const OsDate* a, const OsDate* b)
{
	return a->m_year == b->m_year && a->m_month == b->m_month && a->m_day == b->m_day;
}

BOOL OsDate_cmpOnlyTime(const OsDate* a, const OsDate* b)
{
	return a->m_hour == b->m_hour && a->m_min == b->m_min && a->m_sec == b->m_sec;
}

BOOL OsDate_cmp(const OsDate* a, const OsDate* b)
{
	return OsDate_cmpOnlyDate(a, b) && OsDate_cmpOnlyTime(a, b);
}

UBIG OsDate_getDaysInYearEx(const short y)
{
	return (y % 4 == 0 && y % 100 != 0) || y % 400 == 0 ? 366 : 365;
}
UBIG OsDate_getDaysInYear(const OsDate* self)
{
	return OsDate_getDaysInYearEx(self->m_year);
}
UBIG OsDate_getDaysInMonthEx(const short m, const short y)
{
	if (m == 3 || m == 5 || m == 8 || m == 10)
		return 30;
	else
		if (m == 1)
			return OsDate_getDaysInYearEx(y) == 366 ? 29 : 28;
	return 31;
}
UBIG OsDate_getDaysInMonth(const OsDate* self)
{
	return OsDate_getDaysInMonthEx(self->m_month, self->m_year);
}

UBIG OsDate_getDaysFromYearZero(const OsDate* self)
{
	UBIG days = 0;
	int i;
	for (i = 0; i < self->m_year; i++)
		days += OsDate_getDaysInYearEx(i);
	for (i = 0; i < self->m_month; i++)
		days += OsDate_getDaysInMonthEx(i, self->m_year);
	days += (UBIG)self->m_day;
	return days;
}

UBIG OsDate_getSecondsFromYearZero(const OsDate* self)
{
	UBIG sec = OsDate_getDaysFromYearZero(self) * 24 * 60 * 60;
	sec += (UBIG)self->m_hour * 60 * 60;
	sec += (UBIG)self->m_min * 60;
	sec += self->m_sec;

	return sec;
}
BIG OsDate_differenceSeconds(const OsDate* curr, const OsDate* past)
{
	return ((BIG)OsDate_getSecondsFromYearZero(curr)) - ((BIG)OsDate_getSecondsFromYearZero(past));
}

//Monday = 0, Friday=4, Sunday=6
int OsDate_getWeekDay(const OsDate* self)
{
	return (OsDate_getDaysFromYearZero(self) + 5) % 7;
}

void OsDate_addMonth(OsDate* self)
{
	self->m_month++;
	if (self->m_month == 12)
	{
		self->m_month = 0;
		self->m_year++;
	};
}
void OsDate_subMonth(OsDate* self)
{
	self->m_month--;
	if (self->m_month == -1)
	{
		self->m_month = 11;
		self->m_year--;
	};
}
void OsDate_addDay(OsDate* self)
{
	self->m_day++;
	if (self->m_day == OsDate_getDaysInMonth(self))
	{
		self->m_day = 0;
		OsDate_addMonth(self);
	}
}
void OsDate_subDay(OsDate* self)
{
	self->m_day--;
	if (self->m_day == -1)
	{
		OsDate_subMonth(self);
		self->m_day = (char)OsDate_getDaysInMonth(self) - 1;
	}
}

void OsDate_getStringDateEU(const OsDate* self, char out[64])
{
	snprintf(out, 64, "%d/%d/%d", self->m_day + 1, self->m_month + 1, self->m_year);
}
void OsDate_getStringDateUS(const OsDate* self, char out[64])
{
	snprintf(out, 64, "%d/%d/%d", self->m_month + 1, self->m_day + 1, self->m_year);
}
void OsDate_getStringDateISO(const OsDate* self, char out[64])
{
	snprintf(out, 64, "%d-%d-%d", self->m_year, self->m_month + 1, self->m_day + 1);
}
void OsDate_getStringDateText(const OsDate* self, char out[64], const char* month)
{
	snprintf(out, 64, "%s %d, %d", month, self->m_day + 1, self->m_year);
}

void OsDate_getStringTime2(const OsDate* self, char out[64])
{
	snprintf(out, 64, "%02d:%02d", self->m_hour, self->m_min);
}
void OsDate_getStringTime3(const OsDate* self, char out[64])
{
	snprintf(out, 64, "%02d:%02d:%02d", self->m_hour, self->m_min, self->m_sec);
}

OsDate OsDate_initFromString(const UNI* str, OsDateTYPE type)
{
	OsDate self = OsDate_initEmpty();

	int numbers[6] = { 0, 0, 0, 0, 0, 0 };
	int n = 0;
	while (n < 6 && str && *(str = Std_goOverIgnore(str, _UNI32("0123456789"))))
	{
		numbers[n++] = Std_getNumberFromUNI(str);
		str = Std_goOver(str, _UNI32("0123456789"));
	}

	if (type == OsDate_EU)
	{
		self.m_day = Std_max(0, numbers[0] - 1);
		self.m_month = Std_max(0, numbers[1] - 1);
		self.m_year = numbers[2];
	}
	else
		if (type == OsDate_US)
		{
			self.m_month = Std_max(0, numbers[0] - 1);
			self.m_day = Std_max(0, numbers[1] - 1);
			self.m_year = numbers[2];
		}
		else
			if (type == OsDate_ISO)
			{
				self.m_year = numbers[0];
				self.m_month = Std_max(0, numbers[1] - 1);
				self.m_day = Std_max(0, numbers[2] - 1);
			}

	self.m_hour = numbers[3];
	self.m_min = numbers[4];
	self.m_sec = numbers[5];

	return self;
}

double OsDate_asNumber(const OsDate* self)
{
	return OsDate_getSecondsFromYearZero(self);
}
OsDate OsDate_initFromNumber(double number)
{
	OsDate self = OsDate_initEmpty();
	UBIG sec = Std_dclamp(number, 0, 200000000000);

	{
		UBIG days = sec / (24 * 60 * 60);
		sec %= (24 * 60 * 60);

		//year
		while (1)
		{
			UBIG d = OsDate_getDaysInYearEx(self.m_year);
			if (d > days)
				break;
			days -= d;
			self.m_year++;
		}

		//month
		while (1)
		{
			UBIG d = OsDate_getDaysInMonthEx(self.m_month, self.m_year);
			if (d > days)
				break;
			days -= d;
			self.m_month++;
		}

		//day
		self.m_day = days;
	}

	//hour
	self.m_hour = sec / (60 * 60);
	sec %= (60 * 60);

	//minutes
	self.m_min = sec / (60);
	sec %= (60);

	//seconds
	self.m_sec = sec;

	return self;
}
double OsDate_roundNumber(double number, OsDateTimeTYPE timeFormat)
{
	OsDate date = OsDate_initFromNumber(number);

	switch (timeFormat)
	{
		case OsDate_NONE:
		date.m_hour = 0;
		date.m_min = 0;
		date.m_sec = 0;
		break;

		case OsDate_HM:
		date.m_sec = 0;
		break;

		case OsDate_HMS:
		break;
	}

	return OsDate_asNumber(&date);
}
char* OsDate_getStringDateTime(const OsDate* self, OsDateTYPE formatDate, OsDateTimeTYPE formatTime, const char* month, char time[64])
{
	if (formatDate == OsDate_EU)OsDate_getStringDateEU(self, time);
	if (formatDate == OsDate_US)OsDate_getStringDateUS(self, time);
	if (formatDate == OsDate_ISO)OsDate_getStringDateISO(self, time);
	if (formatDate == OsDate_TEXT)OsDate_getStringDateText(self, time, month);

	if (formatTime > OsDate_NONE)strcat(time, "  ");	//space
	if (formatTime == OsDate_HM)OsDate_getStringTime2(self, time + Std_sizeCHAR(time));
	if (formatTime == OsDate_HMS)OsDate_getStringTime3(self, time + Std_sizeCHAR(time));

	return time;
}
UNI* OsDate_getStringDateTimeUNI(const OsDate* self, OsDateTYPE formatDate, OsDateTimeTYPE formatTime, const char* month)
{
	char time[64];
	OsDate_getStringDateTime(self, formatDate, formatTime, month, time);
	return Std_newUNI_char(time);
}

void LimitTime_reset(LimitTime* self)
{
	self->start = Os_time() - self->dt * 2;
}
LimitTime LimitTime_initDt(double dt_sec)
{
	LimitTime self;
	self.dt = dt_sec;
	LimitTime_reset(&self);	//first call of isTimeout() returns TRUE
	return self;
}
LimitTime LimitTime_initFps(double fps)
{
	return LimitTime_initDt(1.0 / fps);
}
double LimitTime_dt(const LimitTime* self)
{
	return Os_time() - self->start;
}
void LimitTime_start(LimitTime* self)
{
	self->start = Os_time();
}
BOOL LimitTime_isTimeout(LimitTime* self)
{
	double actTime = Os_time();

	BOOL timeout = ((actTime - self->start) >= self->dt);
	if (timeout)
		self->start = actTime;

	return timeout;
}

LimitFps LimitFps_init(void)
{
	LimitFps self;
	self.m_act_frame = 0;
	self.m_full_round = FALSE;
	return self;
}
void LimitFps_start(LimitFps* self)
{
	self->m_time_start = Os_time();
}
double LimitFps_finish(LimitFps* self, const int FPS)
{
	double dt = self->m_frames_dt[self->m_act_frame] = (Os_time() - self->m_time_start);
	self->m_act_frame++;
	if (self->m_act_frame == 32)
	{
		self->m_act_frame = 0;
		self->m_full_round = TRUE;
	}

	const double max_dt = 1.0f / FPS;
	if (dt < max_dt)
	{
		int sl = (int)((max_dt - dt) * 1000);// / 2;	//half
		if (sl)
			OsThread_sleep(sl);
	}

	return dt;
}
double LimitFps_getAvgDT(LimitFps* self)
{
	const int N = self->m_full_round ? 32 : self->m_act_frame;
	double sum = 0;
	int i;
	for (i = 0; i < N; i++)
		sum += self->m_frames_dt[i];
	return sum / N;
}
double LimitFps_getAvgFPS(LimitFps* self)
{
	double fps = 1.0f / LimitFps_getAvgDT(self);
	//printf("FPS(no sleep): %fframes/s\n", fps);	//sleep isn't counted in this
	return fps;
}

typedef struct OsTimeStat_s
{
	double value;
	double time;

	double value_checkPoint;
	double time_checkPoint;
}OsTimeStat;
OsTimeStat OsTimeStat_init(void)
{
	OsTimeStat self;
	self.value = self.value_checkPoint = 0;
	self.time = self.time_checkPoint = Os_time();
	return self;
}
void OsTimeStat_addValue(OsTimeStat* self, double add)
{
	self->value += add;
}
double OsTimeStat_checkPointAndGet(OsTimeStat* self)
{
	double time = Os_time();
	double valueDt = (self->value - self->value_checkPoint) / (time - self->time_checkPoint);

	//reset
	self->value_checkPoint = self->value;
	self->time_checkPoint = time;

	return valueDt;
}
double OsTimeStat_getAll(const OsTimeStat* self)
{
	return self->value / (Os_time() - self->time);
}
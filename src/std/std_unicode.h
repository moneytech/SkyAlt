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

UNI Std_getUNIupper(UNI ch)
{
	if (ch >= 'a' && ch <= 'z')
		ch = ch + ('A' - 'a');
	return ch;
}

UNI Std_getUNIsmall(UNI ch)
{
	if (ch >= 'A' && ch <= 'Z')
		ch = ch - ('A' - 'a');
	return ch;
}

char Std_getCHARsmall(char ch)
{
	if (ch >= 'A' && ch <= 'Z')
		ch = ch - ('A' - 'a');
	return ch;
}

UBIG Std_sizeUNI(const UNI* self)
{
	UBIG n = 0;
	while (self && *self)
		self++, n++;
	return n;
}

UBIG Std_sizeCHAR(const char* self)
{
	UBIG n = 0;
	while (self && *self)
		self++, n++;
	return n;
}

BOOL Std_isUNI(const UNI* self)
{
	return(self && *self); //at least one character
}

BOOL Std_isCHAR(const char* self)
{
	return(self && *self); //at least one character
}

UBIG Std_bytesUNI(const UNI* self)
{
	UBIG n = Std_sizeUNI(self);
	return n ? ((n + 1) * sizeof(UNI)) : 0;
}

BOOL Std_cmpUNI(const UNI* a, const UNI* b)
{
	if (a && b)
	{
		while (*a && *a == *b)
			a++, b++;
		return !*a && !*b;
	}
	return Std_sizeUNI(a) == Std_sizeUNI(b);
}

BOOL Std_cmpUNI_CHAR(const UNI* a, const char* b)
{
	if (a && b)
	{
		while (*a && *a == *b)
			a++, b++;
		return !*a && !*b;
	}
	return Std_sizeUNI(a) == Std_sizeCHAR(b);
}

BOOL Std_cmpUNI_CHAR_small(const UNI* a, const char* b)
{
	if (a && b)
	{
		while (*a && Std_getUNIsmall(*a) == Std_getUNIsmall(*b))
			a++, b++;
		return !*a && !*b;
	}
	return Std_sizeUNI(a) == Std_sizeCHAR(b);
}

BOOL Std_cmpUNI_ex(const UNI* a, const UNI* b, BIG bN)
{
	if (a && b)
	{
		while (*a && *a == *b && bN > 0)
		{
			bN--;
			a++, b++;
		}
		return !*a && (!*b || !bN);
	}
	return a == b;
}

BOOL Std_cmpUNIsmall(const UNI* a, const UNI* b)
{
	if (a && b)
	{
		while (*a && Std_getUNIsmall(*a) == Std_getUNIsmall(*b))
			a++, b++;
		return !*a && !*b;
	}
	return a == b;
}

int Std_cmpUNIascending(const UNI* a, const UNI* b)
{
	if (a && b)
	{
		while (*a && *b)
		{
			if (Std_getUNIsmall(*a) != Std_getUNIsmall(*b))
				return Std_getUNIsmall(*a) > Std_getUNIsmall(*b) ? 1 : -1;
			a++, b++;
		}
	}

	const BIG sA = Std_sizeUNI(a);
	const BIG sB = Std_sizeUNI(b);
	return (sA > sB) - (sA < sB);
}

int Std_cmpCHARascending(const char* a, const char* b)
{
	if (a && b)
	{
		while (*a && *b)
		{
			if (Std_getUNIsmall(*a) != Std_getUNIsmall(*b))
				return Std_getUNIsmall(*a) > Std_getUNIsmall(*b) ? 1 : -1;
			a++, b++;
		}
	}

	const BIG sA = Std_sizeCHAR(a);
	const BIG sB = Std_sizeCHAR(b);
	return (sA > sB) - (sA < sB);
}

BOOL Std_cmpCHAR(const char* a, const char* b)
{
	if (a && b)
	{
		while (*a && *a == *b)
			a++, b++;
		return !*a && !*b;
	}
	return a == b;
}

BOOL Std_cmpCHARsmall(const char* a, const char* b)
{
	if (a && b)
	{
		while (*a && Std_getUNIsmall(*a) == Std_getUNIsmall(*b))
			a++, b++;
		return !*a && !*b;
	}
	return a == b;
}

UNI* Std_newUNI_N(const UBIG N)
{
	return N ? (UNI*)Os_calloc(N + 1, sizeof(UNI)) : 0;
}

UNI* Std_newUNI_char(const char* src)
{
	UNI* self;
	UNI* dst = self = Std_newUNI_N(Std_sizeCHAR(src));
	while (src && *src)
	{
		*dst = *src;
		dst++;
		src++;
	}
	return self;
}

UNI* Std_newUNI_char_n(const char* src, UBIG N)
{
	N = Std_min(Std_sizeCHAR(src), N);
	UNI* dst = Std_newUNI_N(N);
	UBIG i;
	for (i = 0; i < N; i++)
		dst[i] = src[i];
	return dst;
}

UNI* Std_newNumber(double value)
{
	char str[64];
	if (value == (BIG)value)
		snprintf(str, sizeof(str), "%lld", (BIG)value);
	else
		snprintf(str, sizeof(str), "%0.8f", value);

	return Std_newUNI_char(str);
}

UNI* Std_newNumberPrecision(double value, UINT precision)
{
	char prec[16];
	snprintf(prec, sizeof(prec), "%%0.%df", precision);

	char str[64];
	snprintf(str, sizeof(str), prec, value);

	return Std_newUNI_char(str);
}

void Std_setHEX_char(char* self, const UCHAR* arr, const int N)
{
	int i;
	for (i = 0; i < N; i++)
		snprintf(&self[i * 2], 3, "%02x", arr[i]);
	self[N * 2] = 0; //end
}
void Std_setHEX_uni(UNI* self, const UCHAR* arr, const int N)
{
	int i;
	for (i = 0; i < N; i++)
	{
		char ch[3];
		snprintf(ch, 3, "%02x", arr[i]);
		self[i * 2 + 0] = ch[0];
		self[i * 2 + 1] = ch[1];
	}
	self[N * 2] = 0; //end
}

void Std_getFromHEX(const char* str, const int N, UCHAR* out)
{
	int i;
	for (i = 0; i < N / 2; i++)
		sscanf(&str[i * 2], "%02hhX", &out[i]);
}

void Std_getFromHEX_uni(const UNI* str, const int N, UCHAR* out)
{
	int i;
	for (i = 0; i < N / 2; i++)
	{
		char ch[3];
		ch[0] = str[i * 2 + 0];
		ch[1] = str[i * 2 + 1];
		ch[2] = 0;
		sscanf(ch, "%02hhX", &out[i]);
	}
}

UNI* Std_newUNI(const UNI* src)
{
	UNI* dst = 0;
	if (src)
	{
		const UBIG N = Std_sizeUNI(src);
		if (N)
		{
			const UBIG bytes = (N + 1) * sizeof(UNI);
			dst = (UNI*)Os_malloc(bytes);
			Os_memcpy(dst, src, bytes);
		}
	}
	return dst;
}

UNI* Std_newUNI_copy(const UNI* src, UBIG N)
{
	N = Std_min(Std_sizeUNI(src), N);
	UNI* dst = Std_newUNI_N(N);
	Os_memcpy(dst, src, N * sizeof(UNI));
	return dst;
}

UNI* Std_newUNI_copyEx(const UNI* src, UBIG N, UNI exclude)
{
	UNI* ret = Std_newUNI_N(N);
	UNI* dst = ret;

	int i = 0;
	while (src && src[i] && i < N)
	{
		if (src[i] != exclude)
		{
			*dst = src[i];
			dst++;
		}
		i++;
	}
	return ret;
}

char* Std_newCHAR_copyEx(const char* src, UBIG N, UNI exclude)
{
	char* ret = Std_newCHAR_N(N);
	char* dst = ret;

	int i = 0;
	while (src && src[i] && i < N)
	{
		if (src[i] != exclude)
		{
			*dst = src[i];
			dst++;
		}
		i++;
	}
	return ret;
}

void Std_deleteCHAR(char* self)
{
	Os_free(self, Std_sizeCHAR(self) * sizeof(char));
}

void Std_deleteUNI(UNI* self)
{
	Os_free(self, Std_sizeUNI(self) * sizeof(UNI));
}


void Std_replaceCHAR(char** dst, const char* src)
{
	UBIG n = Std_sizeCHAR(*dst);

	if (n == Std_sizeCHAR(src))
		Os_memcpy(*dst, src, n * sizeof(char));
	else
	{
		Std_deleteCHAR(*dst);
		*dst = Std_newCHAR(src);
	}
}

void Std_replaceUNI(UNI** dst, const UNI* src)
{
	UBIG n = Std_sizeUNI(*dst);

	if (n == Std_sizeUNI(src))
		Os_memcpy(*dst, src, n * sizeof(UNI));
	else
	{
		Std_deleteUNI(*dst);
		*dst = Std_newUNI(src);
	}
}

void Std_replaceInsideUNI(UNI** dst, const UNI* find, const UNI* replace)
{
	UBIG findN = Std_sizeUNI(find);
	UBIG replaceN = Std_sizeUNI(replace);

	BIG cp;
	BIG p = 0;
	while ((cp = Std_subUNI((*dst) + p, find)) >= 0)
	{
		p += cp;
		*dst = Std_removeChars(*dst, p, findN);
		*dst = Std_insertUNI(*dst, replace, p);

		p += replaceN;
	}
}


void Std_replaceCharacters(UNI* self, UNI ch)
{
	while (self && *self)
	{
		*self = ch;
		self++;
	}
}

BIG Std_findUNI(const UNI* self, UNI ch)
{
	UBIG i = 0;
	while (self && *self)
	{
		if (*self == ch)
			return i;
		i++;
		self++;
	}
	return (ch == 0) ? i : -1;
}


BIG Std_findCHAR(const char* self, char ch)
{
	UBIG i = 0;
	while (self && *self)
	{
		if (*self == ch)
			return i;
		i++;
		self++;
	}
	return (ch == 0) ? i : -1;
}

BIG Std_findUNIex(const UNI* self, UNI ch, UNI exclude)
{
	UBIG i = 0;
	while (self && *self)
	{
		if (*self == ch && (i == 0 || *(self - 1) != exclude))
			return i;
		i++;
		self++;
	}
	return(ch == 0) ? i : -1;
}

BIG Std_findCHARex(const char* self, UNI ch, UNI exclude)
{
	UBIG i = 0;
	while (self && *self)
	{
		if (*self == ch && (i == 0 || *(self - 1) != exclude))
			return i;
		i++;
		self++;
	}
	return(ch == 0) ? i : -1;
}

BIG Std_findUNI_last(const UNI* self, UNI ch)
{
	UBIG last_pos = -1;
	UBIG i = 0;
	while (self && self[i])
	{
		if (self[i] == ch)
			last_pos = i;
		i++;
	}
	return last_pos;
}

void Std_print(UNI cp)
{
	if (cp < 0x80)
	{
		printf("%c", cp);
	}
	else if (cp < 0x800)
	{
		printf("%c%c", 0xC0 + cp / 0x40, 0x80 + cp % 0x40);
	}
}

void Std_printUNI(const UNI* self)
{
	while (self && *self)
	{
		Std_print(*self);
		self++;
	}
}

void Std_printlnUNI(const UNI* self)
{
	while (self && *self)
	{
		Std_print(*self);
		self++;
	}
	printf("\n");
}

void Std_printlnUNI_sub(const UNI* self, const UNI* end)
{
	while (self && *self && self < end)
	{
		Std_print(*self);
		self++;
	}
	printf("\n");
}

UNI* Std_addUNI(const UNI* a, const UNI* b)
{
	UBIG nA = Std_sizeUNI(a);
	UBIG nB = Std_sizeUNI(b);
	UNI* dst = (UNI*)Os_malloc((nA + nB + 1) * sizeof(UNI));

	Os_memcpy(dst, a, nA * sizeof(UNI));
	Os_memcpy(dst + nA, b, nB * sizeof(UNI));
	dst[nA + nB] = 0;

	return dst;
}

UNI* Std_addAfterUNI(UNI* dst, const UNI* src)
{
	UBIG nDst = Std_sizeUNI(dst);
	UBIG nSrc = Std_sizeUNI(src);

	dst = (UNI*)Os_realloc(dst, (nDst + nSrc + 1) * sizeof(UNI));

	Os_memcpy(&dst[nDst], src, nSrc * sizeof(UNI));
	dst[nDst + nSrc] = 0;

	return dst;
}

UNI* Std_addAfterUNI_char(UNI* dst, const char* src)
{
	UBIG nDst = Std_sizeUNI(dst);
	UBIG nSrc = Std_sizeCHAR(src);

	dst = (UNI*)Os_realloc(dst, (nDst + nSrc + 1) * sizeof(UNI));

	BIG i;
	for (i = 0; i < nSrc; i++)
		dst[nDst + i] = src[i];
	dst[nDst + nSrc] = 0;

	return dst;
}

UNI* Std_insertChar(UNI* dst, const UNI ch, UBIG pos)
{
	UBIG len = Std_sizeUNI(dst);
	dst = (UNI*)Os_realloc(dst, (len + 2) * sizeof(UNI));
	pos = Std_bmin(pos, len);
	Os_memmove(dst + pos + 1, dst + pos, (len - pos) * sizeof(UNI));

	dst[pos] = ch;
	dst[len + 1] = 0;

	return dst;
}

UNI* Std_insertUNI(UNI* dst, const UNI* src, UBIG pos)
{
	UBIG dst_len = Std_sizeUNI(dst);
	UBIG src_len = Std_sizeUNI(src);
	dst = (UNI*)Os_realloc(dst, (dst_len + src_len + 1) * sizeof(UNI));
	pos = Std_bmin(pos, dst_len);

	Os_memmove(dst + pos + src_len, dst + pos, (dst_len - pos) * sizeof(UNI));

	Os_memcpy(&dst[pos], src, src_len * sizeof(UNI));
	dst[dst_len + src_len] = 0;

	return dst;
}

UNI* Std_removeUNI(UNI* dst, int pos)
{
	UBIG len = Std_sizeUNI(dst);
	if (pos < len)
	{
		Os_memmove(dst + pos, dst + pos + 1, (len - pos) * sizeof(UNI));
		if (len)
			dst[Std_max(0, len - 1)] = 0;
	}
	return dst;
}

UNI* Std_removeChars(UNI* dst, int pos, int num)
{
	if (dst)
	{
		UBIG len = Std_sizeUNI(dst);
		if (pos + num < len)
			Os_memmove(dst + pos, dst + pos + num, (len - (pos + num)) * sizeof(UNI));
		if (len)
			dst[Std_max(0, len - num)] = 0;
	}

	return dst;
}

BOOL Std_startWithCHAR(const char* self, const char* find)
{
	if (self && find)
	{
		while (*self && *self == *find)
			self++, find++;
		return *find == 0;
	}
	return self == find;
}

BOOL Std_startWith(const UNI* self, const UNI* find)
{
	if (self && find)
	{
		while (*self && *self == *find)
			self++, find++;
		return *find == 0;
	}
	return self == find;
}

BOOL Std_startWith_small(const UNI* self, const UNI* find)
{
	if (self && find)
	{
		while (*self && Std_getUNIsmall(*self) == Std_getUNIsmall(*find))
			self++, find++;
		return *find == 0;
	}
	return self == find;
}

BOOL Std_startWith_small_char(const UNI* self, const char* find)
{
	if (self && find)
	{
		while (*self && Std_getUNIsmall(*self) == Std_getCHARsmall(*find))
			self++, find++;
		return *find == 0;
	}
	return (void*)self == (void*)find;
}

BIG Std_subUNI(const UNI* self, const UNI* find)
{
	BIG pos = 0;
	while (self && *self)
	{
		if (Std_startWith(self, find))
			return pos;
		self++;
		pos++;
	}
	return self == find ? 0 : -1;
}

BIG Std_subCHAR(const char* self, const char* find)
{
	BIG pos = 0;
	while (self && *self)
	{
		if (Std_startWithCHAR(self, find))
			return pos;
		self++;
		pos++;
	}
	return self == find ? 0 : -1;
}


BIG Std_subUNI_small(const UNI* self, const UNI* find)
{
	BIG pos = 0;
	while (self && *self)
	{
		if (Std_startWith_small(self, find))
			return pos;
		self++;
		pos++;
	}
	return self == find ? 0 : -1;
}

BIG Std_subUNI_small_char(const UNI* self, const char* find)
{
	BIG pos = 0;
	while (self && *self)
	{
		if (Std_startWith_small_char(self, find))
			return pos;
		self++;
		pos++;
	}
	return ((void*)self == (void*)find) ? 0 : -1;
}

UNI* Std_copyUNI(UNI* dst, UBIG dstMax_N, const UNI* src)
{
	BIG pos = 0;
	while (src && src[pos] && pos < dstMax_N)
	{
		dst[pos] = src[pos];
		pos++;
	}
	if (dst)
		dst[pos] = 0;
	return dst;
}
UNI* Std_copyUNI_char(UNI* dst, UBIG dstMax_N, const char* src)
{
	BIG pos = 0;
	while (src && src[pos] && pos < dstMax_N)
	{
		dst[pos] = src[pos];
		pos++;
	}
	if (dst)
		dst[pos] = 0;

	return dst ? &dst[pos] : dst;
}

char* Std_copyCHAR_uni(char* dst, UBIG dstMax_N, const UNI* src)
{
	dstMax_N--;	//end

	BIG pos = 0;
	while (src && src[pos] && pos < dstMax_N)
	{
		dst[pos] = src[pos];
		pos++;
	}
	dst[pos] = 0;
	return dst;
}
char* Std_copyCHAR(char* dst, UBIG dstMax_N, const char* src)
{
	dstMax_N--;	//end

	BIG pos = 0;
	while (src && src[pos] && pos < dstMax_N)
	{
		dst[pos] = src[pos];
		pos++;
	}
	dst[pos] = 0;
	return dst;
}

void Std_convertUpUNI(UNI* self)
{
	while (self && *self)
	{
		*self = Std_getUNIupper(*self);
		self++;
	}
}

const char* Std_findSubCHAR(const char* self, const char* find)
{
	while (self && *self)
	{
		if (Std_startWithCHAR(self, find))
			return self;
		self++;
	}
	return 0;
}

BOOL Std_isDigit(char ch)
{
	return ch == '.' || ch == ',' || ch == '+' || ch == '-' || (ch >= '0' && ch <= '9');
}
BOOL Std_isNotDigit(char ch)
{
	return !Std_isDigit(ch);
}

const UNI* Std_goOver(const UNI* str, const UNI* find)
{
	while (str && *str && Std_findUNI(find, *str) >= 0)
		str++;
	return str;
}

const UNI* Std_goOverIgnore(const UNI* str, const UNI* ignore)
{
	while (str && *str && Std_findUNI(ignore, *str) < 0)
		str++;
	return str;
}

UBIG Std_countDigitsInRowCHAR(const char* str)
{
	UBIG count = 0;
	while (str && *str >= '0' && *str <= '9')
	{
		str++;
		count++;
	}
	return count;
}

UBIG Std_countDigitsInRowUNI(const UNI* str)
{
	UBIG count = 0;
	while (str && *str >= '0' && *str <= '9')
	{
		str++;
		count++;
	}
	return count;
}

UBIG Std_countDigitsAndAlphabetInRow(const UNI* str, BOOL find)
{
	UBIG count = 0;
	while (str && find == ((*str >= '0' && *str <= '9') || (*str >= 'a' && *str <= 'z') || (*str >= 'A' && *str <= 'Z')))
	{
		str++;
		count++;
	}

	return count;
}

double Std_getNumberFromUNI_n(const UNI* str, BIG max_n)
{
	max_n = Std_clamp(max_n, 0, 64);

	char asc[64 + 1];

	BIG i = 0;
	while (str && *str && i < max_n)
		asc[i++] = *(str++);
	asc[i] = 0;

	//replace , with .
	i = 0;
	while (asc[i])
	{
		if (asc[i] == ',')
			asc[i] = '.';
		i++;
	}

	//remove spaces
	i = 0;
	BIG p = 0;
	while (asc[i])
	{
		if (asc[i] != ' ')
			asc[p++] = asc[i];
		i++;
	}
	asc[p] = 0;

	return Os_atof(asc);
}

double Std_getNumberFromUNI(const UNI* str)
{
	return Std_getNumberFromUNI_n(str, 64);
}

UBIG Std_separNumItemsCHAR(const char* self, const UNI separator)
{
	BIG num = 0;
	while (self && *self)
	{
		BIG m = Std_findCHARex(self, separator, 0);
		if (m < 0)
			break;

		self += (m + 1);
		num++;
	}

	return self ? (num + 1) : 0;
}

char* Std_separGetItemCHAR(const char* self, UBIG value, const UNI separator)
{
	BIG m = -1;
	while (value >= 0 && self && *self)
	{
		m = Std_findCHARex(self, separator, 0);
		if (m < 0 || value == 0)
			break;
		self += (m + 1);
		value--;
	}
	return(m == -1) ? Std_newCHAR(self) : Std_newCHAR_copyEx(self, m, 0);
}

UBIG Std_separNumItemsUNI(const UNI* self, const UNI separator)
{
	BIG num = 0;
	while (self && *self)
	{
		BIG m = Std_findUNIex(self, separator, '\\');
		if (m < 0)
			break;

		self += (m + 1);
		num++;
	}

	return self ? (num + 1) : 0;
}

UNI* Std_separGetItemUNI(const UNI* self, UBIG value, const UNI separator)
{
	BIG m = -1;
	while (value >= 0 && self && *self)
	{
		m = Std_findUNIex(self, separator, '\\');
		if (m < 0 || value == 0)
			break;
		self += (m + 1);
		value--;
	}
	return(m == -1) ? Std_newUNI(self) : Std_newUNI_copyEx(self, m, '\\');
}

BOOL Std_SeparGetItemUNI_cmp(const UNI* self, UBIG value, const UNI separator, const UNI* src)
{
	BIG m = -1;
	while (value >= 0 && self && *self)
	{
		m = Std_findUNIex(self, separator, '\\');
		if (m < 0 || value == 0)
			break;
		self += (m + 1);
		value--;
	}
	return(m == -1) ? Std_cmpUNI(self, src) : Std_cmpUNI_ex(src, self, m);
}

BIG Std_separFind(const UNI* self, const UNI* separators)
{
	BIG i = 0;
	while (self)// && *self)
	{
		if (Std_findUNI(separators, *self) >= 0 || *self == 0)
			return i;
		i++;
		self++;
	}

	return -1;
}

void Std_removeLetterUNI(UNI* self, UNI ch)
{
	UNI* curr = self;
	while (self && *self)
	{
		if (*self != ch)
		{
			*curr = *self;
			curr++;
		}
		self++;
	}
	if (curr)
		*curr = 0;
}

void Std_removeLetterCHAR(char* self, char ch)
{
	char* curr = self;
	while (self && *self)
	{
		if (*self != ch)
		{
			*curr = *self;
			curr++;
		}
		self++;
	}
	if (curr)
		*curr = 0;
}

void Std_array_print(UCHAR* data, UBIG data_size)
{
	printf("{");
	UBIG i;
	for (i = 0; i < data_size; i++)
	{
		printf("%d", data[i]);
		if (i + 1 < data_size)
			printf(",");
	}
	printf("};\n");
}

void Std_array_printDouble(double* data, UBIG data_size)
{
	printf("{");
	UBIG i;
	for (i = 0; i < data_size; i++)
	{
		printf("%f", data[i]);
		if (i + 1 < data_size)
			printf(",");
	}
	printf("};\n");
}

char* Std_newCHAR_N(const UBIG N)
{
	return N ? (char*)Os_calloc(N + 1, sizeof(char)) : 0;
}

char* Std_newCHAR(const char* src)
{
	char* dst = 0;
	if (src)
	{
		const UBIG N = Std_sizeCHAR(src);
		if (N)
		{
			const UBIG bytes = (N + 1);
			dst = (char*)Os_malloc(bytes);
			Os_memcpy(dst, src, bytes);
		}
	}
	return dst;
}

char* Std_newCHAR_uni(const UNI* src)
{
	char* self = 0;
	if (src)
	{
		const UBIG N = Std_sizeUNI(src);
		if (N)
		{
			const UBIG bytes = (N + 1);
			char* dst = self = (char*)Os_malloc(bytes);
			while (*src)
			{
				*dst = *src;
				dst++;
				src++;
			}
			*dst = 0;
		}
	}
	return self;
}

char* Std_newCHAR_uni_n(const UNI* src, const UBIG N)
{
	char* dst = Std_newCHAR_N(Std_min(Std_sizeUNI(src), N));
	UBIG i;
	for (i = 0; i < N; i++)
		dst[i] = src[i];
	return dst;
}

char* Std_newCHAR_n(const char* src, const UBIG N)
{
	char* dst = Std_newCHAR_N(Std_min(Std_sizeCHAR(src), N));
	UBIG i;
	for (i = 0; i < N; i++)
		dst[i] = src[i];
	return dst;
}

void Std_rewriteCHAR(char** dst, const char* src)
{
	const UBIG N = Std_sizeCHAR(src);
	*dst = (char*)Os_realloc(*dst, N + 1);
	Os_memcpy(*dst, src, N + 1);
}

char* Std_buildNumber(double value, int precision, char str[64])
{
	if (precision <= 0 && value == Std_roundDown(value))
		snprintf(str, 64, "%lld", (BIG)value);	//snprintf
	else
	{
		char prec[16];
		if (precision >= 0)
			snprintf(prec, sizeof(prec), "%%0.%df", precision);
		else
			Os_memcpy(prec, "%f", 3);
		snprintf(str, 64, prec, value);
	}
	return str;
}

UNI* Std_buildNumberUNI(double value, int precision, UNI out[64])
{
	char str[64];
	Std_buildNumber(value, precision, str);

	UBIG p = 0;
	while (str[p] && p < 64)
	{
		out[p] = str[p];
		p++;
	}
	out[p] = 0;

	return out;
}

void Std_separNumberThousands(UNI* self, char separ)
{
	const int digits = Std_max(3, Std_countDigitsInRowUNI(self));
	const int N = (digits - 1) / 3;

	UBIG selfN = Std_sizeUNI(self);

	int pos = digits % 3 ? digits % 3 : 3;
	int i;
	for (i = 0; i < N; i++)
	{
		Os_memmove(self + pos + 1, self + pos, (selfN - pos) * sizeof(UNI));	//space
		self[pos] = separ;	//write separ
		self[++selfN] = 0;	//write end

		pos += 4;
	}
}

void Std_setCHAR_uni(char* dst, const UNI* src, const int dst_max)
{
	int i = 0;
	while (i < (dst_max - 1) && src && *src)
	{
		dst[i++] = *src;
		src++;
	}
	dst[i] = 0;
}

void Std_setUNI_char(UNI* dst, const char* src, const int dst_max)
{
	int i = 0;
	while (i < (dst_max - 1) && src && *src)
	{
		dst[i++] = *src;
		src++;
	}
	dst[i] = 0;
}

char* Std_addCHAR(const char* a, const char* b)
{
	UBIG nA = Std_sizeCHAR(a);
	UBIG nB = Std_sizeCHAR(b);
	char* dst = (char*)Os_malloc((nA + nB + 1));

	Os_memcpy(dst, a, nA);
	Os_memcpy(dst + nA, b, nB);
	dst[nA + nB] = 0;

	return dst;
}

char* Std_addAfterCHAR(char* dst, const char* src)
{
	UBIG nDst = Std_sizeCHAR(dst);
	UBIG nSrc = Std_sizeCHAR(src);

	if (src)
	{
		dst = (char*)Os_realloc(dst, (nDst + nSrc + 1));

		Os_memcpy(&dst[nDst], src, nSrc);
		dst[nDst + nSrc] = 0;
	}

	return dst;
}

char* Std_addAfterCHAR_uni(char* dst, const UNI* src)
{
	UBIG nDst = Std_sizeCHAR(dst);
	UBIG nSrc = Std_sizeUNI(src);

	dst = (char*)Os_realloc(dst, (nDst + nSrc + 1) * sizeof(char));

	BIG i;
	for (i = 0; i < nSrc; i++)
		dst[nDst + i] = src[i];
	dst[nDst + nSrc] = 0;

	return dst;
}

UNI* Std_newNumberSize(UBIG size)
{
	UBIG mx = 1024ULL * 1024ULL * 1024ULL * 1024ULL;

	char out[64];
	if (size > mx) snprintf(out, sizeof(out), "%.1fTB", ((double)size) / mx);

	if (size > 1024 * 1024 * 1024) snprintf(out, sizeof(out), "%.1fGB", ((double)size) / 1024 / 1024 / 1024);
	else
		if (size > 1024 * 1024) snprintf(out, sizeof(out), "%.1fMB", ((double)size) / 1024 / 1024);
		else
			if (size > 1024) snprintf(out, sizeof(out), "%.1fKB", ((double)size) / 1024);
			else
				snprintf(out, sizeof(out), "%lldB", size);

	return Std_newUNI_char(out);
}

UNI* Std_repeatUNI(const int N, const UNI ch)
{
	UNI* text = Std_newUNI_N(N);
	int i;
	for (i = 0; i < N; i++)
		text[i] = ch;
	return text;
}

int Std_encode_utf8(UNI in, UCHAR* out)
{
	if (in < 0x80)
	{
		if (out)
			*out++ = in;
		return 1;
	}
	else if (in < 0x800)
	{
		if (out)
		{
			*out++ = (in >> 6) + 0xC0;
			*out++ = (in & 0x3F) + 0x80;
		}
		return 2;
	}
	else if (in < 0x10000)
	{
		if (out)
		{
			*out++ = (in >> 12) + 0xE0;
			*out++ = ((in >> 6) & 0x3F) + 0x80;
			*out++ = (in & 0x3F) + 0x80;
		}
		return 3;
	}
	else if (in < 0x110000)
	{
		if (out)
		{
			*out++ = (in >> 18) + 0xF0;
			*out++ = ((in >> 12) & 0x3F) + 0x80;
			*out++ = ((in >> 6) & 0x3F) + 0x80;
			*out++ = (in & 0x3F) + 0x80;
		}
		return 4;
	}

	return -1;
}
char* Std_utf32_to_utf8(const UNI* in)
{
	BIG bytes = Std_sizeUNI(in) * 4;
	char* origOut = Std_newCHAR_N(bytes);
	UCHAR* out = (UCHAR*)origOut;

	BIG rc = 0;
	int units;
	UCHAR encoded[4];
	while (*in > 0)
	{
		units = Std_encode_utf8(*in, encoded);
		if (units == -1)
		{
			Os_free(out, bytes * sizeof(char));
			out = 0;
			break;
		}

		if (rc + units <= bytes)
		{
			*out++ = encoded[0];
			if (units > 1)
				*out++ = encoded[1];
			if (units > 2)
				*out++ = encoded[2];
			if (units > 3)
				*out++ = encoded[3];
		}

		const UBIG SSIZE_MAX = 0xffffffffffffffff;
		if (SSIZE_MAX - units >= rc)
			rc += units;
		else
		{
			Os_free(out, bytes * sizeof(char));
			out = 0;
			break;
		}
		in++;
	}

	return origOut;
}

int Std_ishex(int x)
{
	return(x >= '0' && x <= '9') ||
		(x >= 'a' && x <= 'f') ||
		(x >= 'A' && x <= 'F');
}

BIG Std_urlDecode(const char* s, char* dec)
{
	char* o;
	const char* end = s + Std_sizeCHAR(s);
	int c;

	for (o = dec; s <= end; o++)
	{
		c = *s++;
		if (c == '+') c = ' ';
		else if (c == '%' && (!Std_ishex(*s++) || !Std_ishex(*s++) || !sscanf(s - 2, "%2x", &c)))
			return -1;

		if (dec) *o = c;
	}

	return o - dec;
}

char* Std_newCHAR_urlDecode(const char* url)
{
	char* self = (char*)Os_malloc(Std_sizeCHAR(url) + 1);
	Std_urlDecode(url, self);
	return self;
}

static int _Std_getRfc3986(int i)
{
	return Os_isalnum(i) || i == '~' || i == '-' || i == '.' || i == '_' ? i : 0;
}
/*static int _Std_getHTML5(int i)
{
	return Os_isalnum(i) || i == '*' || i == '-' || i == '.' || i == '_' ? i : (i == ' ') ? '+' : 0;
}*/
void Std_urlEncode(const UNI* s, char* out)
{
	while (s && *s)
	{
		int t = (*s >= 0 && *s < 256) ? _Std_getRfc3986(*s) : 0;
		//int t = (*s < 256) ? _Std_getHTML5(*s) : 0;
		if (t)
		{
			sprintf(out, "%c", t);
			out++;
		}
		else
		{
			UCHAR encoded[4];
			int units = Std_encode_utf8(*s, encoded);

			int i;
			for (i = 0; i < units; i++)
			{
				sprintf(out, "%%%02X", encoded[i]);
				while (*++out);
			}
		}

		s++;
	}
}

char* Std_newCHAR_urlEncode(const UNI* url)
{
	char* self = (char*)Os_malloc(Std_sizeUNI(url) * 3 + 1);
	Std_urlEncode(url, self);
	return self;
}

StdString StdString_init()
{
	StdString self;
	self.str = 0;
	self.n = 0;
	return self;
}
StdString StdString_initCopy(const StdString* src)
{
	StdString self;
	self.str = Std_newUNI(src->str);
	self.n = Std_sizeUNI(src->str);
	return self;
}

void StdString_freeIgnore(StdString* self)
{
	Os_memset(self, sizeof(StdString));
}
void StdString_free(StdString* self)
{
	Std_deleteUNI(self->str);
	StdString_freeIgnore(self);
}
UBIG StdString_size(const StdString* self)
{
	return Std_sizeUNI(self->str);
}
void StdString_empty(StdString* self)
{
	if (self->n)
		self->str[0] = 0;
}

void StdString_resize(StdString* self, UBIG N)
{
	if (self->n != N)
	{
		self->n = N;
		self->str = Os_realloc(self->str, (self->n + 1) * sizeof(UNI));
		self->str[self->n] = 0;
	}
}

void StdString_fit(StdString* self)
{
	self->n = Std_sizeUNI(self->str);
	StdString_resize(self, self->n);
}

static void _StdString_setUNI_pos(StdString* self, const UNI* str, UBIG start, const UBIG strN)
{
	if (start + strN >= self->n)
		StdString_resize(self, start + strN);

	Std_copyUNI(&self->str[start], strN, str);

	if (self->n)
		self->str[start + strN] = 0;
}

static void _StdString_setCHAR_pos(StdString* self, const char* str, UBIG start, const UBIG strN)
{
	if (start + strN >= self->n)
		StdString_resize(self, start + strN);

	Std_copyUNI_char(&self->str[start], strN, str);

	if (self->n)
		self->str[start + strN] = 0;
}

void StdString_setUNI(StdString* self, const UNI* str)
{
	_StdString_setUNI_pos(self, str, 0, Std_sizeUNI(str));
}
void StdString_setUNI_n(StdString* self, const UNI* str, UBIG n)
{
	_StdString_setUNI_pos(self, str, 0, n);
}
void StdString_addUNI(StdString* self, const UNI* str)
{
	_StdString_setUNI_pos(self, str, Std_sizeUNI(self->str), Std_sizeUNI(str));
}

void StdString_setCHAR(StdString* self, const char* str)
{
	_StdString_setCHAR_pos(self, str, 0, Std_sizeCHAR(str));
}
void StdString_setCHAR_n(StdString* self, const char* str, UBIG n)
{
	_StdString_setCHAR_pos(self, str, 0, Std_sizeCHAR(str));
}
void StdString_addCHAR(StdString* self, const char* str)
{
	_StdString_setCHAR_pos(self, str, Std_sizeUNI(self->str), Std_sizeCHAR(str));
}

void StdString_addNumber(StdString* self, int precision, double value)
{
	UNI nmbr[64];
	Std_buildNumberUNI(value, precision, nmbr);
	StdString_addUNI(self, nmbr);
}

BOOL StdString_cmp(const StdString* a, const StdString* b)
{
	return Std_cmpUNI(a->str, b->str);
}

void StdString_print(StdString* self)
{
	Std_printlnUNI(self->str);
}

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

StdBigs StdBigs_init(void)
{
	StdBigs self;
	self.ptrs = 0;
	self.num = 0;
	self.alloc = 0;
	return self;
}

void StdBigs_resize(StdBigs* self, UBIG num);

StdBigs StdBigs_initCopy(const StdBigs* src)
{
	StdBigs self = StdBigs_init();

	StdBigs_resize(&self, src->num);
	Os_memcpy(self.ptrs, src->ptrs, src->num * sizeof(BIG));

	return self;
}

void StdBigs_free(StdBigs* self)
{
	Os_free(self->ptrs, self->alloc * sizeof(BIG));
	Os_memset(self, sizeof(StdBigs));
}

void StdBigs_setAlloc(StdBigs* self, const UBIG n)
{
	self->num = self->alloc = n;
}

void StdBigs_clear(StdBigs* self)
{
	self->num = 0;
}

void StdBigs_resize(StdBigs* self, UBIG num)
{
	if (num > self->alloc)
	{
		self->ptrs = (BIG*)Os_realloc(self->ptrs, num * sizeof(BIG));
		Os_memset(&self->ptrs[self->alloc], (num - self->alloc) * sizeof(BIG));
		self->alloc = num;
	}
	self->num = num;
}
BIG StdBigs_add(StdBigs* self, BIG value)
{
	StdBigs_resize(self, self->num + 1);
	self->ptrs[self->num - 1] = value;
	return value;
}
void StdBigs_remove(StdBigs* self, UBIG i)
{
	Os_memmove(&self->ptrs[i], &self->ptrs[i + 1], (self->num - i - 1) * sizeof(BIG));
	self->num--;
}

void StdBigs_insert(StdBigs* self, UBIG i, BIG value)
{
	UBIG oldN = self->num;
	StdBigs_resize(self, oldN + 1);

	if (i < oldN)
		Os_memmove(&self->ptrs[i + 1], &self->ptrs[i], (oldN - i) * sizeof(BIG));

	self->ptrs[i] = value;
}

UBIG StdBigs_insertShort(StdBigs* self, BIG value)
{
	UBIG i;
	for (i = 0; i < self->num; i++)
	{
		if (self->ptrs[i] > value)
		{
			StdBigs_insert(self, i, value);
			return i;
		}
	}

	StdBigs_add(self, value);
	return self->num - 1;
}

BIG StdBigs_find(const StdBigs* self, const BIG value)
{
	UBIG i;
	for (i = 0; i < self->num; i++)
		if (self->ptrs[i] == value)
			return i;
	return -1;
}
BOOL StdBigs_removeFind(StdBigs* self, const BIG value)
{
	BIG i = StdBigs_find(self, value);
	BOOL ok = (i >= 0);
	if (ok)
		StdBigs_remove(self, i);
	return ok;
}

void StdBigs_swap(StdBigs* self, UBIG iA, BIG iB)
{
	BIG backup = self->ptrs[iA];
	self->ptrs[iA] = self->ptrs[iB];
	self->ptrs[iB] = backup;
}

BIG StdBigs_get(StdBigs* self, BIG i)
{
	return (i >= 0 && i < self->num) ? self->ptrs[i] : 0;
}
BIG StdBigs_getNeg(StdBigs* self, BIG i)
{
	return (i >= 0 && i < self->num) ? self->ptrs[i] : -1;
}

BIG StdBigs_last(StdBigs* self)
{
	return StdBigs_get(self, self->num - 1);
}

void StdBigs_addArr(StdBigs* self, StdBigs* src)
{
	UBIG start = self->num;
	StdBigs_resize(self, self->num + src->num);
	Os_memcpy(&self->ptrs[start], &src->ptrs[0], src->num * sizeof(UBIG));
}

void StdBigs_reversEx(StdBigs* self, const UBIG start, const UBIG end)
{
	UBIG i;
	for (i = start; i < end / 2; i++)
		StdBigs_swap(self, i, end - 1 - i);
}

void StdBigs_revers(StdBigs* self)
{
	StdBigs_reversEx(self, 0, self->num);
}

void StdBigs_println(const StdBigs* self)
{
	UBIG i;
	for (i = 0; i < self->num; i++)
		printf("%lld\n", self->ptrs[i]);
}

void StdBigs_rotate(StdBigs* self, BIG start, BIG end)
{
	BIG tmp = self->ptrs[end];
	Os_memmove(&self->ptrs[start + 1], &self->ptrs[start], sizeof(BIG) * (end - start));
	self->ptrs[start] = tmp;
}

int _StdBigs_cmp(const void* context, const void* a, const void* b)
{
	return (*(BIG*)a > * (BIG*)b) - (*(BIG*)a < *(BIG*)b);
}
void StdBigs_qshortEx(StdBigs* self, const BIG start, const BIG end, const BOOL ascending)
{
	Os_qsort(&self->ptrs[start], end - start, sizeof(BIG), _StdBigs_cmp, 0);
	if (!ascending)
		StdBigs_reversEx(self, start, end);
}
void StdBigs_qshort(StdBigs* self, const BOOL ascending)
{
	StdBigs_qshortEx(self, 0, self->num, ascending);
}

void StdBigs_copy(StdBigs* dst, const StdBigs* src)
{
	StdBigs_resize(dst, src->num);
	Os_memcpy(dst->ptrs, src->ptrs, src->num * sizeof(BIG));
}

void StdBigs_setAll(StdBigs* self, BIG value)
{
	UBIG i;
	for (i = 0; i < self->num; i++)
		self->ptrs[i] = value;
}
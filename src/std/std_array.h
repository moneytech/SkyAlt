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

StdArr StdArr_init(void)
{
	StdArr self;
	self.ptrs = 0;
	self.num = 0;
	return self;
}

void StdArr_resize(StdArr* self, UBIG num);

StdArr StdArr_initCopy(const StdArr* src)
{
	StdArr self = StdArr_init();

	StdArr_resize(&self, src->num);
	Os_memcpy(self.ptrs, src->ptrs, src->num * sizeof(void*));

	return self;
}

StdArr StdArr_initCopyFn(const StdArr* src, void* (*copyFn)(void*))
{
	StdArr self = StdArr_init();

	StdArr_resize(&self, src->num);
	UBIG i;
	for (i = 0; i < src->num; i++)
		self.ptrs[i] = copyFn(src->ptrs[i]);

	return self;
}

void StdArr_freeBase(StdArr* self)
{
	Os_free(self->ptrs, self->num * sizeof(void*));
	Os_memset(self, sizeof(StdArr));
}
void StdArr_freeItems(StdArr* self, const int item_size)
{
	UBIG i;
	for (i = 0; i < self->num; i++)
	{
		Os_free(self->ptrs[i], item_size);
	}
	StdArr_freeBase(self);
}
void StdArr_freeFn(StdArr* self, void (*freeFn)(void*))
{
	UBIG i;
	for (i = 0; i < self->num; i++)
		if (self->ptrs[i])
			freeFn(self->ptrs[i]);

	StdArr_freeBase(self);
}
void StdArr_freeFnPrm(StdArr* self, void (*freeFn)(void*, void*), void* prm)
{
	UBIG i;
	for (i = 0; i < self->num; i++)
		if (self->ptrs[i])
			freeFn(self->ptrs[i], prm);

	StdArr_freeBase(self);
}

void StdArr_resize(StdArr* self, UBIG num)
{
	self->ptrs = (void**)Os_realloc(self->ptrs, num * sizeof(void*));

	if (num > self->num)
		Os_memset(&self->ptrs[self->num], (num - self->num) * sizeof(void*));

	self->num = num;
}
void* StdArr_add(StdArr* self, void* ptr)
{
	StdArr_resize(self, self->num + 1);
	self->ptrs[self->num - 1] = ptr;
	return ptr;
}
void StdArr_remove(StdArr* self, UBIG i)
{
	Os_memmove(&self->ptrs[i], &self->ptrs[i + 1], (self->num - i - 1) * sizeof(void*));
	StdArr_resize(self, self->num - 1);
}

void StdArr_insert(StdArr* self, UBIG i, void* ptr)
{
	UBIG oldN = self->num;
	StdArr_resize(self, i < oldN ? oldN + 1 : i + 1);

	if (i < oldN)
		Os_memmove(&self->ptrs[i + 1], &self->ptrs[i], (oldN - i) * sizeof(void*));

	self->ptrs[i] = ptr;
}
BIG StdArr_find(const StdArr* self, const void* ptr)
{
	UBIG i;
	for (i = 0; i < self->num; i++)
		if (self->ptrs[i] == ptr)
			return i;
	return -1;
}
BOOL StdArr_removeFind(StdArr* self, const void* ptr)
{
	BIG i = StdArr_find(self, ptr);
	BOOL ok = (i >= 0);
	if (ok)
		StdArr_remove(self, i);
	return ok;
}

void StdArr_swap(StdArr* self, UBIG iA, BIG iB)
{
	void* backup = self->ptrs[iA];
	self->ptrs[iA] = self->ptrs[iB];
	self->ptrs[iB] = backup;
}

void* StdArr_get(StdArr* self, UBIG i)
{
	return (i < self->num) ? self->ptrs[i] : 0;
}

void* StdArr_last(StdArr* self)
{
	return StdArr_get(self, self->num - 1);
}

void StdArr_addArr(StdArr* self, StdArr* src)
{
	UBIG start = self->num;
	StdArr_resize(self, self->num + src->num);
	Os_memcpy(&self->ptrs[start], &src->ptrs[0], src->num * sizeof(UBIG));
}

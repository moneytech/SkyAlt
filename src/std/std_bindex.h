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

typedef struct StdBIndexGroup_s
{
	UBIG* ids;
	BIG* hashes;
	UBIG num;
	UBIG num_alloc;

	BIG minValue;
	BIG maxValue;
} StdBIndexGroup;

void StdBIndexGroup_free(StdBIndexGroup* self)
{
	Os_free(self->ids, self->num_alloc * sizeof(UBIG));
	Os_free(self->hashes, self->num_alloc * sizeof(BIG));
	Os_memset(self, sizeof(StdBIndexGroup));
}

UBIG StdBIndexGroup_size(const StdBIndexGroup* self)
{
	return sizeof(StdBIndexGroup) + self->num_alloc * (sizeof(UBIG) + sizeof(BIG));
}

BIG StdBIndexGroup_search(const StdBIndexGroup* self, const BIG hash)
{
	if (hash < self->minValue || hash > self->maxValue)
		return -1;

	int i;
	for (i = 0; i < self->num; i++)
		if (self->hashes[i] == hash)
			return self->ids[i];
	return -1;
}

static void StdBIndexGroup_resize(StdBIndexGroup* self, BIG N)
{
	const int ADD_EXTRA = 32;
	self->num = N;

	if (N > self->num_alloc || (N + ADD_EXTRA) < self->num_alloc)
	{
		self->num_alloc = (N > self->num_alloc) ? (N + ADD_EXTRA) : N;

		self->ids = Os_realloc(self->ids, self->num_alloc * sizeof(UBIG));
		self->hashes = Os_realloc(self->hashes, self->num_alloc * sizeof(BIG));
	}
}

void StdBIndexGroup_add(StdBIndexGroup* self, const BIG hash, UBIG id)
{
	StdBIndexGroup_resize(self, self->num + 1);

	self->hashes[self->num - 1] = hash;
	self->ids[self->num - 1] = id;

	if (self->num == 1)
	{
		self->minValue = hash;
		self->maxValue = hash;
	}
	else
	{
		self->minValue = Std_bmin(self->minValue, hash);
		self->maxValue = Std_bmax(self->maxValue, hash);
	}
}

BOOL StdBIndexGroup_remove(StdBIndexGroup* self, const BIG hash)
{
	int i;
	for (i = 0; i < self->num; i++)
	{
		if (self->hashes[i] == hash)
		{
			Os_memmove(&self->ids[i], &self->ids[i + 1], (self->num - i - 1) * sizeof(UBIG));
			Os_memmove(&self->hashes[i], &self->hashes[i + 1], (self->num - i - 1) * sizeof(BIG));

			StdBIndexGroup_resize(self, self->num - 1);
			return i;
		}
	}
	return -1;
}

typedef struct StdBIndex_s
{
	StdBIndexGroup* groups; //65 536x
} StdBIndex;

StdBIndex* StdBIndex_new(void)
{
	StdBIndex* self = Os_malloc(sizeof(StdBIndex));
	self->groups = Os_calloc(sizeof(StdBIndexGroup), 65536);
	return self;
}

void StdBIndex_clear(StdBIndex* self)
{
	int i;
	for (i = 0; i < 65536; i++)
		StdBIndexGroup_free(&self->groups[i]);
}

void StdBIndex_delete(StdBIndex* self)
{
	StdBIndex_clear(self);
	Os_free(self->groups, 65536 * sizeof(StdBIndexGroup));

	Os_free(self, sizeof(StdBIndex));
}

UBIG StdBIndex_size(const StdBIndex* self)
{
	UBIG sum = 0;
	int i;
	for (i = 0; i < 65536; i++)
		sum += StdBIndexGroup_size(&self->groups[i]);
	return sum;
}

static StdBIndexGroup* _StdBIndex_getGroup(StdBIndex* self, BIG hash)
{
	//int a = hash % 65536;
	return &self->groups[(USHORT)hash];
}

static StdBIndexGroup* _StdBIndex_getGroupConst(const StdBIndex* self, BIG hash)
{
	return &self->groups[(USHORT)hash];
}

BIG StdBIndex_search(const StdBIndex* self, BIG hash)
{
	return StdBIndexGroup_search(_StdBIndex_getGroupConst(self, hash), hash);
}

void StdBIndex_add(StdBIndex* self, BIG hash, UBIG id)
{
	StdBIndexGroup_add(_StdBIndex_getGroup(self, hash), hash, id);
}

BOOL StdBIndex_remove(StdBIndex* self, BIG hash)
{
	return StdBIndexGroup_remove(_StdBIndex_getGroup(self, hash), hash);
}

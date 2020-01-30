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

#define TableHald_NUM 512
#define TableHald_NUMBITS (TableHald_NUM*64) //every UBIG has 64 bits

typedef struct TableItem_s
{
	union
	{
		double flt;
		double* fltArr;

		UNI* string;

		UBIG bits;
		FileRow fileId;
	};
} TableItem;

UBIG TableItem_bytes(void)
{
	return(TableHald_NUM + 0) * sizeof(TableItem);
}

TableItem* TableItem_new(double defValue)
{
	TableItem* self = Os_calloc((TableHald_NUM + 0), sizeof(TableItem)); //reset to 0

	int i;
	for (i = 0; i < TableHald_NUM; i++)
		self[i].flt = defValue;

	return self;
}

void TableItem_delete(TableItem* self)
{
	Os_free(self, TableItem_bytes());
}

typedef struct TableItemsBlock_s
{
	TableItem* items;
	BOOL changed_save;	//smazat, mít pøímo v TableItems(zbyteènì zvyšuje velikost na 16B a realloc je pomalejší) ...
	BOOL changed_exe;	//smazat, mít pøímo v TableItems ...
} TableItemsBlock;

TableItemsBlock TableItemsBlock_init(double defValue)
{
	TableItemsBlock self;
	self.items = TableItem_new(defValue);
	self.changed_save = FALSE;
	self.changed_exe = FALSE;
	return self;
}

void TableItemsBlock_free(TableItemsBlock* self)
{
	TableItem_delete(self->items);
	Os_memset(self, sizeof(TableItemsBlock));
}

UBIG TableItemsBlock_bytes(void)
{
	return TableItem_bytes() + sizeof(TableItemsBlock);
}

void TableItemsBlock_resetAllTo0(TableItemsBlock* self)
{
	Os_memset(self->items, TableItem_bytes()); //reset to 0
	self->changed_save = FALSE;
	self->changed_exe = FALSE;
}

void TableItemsBlock_resetAllTo1(TableItemsBlock* self)
{
	Os_memsetEx(self->items, 255, TableItem_bytes()); //reset to 1
	self->changed_save = TRUE;
	self->changed_exe = TRUE;
}

typedef struct TableItems_s
{
	TableItemsBlock* blocks;
	UBIG num;
	double defValue;
} TableItems;

TableItems TableItems_init(double defValue)
{
	TableItems self;
	self.num = 0;
	self.blocks = 0;
	self.defValue = defValue;
	return self;
}

void TableItems_free(TableItems* self)
{
	UBIG i;
	for (i = 0; i < self->num; i++)
		TableItemsBlock_free(&self->blocks[i]);

	Os_free(self->blocks, self->num * sizeof(TableItemsBlock));
}

void TableItems_clear(TableItems* self)
{
	TableItems_free(self);
	*self = TableItems_init(self->defValue);
}

TableItem* TableItems_get(TableItems* self, const UBIG r) //slow as shit
{
	return &self->blocks[r / TableHald_NUM].items[r % TableHald_NUM];
}

const TableItem* TableItems_getConst(const TableItems* self, const UBIG r) //slow as shit
{
	return &self->blocks[r / TableHald_NUM].items[r % TableHald_NUM];
}

BOOL TableItems_getBit(TableItems* self, const UBIG i)
{
	UBIG value = self->blocks[i / TableHald_NUMBITS].items[i % TableHald_NUMBITS / 64].bits;
	UCHAR bit = i % 64;
	return(value >> bit) & 1;
}

void TableItems_setBitTo1(TableItems* self, const UBIG i)
{
	TableItemsBlock* block = &self->blocks[i / TableHald_NUMBITS];
	block->changed_save = TRUE;
	block->changed_exe = TRUE;

	UBIG* value = &block->items[i % TableHald_NUMBITS / 64].bits;
	UCHAR bit = i % 64;
	*value |= (1ULL << bit);
}

/*void TableItems_setBitTo0(TableItems* self, const UBIG i)
{
	UBIG* value = &self->blocks[i / TableHald_NUMBITS].items[i % TableHald_NUMBITS / 64].bits;
	UCHAR bit = i % 64;
 *value &= ~(1L << bit);
}*/

BOOL TableItems_isValid(const TableItems* self, const UBIG r)
{
	return FileRow_is(self->blocks[r / TableHald_NUM].items[r % TableHald_NUM].fileId);
}

void TableItems_invalidate(TableItems* self, const UBIG r)
{
	FileRow_invalidate(&self->blocks[r / TableHald_NUM].items[r % TableHald_NUM].fileId);
}

static void _TableItems_addEx(TableItems* self, const UBIG numBlocks)
{
	if (numBlocks > self->num)
		self->blocks = Os_realloc(self->blocks, numBlocks * sizeof(TableItemsBlock));
	UBIG i;
	for (i = self->num; i < numBlocks; i++)
		self->blocks[i] = TableItemsBlock_init(self->defValue);
	self->num = numBlocks;
}

void TableItems_add(TableItems* self, const UBIG NUM_ROWS)
{
	_TableItems_addEx(self, (NUM_ROWS / TableHald_NUM) + 1);
}

void TableItems_addBits(TableItems* self, const UBIG NUM_ROWS)
{
	_TableItems_addEx(self, (NUM_ROWS / TableHald_NUMBITS) + 1);
}

TableItem* TableItems_first(TableItems* self)
{
	return &self->blocks[0].items[0];
}

UBIG TableItems_bytes(TableItems* self, const UBIG NUM_ROWS)
{
	return self->num * TableItemsBlock_bytes() + sizeof(TableItems);
}

void TableItems_maintenance(TableItems* self, const UBIG NUM_ROWS, TableItems* active)
{
	UBIG move = 0;
	UBIG i;
	for (i = 0; i < NUM_ROWS; i++)
	{
		if (!TableItems_isValid(active, i))
			move++;
		else
			if (move)
				*TableItems_get(self, i - move) = *TableItems_get(self, i);
	}

	const UBIG new_num = ((NUM_ROWS - move) / TableHald_NUM) + 1;
	for (i = new_num; i < self->num; i++)
		TableItemsBlock_free(&self->blocks[i]);

	self->num = new_num;
	self->blocks = Os_realloc(self->blocks, self->num * sizeof(TableItemsBlock));
}

void TableItems_resetAllTo0(TableItems* self)
{
	UBIG i;
	for (i = 0; i < self->num; i++)
		TableItemsBlock_resetAllTo0(&self->blocks[i]);
}

void TableItems_resetAllTo1(TableItems* self)
{
	UBIG i;
	for (i = 0; i < self->num; i++)
		TableItemsBlock_resetAllTo1(&self->blocks[i]);
}

BOOL TableItems_cmp(const TableItems* a, const TableItems* b, const UBIG NUM_ROWS)
{
	UBIG i;
	for (i = 0; i < NUM_ROWS; i++)
	{
		if (TableItems_get((TableItems*)a, i)->flt != TableItems_get((TableItems*)b, i)->flt)
			return FALSE;
	}
	return TRUE;
}

void TableItems_copyContent(TableItems* self, TableItems* src, const UBIG NUM_ROWS)
{
	UBIG i;
	for (i = 0; i < NUM_ROWS; i++)
		*TableItems_get((TableItems*)self, i) = *TableItems_get((TableItems*)src, i);
}

BOOL TableItems_isChangedSave(const TableItems* self)
{
	UBIG i;
	for (i = 0; i < self->num; i++)
		if (self->blocks[i].changed_save)
			return TRUE;
	return FALSE;
}

BOOL TableItems_isChangedExe(const TableItems* self)
{
	UBIG i;
	for (i = 0; i < self->num; i++)
		if (self->blocks[i].changed_exe)
			return TRUE;
	return FALSE;
}

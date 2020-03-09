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

typedef struct GuiItemSwitch_s
{
	GuiItem base;
	StdArr items;	//<GuiItem*>
	DbValue value;
} GuiItemSwitch;

GuiItem* GuiItemSwitch_getItem(GuiItemSwitch* self, int i)
{
	return self->items.ptrs[i];
}
GuiItem* GuiItemSwitch_addItem(GuiItemSwitch* self, int pos, GuiItem* item)
{
	StdArr_insert(&self->items, pos, item);
	return item;
}

GuiItem* GuiItemSwitch_new(Quad2i coord, DbValue value)
{
	GuiItemSwitch* self = Os_malloc(sizeof(GuiItemSwitch));
	self->base = GuiItem_init(GuiItem_SWITCH, coord);
	self->items = StdArr_init();
	self->value = value;
	return(GuiItem*)self;
}

GuiItem* GuiItemSwitch_newCopy(GuiItemSwitch* src, BOOL copySub)
{
	GuiItemSwitch* self = Os_malloc(sizeof(GuiItemSwitch));
	*self = *src;
	GuiItem_initCopy(&self->base, &src->base, copySub);

	self->items = StdArr_init();
	int i;
	for (i = 0; i < src->items.num; i++)
		GuiItemSwitch_addItem(self, i, GuiItem_newCopy(GuiItemSwitch_getItem(src, i), TRUE));

	self->value = DbValue_initCopy(&src->value);

	return(GuiItem*)self;
}

void GuiItemSwitch_delete(GuiItemSwitch* self)
{
	StdArr_freeFn(&self->items, (StdArrFREE)&GuiItem_delete);

	DbValue_free(&self->value);

	GuiItem_free(&self->base);
	Os_free(self, sizeof(GuiItemSwitch));
}

GuiItem* GuiItemSwitch_getActiveItem(GuiItemSwitch* self)
{
	int i = DbValue_getNumber(&self->value);
	return (i >= 0 && i < self->items.num) ? GuiItemSwitch_getItem(self, i) : 0;
}

double GuiItemSwitch_getNumber(const GuiItemSwitch* self)
{
	return self ? DbValue_getNumber(&self->value) : 0;
}
void GuiItemSwitch_setNumber(GuiItemSwitch* self, double value)
{
	DbValue_setNumber(&self->value, value);
}

void GuiItemSwitch_draw(GuiItemSwitch* self, Image4* img, Quad2i coord, Win* win)
{
}

void GuiItemSwitch_update(GuiItemSwitch* self, Quad2i coord, Win* win)
{
	BOOL changed = DbValue_hasChanged(&self->value);
	if (changed)
		GuiItem_setResize(&self->base, TRUE);

	GuiItem_setRedraw(&self->base, changed);
}

void GuiItemSwitch_touch(GuiItemSwitch* self, Quad2i coord, Win* win)
{
}

GuiItemLayout* GuiItemSwitch_resize(GuiItemSwitch* self, GuiItemLayout* layout, Win* win)
{
	if (!self->base.resize)
		return layout;

	GuiItemSwitch* sw = (GuiItemSwitch*)self;

	GuiItem_freeSubs(&self->base);

	GuiItem* addItem = GuiItemSwitch_getActiveItem(sw);
	if (addItem)
	{
		addItem = GuiItem_newCopy(addItem, TRUE);
		GuiItem_setGrid(addItem, self->base.grid);
		GuiItem_addSubName(&self->base, "add", addItem);
	}

	return layout;
}
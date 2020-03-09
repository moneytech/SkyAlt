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

typedef struct GuiItemCalendarBig_s
{
	GuiItem base;

	DbValue format;
	DbValue value;

	OsDate currDate;
	OsDate currPageDate;
}GuiItemCalendarBig;

GuiItemCalendarBig* GuiItemCalendarBig_new(Quad2i grid, DbValue value)
{
	GuiItemCalendarBig* self = Os_malloc(sizeof(GuiItemCalendarBig));
	self->base = GuiItem_init(GuiItem_CALENDAR_BIG, grid);

	self->value = value;

	return self;
}

GuiItem* GuiItemCalendarBig_newCopy(GuiItemCalendarBig* src, BOOL copySub)
{
	GuiItemCalendarBig* self = Os_malloc(sizeof(GuiItemCalendarBig));
	*self = *src;

	GuiItem_initCopy(&self->base, &src->base, copySub);
	self->value = DbValue_initCopy(&src->value);
	//...
	return (GuiItem*)self;
}

void GuiItemCalendarBig_delete(GuiItemCalendarBig* self)
{
	DbValue_free(&self->value);

	GuiItem_free(&self->base);
	Os_free(self, sizeof(GuiItemCalendarBig));
}

void GuiItemCalendarBig_update(GuiItemCalendarBig* self, Quad2i coord, Win* win)
{
	//...

	GuiItem_setRedraw(&self->base, (DbValue_hasChanged(&self->format) || DbValue_hasChanged(&self->value)));
}

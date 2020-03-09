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

typedef struct GuiItemBox_s
{
	GuiItem base;
	Rgba cd;
	BOOL empty;	//transparent

	GuiImage* image;

	DbValue value;
} GuiItemBox;

GuiItem* GuiItemBox_newEmpty(Quad2i coord)
{
	GuiItemBox* self = Os_malloc(sizeof(GuiItemBox));
	self->base = GuiItem_init(GuiItem_BOX, coord);
	self->cd = Rgba_initBlack();
	self->empty = TRUE;
	self->image = 0;
	self->value = DbValue_initEmpty();
	self->base.icon_draw_back = FALSE;
	return (GuiItem*)self;
}

GuiItem* GuiItemBox_newCd(Quad2i coord, Rgba cd)
{
	GuiItemBox* self = (GuiItemBox*)GuiItemBox_newEmpty(coord);
	self->cd = cd;
	self->empty = FALSE;
	return (GuiItem*)self;
}

GuiItem* GuiItemBox_newImage(Quad2i coord, GuiImage* image)
{
	GuiItemBox* self = (GuiItemBox*)GuiItemBox_newEmpty(coord);
	self->empty = FALSE;
	self->image = image;
	return (GuiItem*)self;
}

GuiItem* GuiItemBox_newShortcut(Quad2i coord, UBIG shortKey_extra, UNI shortKey_id, GuiItemCallback* click)
{
	GuiItemBox* self = (GuiItemBox*)GuiItemBox_newEmpty(coord);
	GuiItem_setShortcutKey(&self->base, TRUE, shortKey_extra, shortKey_id, click);
	return (GuiItem*)self;
}

GuiItem* GuiItemBox_newCopy(GuiItemBox* src, BOOL copySub)
{
	GuiItemBox* self = Os_malloc(sizeof(GuiItemBox));
	*self = *src;
	GuiItem_initCopy(&self->base, &src->base, copySub);

	if (self->image)	self->image = GuiImage_newCopy(src->image);
	return (GuiItem*)self;
}

void GuiItemBox_delete(GuiItemBox* self)
{
	if (self->image)	GuiImage_delete(self->image);
	GuiItem_free(&self->base);
	Os_free(self, sizeof(GuiItemBox));
}

void GuiItemBox_draw(GuiItemBox* self, Image4* img, Quad2i coord, Win* win)
{
	if (!self->empty)
	{
		if (self->image)
			GuiImage_draw(self->image, img, coord, self->base.back_cd);
		else
			Image4_drawBoxQuad(img, coord, self->base.back_cd);
	}
}

void GuiItemBox_update(GuiItemBox* self, Quad2i coord, Win* win)
{
	if (self->image)
		GuiImage_update(self->image, coord.size);
}

void GuiItemBox_touch(GuiItemBox* self, Quad2i coord, Win* win)
{
	Rgba back_cd = self->cd;
	Rgba front_cd = self->cd;
	_GuiItem_updateFinalCd(&self->base, back_cd, front_cd, coord, win);
}

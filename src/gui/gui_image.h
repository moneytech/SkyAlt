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

typedef struct GuiImage_s
{
	UINT channels;
	union
	{
		Image1 imgOrig1;
		Image4 imgOrig4;
	};

	union
	{
		Image1 imgResize1;
		Image4 imgResize4;
	};
}GuiImage;

GuiImage* GuiImage_new1(Image1 image)
{
	GuiImage* self = Os_malloc(sizeof(GuiImage));

	self->channels = 1;
	self->imgOrig1 = image;
	self->imgResize1 = Image1_init();

	return self;
}

GuiImage* GuiImage_new4(Image4 image)
{
	GuiImage* self = Os_malloc(sizeof(GuiImage));

	self->channels = 4;
	self->imgOrig4 = image;
	self->imgResize4 = Image4_init();

	return self;
}

GuiImage* GuiImage_newCopy(GuiImage* src)
{
	if (src)
	{
		GuiImage* self = Os_malloc(sizeof(GuiImage));
		*self = *src;

		if (self->channels == 1)
		{
			self->imgOrig1 = Image1_initCopy(&src->imgOrig1);
			self->imgResize1 = Image1_initCopy(&src->imgResize1);
		}
		else
			if (self->channels == 4)
			{
				self->imgOrig4 = Image4_initCopy(&src->imgOrig4);
				self->imgResize4 = Image4_initCopy(&src->imgResize4);
			}

		return self;
	}

	return 0;
}

void GuiImage_delete(GuiImage* self)
{
	if (self)
	{
		if (self->channels == 1)
		{
			Image1_free(&self->imgOrig1);
			Image1_free(&self->imgResize1);
		}
		else
			if (self->channels == 4)
			{
				Image4_free(&self->imgOrig4);
				Image4_free(&self->imgResize4);
			}

		Os_free(self, sizeof(GuiImage));
	}
}

Vec2i GuiImage_getSize(const GuiImage* self)
{
	return Vec2i_max(self->imgResize1.size, self->imgResize4.size);
}

void GuiImage_draw(GuiImage* self, Image4* img, Quad2i coord, Rgba cd)
{
	//Vec2i size1 = Vec2i_subRatio(coord.size, self->imgOrig1.size);
	//Vec2i size4 = Vec2i_subRatio(coord.size, self->imgOrig4.size);

	if (self->channels == 1)// && Vec2i_cmp(self->imgResize1.size, size1))
	{
		Image4_copyImage1(img, Quad2i_getSubStart(coord, self->imgResize1.size), cd, &self->imgResize1);
	}
	else
		if (self->channels == 4)// && Vec2i_cmp(self->imgResize4.size, size4))
		{
			Image4_copyImage4(img, Quad2i_getSubStart(coord, self->imgResize4.size), &self->imgResize4);
		}
}

BOOL GuiImage_update(GuiImage* self, Vec2i sz)
{
	BOOL changed = FALSE;

	if (self->channels == 1)
	{
		Vec2i size = Vec2i_subRatio(sz, self->imgOrig1.size);
		if (!Vec2i_cmp(size, self->imgResize1.size))
		{
			Image1_resize(&self->imgResize1, size);
			Image1_scale(&self->imgResize1, &self->imgOrig1);

			changed = TRUE;
		}
	}
	else
		if (self->channels == 4)
		{
			Vec2i size = Vec2i_subRatio(sz, self->imgOrig4.size);
			if (!Vec2i_cmp(size, self->imgResize4.size))
			{
				Image4_resize(&self->imgResize4, size);
				Image4_scale(&self->imgResize4, &self->imgOrig4);

				changed = TRUE;
			}
		}

	return changed;
}

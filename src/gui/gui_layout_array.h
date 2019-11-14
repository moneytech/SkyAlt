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

typedef struct GuiItemLayoutArray_s
{
	int* inputs;
	int* outputs;
	int num;
}GuiItemLayoutArray;
GuiItemLayoutArray GuiItemLayoutArray_init(void)
{
	GuiItemLayoutArray self;
	self.inputs = 0;
	self.outputs = 0;
	self.num = 0;
	return self;
}

GuiItemLayoutArray GuiItemLayoutArray_initCopy(GuiItemLayoutArray* src)
{
	GuiItemLayoutArray self;
	self.num = src->num;

	const int BYTES = src->num * sizeof(int);
	self.inputs = Os_malloc(BYTES);
	self.outputs = Os_malloc(BYTES);
	Os_memcpy(self.inputs, src->inputs, BYTES);
	Os_memcpy(self.outputs, src->outputs, BYTES);

	return self;
}

void GuiItemLayoutArray_clear(GuiItemLayoutArray* self)
{
	Os_free(self->inputs, self->num * sizeof(int));
	Os_free(self->outputs, self->num * sizeof(int));
	self->inputs = 0;
	self->outputs = 0;
	self->num = 0;
}

void GuiItemLayoutArray_free(GuiItemLayoutArray* self)
{
	GuiItemLayoutArray_clear(self);
	Os_memset(self, sizeof(GuiItemLayoutArray));
}

void GuiItemLayoutArray_resize(GuiItemLayoutArray* self, UINT num)
{
	if (self->num != num)
	{
		self->inputs = Os_realloc(self->inputs, num * sizeof(int));
		self->outputs = Os_realloc(self->outputs, num * sizeof(int));

		int i;
		for (i = self->num; i < num; i++)
		{
			self->inputs[i] = 1;
			self->outputs[i] = 0;
		}

		self->num = num;
	}
}

//value:
	//default is 1
	//negative = hide when view is too small
void GuiItemLayoutArray_add(GuiItemLayoutArray* self, UINT pos, int value)
{
	if (pos >= self->num)
		GuiItemLayoutArray_resize(self, pos + 1);

	self->inputs[pos] = value;
}

Vec2i GuiItemLayoutArray_convert(const GuiItemLayoutArray* self, const int cell, const int start, const int end)
{
	Vec2i ret = Vec2i_init();

	int i;
	for (i = 0; i < end; i++)
	{
		BOOL ok = (i < self->num);

		if (i < start)
			ret.x += ok ? self->outputs[i] : cell;
		else
			ret.y += ok ? self->outputs[i] : cell;
	}

	return ret;
}

int GuiItemLayoutArray_outputAll(const GuiItemLayoutArray* self)
{
	int i;
	int sum = 0;
	for (i = 0; i < self->num; i++)
		sum += self->outputs[i];
	return sum;
}

static int _GuiItemLayoutArray_outputMin(const GuiItemLayoutArray* self)
{
	int i;
	int index = 0;
	int act = self->outputs[0];

	for (i = 0; i < self->num; i++)
		if (self->outputs[i] < act)
			act = self->outputs[i], index = i;

	return index;
}

//negative has higher priority than positive => when all negative are 1 cell than it will stretch down positive
static int _GuiItemLayoutArray_outputMax(const GuiItemLayoutArray* self, int cell)
{
	int i;
	int index = 0;
	int act = self->outputs[0];
	BOOL negativeFound = FALSE;

	//try to find some negative
	for (i = 0; i < self->num; i++)
	{
		if (self->inputs[i] < 0)
			act = self->outputs[i], index = i;
	}

	//find best negative
	for (i = 0; i < self->num; i++)
	{
		if (self->inputs[i] < 0 && self->outputs[i] >= act && self->outputs[i] > cell)
			act = self->outputs[i], index = i, negativeFound = TRUE;
	}

	if (!negativeFound)
	{
		for (i = 0; i < self->num; i++)
		{
			if (self->outputs[i] > act)
				act = self->outputs[i], index = i;
		}
	}
	return index;
}

//note: this is use for ide_mode
void GuiItemLayoutArray_updateDefault(GuiItemLayoutArray* self, int cell)
{
	int i;
	for (i = 0; i < self->num; i++)
	{
		self->outputs[i] = cell;
	}
}

void GuiItemLayoutArray_update(GuiItemLayoutArray* self, int cell, int window, BOOL scaleDown)
{
	int i;
	int allPixels = 0;
	for (i = 0; i < self->num; i++)
	{
		BIG c = Std_abs(self->inputs[i]);

		self->outputs[i] = cell * c;

		allPixels += self->outputs[i];
	}

	if (self->num && scaleDown)
	{
		//round to cells
		while (allPixels > window)		//scale down
		{
			int* p = &self->outputs[_GuiItemLayoutArray_outputMax(self, cell)];
			if (*p > cell)	//minimum is one cell
			{
				*p -= 1;//cell;
				allPixels -= 1;//cell;
			}
			else
				break;
		}
	}
}

void GuiItemLayoutArray_updateLittle(GuiItemLayoutArray* self, int cell, int window)
{
	int allPixels = 0;
	int i;
	for (i = 0; i < self->num; i++)
		allPixels += self->outputs[i];

	if (self->num)
	{
		if ((window - allPixels) < cell)	//only if it's in the middle of last cell
		{
			//round to pixels
			while (allPixels < window)		//scale up
			{
				self->outputs[_GuiItemLayoutArray_outputMin(self)]++;
				allPixels++;
			}
		}
	}
}

int GuiItemLayoutArray_getCloseCell(GuiItemLayoutArray* self, int pos)
{
	int allPixels = 0;
	int allPixelsLast = 0;
	int i;
	for (i = 0; i < self->num; i++)
	{
		allPixels += self->outputs[i];

		if (pos >= allPixelsLast && pos < allPixels)
			return (pos - allPixelsLast) < (allPixels - pos) ? i : i + 1;

		allPixelsLast = allPixels;
	}

	return self->num;
}

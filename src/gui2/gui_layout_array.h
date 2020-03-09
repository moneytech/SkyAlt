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

typedef struct GuiLayoutArray_s
{
	int* inputs;
	int* outputs;
	int num;
}GuiLayoutArray;
GuiLayoutArray GuiLayoutArray_init(void)
{
	GuiLayoutArray self;
	self.inputs = 0;
	self.outputs = 0;
	self.num = 0;
	return self;
}

GuiLayoutArray GuiLayoutArray_initCopy(const GuiLayoutArray* src)
{
	GuiLayoutArray self;
	self.num = src->num;

	const int BYTES = src->num * sizeof(int);
	self.inputs = Os_malloc(BYTES);
	self.outputs = Os_malloc(BYTES);
	Os_memcpy(self.inputs, src->inputs, BYTES);
	Os_memcpy(self.outputs, src->outputs, BYTES);

	return self;
}

void GuiLayoutArray_clear(GuiLayoutArray* self)
{
	Os_free(self->inputs, self->num * sizeof(int));
	Os_free(self->outputs, self->num * sizeof(int));
	self->inputs = 0;
	self->outputs = 0;
	self->num = 0;
}

void GuiLayoutArray_free(GuiLayoutArray* self)
{
	GuiLayoutArray_clear(self);
	Os_memset(self, sizeof(GuiLayoutArray));
}

void GuiLayoutArray_resize(GuiLayoutArray* self, UINT num)
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
void GuiLayoutArray_add(GuiLayoutArray* self, UINT pos, int value)
{
	if (pos >= self->num)
		GuiLayoutArray_resize(self, pos + 1);

	self->inputs[pos] = value;
}

Vec2i GuiLayoutArray_convert(const GuiLayoutArray* self, const int cell, const int start, const int end)
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

int GuiLayoutArray_outputAll(const GuiLayoutArray* self)
{
	int i;
	int sum = 0;
	for (i = 0; i < self->num; i++)
		sum += self->outputs[i];
	return sum;
}

static int _GuiLayoutArray_outputMin(const GuiLayoutArray* self)
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
static int _GuiLayoutArray_outputMax(const GuiLayoutArray* self, int cell)
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
void GuiLayoutArray_updateDefault(GuiLayoutArray* self, int cell)
{
	int i;
	for (i = 0; i < self->num; i++)
	{
		self->outputs[i] = cell;
	}
}

void GuiLayoutArray_update(GuiLayoutArray* self, int cell, int window, BOOL scaleDown)
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
			int* p = &self->outputs[_GuiLayoutArray_outputMax(self, cell)];
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

void GuiLayoutArray_updateLittle(GuiLayoutArray* self, int cell, int window)
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
				self->outputs[_GuiLayoutArray_outputMin(self)]++;
				allPixels++;
			}
		}
	}
}

int GuiLayoutArray_getCloseCell(GuiLayoutArray* self, int pos)
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

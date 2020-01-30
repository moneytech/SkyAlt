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

typedef enum
{
	GuiItemChart_COLUMN_NORMAL,
	GuiItemChart_COLUMN_STACK,
	GuiItemChart_COLUMN_STACK_PROC,

	GuiItemChart_PIE,
	//GuiItemChart_PIE_STACK,

	GuiItemChart_POINT,
	GuiItemChart_XY_POINT,
	//GuiItemChart_XY_POINT_ORDERED,

	GuiItemChart_LINE,
	GuiItemChart_XY_LINE,
	GuiItemChart_XY_LINE_ORDERED,
}GuiItemChart_TYPE;

typedef struct GuiItemChart_s
{
	GuiItem base;

	BIG viewRow;
	DbRows filter;

	DbValue type;

	DbValue showColumnsValue;
	DbValue showRowsValue;

	//DbValue textCd;
	DbValue groupCount;
	//DbValue groupCountCd;
	DbValue width;

	Image4 result;

	GuiScroll scrollH;
	double minY;
	double maxY;

	double minX;
	double maxX;

	int precision;

	StdString tempStr;

	StdBigs tempPoses;

	BOOL fixed100;
}GuiItemChart;

GuiItem* GuiItemChart_new(Quad2i grid, BIG viewRow, DbRows filter, DbValue scrollH)
{
	GuiItemChart* self = Os_malloc(sizeof(GuiItemChart));
	self->base = GuiItem_init(GuiItem_CHART, grid);

	self->viewRow = viewRow;
	self->filter = filter;

	self->type = DbValue_initOption(viewRow, "chart_type", 0);

	self->showColumnsValue = DbValue_initOption(viewRow, "show_columns_values", _UNI32("1"));
	self->showRowsValue = DbValue_initOption(viewRow, "show_rows_values", _UNI32("1"));
	//self->textCd = DbValue_initOption(viewRow, "text_cd", 0);
	self->width = DbValue_initOption(viewRow, "chart_width", 0);

	self->groupCount = DbRows_getSubOption(viewRow, "chart_groups", "group_count", 0);
	//self->groupCountCd = DbRows_getSubOption(viewRow, "chart_groups", "group_count_cd", 0);

	self->result = Image4_init();

	self->scrollH = GuiScroll_init(scrollH);
	self->minY = 0;
	self->maxY = 0;
	self->minX = 0;
	self->maxX = 0;

	self->precision = DbValue_getOptionNumber(viewRow, "precision", 2);

	self->fixed100 = FALSE;

	self->tempStr = StdString_init();
	self->tempPoses = StdBigs_init();

	return (GuiItem*)self;
}

GuiItem* GuiItemChart_newCopy(GuiItemChart* src, BOOL copySub)
{
	GuiItemChart* self = Os_malloc(sizeof(GuiItemChart));
	*self = *src;
	GuiItem_initCopy(&self->base, &src->base, copySub);

	self->filter = DbRows_initCopy(&src->filter);
	self->type = DbValue_initCopy(&src->type);

	self->showColumnsValue = DbValue_initCopy(&src->showColumnsValue);
	self->showRowsValue = DbValue_initCopy(&src->showRowsValue);
	//self->textCd = DbValue_initCopy(&src->textCd);
	self->groupCount = DbValue_initCopy(&src->groupCount);
	//self->groupCountCd = DbValue_initCopy(&src->groupCountCd);
	self->width = DbValue_initCopy(&src->width);

	self->result = Image4_initCopy(&src->result);

	self->scrollH = GuiScroll_initCopy(&src->scrollH);

	self->tempStr = StdString_init();
	self->tempPoses = StdBigs_init();

	return (GuiItem*)self;
}

void GuiItemChart_delete(GuiItemChart* self)
{
	StdString_free(&self->tempStr);
	StdBigs_free(&self->tempPoses);

	DbRows_free(&self->filter);
	DbValue_free(&self->type);
	DbValue_free(&self->showColumnsValue);
	DbValue_free(&self->showRowsValue);
	//DbValue_free(&self->textCd);
	DbValue_free(&self->groupCount);
	//DbValue_free(&self->groupCountCd);
	DbValue_free(&self->width);

	Image4_free(&self->result);

	GuiScroll_free(&self->scrollH);

	GuiItem_free(&self->base);
	Os_free(self, sizeof(GuiItemChart));
}

Image1 GuiItemChart_getIcon(BIG row)
{
	GuiItemChart_TYPE type = DbValue_getOptionNumber(row, "chart_type", 0);
	switch (type)
	{
		case GuiItemChart_COLUMN_NORMAL: return UiIcons_init_chart_column_normal();
		case GuiItemChart_COLUMN_STACK: return UiIcons_init_chart_column_stack();
		case GuiItemChart_COLUMN_STACK_PROC: return UiIcons_init_chart_column_stack_proc();

		case GuiItemChart_PIE: return UiIcons_init_chart_pie();
			//case GuiItemChart_PIE_STACK: return UiIcons_init_chart_pie();

		case GuiItemChart_POINT: return UiIcons_init_chart_point();
		case GuiItemChart_XY_POINT: return UiIcons_init_chart_point_xy();

		case GuiItemChart_LINE: return UiIcons_init_chart_line();
		case GuiItemChart_XY_LINE: return UiIcons_init_chart_line_xy();
		case GuiItemChart_XY_LINE_ORDERED: return UiIcons_init_chart_line_xy_ordered();
	};
	return Image1_init();
}

static BOOL _GuiItemChart_isTypeSum(const GuiItemChart* self)
{
	GuiItemChart_TYPE type = DbValue_getNumber(&self->type);
	return type == GuiItemChart_COLUMN_STACK || type == GuiItemChart_COLUMN_STACK_PROC;
}

static BOOL _GuiItemChart_isTypeSingle(const GuiItemChart* self)
{
	GuiItemChart_TYPE type = DbValue_getNumber(&self->type);
	return _GuiItemChart_isTypeSum(self) || type == GuiItemChart_POINT || type == GuiItemChart_LINE;
}

BOOL GuiItemChart_isTypeXY_ORDERED_public(UINT type)
{
	return type == GuiItemChart_XY_LINE_ORDERED;// || type == GuiItemChart_XY_POINT_ORDERED;
}
BOOL GuiItemChart_isTypeXY_public(UINT type)
{
	return type == GuiItemChart_XY_LINE || type == GuiItemChart_XY_POINT || GuiItemChart_isTypeXY_ORDERED_public(type);
}
BOOL GuiItemChart_isTypePIE_public(UINT type)
{
	return type == GuiItemChart_PIE;
}

static BOOL _GuiItemChart_isTypeXY_ORDERED(const GuiItemChart* self)
{
	GuiItemChart_TYPE type = DbValue_getNumber(&self->type);
	return GuiItemChart_isTypeXY_ORDERED_public(type);
}

/*static BOOL _GuiItemChart_isTypeXY(const GuiItemChart* self)
{
	GuiItemChart_TYPE type = DbValue_getNumber(&self->type);
	return GuiItemChart_isTypeXY_public(type);
}*/

static BOOL _GuiItemChart_isTypePIE(const GuiItemChart* self)
{
	GuiItemChart_TYPE type = DbValue_getNumber(&self->type);
	return GuiItemChart_isTypePIE_public(type);
}

BIG GuiItemChart_getRow(GuiItemChart* self)
{
	return DbRows_getBaseRow(&self->filter);
}
void GuiItemChart_setRow(GuiItemChart* self, BIG row)
{
	DbRows_setBaseRow(&self->filter, row);
}

DbTable* GuiItemChart_getTable(const GuiItemChart* self)
{
	return DbRoot_findParentTable(self->viewRow);
}

UBIG GuiItemChart_numColumns(const GuiItemChart* self)
{
	return DbRows_getSubsNum(self->viewRow, "chart_columns", TRUE);
}
DbColumn* GuiItemChart_getColumnsColumn(const GuiItemChart* self, UBIG i)
{
	return DbRows_getSubsColumn(self->viewRow, "chart_columns", TRUE, i);
}
BIG GuiItemChart_getColumnsRow(const GuiItemChart* self, UBIG i)
{
	return DbRows_getSubsRow(self->viewRow, "chart_columns", TRUE, i);
}
Rgba GuiItemChart_getColumnsCd(const GuiItemChart* self, UBIG i)
{
	BIG crow = GuiItemChart_getColumnsRow(self, i);
	return Rgba_initFromNumber(DbValue_getOptionNumber(crow, "cd", 0));
}

UBIG GuiItemChart_numRows(const GuiItemChart* self)
{
	return DbRows_getSubsNum(self->viewRow, "chart_rows", TRUE);
}
DbColumn* GuiItemChart_getRowsColumn(const GuiItemChart* self, UBIG i)
{
	return DbRows_getSubsColumn(self->viewRow, "chart_rows", TRUE, i);
}
BIG GuiItemChart_getRowsRow(const GuiItemChart* self, UBIG i)
{
	return DbRows_getSubsRow(self->viewRow, "chart_rows", TRUE, i);
}

UBIG GuiItemChart_numRowsLines(const GuiItemChart* self)
{
	return DbValue_getNumber(&self->showRowsValue) ? GuiItemChart_numRows(self) : 0;
}

UBIG GuiItemChart_numGroups(const GuiItemChart* self)
{
	return DbRows_hasFilterSubActive(self->viewRow, "chart_groups") ? DbRows_getSubsNum(self->viewRow, "chart_groups", TRUE) : 0;
}
DbColumn* GuiItemChart_getGroup(const GuiItemChart* self, UBIG i)
{
	return DbRows_getSubsColumn(self->viewRow, "chart_groups", TRUE, i);
}
BIG GuiItemChart_getGroupRow(const GuiItemChart* self, UBIG i)
{
	return DbRows_getSubsRow(self->viewRow, "chart_groups", TRUE, i);
}

BOOL GuiItemChart_getGroupAscending(const GuiItemChart* self, UBIG i)
{
	BIG crow = GuiItemChart_getGroupRow(self, i);
	return DbValue_getOptionNumber(crow, "ascending", 0);
}

BOOL GuiItemChart_isGroupCount(const GuiItemChart* self)
{
	return GuiItemChart_numGroups(self) ? DbValue_getNumber(&self->groupCount) : 0;
}

Rgba _GuiItemChart_getTextCd(const GuiItemChart* self)
{
	return Rgba_initBlack();
	//return Rgba_initFromNumber(DbValue_getNumber(&self->textCd));
}

static double _GuiItemChart_getValues(GuiItemChart* self, BIG row, DbColumn** columns, double* values)
{
	double sum = 0;
	BIG i = 0;
	while (*columns)
	{
		values[i] = Std_dmax(0, DbColumn_getFlt(*columns, row, 0));	//no negative values
		sum += values[i];
		columns++;
		i++;
	}

	return sum;
}

static void _GuiItemChart_drawValueUNI(GuiItemChart* self, Image4* img, Vec2i pos, BOOL showValue, Rgba titleCd, DbColumn* column, BIG row)
{
	if (showValue)
	{
		int textH = _GuiItem_textSize(1, OsWinIO_cellSize());
		OsFont* font = OsWinIO_getFontDefault();
		Image4_drawTextBackground(img, pos, TRUE, font, DbColumn_getStringCopyWithFormatLong(column, row, &self->tempStr), textH, 0, titleCd, self->base.back_cd, 2);
	}
}

static Vec2f _GuiItemChart_draw_COLUMN_NORMAL(GuiItemChart* self, Image4* img, DbColumn** columns, Rgba* columnsCds, Rgba titleCd, const BIG N_COLS, BIG it, BIG it_old, const float jump, Quad2i area, double* values, double* values_old, float move, BOOL showValue, DbColumn* rowsX)
{
	Vec2f mv = Vec2f_init2(move, move);

	const double diffY = (self->maxY - self->minY);
	const float mult = diffY ? area.size.y / diffY : 0;
	const float zero = diffY ? area.size.y / diffY * self->minY : 0;

	_GuiItemChart_getValues(self, it, columns, values);
	BIG j;
	for (j = 0; j < N_COLS; j++)
	{
		Quad2i box;
		box.size.x = Std_bmax(jump, 4) - 1;	//-1 is space
		box.size.y = mult * values[j];
		box.start.x = area.start.x + mv.y + jump / 2;
		box.start.y = Quad2i_end(area).y - box.size.y + zero;
		box = Quad2i_repair(box, 0);

		//graph
		Image4_drawBoxQuad(img, box, columnsCds[j]);

		//info
		_GuiItemChart_drawValueUNI(self, img, Quad2i_getMiddle(box), showValue, titleCd, columns[j], it);

		mv.y += jump;
	}
	mv.y += jump;

	return mv;
}

static Vec2f _GuiItemChart_draw_COLUMN_STACK(GuiItemChart* self, Image4* img, DbColumn** columns, Rgba* columnsCds, Rgba titleCd, const BIG N_COLS, BIG it, BIG it_old, const float jump, Quad2i area, double* values, double* values_old, float move, BOOL showValue, DbColumn* rowsX)
{
	Vec2f mv = Vec2f_init2(move, move);

	const double diffY = (self->maxY - self->minY);
	const float mult = diffY ? area.size.y / diffY : 0;

	_GuiItemChart_getValues(self, it, columns, values);

	int y = 0;
	BIG j;
	for (j = 0; j < N_COLS; j++)
	{
		Quad2i box;
		box.size.x = Std_bmax(jump, 4) - 1;	//-1 is space
		box.size.y = mult * Std_dmax(0, values[j]);	//no negative values
		box.start.x = area.start.x + mv.y + jump / 2;
		box.start.y = Quad2i_end(area).y - box.size.y - y;
		box = Quad2i_repair(box, 0);

		//graph
		Image4_drawBoxQuad(img, box, columnsCds[j]);

		//info
		_GuiItemChart_drawValueUNI(self, img, Quad2i_getMiddle(box), showValue, titleCd, columns[j], it);

		y += box.size.y;
	}
	mv.y += jump * 2;

	return mv;
}

static Vec2f _GuiItemChart_draw_COLUMN_STACK_PROC(GuiItemChart* self, Image4* img, DbColumn** columns, Rgba* columnsCds, Rgba titleCd, const BIG N_COLS, BIG it, BIG it_old, const float jump, Quad2i area, double* values, double* values_old, float move, BOOL showValue, DbColumn* rowsX)
{
	Vec2f mv = Vec2f_init2(move, move);

	double sum = _GuiItemChart_getValues(self, it, columns, values);

	int y = 0;
	BIG j;
	for (j = 0; j < N_COLS; j++)
	{
		Quad2i box;
		box.size.x = Std_bmax(jump, 4) - 1;	//-1 is space
		box.size.y = sum ? area.size.y * Std_dmax(0, (values[j] / sum)) : 0;
		box.start.x = area.start.x + mv.y + jump / 2;
		box.start.y = Quad2i_end(area).y - box.size.y - y;
		box = Quad2i_repair(box, 0);

		//graph
		Image4_drawBoxQuad(img, box, columnsCds[j]);

		//info
		_GuiItemChart_drawValueUNI(self, img, Quad2i_getMiddle(box), showValue, titleCd, columns[j], it);

		y += box.size.y;
	}
	mv.y += jump * 2;

	return mv;
}

static Vec2f _GuiItemChart_draw_POINT(GuiItemChart* self, Image4* img, DbColumn** columns, Rgba* columnsCds, Rgba titleCd, const BIG N_COLS, BIG it, BIG it_old, const float jump, Quad2i area, double* values, double* values_old, float move, BOOL showValue, DbColumn* rowsX)
{
	Vec2f mv = Vec2f_init2(move, move);

	const double diffY = (self->maxY - self->minY);
	float multY = diffY ? area.size.y / diffY : 0;
	const float zeroY = diffY ? area.size.y / diffY * self->minY : 0;

	_GuiItemChart_getValues(self, it, columns, values);

	BIG j;
	for (j = 0; j < N_COLS; j++)
	{
		Vec2i mid = Vec2i_init2(area.start.x + mv.y + jump / 2, Quad2i_end(area).y + zeroY - multY * values[j]);

		//graph
		Image4_drawCircle(img, mid, Std_max(2, OsWinIO_cellSize() / 6), columnsCds[j]);

		//info
		_GuiItemChart_drawValueUNI(self, img, Vec2i_init2(mid.x, mid.y - OsWinIO_cellSize() / 2), showValue, titleCd, columns[j], it);
	}
	mv.y += jump * 2;

	return mv;
}

static Vec2f _GuiItemChart_draw_LINE(GuiItemChart* self, Image4* img, DbColumn** columns, Rgba* columnsCds, Rgba titleCd, const BIG N_COLS, BIG it, BIG it_old, const float jump, Quad2i area, double* values, double* values_old, float move, BOOL showValue, DbColumn* rowsX)
{
	Vec2f mv = Vec2f_init2(move, move);

	const int cell = OsWinIO_cellSize();

	const double diffY = (self->maxY - self->minY);
	float mult = diffY ? area.size.y / diffY : 0;
	const float zero = diffY ? area.size.y / diffY * self->minY : 0;

	_GuiItemChart_getValues(self, it, columns, values);
	if (it_old >= 0)
		_GuiItemChart_getValues(self, it_old, columns, values_old);

	BIG j;
	for (j = 0; j < N_COLS; j++)
	{
		Vec2i mid_old = Vec2i_init2(area.start.x + mv.y - jump * 2 + jump / 2, Quad2i_end(area).y + zero - mult * values_old[j]);
		Vec2i mid = Vec2i_init2(area.start.x + mv.y + jump / 2, Quad2i_end(area).y + zero - mult * values[j]);

		//graph
		if (it_old >= 0)
			Image4_drawLine(img, mid_old, mid, Std_max(2, cell / 10), columnsCds[j]);
		Image4_drawCircle(img, mid, Std_max(2, cell / 6), columnsCds[j]);

		//info
		_GuiItemChart_drawValueUNI(self, img, Vec2i_init2(mid.x, mid.y - OsWinIO_cellSize() / 2), showValue, titleCd, columns[j], it);
	}
	mv.y += jump * 2;

	return mv;
}

static Vec2f _GuiItemChart_draw_XY_POINT(GuiItemChart* self, Image4* img, DbColumn** columns, Rgba* columnsCds, Rgba titleCd, const BIG N_COLS, BIG it, BIG it_old, const float jump, Quad2i area, double* values, double* values_old, float move, BOOL showValue, DbColumn* rowsX)
{
	//x-space start/end
	const int ST = jump / 4;
	area.size.x -= ST * 2;

	const double diffX = (self->maxX - self->minX);
	const double diffY = (self->maxY - self->minY);
	float multX = diffX ? area.size.x / diffX : 0;
	float multY = diffY ? area.size.y / diffY : 0;

	const float zeroX = multX * self->minX;
	const float zeroY = multY * self->minY;

	_GuiItemChart_getValues(self, it, columns, values);

	const double rowXval = rowsX ? DbColumn_getFlt(rowsX, it, 0) : 0;
	move = area.start.x + ST - zeroX + multX * rowXval;

	BIG j;
	for (j = 0; j < N_COLS; j++)
	{
		Vec2i mid = Vec2i_init2(move, Quad2i_end(area).y + zeroY - multY * values[j]);

		//graph
		Image4_drawCircle(img, mid, Std_max(2, OsWinIO_cellSize() / 6), columnsCds[j]);

		//info
		_GuiItemChart_drawValueUNI(self, img, Vec2i_init2(mid.x, mid.y - OsWinIO_cellSize() / 2), showValue, titleCd, columns[j], it);
	}

	return Vec2f_init2(move, move);
}

static Vec2f _GuiItemChart_draw_XY_LINE(GuiItemChart* self, Image4* img, DbColumn** columns, Rgba* columnsCds, Rgba titleCd, const BIG N_COLS, BIG it, BIG it_old, const float jump, Quad2i area, double* values, double* values_old, float move, BOOL showValue, DbColumn* rowsX)
{
	//x-space start/end
	const int ST = jump / 4;
	area.size.x -= ST * 2;

	const double diffX = (self->maxX - self->minX);
	const double diffY = (self->maxY - self->minY);
	float multX = diffX ? area.size.x / diffX : 0;
	float multY = diffY ? area.size.y / diffY : 0;

	const float zeroX = multX * self->minX;
	const float zeroY = multY * self->minY;

	_GuiItemChart_getValues(self, it, columns, values);
	if (it_old >= 0)
		_GuiItemChart_getValues(self, it_old, columns, values_old);

	const double rowXval = rowsX ? DbColumn_getFlt(rowsX, it, 0) : 0;
	const double rowXval_old = (rowsX && it_old >= 0) ? DbColumn_getFlt(rowsX, it_old, 0) : 0;

	BIG j;
	for (j = 0; j < N_COLS; j++)
	{
		Vec2i mid_old = Vec2i_init2(area.start.x + ST - zeroX + multX * rowXval_old, Quad2i_end(area).y + zeroY - multY * values_old[j]);
		Vec2i mid = Vec2i_init2(area.start.x + ST - zeroX + multX * rowXval, Quad2i_end(area).y + zeroY - multY * values[j]);

		move = mid.x;

		//graph
		if (it_old >= 0)
		{
			Image4_drawLine(img, mid_old, mid, Std_max(2, OsWinIO_cellSize() / 10), columnsCds[j]);

			{
				Vec2i middle = Vec2i_aprox(mid, mid_old, 0.5f);
				Vec2i v = Vec2i_sub(mid, mid_old);
				v = Vec2i_getLen(v, Std_max(4, OsWinIO_cellSize() / 3));
				Image4_drawArrow(img, middle, Vec2i_sub(middle, v), Std_max(4, OsWinIO_cellSize() / 3), columnsCds[j]);
			}
		}
		Image4_drawCircle(img, mid, Std_max(2, OsWinIO_cellSize() / 6), columnsCds[j]);

		//info
		_GuiItemChart_drawValueUNI(self, img, Vec2i_init2(mid.x, mid.y - OsWinIO_cellSize() / 2), showValue, titleCd, columns[j], it);
	}

	return Vec2f_init2(move, move);
}

static Vec2f _GuiItemChart_draw_XY_LINE_ORDERED(GuiItemChart* self, Image4* img, DbColumn** columns, Rgba* columnsCds, Rgba titleCd, const BIG N_COLS, BIG it, BIG it_old, const float jump, Quad2i area, double* values, double* values_old, float move, BOOL showValue, DbColumn* rowsX)
{
	//x-space start/end
	const int ST = jump / 4;
	area.size.x -= ST * 2;

	const double diffX = (self->maxX - self->minX);
	const double diffY = (self->maxY - self->minY);
	float multX = diffX ? area.size.x / diffX : 0;
	float multY = diffY ? area.size.y / diffY : 0;

	const float zeroX = multX * self->minX;
	const float zeroY = multY * self->minY;

	_GuiItemChart_getValues(self, it, columns, values);
	if (it_old >= 0)
		_GuiItemChart_getValues(self, it_old, columns, values_old);

	const double rowXval = rowsX ? DbColumn_getFlt(rowsX, it, 0) : 0;
	const double rowXval_old = (rowsX && it_old >= 0) ? DbColumn_getFlt(rowsX, it_old, 0) : 0;

	BIG j;
	for (j = 0; j < N_COLS; j++)
	{
		Vec2i mid_old = Vec2i_init2(area.start.x + ST - zeroX + multX * rowXval_old, Quad2i_end(area).y + zeroY - multY * values_old[j]);
		Vec2i mid = Vec2i_init2(area.start.x + ST - zeroX + multX * rowXval, Quad2i_end(area).y + zeroY - multY * values[j]);
		move = mid.x;

		//graph
		if (it_old >= 0)
			Image4_drawLine(img, mid_old, mid, Std_max(2, OsWinIO_cellSize() / 10), columnsCds[j]);
		Image4_drawCircle(img, mid, Std_max(2, OsWinIO_cellSize() / 6), columnsCds[j]);

		//info
		_GuiItemChart_drawValueUNI(self, img, Vec2i_init2(mid.x, mid.y - OsWinIO_cellSize() / 2), showValue, titleCd, columns[j], it);
	}

	return Vec2f_init2(move, move);
}

typedef Vec2f GuiItemChart_drawFN(GuiItemChart* self, Image4* img, DbColumn** columns, Rgba* columnsCds, Rgba titleCd, const BIG N_COLS, BIG it, BIG it_old, const float jump, Quad2i area, double* values, double* values_old, float move, BOOL showValue, DbColumn* rowsX);

static GuiItemChart_drawFN* _GuiItemChart_getDrawType(GuiItemChart* self)
{
	GuiItemChart_TYPE type = DbValue_getNumber(&self->type);
	switch (type)
	{
		case GuiItemChart_COLUMN_NORMAL:		return &_GuiItemChart_draw_COLUMN_NORMAL;
		case GuiItemChart_COLUMN_STACK:			return &_GuiItemChart_draw_COLUMN_STACK;
		case GuiItemChart_COLUMN_STACK_PROC:	return &_GuiItemChart_draw_COLUMN_STACK_PROC;

		case GuiItemChart_PIE:					return 0;
			//case GuiItemChart_PIE_STACK:			return &_GuiItemChart_draw_PIE_STACK;

		case GuiItemChart_POINT:				return &_GuiItemChart_draw_POINT;
		case GuiItemChart_XY_POINT:				return &_GuiItemChart_draw_XY_POINT;
			//case GuiItemChart_XY_POINT_ORDERED:	return &_GuiItemChart_draw_XY_POINT_ORDERED;

		case GuiItemChart_LINE:					return &_GuiItemChart_draw_LINE;
		case GuiItemChart_XY_LINE:				return &_GuiItemChart_draw_XY_LINE;
		case GuiItemChart_XY_LINE_ORDERED:		return &_GuiItemChart_draw_XY_LINE_ORDERED;
	}
	return 0;
}

int _GuiItemChart_cmpFlt(const void* context, const void* a, const void* b)
{
	const DbColumn* self = context;

	double fa = DbColumn_getFlt(self, *(BIG*)a, 0);
	double fb = DbColumn_getFlt(self, *(BIG*)b, 0);
	return (fa > fb) - (fa < fb);
}

static void _GuiItemChart_drawRowX(GuiItemChart* self, Image4* img, DbColumn** rowsLines, BIG it, Vec2f move, Quad2i area)
{
	const int TITLE_SPACE_X = 4;
	const int cell = OsWinIO_cellSize();
	int textH = _GuiItem_textSize(1, cell);
	OsFont* font = OsWinIO_getFontDefault();

	int titleY = Quad2i_end(area).y + cell;

	int i;
	for (i = 0; i < GuiItemChart_numRowsLines(self); i++)
	{
		DbColumn_getStringCopyWithFormatLong(rowsLines[i], it, &self->tempStr);
		int px = area.start.x + move.x + (move.y - move.x) / 2;
		Image4_drawBoxQuad(img, Quad2i_init4(px, titleY + cell * i - cell / 2, 1, cell), self->base.front_cd);
		Image4_drawTextBackground(img, Vec2i_init2(px, titleY + cell * i), TRUE, font, self->tempStr.str, textH, 0, self->base.front_cd, self->base.back_cd, TITLE_SPACE_X);
	}
}

static void _GuiItemChart_drawColumn(GuiItemChart* self, Image4* img, DbColumn** rowsLines, DbColumn** columns, Rgba* columnsCds, Rgba titleCd, const BIG N_COLS, DbColumnN* rows, UBIG row, const float jump, Quad2i area, double* values, double* values_old, BOOL showValue, GuiItemChart_drawFN* func, const UBIG N_ROWS2)
{
	const double progressStart = area.start.x / (double)img->size.x;

	const int cell = OsWinIO_cellSize();
	int textH = _GuiItem_textSize(1, cell);
	OsFont* font = OsWinIO_getFontDefault();

	int titleY = Quad2i_end(area).y + cell;

	DbColumn* rowsX = GuiItemChart_getRowsColumn(self, 0);

	Vec2f move = Vec2f_init();
	UBIG i;

	BOOL isPIE = _GuiItemChart_isTypePIE(self);
	//BOOL isXY = _GuiItemChart_isTypeXY(self);
	//const int TITLE_SPACE_X = 4;

	//title
	if (!isPIE)// || isXY)
	{
		//background
		//Rgba cd = Rgba_aprox(g_theme.white, g_theme.main, 0.1f);
		//Image4_drawBoxQuad(img, Quad2i_init4(area.start.x, area.start.y + area.size.y + cell / 2, area.size.x, cell), cd);

		//mid line
		for (i = 0; i < GuiItemChart_numRowsLines(self); i++)
			Image4_drawBoxQuad(img, Quad2i_init4(area.start.x, titleY + i * cell, area.size.x, 1), self->base.front_cd);
	}

	if (GuiItemChart_isGroupCount(self))
	{
		move = func(self, img, columns, columnsCds, titleCd, N_COLS, row, -1, jump, area, values, values_old, move.y, showValue, 0);
		StdProgress_setExx("DRAWING", 1, 1, progressStart);
	}
	else
	{
		if (rowsX && _GuiItemChart_isTypeXY_ORDERED(self))
		{
			if (rows)
				DbColumnN_getArrayPoses(rows, row, &self->tempPoses);
			else
				DbRows_getArrayPoses(&self->filter, &self->tempPoses);
			Os_qsort(self->tempPoses.ptrs, self->tempPoses.num, sizeof(UBIG), _GuiItemChart_cmpFlt, rowsX);

			BIG it_old = -1;
			UBIG N_ROWS = self->tempPoses.num;
			for (i = 0; i < N_ROWS && StdProgress_is(); i++)
			{
				BIG it = self->tempPoses.ptrs[i];

				//graph
				move = func(self, img, columns, columnsCds, titleCd, N_COLS, it, it_old, jump, area, values, values_old, move.y, showValue, rowsX);

				//title
				_GuiItemChart_drawRowX(self, img, rowsLines, it, move, area);

				it_old = it;
				StdProgress_setExx("DRAWING", i, N_ROWS, progressStart);
			}
		}
		else
			if (_GuiItemChart_isTypePIE(self))
			{
				const float jmpX = N_COLS ? area.size.x / N_COLS : 0;

				const int rad = Std_min(jmpX, area.size.y - cell) / 2.1;

				BIG cl;
				for (cl = 0; cl < N_COLS && StdProgress_is(); cl++)
				{
					Rgba cd = columnsCds[cl];

					Quad2i ar = Quad2i_init4(area.start.x + cl * jmpX, area.start.y, jmpX, area.size.y);

					double sum = 0;
					double oldAngle = 0;

					if (rows)
					{
						i = 0;
						while (DbColumnN_jump(rows, row, &i, 1) >= 0 && StdProgress_is())
						{
							BIG it = DbColumnN_getIndex(rows, row, i);
							sum += Std_dmax(0, DbColumn_getFlt(columns[cl], it, 0));
							i++;
						}

						i = 0;
						while (DbColumnN_jump(rows, row, &i, 1) >= 0 && StdProgress_is())
						{
							BIG it = DbColumnN_getIndex(rows, row, i);

							double value = DbColumn_getFlt(columns[cl], it, 0);
							double newAngle = oldAngle + Std_dmax(0, Std_dmax(0, value / sum)) * (2 * M_PI);
							//graph
							Image4_drawCircleEx(img, Quad2i_getMiddle(ar), 0, rad, Rgba_getNextHue(&cd), ar, oldAngle, newAngle);
							//info
							_GuiItemChart_drawValueUNI(self, img, Image4_getCircleMid(Quad2i_getMiddle(ar), oldAngle, newAngle, rad / 1.5), showValue, titleCd, columns[cl], it);

							oldAngle = newAngle;
							StdProgress_setExx("DRAWING", cl * N_ROWS2 + i, N_COLS * N_ROWS2, progressStart);
							i++;
						}
					}
					else
					{
						UBIG N_ROWS = DbRows_getSize(&self->filter);
						for (i = 0; i < N_ROWS && StdProgress_is(); i++)
						{
							BIG it = DbRows_getRow(&self->filter, i);
							sum += Std_dmax(0, DbColumn_getFlt(columns[cl], it, 0));
						}

						for (i = 0; i < N_ROWS && StdProgress_is(); i++)
						{
							BIG it = DbRows_getRow(&self->filter, i);

							double value = DbColumn_getFlt(columns[cl], it, 0);
							double newAngle = oldAngle + Std_dmax(0, Std_dmax(0, value / sum)) * (2 * M_PI);
							//graph
							Image4_drawCircleEx(img, Quad2i_getMiddle(ar), 0, rad, Rgba_getNextHue(&cd), ar, oldAngle, newAngle);
							//info
							_GuiItemChart_drawValueUNI(self, img, Image4_getCircleMid(Quad2i_getMiddle(ar), oldAngle, newAngle, rad / 1.5), showValue, titleCd, columns[cl], it);

							oldAngle = newAngle;
							StdProgress_setExx("DRAWING", i, N_ROWS, progressStart);
						}
					}

					//column name
					DbColumn* column = GuiItemChart_getColumnsColumn(self, cl);
					if (column)
					{
						UNI name[64];
						Image4_drawText(&self->result, Vec2i_init2(ar.start.x + ar.size.x / 2, Quad2i_getMiddle(ar).y - rad - cell / 2), TRUE, font, DbColumn_getName(column, name, 64), textH, 0, self->base.front_cd);
					}
				}
			}
			else
				if (rows)
				{
					BIG it_old = -1;
					UBIG i = 0;
					while (DbColumnN_jump(rows, row, &i, 1) >= 0 && StdProgress_is())
					{
						BIG it = DbColumnN_getIndex(rows, row, i);

						//graph
						move = func(self, img, columns, columnsCds, titleCd, N_COLS, it, it_old, jump, area, values, values_old, move.y, showValue, 0);

						//title
						_GuiItemChart_drawRowX(self, img, rowsLines, it, move, area);

						it_old = it;
						StdProgress_setExx("DRAWING", i, N_ROWS2, progressStart);
						i++;
					}
				}
				else
				{
					BIG it_old = -1;
					UBIG N_ROWS = DbRows_getSize(&self->filter);
					for (i = 0; i < N_ROWS && StdProgress_is(); i++)
					{
						BIG it = DbRows_getRow(&self->filter, i);

						//graph
						move = func(self, img, columns, columnsCds, titleCd, N_COLS, it, it_old, jump, area, values, values_old, move.y, showValue, rowsX);

						//title
						_GuiItemChart_drawRowX(self, img, rowsLines, it, move, area);

						it_old = it;
						StdProgress_setExx("DRAWING", i, N_ROWS, progressStart);
					}
				}
	}
}

static Quad2i _GuiItemChart_getArea(GuiItemChart* self, Quad2i coord)
{
	//BOOL showTitle = GuiItemChart_getRowsColumn(self, 0) != 0;
	//showTitle |= _GuiItemChart_isTypeXY(self);

	const int cell = OsWinIO_cellSize();

	Quad2i area = coord;

	//top(legend)
	area.start.y += cell * 1.5f;
	area.size.y -= cell * 1.5f;

	//bottom
	area.size.y -= cell / 2;

	//if (showTitle)	//bottom
	area.size.y -= GuiItemChart_numRowsLines(self) * cell;

	area.size.y = Std_max(area.size.y, 0);

	return area;
}

static void _GuiItemChart_drawGraph(GuiItemChart* self, Quad2i coord, UBIG row, DbColumn** rowsLines, DbColumn** columns, Rgba* columnsCds, DbColumnN* rows)
{
	//Image4_drawBorder(&self->result, Quad2i_addSpace(coord, 2), 1, self->base.front_cd);
	//ligher or darker background color ...

	BOOL showColumnsValue = DbValue_getNumber(&self->showColumnsValue);

	BIG N_COLS = 0;
	while (columns[N_COLS])	N_COLS++;
	const BIG N_ROWS = GuiItemChart_isGroupCount(self) ? 1 : (rows ? DbColumnN_sizeActive(rows, row) : DbRows_getSize(&self->filter));
	const double N = _GuiItemChart_isTypeSingle(self) ? (N_ROWS * 2) : (N_ROWS * N_COLS + N_ROWS);

	//DbColumn* columnTitle = _GuiItemChart_isTypePIE(self) ? 0 : GuiItemChart_getRowsColumn(self, 0);

	Rgba titleCd = _GuiItemChart_getTextCd(self);

	double* values = Os_malloc(N_COLS * sizeof(double));
	double* values_old = Os_malloc(N_COLS * sizeof(double));

	Quad2i area = _GuiItemChart_getArea(self, coord);
	const float jump = N ? area.size.x / N : 0;

	_GuiItemChart_drawColumn(self, &self->result, rowsLines, columns, columnsCds, titleCd, N_COLS, rows, row, jump, area, values, values_old, showColumnsValue, _GuiItemChart_getDrawType(self), N_ROWS);

	Os_free(values, N_COLS * sizeof(double));
	Os_free(values_old, N_COLS * sizeof(double));

	//group vertical line(grey split)
	{
		const int numLines = _GuiItemChart_isTypePIE(self) ? 0 : GuiItemChart_numRowsLines(self);

		const int cell = OsWinIO_cellSize();
		Rgba cdT = Rgba_aprox(self->base.front_cd, self->base.back_cd, 0.8f);
		Image4_drawChessQuad(&self->result, Quad2i_init4(coord.start.x + coord.size.x, cell, 1, coord.size.y - cell - (numLines * cell)), Vec2i_init2(1, cell / 2), cdT);
	}
}

static double _GuiItemChart_getGroupCount(DbFilter* filter, UBIG row)
{
	return DbColumn_getFlt((DbColumn*)DbFilter_getColumnGroupCount(filter), row, 0);
}

static int _GuiItemChart_getAxisYWidth(GuiItemChart* self)
{
	return _GuiItemChart_isTypePIE(self) ? 0 : OsWinIO_cellSize() * 2;
}

static void _GuiItemChart_drawGraphGroup(GuiItemChart* self, DbFilter* filter, DbColumn** rowsLines, DbColumn** columns, Rgba* columnsCds, UBIG row, const Vec2i graph_size, UBIG* posX, UBIG bottomY, UBIG deep)
{
	DbColumnN* subs = DbFilter_getColumnGroupSubs(filter);
	DbColumnN* rows = DbFilter_getColumnGroupRows(filter);

	if (DbColumnN_sizeActive(subs, row) == 0)
	{
		//final graph
		Quad2i coord = Quad2i_init4(*posX, 0, graph_size.x, graph_size.y - bottomY);
		_GuiItemChart_drawGraph(self, coord, row, rowsLines, columns, columnsCds, rows);
		*posX += graph_size.x;
	}

	const double countMax = _GuiItemChart_getGroupCount(filter, row);

	//int group = 0;
	UBIG i = 0;
	while (DbColumnN_jump(subs, row, &i, 1) >= 0 && StdProgress_is())
	{
		BIG it = DbColumnN_getIndex(subs, row, i);

		UBIG posX_backup = *posX;

		const double countRatio = countMax ? _GuiItemChart_getGroupCount(filter, it) / countMax : 0;
		const UBIG W = Std_max(1, graph_size.x * countRatio);

		//subs
		_GuiItemChart_drawGraphGroup(self, filter, rowsLines, columns, columnsCds, it, Vec2i_init2(W, graph_size.y), posX, bottomY + OsWinIO_cellSize(), deep + 1);

		//vertical line(grey split)
		/*{
			DbColumn* columnTitle = GuiItemChart_getXTitleColumn(self);
			const int cell = OsWinIO_cellSize();
			const int width = _GuiItemChart_getAxisYWidth(self);

			//const startY = cell;

			Rgba cdT = Rgba_aprox(self->base.front_cd, self->base.back_cd, 0.5f);
			Image4_drawBoxQuad(&self->result, Quad2i_init4(*posX, cell, 1, graph_size.y - bottomY - cell - cell - (columnTitle?cell:0)), cdT);
		}*/

		//title
		{
			const int cell = OsWinIO_cellSize();
			int textH = _GuiItem_textSize(1, cell);
			OsFont* font = OsWinIO_getFontDefault();

			Vec2i mid = Vec2i_init2(posX_backup + W / 2, graph_size.y - bottomY - cell / 2);
			BIG titleRow = DbColumnN_getFirstRow(rows, it);
			DbColumn_getStringCopyWithFormatLong(GuiItemChart_getGroup(self, deep), titleRow, &self->tempStr);

			//line
			Quad2i line = Quad2i_init4(posX_backup, mid.y, W, 1);
			line = Quad2i_addSpaceX(line, cell / 4);
			Image4_drawBoxQuad(&self->result, line, self->base.front_cd);

			//background
			Quad2i back = Image4_drawTextCoord(mid, TRUE, font, self->tempStr.str, textH, 0);
			back = Quad2i_addSpaceX(back, -cell / 4);
			Image4_drawBoxQuad(&self->result, back, self->base.back_cd);

			//text
			const UNI* groupName = Std_sizeUNI(self->tempStr.str) ? self->tempStr.str : _UNI32("{ }");
			Image4_drawText(&self->result, mid, TRUE, font, groupName, textH, 0, self->base.front_cd);
		}

		i++;
	}
}

/*static UBIG _GuiItemChart_numGraphs(GuiItemChart* self, DbFilter* filter, UBIG row)
{
	UBIG n = 0;

	DbColumnN* subs = DbFilter_getColumnGroupSubs(filter);

	if (DbColumnN_sizeActive(subs, row) == 0)
		return 1;

	UBIG i = 0;
	while (DbColumnN_jump(subs, row, &i, 1) >= 0)
	{
		BIG it = DbColumnN_getIndex(subs, row, i);
		n += _GuiItemChart_numGraphs(self, filter, it);
	}
	return n;
}*/

static int _GuiItemChart_drawAxisY(GuiItemChart* self, Image4* img, Quad2i coord)
{
	const int width = _GuiItemChart_getAxisYWidth(self);
	if (width == 0)
		return width;

	const int cell = OsWinIO_cellSize();
	int textH = _GuiItem_textSize(1, cell);
	OsFont* font = OsWinIO_getFontDefault();

	Quad2i area = _GuiItemChart_getArea(self, coord);
	area.size.y -= GuiItemChart_numGroups(self) * cell;

	const double diffY = (self->maxY - self->minY);
	const float mult = diffY ? area.size.y / diffY : 0;
	const float zero = diffY ? area.size.y / diffY * self->minY : 0;
	int bottomY = area.start.y + area.size.y;

	//background
	Rgba cd = self->base.back_cd;
	Image4_drawBoxQuad(img, Quad2i_init4(coord.start.x, coord.start.y, width, coord.size.y), cd);

	//line
	Image4_drawBoxQuad(img, Quad2i_init4(area.start.x + width / 2, area.start.y, 1, area.size.y), self->base.front_cd);

	Vec2i p;
	p.x = coord.start.x + width / 2;
	{
		UNI nmbr[64];
		//lines
		if (mult)
		{
			Rgba cdT = Rgba_aprox(self->base.front_cd, self->base.back_cd, 0.5f);

			double jump = 0.0001;
			while (jump * mult < cell)
				jump *= 10;

			double i;
			for (i = +jump; i < self->maxY; i += jump)
			{
				Std_buildNumberUNI(i, self->precision, nmbr);
				p.y = bottomY + zero - (i * mult);
				Image4_drawBoxQuad(img, Quad2i_init4(p.x - width / 3, p.y, width / 3 * 2, 1), cdT);
				Image4_drawTextBackground(img, p, TRUE, font, nmbr, textH, 0, cdT, cd, 4);
			}

			for (i = -jump; i > self->minY; i -= jump)
			{
				Std_buildNumberUNI(i, self->precision, nmbr);
				p.y = bottomY + zero - (i * mult);
				Image4_drawBoxQuad(img, Quad2i_init4(p.x - width / 3, p.y, width / 3 * 2, 1), cdT);
				Image4_drawTextBackground(img, p, TRUE, font, nmbr, textH, 0, cdT, cd, 4);
			}
		}

		//zero(0)
		//if (Std_abs(self->maxY) > 1 && Std_abs(self->minY) > 1)
		{
			double v = 0;
			Std_buildNumberUNI(v, self->precision, nmbr);
			p.y = bottomY + zero;
			Image4_drawBoxQuad(img, Quad2i_init4(p.x - width / 3, p.y, width / 3 * 2, 1), self->base.front_cd);
			Image4_drawTextBackground(img, p, TRUE, font, nmbr, textH, 0, self->base.front_cd, cd, 4);
		}

		//text-top(max)
		Std_buildNumberUNI(self->maxY, self->precision, nmbr);
		p.y = bottomY + zero - (self->maxY * mult);
		Image4_drawBoxQuad(img, Quad2i_init4(p.x - width / 3, p.y, width / 3 * 2, 1), self->base.front_cd);
		Image4_drawTextBackground(img, p, TRUE, font, nmbr, textH, 0, self->base.front_cd, cd, 4);

		//text-min
		Std_buildNumberUNI(self->minY, self->precision, nmbr);
		p.y = bottomY + zero - (self->minY * mult);
		Image4_drawBoxQuad(img, Quad2i_init4(p.x - width / 3, p.y, width / 3 * 2, 1), self->base.front_cd);
		Image4_drawTextBackground(img, p, TRUE, font, nmbr, textH, 0, self->base.front_cd, cd, 4);
	}

	bottomY += cell;

	//title
	if (DbValue_getNumber(&self->showRowsValue))
	{
		int i;
		for (i = 0; i < GuiItemChart_numRows(self); i++)
		{
			DbColumn* column = GuiItemChart_getRowsColumn(self, i);
			if (column)
			{
				UNI name[64];
				DbColumn_getName(column, name, 64);
				p.y = bottomY + i * cell;
				Image4_drawText(img, p, TRUE, font, name, textH, 0, self->base.front_cd);
			}
		}
		bottomY += GuiItemChart_numRows(self) * cell;
	}

	//group titles
	int i;
	for (i = 0; i < GuiItemChart_numGroups(self); i++)
	{
		DbColumn* column = GuiItemChart_getGroup(self, i);
		if (column)
		{
			UNI name[64];
			DbColumn_getName(column, name, 64);
			p.y = bottomY + i * cell;
			Image4_drawText(img, p, TRUE, font, name, textH, 0, self->base.front_cd);
		}
	}

	return width;
}

static void _GuiItemChart_drawResult(GuiItemChart* self, DbFilter* filter, DbColumn** rowsLines, DbColumn** columns, Rgba* columnsCds)
{
	//double st = Os_time();

	const int cell = OsWinIO_cellSize();

	//legend
	{
		int textH = _GuiItem_textSize(1, cell);
		OsFont* font = OsWinIO_getFontDefault();
		Quad2i cdBox = Quad2i_init4(0, 0, cell * 2, cell);
		cdBox = Quad2i_addSpace(cdBox, cell / 5);

		int x = cell / 2;

		if (GuiItemChart_isGroupCount(self))
		{
			cdBox.start.x = x;
			Image4_drawBoxQuad(&self->result, cdBox, columnsCds[0]);
			x += cdBox.size.x + cell / 4;

			Image4_drawText(&self->result, Vec2i_init2(x, cell / 2), FALSE, font, Lang_find("CHART_COUNT"), textH, 0, self->base.front_cd);
		}
		else
		{
			const BIG N_COLS = GuiItemChart_numColumns(self);
			BIG i;
			for (i = 0; i < N_COLS; i++)
			{
				DbColumn* column = GuiItemChart_getColumnsColumn(self, i);
				if (column)
				{
					UNI name[64];

					cdBox.start.x = x;
					Image4_drawBoxQuad(&self->result, cdBox, GuiItemChart_getColumnsCd(self, i));
					x += cdBox.size.x + cell / 4;

					Image4_drawText(&self->result, Vec2i_init2(x, cell / 2), FALSE, font, DbColumn_getName(column, name, 64), textH, 0, self->base.front_cd);
					x += OsFont_getTextSizeX(font, name, textH) + cell;
				}
			}
		}
	}

	Quad2i coord = Quad2i_init4(0, 0, self->result.size.x, self->result.size.y);

	//background lines
	if (!_GuiItemChart_isTypePIE(self))
	{
		Rgba cdT = Rgba_aprox(self->base.front_cd, self->base.back_cd, 0.8f);
		Quad2i area = _GuiItemChart_getArea(self, coord);
		area.size.y -= GuiItemChart_numGroups(self) * cell;

		const double diffY = (self->maxY - self->minY);
		const float mult = diffY ? area.size.y / diffY : 0;
		const float zero = diffY ? area.size.y / diffY * self->minY : 0;
		int bottomY = area.start.y + area.size.y;

		int y;

		//zero(0)
		y = bottomY + zero;
		Image4_drawBoxQuad(&self->result, Quad2i_init4(area.start.x, y, area.size.x, 1), cdT);

		//text-top(max)
		y = bottomY + zero - (self->maxY * mult);
		Image4_drawBoxQuad(&self->result, Quad2i_init4(area.start.x, y, area.size.x, 1), cdT);

		//text-min
		y = bottomY + zero - (self->minY * mult);
		Image4_drawBoxQuad(&self->result, Quad2i_init4(area.start.x, y, area.size.x, 1), cdT);

		{
			int max_iter = 1000;
			double jump = 0.0001;
			while (jump * mult < cell && max_iter)
			{
				jump *= 10;
				max_iter--;
			}
			jump /= 2;

			double i;
			for (i = +jump; i < self->maxY; i += jump)
			{
				y = bottomY + zero - (i * mult);
				Image4_drawBoxQuad(&self->result, Quad2i_init4(area.start.x, y, area.size.x, 1), cdT);
			}

			for (i = -jump; i > self->minY; i -= jump)
			{
				y = bottomY + zero - (i * mult);
				Image4_drawBoxQuad(&self->result, Quad2i_init4(area.start.x, y, area.size.x, 1), cdT);
			}
		}
	}

	//graphs
	if (GuiItemChart_isGroupCount(self) || DbFilter_numGroupTable(filter))
	{
		//UBIG graphI = 0;
		UBIG posX = 0;
		if (DbFilter_numGroupTable(filter))
			_GuiItemChart_drawGraphGroup(self, filter, rowsLines, columns, columnsCds, 0, coord.size, &posX, 0, 0);
	}
	else
		_GuiItemChart_drawGraph(self, coord, 0, rowsLines, columns, columnsCds, 0);

	StdBigs_free(&self->tempPoses);

	//Os_printTime("graph", st);
}

static int _GuiItemChart_getMinMaxValues(GuiItemChart* self, DbFilter* filter, DbColumn*** out_columns, Rgba** out_columnsCds)
{
	const BOOL isGroupCount = GuiItemChart_isGroupCount(self);

	const BIG N_COLS = isGroupCount ? 1 : GuiItemChart_numColumns(self);
	DbColumn** columns = Os_calloc(N_COLS + 1, sizeof(DbColumn*));
	Rgba* columnsCds = Os_calloc(N_COLS + 1, sizeof(Rgba));

	self->minY = Std_maxDouble();
	self->maxY = Std_minDouble();

	if (isGroupCount)
	{
		columns[0] = (DbColumn*)DbFilter_getColumnGroupCount(filter);
		columnsCds[0] = N_COLS ? GuiItemChart_getColumnsCd(self, 0) : g_theme.black;
		DbFilter_getMinMaxCount(filter, &self->minY, &self->maxY);

		self->precision = 0;
	}
	else
	{
		int c = 0;
		BIG i;
		for (i = 0; i < N_COLS; i++)
		{
			DbColumn* column = GuiItemChart_getColumnsColumn(self, i);
			Rgba cd = GuiItemChart_getColumnsCd(self, i);

			if (column && DbColumn_isType1(column))
			{
				columns[c] = (DbColumn*)column;
				columnsCds[c] = cd;
				c++;
			}
		}

		if (_GuiItemChart_isTypeSum(self))
		{
			DbRows_getColumnsMinMax(&self->filter, columns, &self->minY, &self->maxY);
		}
		else
		{
			DbColumn** cols = columns;
			while (*cols)
			{
				DbRows_getColumnMinMax(&self->filter, *cols, &self->minY, &self->maxY);
				cols++;
			}
		}
	}

	if (self->minY == Std_maxDouble())	self->minY = 0;
	if (self->maxY == Std_minDouble())	self->maxY = 0;

	if (self->minY > 0)
		self->minY = 0;

	*out_columns = columns;
	*out_columnsCds = columnsCds;

	return N_COLS + 1;
}

static int _GuiItemChart_resize(GuiItemChart* self, DbFilter* filter, Quad2i coord, Win* win)
{
	int itemX = Std_max(0, coord.size.x - _GuiItemChart_getAxisYWidth(self));

	if (!self->fixed100)
	{
		switch ((UINT)DbValue_getNumber(&self->width))
		{
			case 0:	itemX *= 0.5f;	break;
			case 1:	itemX *= 1.0f;	break;
			case 2:	itemX *= 1.5f;	break;
			case 3:	itemX *= 2.0f;	break;
			case 4:	itemX *= 3.0f;	break;
		}
	}

	Vec2i size = Vec2i_init2(itemX, coord.size.y);

	//move up to make space for scroll
	if (size.x > coord.size.x - _GuiItemChart_getAxisYWidth(self))
		size.y = Std_max(0, size.y - GuiScroll_widthWin(win));

	Image4_resize(&self->result, size);
	Image4_drawBoxQuad(&self->result, Image4_getSizeQuad(&self->result), self->base.back_cd);

	return itemX;
}

void GuiItemChart_draw(GuiItemChart* self, Image4* img, Quad2i coord, Win* win)
{
	int widthAxis = _GuiItemChart_drawAxisY(self, img, Quad2i_init4(coord.start.x, coord.start.y, self->result.size.x, self->result.size.y));

	coord.start.x += widthAxis;
	coord.size.x -= widthAxis;

	img->rect.start.x += widthAxis;
	img->rect.size.x -= widthAxis;
	Image4_repairRect(img);

	{
		const UBIG WHEEL = GuiScroll_getWheelRow(&self->scrollH);
		Vec2i start = coord.start;
		start.x -= WHEEL;
		Image4_copyImage4(img, start, &self->result);
	}

	//scroll
	GuiScroll_set(&self->scrollH, self->result.size.x, coord.size.x, 1);
	const int scroll_width = GuiScroll_widthWin(win);
	if (GuiScroll_is(&self->scrollH))
		GuiScroll_drawH(&self->scrollH, Vec2i_init2(coord.start.x, coord.start.y + coord.size.y - scroll_width), img, win);
}

static DbFilter* _GuiItemChart_buildFilter(GuiItemChart* self)
{
	DbFilter* filter = DbFilter_new(self->viewRow);

	const UBIG N = GuiItemChart_numGroups(self);
	UBIG i;
	for (i = 0; i < N; i++)
	{
		DbColumn* column = GuiItemChart_getGroup(self, i);
		if (column)
			DbFilter_addGroup(filter, column, GuiItemChart_getGroupAscending(self, i), TRUE);
	}

	DbFilter_execute(filter);

	return filter;
}

void GuiItemChart_update(GuiItemChart* self, Quad2i coord, Win* win)
{
	_GuiItem_updateFinalCd(&self->base, GuiItemTheme_getWhite_Background(), g_theme.black, coord, win);

	BOOL changed = DbRows_hasChanged(&self->filter);
	changed |= DbValue_hasChanged(&self->type);

	if (Vec2i_isZero(self->result.size) || changed)
	{
		DbFilter* filter = _GuiItemChart_buildFilter(self);

		_GuiItemChart_resize(self, filter, coord, win);

		DbColumn** columns;
		Rgba* columnCds;
		const int num_columns = _GuiItemChart_getMinMaxValues(self, filter, &columns, &columnCds);

		self->minX = Std_maxDouble();
		self->maxX = Std_minDouble();
		DbColumn* rowsX = GuiItemChart_getRowsColumn(self, 0);
		if (rowsX)
			DbRows_getColumnMinMax(&self->filter, rowsX, &self->minX, &self->maxX);
		if (self->minX == Std_maxDouble())	self->minX = 0;
		if (self->maxX == Std_minDouble())	self->maxX = 0;

		const int num_rows = GuiItemChart_numRowsLines(self);
		DbColumn** rowsLines = Os_calloc(num_rows, sizeof(DbColumn*));
		int i;
		for (i = 0; i < GuiItemChart_numRowsLines(self); i++)
			rowsLines[i] = GuiItemChart_getRowsColumn(self, i);

		_GuiItemChart_drawResult(self, filter, rowsLines, columns, columnCds);

		Os_free(rowsLines, num_rows * sizeof(DbColumn*));
		Os_free(columns, num_columns * sizeof(DbColumn*));
		Os_free(columnCds, num_columns * sizeof(Rgba));
		DbFilter_delete(filter);
	}

	GuiItem_setRedraw(&self->base, changed || GuiScroll_getRedrawAndReset(&self->scrollH));
}

void GuiItemChart_touch(GuiItemChart* self, Quad2i coord, Win* win)
{
	Rgba back_cd = GuiItemTheme_getWhite_Background();
	Rgba front_cd = g_theme.black;

	const int scroll_width = GuiScroll_widthWin(win);
	GuiScroll_touchH(&self->scrollH, self, coord, Vec2i_init2(coord.start.x, coord.start.y + coord.size.y - scroll_width), win, FALSE);

	_GuiItem_updateFinalCd(&self->base, back_cd, front_cd, coord, win);
}

Image4 GuiItemChart_renderImage(GuiItemChart* self, Vec2i size, Win* win)
{
	Image4 img = Image4_initSize(size);

	self->fixed100 = TRUE;

	Quad2i coord = Quad2i_init2(Vec2i_init(), size);
	GuiItemChart_touch(self, coord, win);
	GuiItemChart_update(self, coord, win);
	GuiItemChart_draw(self, &img, coord, win);

	self->fixed100 = FALSE;

	/*const int axisWidth = _GuiItemChart_getAxisYWidth(self);

	coord.size = Vec2i_init2(axisWidth + self->result.size.x, self->result.size.y);
	Image4 img = Image4_initSize(coord.size);

	//draw axis
	int widthAxis = _GuiItemChart_drawAxisY(self, &img, coord);

	//draw graph
	Image4_copyImage4(&img, Vec2i_init2(widthAxis, 0), &self->result);*/

	return img;
}

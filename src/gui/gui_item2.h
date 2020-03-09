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

BOOL GuiItem_hasScroll(const GuiItem* self)
{
	if (self->type == GuiItem_LAYOUT)
		return GuiItemLayout_hasScrollV((GuiItemLayout*)self);

	return (self->type == GuiItem_TABLE || self->type == GuiItem_LIST || self->type == GuiItem_TODO);
}

BOOL GuiItem_hasScrollLayoutV(const GuiItem* self)
{
	return(self->type == GuiItem_LAYOUT) ? GuiItemLayout_hasScrollV((GuiItemLayout*)self) : FALSE;
}

BOOL GuiItem_hasScrollLayoutH(const GuiItem* self)
{
	return(self->type == GuiItem_LAYOUT) ? GuiItemLayout_hasScrollH((GuiItemLayout*)self) : FALSE;
}

GuiItem* GuiItem_newCopy(GuiItem* src, BOOL copySub)
{
	if (!src)
		return 0;

	switch (src->type)
	{
		case GuiItem_BOX: return GuiItemBox_newCopy((GuiItemBox*)src, copySub);
		case GuiItem_TEXT: return GuiItemText_newCopy((GuiItemText*)src, copySub);
		case GuiItem_TEXT_MULTI:return GuiItemTextMulti_newCopy((GuiItemTextMulti*)src, copySub);
		case GuiItem_BUTTON: return GuiItemButton_newCopy((GuiItemButton*)src, copySub);
		case GuiItem_EDIT: return GuiItemEdit_newCopy((GuiItemEdit*)src, copySub);
		case GuiItem_CHECK: return GuiItemCheck_newCopy((GuiItemCheck*)src, copySub);
		case GuiItem_SLIDER: return GuiItemSlider_newCopy((GuiItemSlider*)src, copySub);
		case GuiItem_RATING: return GuiItemRating_newCopy((GuiItemRating*)src, copySub);
		case GuiItem_COMBO_STATIC: return GuiItemComboStatic_newCopy((GuiItemComboStatic*)src, copySub);
		case GuiItem_COMBO_DYNAMIC: return GuiItemComboDynamic_newCopy((GuiItemComboDynamic*)src, copySub);
		case GuiItem_MENU: return GuiItemMenu_newCopy((GuiItemMenu*)src, copySub);
		case GuiItem_TAGS: return GuiItemTags_newCopy((GuiItemTags*)src, copySub);
		case GuiItem_TABLE: return GuiItemTable_newCopy((GuiItemTable*)src, copySub);
		case GuiItem_GROUP: return GuiItemGroup_newCopy((GuiItemGroup*)src, copySub);
		case GuiItem_LAYOUT: return GuiItemLayout_newCopy((GuiItemLayout*)src, copySub);
			//case GuiItem_DESIGN: return GuiItemDesign_newCopy((GuiItemDesign*)src, copySub);
		case GuiItem_LIST: return GuiItemList_newCopy((GuiItemList*)src, copySub);
			//case GuiItem_TODO: return GuiItemTodo_newCopy((GuiItemTodo*)src, copySub);
			//case GuiItem_KANBAN: return GuiItemKanban_newCopy((GuiItemKanban*) src, copySub);
		case GuiItem_CHART: return GuiItemChart_newCopy((GuiItemChart*)src, copySub);
		case GuiItem_CALENDAR_SMALL: return GuiItemCalendarSmall_newCopy((GuiItemCalendarSmall*)src, copySub);
		case GuiItem_CALENDAR_BIG: return GuiItemCalendarBig_newCopy((GuiItemCalendarBig*)src, copySub);
			//case GuiItem_TIMELINE: return GuiItemTimeline_newCopy((GuiItemTimeline*)src, copySub);
		case GuiItem_PARTICLES: return 0;
		case GuiItem_FILE: return GuiItemFile_newCopy((GuiItemFile*)src, copySub);
		case GuiItem_SWITCH: return GuiItemSwitch_newCopy((GuiItemSwitch*)src, copySub);
		case GuiItem_MAP: return GuiItemMap_newCopy((GuiItemMap*)src, copySub);
		case GuiItem_COLOR: return GuiItemColor_newCopy((GuiItemColor*)src, copySub);

		case GuiItem_LEVEL: return GuiItemLevel_newCopy((GuiItemLevel*)src, copySub);

		default:
		break;
	}
	return 0;
}

void GuiItem_closeParentLevel(GuiItem* self)
{
	GuiItem* parent = self->parent;

	if (self->type == GuiItem_LEVEL)
		GuiItemLevel_close((GuiItemLevel*)self);

	if (parent)
		GuiItem_closeParentLevel(parent);
}

BIG GuiItem_getRow(GuiItem* self)
{
	switch (self->type)
	{
		case GuiItem_BOX:
		return DbValue_getRow(&((GuiItemBox*)self)->value);

		case GuiItem_TEXT:
		return DbValue_getRow(&((GuiItemText*)self)->text);

		case GuiItem_TEXT_MULTI:
		return DbValue_getRow(&((GuiItemTextMulti*)self)->text);

		case GuiItem_BUTTON:
		return DbValue_getRow(&((GuiItemButton*)self)->text);

		case GuiItem_EDIT:
		return DbValue_getRow(&((GuiItemEdit*)self)->text);

		case GuiItem_CHECK:
		return DbValue_getRow(&((GuiItemCheck*)self)->value);

		case GuiItem_SLIDER:
		return DbValue_getRow(&((GuiItemSlider*)self)->value);
		case GuiItem_RATING:
		return DbValue_getRow(&((GuiItemRating*)self)->value);

		case GuiItem_COMBO_STATIC:
		return DbValue_getRow(&((GuiItemComboStatic*)self)->value);

		case GuiItem_COMBO_DYNAMIC:
		return DbRows_getBaseRow(&((GuiItemComboDynamic*)self)->value);

		case GuiItem_MENU:
		return GuiItemMenu_getRow((GuiItemMenu*)self);

		case GuiItem_TAGS:
		return GuiItemTags_getRow((GuiItemTags*)self);

		case GuiItem_TABLE:
		return GuiItemTable_getBaseRow((GuiItemTable*)self);

		case GuiItem_GROUP:
		return GuiItemGroup_getBaseRow((GuiItemGroup*)self);

		case GuiItem_KANBAN:
		break;

		case GuiItem_CHART:
		return GuiItemChart_getRow((GuiItemChart*)self);
		break;

		case GuiItem_CALENDAR_SMALL:
		break;

		case GuiItem_CALENDAR_BIG:
		break;

		case GuiItem_TIMELINE:
		break;

		case GuiItem_LAYOUT:
		return GuiItemLayout_getRow((GuiItemLayout*)self);

		case GuiItem_DESIGN:
		break;

		case GuiItem_LIST:
		return GuiItemList_getRow((GuiItemList*)self);

		//case GuiItem_TODO:
		//return GuiItemTodo_getRow((GuiItemTodo*)self);

		case GuiItem_PARTICLES:
		break;

		case GuiItem_FILE:
		return ((GuiItemFile*)self)->info.row;

		case GuiItem_SWITCH:
		break;

		case GuiItem_MAP:
		return GuiItemMap_getBaseRow((GuiItemMap*)self);

		case GuiItem_COLOR:
		return DbValue_getRow(&((GuiItemColor*)self)->value);

		case GuiItem_LEVEL:
		break;

		default:
		break;
	}
	return -1;
}

void GuiItem_setDropRow(GuiItem* self, BIG row)
{
	if (self->dropMove.column)
		DbRows_setBaseRow(&self->dropMove, row);
	DbRows_setBaseRow(&self->dropMoveIn, row);

	BIG i;
	for (i = 0; i < self->subs.num; i++)
		GuiItem_setDropRow(self->subs.ptrs[i], row);
}

void GuiItem_setRow(GuiItem* self, BIG row, UBIG index)
{
	if (!self->changeRow)
		return;

	BOOL isIcon = (self->iconCallback && GuiItem_getRow(self) != row);

	switch (self->type)
	{
		case GuiItem_BOX:
		DbValue_setRow(&((GuiItemBox*)self)->value, row, index);
		break;

		case GuiItem_TEXT:

		DbValue_setRow(&((GuiItemText*)self)->text, row, index);
		DbValue_setRow(&((GuiItemText*)self)->description, row, index);
		break;

		case GuiItem_TEXT_MULTI:
		DbValue_setRow(&((GuiItemTextMulti*)self)->text, row, index);
		break;

		case GuiItem_BUTTON:
		DbValue_setRow(&((GuiItemButton*)self)->text, row, index);
		break;

		case GuiItem_EDIT:
		DbValue_setRow(&((GuiItemEdit*)self)->text, row, index);
		DbValue_setRow(&((GuiItemEdit*)self)->description, row, index);
		break;

		case GuiItem_CHECK:
		DbValue_setRow(&((GuiItemCheck*)self)->value, row, index);
		DbValue_setRow(&((GuiItemCheck*)self)->description, row, index);
		break;

		case GuiItem_SLIDER:
		DbValue_setRow(&((GuiItemSlider*)self)->value, row, index);
		DbValue_setRow(&((GuiItemSlider*)self)->description, row, index);
		DbValue_setRow(&((GuiItemSlider*)self)->left, row, index);
		DbValue_setRow(&((GuiItemSlider*)self)->right, row, index);
		break;

		case GuiItem_RATING:
		DbValue_setRow(&((GuiItemRating*)self)->value, row, index);
		DbValue_setRow(&((GuiItemRating*)self)->description, row, index);
		break;

		case GuiItem_COMBO_STATIC:
		DbValue_setRow(&((GuiItemComboStatic*)self)->value, row, index);
		break;

		case GuiItem_COMBO_DYNAMIC:
		DbRows_setBaseRow(&((GuiItemComboDynamic*)self)->value, row);
		break;

		case GuiItem_MENU:
		GuiItemMenu_setRow((GuiItemMenu*)self, row, index);
		break;

		case GuiItem_TAGS:
		GuiItemTags_setRow((GuiItemTags*)self, row);
		break;

		case GuiItem_TABLE:
		GuiItemTable_setBaseRow((GuiItemTable*)self, row);
		break;

		case GuiItem_GROUP:
		GuiItemGroup_setBaseRow((GuiItemGroup*)self, row);
		break;

		case GuiItem_KANBAN:
		break;

		case GuiItem_CHART:
		GuiItemChart_setRow((GuiItemChart*)self, row);
		break;

		case GuiItem_CALENDAR_SMALL:
		break;

		case GuiItem_CALENDAR_BIG:
		break;

		case GuiItem_TIMELINE:
		break;

		case GuiItem_LAYOUT:
		GuiItemLayout_setRow((GuiItemLayout*)self, row, index);
		break;

		case GuiItem_DESIGN:
		break;

		case GuiItem_LIST:
		GuiItemList_setRow((GuiItemList*)self, row);
		break;

		case GuiItem_TODO:
		//GuiItemTodo_setRow((GuiItemTodo*)self, row);
		break;

		case GuiItem_PARTICLES:
		break;

		case GuiItem_FILE:
		DbValue_setRow(&((GuiItemFile*)self)->info, row, index);
		break;

		case GuiItem_SWITCH:
		break;
		case GuiItem_MAP:
		GuiItemMap_setBaseRow((GuiItemMap*)self, row);
		break;

		case GuiItem_COLOR:
		DbValue_setRow(&((GuiItemColor*)self)->value, row, index);
		break;

		case GuiItem_LEVEL:
		break;

		default:
		break;
	}

	if (isIcon)
		self->iconCallback(self);

	if (self->type != GuiItem_TABLE && self->type != GuiItem_LIST && self->type != GuiItem_LEVEL)
	{
		BIG i;
		for (i = 0; i < self->subs.num; i++)
			GuiItem_setRow(self->subs.ptrs[i], row, index);
	}
}

void GuiItem_resize(GuiItem* self, GuiItemLayout* layout, Win* win)
{
	if (self->resize)
	{
		if (self != &layout->base)
		{
			//this is only for Layout, others don't have Rows/Columns weights
			self->coordScreen = GuiItemLayout_convert(layout, OsWinIO_cellSize(), self->grid); //computes real_coord from grid_pos
			self->coordScreen.start = Vec2i_add(layout->base.coordScreen.start, self->coordScreen.start); //relative to parent
		}

		self->coordMove = self->coordMoveCut = self->coordScreen;
	}

	switch (self->type)
	{
		case GuiItem_LAYOUT:
		case GuiItem_DESIGN:
		layout = GuiItemLayout_resize((GuiItemLayout*)self, layout, win);
		break;

		case GuiItem_TABLE:
		layout = GuiItemTable_resize((GuiItemTable*)self, layout, win);
		break;

		case GuiItem_GROUP:
		layout = GuiItemGroup_resize((GuiItemGroup*)self, layout, win);
		break;

		case GuiItem_KANBAN:
		//layout = GuiItemKanban_resize((GuiItemKanban*)self, layout, win);
		break;

		case GuiItem_CALENDAR_SMALL:
		layout = GuiItemCalendarSmall_resize((GuiItemCalendarSmall*)self, layout, win);
		break;

		case GuiItem_LIST:
		layout = GuiItemList_resize((GuiItemList*)self, layout, win);
		break;

		case GuiItem_SWITCH:
		GuiItemSwitch_resize((GuiItemSwitch*)self, layout, win);
		break;

		case GuiItem_LEVEL:
		GuiItemLevel_resize((GuiItemLevel*)self, layout, win);
		break;

		case GuiItem_COLOR:
		layout = GuiItemColor_resize((GuiItemColor*)self, layout, win);
		break;

		case GuiItem_TAGS:
		layout = GuiItemTags_resize((GuiItemTags*)self, layout, win);
		break;

		case GuiItem_MAP:
		layout = GuiItemMap_resize((GuiItemMap*)self, layout, win);
		break;
		
		case GuiItem_FILE:
		layout = GuiItemFile_resize((GuiItemFile*)self, layout, win);
		break;
	
		default:
		break;
	}

	if (self->resize)
		GuiItem_update(self, win);	//items visibility

	BIG i;
	for (i = 0; i < self->subs.num; i++)
		GuiItem_resize(self->subs.ptrs[i], layout, win);

	self->resize = FALSE;
}

void GuiItem_delete(GuiItem* self)
{
	switch (self->type)
	{
		case GuiItem_BOX: GuiItemBox_delete((GuiItemBox*)self);
			break;
		case GuiItem_TEXT: GuiItemText_delete((GuiItemText*)self);
			break;
		case GuiItem_TEXT_MULTI:GuiItemTextMulti_delete((GuiItemTextMulti*)self);
			break;
		case GuiItem_BUTTON: GuiItemButton_delete((GuiItemButton*)self);
			break;
		case GuiItem_EDIT: GuiItemEdit_delete((GuiItemEdit*)self);
			break;
		case GuiItem_CHECK: GuiItemCheck_delete((GuiItemCheck*)self);
			break;
		case GuiItem_SLIDER: GuiItemSlider_delete((GuiItemSlider*)self);
			break;
		case GuiItem_RATING: GuiItemRating_delete((GuiItemRating*)self);
			break;
		case GuiItem_COMBO_STATIC: GuiItemComboStatic_delete((GuiItemComboStatic*)self);
			break;
		case GuiItem_COMBO_DYNAMIC: GuiItemComboDynamic_delete((GuiItemComboDynamic*)self);
			break;
		case GuiItem_MENU: GuiItemMenu_delete((GuiItemMenu*)self);
			break;
		case GuiItem_TAGS: GuiItemTags_delete((GuiItemTags*)self);
			break;

		case GuiItem_TABLE:
		GuiItemTable_delete((GuiItemTable*)self);
		break;
		case GuiItem_GROUP:
		GuiItemGroup_delete((GuiItemGroup*)self);
		break;

		case GuiItem_KANBAN:
		//GuiItemKanban_delete((GuiItemKanban*) self);
		break;
		case GuiItem_CHART:
		GuiItemChart_delete((GuiItemChart*)self);
		break;
		case GuiItem_CALENDAR_SMALL: GuiItemCalendarSmall_delete((GuiItemCalendarSmall*)self);
			break;
		case GuiItem_CALENDAR_BIG: GuiItemCalendarBig_delete((GuiItemCalendarBig*)self);
			break;
			//case GuiItem_TIMELINE: GuiItemTimeline_delete((GuiItemTimeline*)self);
			//	break;

		case GuiItem_LAYOUT: GuiItemLayout_delete((GuiItemLayout*)self);
			break;
			//case GuiItem_DESIGN: GuiItemDesign_delete((GuiItemDesign*)self);
			//	break;
		case GuiItem_LIST: GuiItemList_delete((GuiItemList*)self);
			break;
			//case GuiItem_TODO: GuiItemTodo_delete((GuiItemTodo*)self);
			//	break;
		case GuiItem_PARTICLES: GuiItemParticles_delete((GuiItemParticles*)self);
			break;
		case GuiItem_FILE: GuiItemFile_delete((GuiItemFile*)self);
			break;

		case GuiItem_SWITCH: GuiItemSwitch_delete((GuiItemSwitch*)self);
			break;
		case GuiItem_MAP:	GuiItemMap_delete((GuiItemMap*)self);
			break;
		case GuiItem_COLOR: GuiItemColor_delete((GuiItemColor*)self);
			break;
		case GuiItem_LEVEL: GuiItemLevel_delete((GuiItemLevel*)self);
			break;
		default:
		break;
	}

	OsWinIO_tryRemoveCursorGuiItem(self);
}

void GuiItem_update(GuiItem* self, Win* win)
{
	if (!self->show || self->remove)
		return;

	if (Quad2i_isZero(self->coordMoveCut))
		return;

	Quad2i coord = self->coordMove;

	if (self->icon)
		GuiImage_update(self->icon, GuiItem_getIconCoord(&coord).size);

	_GuiItem_updateEnable(self);

	switch (self->type)
	{
		case GuiItem_BOX:
		GuiItemBox_update((GuiItemBox*)self, coord, win);
		break;
		case GuiItem_TEXT: GuiItemText_update((GuiItemText*)self, coord, win);
			break;
		case GuiItem_TEXT_MULTI:GuiItemTextMulti_update((GuiItemTextMulti*)self, coord, win);
			break;
		case GuiItem_BUTTON: GuiItemButton_update((GuiItemButton*)self, coord, win);
			break;
		case GuiItem_EDIT: GuiItemEdit_update((GuiItemEdit*)self, coord, win);
			break;
		case GuiItem_CHECK: GuiItemCheck_update((GuiItemCheck*)self, coord, win);
			break;
		case GuiItem_SLIDER: GuiItemSlider_update((GuiItemSlider*)self, coord, win);
			break;
		case GuiItem_RATING: GuiItemRating_update((GuiItemRating*)self, coord, win);
			break;
		case GuiItem_COMBO_STATIC: GuiItemComboStatic_update((GuiItemComboStatic*)self, coord, win);
			break;
		case GuiItem_COMBO_DYNAMIC: GuiItemComboDynamic_update((GuiItemComboDynamic*)self, coord, win);
			break;
		case GuiItem_MENU: GuiItemMenu_update((GuiItemMenu*)self, coord, win);
			break;
		case GuiItem_TAGS: GuiItemTags_update((GuiItemTags*)self, coord, win);
			break;

		case GuiItem_TABLE:
		GuiItemTable_update((GuiItemTable*)self, coord, win);
		break;
		case GuiItem_GROUP:
		GuiItemGroup_update((GuiItemGroup*)self, coord, win);
		break;

		case GuiItem_KANBAN:
		//GuiItemKanban_update((GuiItemKanban*) self, coord, win);
		break;
		case GuiItem_CHART:
		GuiItemChart_update((GuiItemChart*)self, coord, win);
		break;
		case GuiItem_CALENDAR_SMALL: GuiItemCalendarSmall_update((GuiItemCalendarSmall*)self, coord, win);
			break;
		case GuiItem_CALENDAR_BIG: GuiItemCalendarBig_update((GuiItemCalendarBig*)self, coord, win);
			break;
			//case GuiItem_TIMELINE: GuiItemTimeline_update((GuiItemTimeline*)self, coord, win);
			//	break;

		case GuiItem_LIST: GuiItemList_update((GuiItemList*)self, coord, win);
			break;
			//case GuiItem_TODO: GuiItemTodo_update((GuiItemTodo*)self, coord, win);
			//	break;
		case GuiItem_LAYOUT: GuiItemLayout_update((GuiItemLayout*)self, coord, win);
			break;
			//case GuiItem_DESIGN: GuiItemDesign_update((GuiItemDesign*)self, coord, win);
			//	break;
		case GuiItem_PARTICLES: GuiItemParticles_update((GuiItemParticles*)self, coord, win);
			break;
		case GuiItem_FILE: GuiItemFile_update((GuiItemFile*)self, coord, win);
			break;

		case GuiItem_SWITCH: GuiItemSwitch_update((GuiItemSwitch*)self, coord, win);
			break;
		case GuiItem_MAP:	GuiItemMap_update((GuiItemMap*)self, coord, win);
			break;
		case GuiItem_COLOR: GuiItemColor_update((GuiItemColor*)self, coord, win);
			break;
		case GuiItem_LEVEL: GuiItemLevel_update((GuiItemLevel*)self, coord, win);
			break;
		default:
		break;
	}

	int i;
	for (i = 0; i < self->subs.num; i++)
		GuiItem_update(self->subs.ptrs[i], win);
}

void GuiItem_updateCoord(GuiItem* self, Vec2i layoutMove, Quad2i parentRect, Win* win)
{
	if (!self->show || self->remove)
		return;

	if (self->type == GuiItem_LEVEL)
	{
		self->coordMove = self->coordScreen;
		self->coordMoveCut = self->coordMove;
	}
	else
	{
		self->coordMove.start = Vec2i_add(self->coordScreen.start, layoutMove);
		self->coordMove.size = self->coordScreen.size;

		self->coordMoveCut = Quad2i_getIntersect(self->coordMove, parentRect);
	}

	if (self->type == GuiItem_LAYOUT)
	{
		layoutMove.x -= GuiScroll_getWheel(&((GuiItemLayout*)self)->scrollH);
		layoutMove.y -= GuiScroll_getWheel(&((GuiItemLayout*)self)->scrollV);
	}

	int i;
	for (i = 0; i < self->subs.num; i++)
		GuiItem_updateCoord(self->subs.ptrs[i], layoutMove, self->coordMoveCut, win);
}

BOOL g_drag_active = FALSE;
BOOL g_drag_dontRemove;
BIG g_drag_movingRow = -1;
char* g_drag_nameSrc = 0;
DbRows g_drag_rows;

BOOL g_changeSizeActive = FALSE;

BIG GuiItem_getDropRow(GuiItem* self)
{
	BIG dstRow = GuiItem_findAttributeNoWarning(self, "dropRow");
	if (dstRow < 0)
		dstRow = GuiItem_getRow(self);
	return dstRow;
}

GuiItem* GuiItem_findOrigDrag(GuiItem* self)
{
	int i;
	for (i = 0; i < self->subs.num; i++)
	{
		GuiItem* t = GuiItem_findOrigDrag(self->subs.ptrs[i]);
		if (t)
			return t;
	}

	return (self->dropMove.column == g_drag_rows.column && self->dropMove.row == g_drag_rows.row && GuiItem_getDropRow(self) == g_drag_movingRow) ? self : 0;
}

void GuiItem_touchPrior(GuiItem* self, Win* win)
{
	if (!self->show || self->remove)
		return;

	if (Quad2i_isZero(self->coordMoveCut) && !self->changeSizeActive)
		return;

	Quad2i coord = self->coordMoveCut;

	int i;
	for (i = 0; i < self->subs.num; i++)
		GuiItem_touchPrior(self->subs.ptrs[i], win);

	const BOOL active = (g_drag_active || g_changeSizeActive);

	//auto-scroll: if touch position is on edge
	if (active)
	{
		GuiItemList* list = (self->type == GuiItem_LIST && Quad2i_inside(coord, OsWinIO_getTouchPos())) ? (GuiItemList*)self : 0;
		if (list)
		{
			if (GuiScroll_dragScrollV(&list->scroll, list->base.coordMove))
			{
				GuiItem_update(&list->base, win);
				GuiItem_setRedraw(&list->base, TRUE);
			}
		}

		GuiItemLayout* layout = (self->type == GuiItem_LAYOUT && Quad2i_inside(coord, OsWinIO_getTouchPos())) ? (GuiItemLayout*)self : 0;
		if (layout)
		{
			if ((layout->showScrollV && GuiScroll_dragScrollV(&layout->scrollV, layout->base.coordMove)) ||
				(layout->showScrollH && GuiScroll_dragScrollH(&layout->scrollH, layout->base.coordMove)))
				GuiItem_setRedraw(&layout->base, TRUE);
		}
	}

	if (self->touch && GuiItem_isEnable(self) && (OsWinIO_canActiveRenderItem(self) || active))
	{
		BOOL startTouch = (OsWinIO_getTouch_action() == Win_TOUCH_DOWN_S) || (OsWinIO_getTouch_action() == Win_TOUCH_FORCE_DOWN_S);
		BOOL endTouch = (OsWinIO_getTouch_action() == Win_TOUCH_DOWN_E) || (OsWinIO_getTouch_action() == Win_TOUCH_FORCE_DOWN_E);

		if (DbValue_getRow(&self->changeSizeValue) >= 0)
		{
			BOOL active = self->changeSizeActive;
			BOOL touch = startTouch || active;
			BOOL inside = Quad2i_inside(GuiItem_getChangeSizeRect(self), OsWinIO_getTouchPos());

			if (startTouch && inside)
			{
				self->changeSizeActive = TRUE;
				g_changeSizeActive = TRUE;
			}

			if (inside && touch) //full touch
				OsWinIO_setActiveRenderItem(self);

			if (active)
				GuiItemRoot_setDrawRectOver(GuiItem_getChangeSizeRectMove(self, OsWinIO_getTouchPos()));

			if (active && endTouch) //end
			{
				GuiItemEdit_saveCache();

				DbValue_setNumber(&self->changeSizeValue, GuiItem_getChangeSizePos(self, OsWinIO_getTouchPos()));
			}

			//cursor
			if (inside || active)
			{
				Win_updateCursor(win, self->changeSizeVertical ? Win_CURSOR_COL_RESIZE : Win_CURSOR_ROW_RESIZE);
			}
			if (active && endTouch)
			{
				OsWinIO_resetActiveRenderItem();
				self->changeSizeActive = FALSE;
			}
			if (g_changeSizeActive && endTouch)
				g_changeSizeActive = FALSE;
		}

		if (DbRows_is(&self->dropMove) && (g_drag_active || self->dropMoveNameSrc))
		{
			Quad2i iconCoord = coord;
			if (self->icon)
				iconCoord = GuiItem_getIconCoord(&coord);

			BOOL touch = startTouch || g_drag_active;
			BOOL inside = Quad2i_inside(iconCoord, OsWinIO_getTouchPos());

			if (startTouch && inside && Std_sizeCHAR(self->dropMoveNameSrc))
			{
				g_drag_active = TRUE;
				g_drag_dontRemove = self->dropDontRemove;
				g_drag_movingRow = GuiItem_getDropRow(self);
				g_drag_nameSrc = Std_newCHAR(self->dropMoveNameSrc);
				g_drag_rows = DbRows_initCopy(&self->dropMove);
			}

			if (inside && touch) //full touch
			{
				OsWinIO_setActiveRenderItem(self);	//only deactivate mouseOver for Button, EditBox etc., otherwise not use

				if (OsWinIO_getTouchNum() >= 2)	//double-click
				{
					if (self->iconDoubleTouch)
						self->iconDoubleTouch(self);
					OsWinIO_resetNumTouch();
				}
			}

			if (g_drag_active)
			{
				BIG dstRow = -1;

				GuiItem* parent = GuiItem_getBaseParent(self);
				BOOL findIn;
				GuiItem* find = GuiItem_findDropName(parent, g_drag_nameSrc, OsWinIO_getTouchPos(), 0, &findIn);
				if (find)
				{
					dstRow = GuiItem_getDropRow(find);

					//ignore self
					if (find->dropMove.column == g_drag_rows.column && find->dropMove.row == g_drag_rows.row && dstRow == g_drag_movingRow)
						find = 0;
				}

				//set draw
				{
					if (find)	//dst
						GuiItemRoot_setDrawRectOver(GuiItem_getDropRect(find, OsWinIO_getTouchPos(), findIn));

					//src
					GuiItem* src = GuiItem_findOrigDrag(parent);
					if (src)
						GuiItemRoot_setDrawRectOver2(src->coordMoveCut);
				}

				if (endTouch && find) //end
				{
					GuiItemEdit_saveCache();

					if (!g_drag_dontRemove && !find->dropDontRemove)
						DbRows_removeRow(&g_drag_rows, g_drag_movingRow);

					if (find->dropCallback)
						find->dropCallback(find, dstRow, g_drag_movingRow, findIn);

					if (findIn)
					{
						DbRows_addLinkRow(&find->dropMoveIn, g_drag_movingRow);
					}
					else
					if(dstRow >= 0)
					{
						if (GuiItem_isDropBefore(find, OsWinIO_getTouchPos()))
							DbRows_insertIDbefore(&find->dropMove, g_drag_movingRow, dstRow);
						else
							DbRows_insertIDafter(&find->dropMove, g_drag_movingRow, dstRow);
					}
				}
			}

			//cursor
			if (inside || g_drag_active)
			{
				Win_updateCursor(win, Win_CURSOR_MOVE);
			}
			if (g_drag_active && endTouch)
			{
				g_drag_active = FALSE;
				g_drag_dontRemove = FALSE;
				g_drag_movingRow = -1;
				Std_deleteCHAR(g_drag_nameSrc);
				g_drag_nameSrc = 0;
				DbRows_free(&g_drag_rows);

				OsWinIO_resetActiveRenderItem();
			}
		}
	}
}

void GuiItem_touch(GuiItem* self, Win* win)
{
	if (!self->show || self->remove)
		return;

	if (Quad2i_isZero(self->coordMoveCut))
		return;

	Quad2i coord = self->coordMove;	//not 'coordMoveCut' because click on Rating/Slider/etc is off

	if (self->type == GuiItem_TABLE) //call before because edit can reset Copy/Paste/Shift
	{
		GuiItemTable_key((GuiItemTable*)self, coord, win);
		GuiItemTable_touch((GuiItemTable*)self, coord, win);
	}

	//avoid clicking through button to underline dialog/list with scroll!
	if (self->touch && self->type != GuiItem_LEVEL)
	{
		if (self->parent)
			self->touch = self->parent->touch;

		if (self->touch)
			self->touch = Quad2i_inside(coord, OsWinIO_getTouchPos());
	}

	int i;
	for (i = 0; i < self->subs.num; i++)
		GuiItem_touch(self->subs.ptrs[i], win);

	if (self->type == GuiItem_MAP)
		GuiItemMap_key((GuiItemMap*)self, coord, win);

	if (self->type == GuiItem_EDIT)
		GuiItemEdit_key((GuiItemEdit*)self, coord, win);

	if (self->type == GuiItem_LEVEL)
		GuiItemLevel_key((GuiItemLevel*)self, coord, win);

	if (self->icon)
		GuiItem_getIconCoord(&coord);

	switch (self->type)
	{
		case GuiItem_BOX: GuiItemBox_touch((GuiItemBox*)self, coord, win);
			break;
		case GuiItem_TEXT: GuiItemText_touch((GuiItemText*)self, coord, win);
			break;
		case GuiItem_TEXT_MULTI:GuiItemTextMulti_touch((GuiItemTextMulti*)self, coord, win);
			break;
		case GuiItem_BUTTON: GuiItemButton_touch((GuiItemButton*)self, coord, win);
			break;
		case GuiItem_EDIT: GuiItemEdit_touch((GuiItemEdit*)self, coord, win);
			break;
		case GuiItem_CHECK: GuiItemCheck_touch((GuiItemCheck*)self, coord, win);
			break;
		case GuiItem_SLIDER: GuiItemSlider_touch((GuiItemSlider*)self, coord, win);
			break;
		case GuiItem_RATING: GuiItemRating_touch((GuiItemRating*)self, coord, win);
			break;
		case GuiItem_COMBO_STATIC: GuiItemComboStatic_touch((GuiItemComboStatic*)self, coord, win);
			break;
		case GuiItem_COMBO_DYNAMIC: GuiItemComboDynamic_touch((GuiItemComboDynamic*)self, coord, win);
			break;
		case GuiItem_MENU: GuiItemMenu_touch((GuiItemMenu*)self, coord, win);
			break;

		case GuiItem_TAGS: GuiItemTags_touch((GuiItemTags*)self, coord, win);
			break;

		case GuiItem_TABLE: break;
		case GuiItem_GROUP:	GuiItemGroup_touch((GuiItemGroup*)self, coord, win);
			break;

		case GuiItem_KANBAN: break;
		case GuiItem_CHART:
		GuiItemChart_touch((GuiItemChart*)self, coord, win);
		break;
		case GuiItem_CALENDAR_SMALL: break;
		case GuiItem_CALENDAR_BIG: break;
		case GuiItem_TIMELINE: break;
		case GuiItem_LIST: GuiItemList_touch((GuiItemList*)self, coord, win);
			break;
			//case GuiItem_TODO: GuiItemTodo_touch((GuiItemTodo*)self, coord, win);
			//	break;
		case GuiItem_LAYOUT: GuiItemLayout_touch((GuiItemLayout*)self, coord, win);
			break;
			//case GuiItem_DESIGN: GuiItemDesign_touch((GuiItemDesign*)self, coord, win);
			//	break;

		case GuiItem_PARTICLES: GuiItemParticles_touch((GuiItemParticles*)self, coord, win);
			break;
		case GuiItem_FILE: GuiItemFile_touch((GuiItemFile*)self, coord, win);
			break;

		case GuiItem_SWITCH: GuiItemSwitch_touch((GuiItemSwitch*)self, coord, win);
			break;
		case GuiItem_MAP:	GuiItemMap_touch((GuiItemMap*)self, coord, win);
			break;
		case GuiItem_COLOR: GuiItemColor_touch((GuiItemColor*)self, coord, win);
			break;
		case GuiItem_LEVEL: GuiItemLevel_touch((GuiItemLevel*)self, coord, win);
			break;
		default:
		break;
	}

	if (self->loopTouch)
		self->loopTouch(self);
}

void GuiItem_addRedraw(GuiItem* self, Win* win)
{
	if (!self->show || self->remove)
		return;

	int i;
	for (i = 0; i < self->subs.num; i++)
		GuiItem_addRedraw(self->subs.ptrs[i], win);

	if (self->redraw)
	{
		GuiItemRoot_addBufferRect(self->coordMove);
		self->redraw = FALSE;
	}
}

Quad2i GuiItem_getCoordSpace(GuiItem* self, Quad2i coord)
{
	switch (self->type)
	{
		case GuiItem_LAYOUT: return GuiItemLayout_getCoordSpace((GuiItemLayout*)self, coord);
			break;

		case GuiItem_BUTTON: return GuiItemButton_getCoordSpace((GuiItemButton*)self, coord);
			break;
		case GuiItem_MENU:	return GuiItemMenu_getCoordSpace((GuiItemMenu*)self, coord);

		default:
		return coord;
	}
}

void GuiItem_draw(GuiItem* self, Win* win, Image4* img)
{
	if (Quad2i_isZero(img->rect))
		return;

	if (!self->show || self->remove)
		return;

	Quad2i coord = self->coordMove;

	Quad2i img_rect_backup = Quad2i_getIntersect(img->rect, coord);
	Image4_setRect(img, img_rect_backup);

	const BOOL realDraw = !Quad2i_isZero(img->rect);
	if (realDraw)
	{
		if (self->icon)
		{
			Rgba iconCd = self->front_cd;

			if (self->icon_alternativeColor)
				iconCd = g_theme.main;

			Quad2i coordBack = GuiItem_getCoordSpace(self, GuiItem_getIconCoordBack(coord));
			if (self->icon_draw_back)
			{
				Image4_drawBoxQuad(img, coordBack, self->back_cd);

				if (Rgba_cmp(iconCd, self->back_cd))
					iconCd = Rgba_aprox(self->front_cd, self->back_cd, 0.5f);
			}

			GuiImage_draw(self->icon, img, GuiItem_getIconCoord(&coord), iconCd);

			if (self->type == GuiItem_BUTTON)
			{
				GuiItemButton_drawPress(img, self->front_cd, coordBack, ((GuiItemButton*)self)->stayPressedLeft, FALSE);
				GuiItemButton_drawSeparLine((GuiItemButton*)self, img, coordBack);
			}
				

			Image4_setRect(img, Quad2i_getIntersect(img->rect, coord));
		}

		switch (self->type)
		{
			case GuiItem_BOX: GuiItemBox_draw((GuiItemBox*)self, img, coord, win);
				break;
			case GuiItem_TEXT: GuiItemText_draw((GuiItemText*)self, img, coord, win);
				break;
			case GuiItem_TEXT_MULTI: GuiItemTextMulti_draw((GuiItemTextMulti*)self, img, coord, win);
				break;
			case GuiItem_BUTTON: GuiItemButton_draw((GuiItemButton*)self, img, coord, win);
				break;
			case GuiItem_EDIT: GuiItemEdit_draw((GuiItemEdit*)self, img, coord, win);
				break;
			case GuiItem_CHECK: GuiItemCheck_draw((GuiItemCheck*)self, img, coord, win);
				break;
			case GuiItem_SLIDER: GuiItemSlider_draw((GuiItemSlider*)self, img, coord, win);
				break;
			case GuiItem_RATING: GuiItemRating_draw((GuiItemRating*)self, img, coord, win);
				break;
			case GuiItem_COMBO_STATIC: GuiItemComboStatic_draw((GuiItemComboStatic*)self, img, coord, win);
				break;
			case GuiItem_COMBO_DYNAMIC: GuiItemComboDynamic_draw((GuiItemComboDynamic*)self, img, coord, win);
				break;
			case GuiItem_MENU:
			GuiItemMenu_draw((GuiItemMenu*)self, img, coord, win);
			break;

			case GuiItem_TAGS: GuiItemTags_draw((GuiItemTags*)self, img, coord, win);
				break;

			case GuiItem_TABLE:
			GuiItemTable_draw((GuiItemTable*)self, img, coord, win);
			break;
			case GuiItem_GROUP:
			GuiItemGroup_draw((GuiItemGroup*)self, img, coord, win);
			break;

			case GuiItem_KANBAN: break;
			case GuiItem_CHART:
			GuiItemChart_draw((GuiItemChart*)self, img, coord, win);
			break;
			case GuiItem_CALENDAR_SMALL://	GuiItemCalendar_draw((GuiItemCalendar*)self, img, coord, win);
			break;
			case GuiItem_CALENDAR_BIG: break;
			case GuiItem_TIMELINE: break;

			case GuiItem_LIST: GuiItemList_draw((GuiItemList*)self, img, coord, win);
				break;
				//case GuiItem_TODO: GuiItemTodo_draw((GuiItemTodo*)self, img, coord, win);
				//	break;
			case GuiItem_LAYOUT: GuiItemLayout_draw((GuiItemLayout*)self, img, coord, win);
				break;
				//case GuiItem_DESIGN: GuiItemDesign_draw((GuiItemDesign*)self, img, coord, win);
				//	break;

			case GuiItem_PARTICLES: GuiItemParticles_draw((GuiItemParticles*)self, img, coord, win);
				break;
			case GuiItem_FILE: GuiItemFile_draw((GuiItemFile*)self, img, coord, win);
				break;

			case GuiItem_SWITCH: GuiItemSwitch_draw((GuiItemSwitch*)self, img, coord, win);
				break;
			case GuiItem_MAP:	GuiItemMap_draw((GuiItemMap*)self, img, coord, win);
				break;
			case GuiItem_COLOR: GuiItemColor_draw((GuiItemColor*)self, img, coord, win);
				break;
			case GuiItem_LEVEL: GuiItemLevel_draw((GuiItemLevel*)self, img, coord, win);
				break;
			default:
			break;
		}
	}

	int i;
	for (i = 0; i < self->subs.num; i++)
	{
		Image4_setRect(img, img_rect_backup);
		GuiItem_draw(self->subs.ptrs[i], win, img);
	}

	Image4_setRect(img, img_rect_backup);

	if (realDraw)
	{
		if (self->type == GuiItem_LAYOUT)
			GuiItemLayout_drawPost((GuiItemLayout*)self, img, coord, win);

		//if (self->type == GuiItem_DESIGN)
		//	GuiItemDesign_drawPost((GuiItemDesign*)self, img, coord, win);

		if (self->type == GuiItem_TABLE)
			GuiItemTable_drawPost((GuiItemTable*)self, img, coord, win);

		if (self->border)
			Image4_drawBorder(img, coord, 1, Rgba_initBlack());


		if (self->changeSizeDraw && DbValue_getRow(&self->changeSizeValue) >= 0)
		{
			const int FAT = 3;
			Quad2i q;
			if(self->changeSizeVertical)
				q = Quad2i_init4(coord.start.x + coord.size.x - FAT, coord.start.y, FAT*2, coord.size.y);
			else
				q = Quad2i_init4(coord.start.x, coord.start.y + coord.size.y - FAT, coord.size.x, FAT*2);

			img->rect.size.x += FAT;
			img->rect.size.y += FAT;
			Image4_drawBoxQuad(img, q, self->changeSizeDrawColor);
		}

	}

	Image4_setRect(img, img_rect_backup);
}

static BOOL _GuiItem_setNextCursor(GuiItem* self, BOOL* found)
{
	if (self->type == GuiItem_EDIT)
	{
		GuiItemEdit* edit = (GuiItemEdit*)self;
		*found |= (edit->tabAway && OsWinIO_isCursorGuiItem(self));

		if (*found && GuiItem_isEnable(self) && GuiItemEdit_setCursor(edit, TRUE))
			return TRUE;
	}

	int i;
	for (i = 0; i < self->subs.num; i++)
		if (_GuiItem_setNextCursor(self->subs.ptrs[i], found))
			return TRUE;

	return FALSE;
}

BOOL GuiItem_setFirstCursor(GuiItem* self)
{
	BOOL found = TRUE;
	return _GuiItem_setNextCursor(self, &found);
}

BOOL GuiItem_setNextCursor(GuiItem* self)
{
	BOOL found = FALSE;
	if (!_GuiItem_setNextCursor(self, &found))
		return GuiItem_setFirstCursor(self); //loop
	return TRUE;
}

//note: other buttons are reset!
void GuiItem_setPressed(GuiItem* self, GuiItemButton* button)
{
	if (self->type == GuiItem_BUTTON)
		((GuiItemButton*)self)->stayPressed = (self == &button->base);

	BIG i;
	for (i = 0; i < self->subs.num; i++)
		GuiItem_setPressed(self->subs.ptrs[i], button);
}

GuiItemButton* GuiItem_findPressed(GuiItem* self)
{
	if (self->type == GuiItem_BUTTON && ((GuiItemButton*)self)->stayPressed)
		return(GuiItemButton*)self;

	BIG i;
	for (i = 0; i < self->subs.num; i++)
	{
		GuiItemButton* b = GuiItem_findPressed(self->subs.ptrs[i]);
		if (b)
			return b;
	}

	return 0;
}

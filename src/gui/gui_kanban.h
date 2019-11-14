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







typedef struct GuiItemKanban_s
{
	GuiItem base;

	DbValue columnLink;
	DbValues lanes;

	//DbColumnString* lane_title;
	//DbColumnLink* lane_titleColumn;

	//DbFilter* filter;
	
	GuiItem* skin;
}GuiItemKanban;


GuiItemKanban* GuiItemKanban_new(Quad2i grid, BIG kanbanRow)
{
	GuiItemKanban* self = Os_malloc(sizeof(GuiItemKanban));
	self->base = GuiItem_init(GuiItem_KANBAN, grid);
	
	self->columnLink = DbRows_getColumn(kanbanRow, _UNI32("column"));

	self->lanes = DbRows_getOptions(kanbanRow, _UNI32("lanes"), _UNI32("value"), FALSE);

	self->skin = skin;	//int GuiTable_???() ...

	return self;
}

GuiItem* GuiItemKanban_newCopy(GuiItemKanban* src)
{
	GuiItemKanban* self = Os_malloc(sizeof(GuiItemKanban));
	*self = *src;
	GuiItem_initCopy(&self->base, &src->base);

	self->columnLink = DbValue_initCopy(&src->columnLink);
	self->lanes = DbValues_initCopy(&src->lanes);
	
	self->skin = GuiItem_newCopy(src->skin);
	//self->filter = DbFilter_newCopy(src->filter);
	
	return (GuiItem*)self;
}

void GuiItemKanban_delete(GuiItemKanban* self)
{
	DbValue_free(&self->columnLink);
	DbValues_free(&self->lanes);

	GuiItem_delete(self->skin);
	//DbFilter_delete(self->filter);
	
	
	GuiItem_free(&self->base);
	Os_free(self, sizeof(GuiItemKanban));
}






static BOOL _GuiItemKanban_isLaneEnable(const GuiItemKanban* self, UBIG i)
{
	return DbValue_isRowEnable(&self->lanes.values[i]);

	//*out_groupRow = DbRows_getRow(self->lanes, posHard);
	//return AppGui_option_isEnable(App_getGui(), *out_groupRow);
}

static UBIG _GuiItemKanban_getNumLanes(GuiItemKanban* self)
{
	UBIG N = self->lanes.num;
	const UBIG NN = N;
	int i;
	for(i=0; i < NN; i++)
		if(!_GuiItemKanban_isLaneEnable(self, i))
			N--;
	return N;
}

/*static BIG _GuiItemKanban_getLaneEnableRow(const GuiItemKanban* self, UBIG i)
{
	if(self->lanes)
	{
		UBIG N = DbRows_getSize(self->lanes);
		int ii;
		for(ii=0; ii < N; ii++)
		{
			BIG row;
			if(_GuiItemKanban_isLaneEnable(self, ii, &row))
			{
				if(i == 0)
					return row;
				i--;
			}
		}
		return N;
	}
	return -1;
}*/









static DbColumn* GuiItemKanban_getMainColumn(const GuiItemKanban* self)
{
	return self->columnLink.column;
	//UBIG colRow = DbRows_getRow(self->columnLink, 0);
	//return (colRow >= 0) ? DbRoot_getColumnRow(colRow) : 0;
}


static DbTable* GuiItemKanban_getTable(const GuiItemKanban* self)
{
	return DbColumn_getTable(GuiItemKanban_getMainColumn(self));
	//DbColumn* column = GuiItemKanban_getMainColumn(self);
	//return column ? DbColumn_getParent(column) : 0;
}





/*static DbColumnNumber* GuiItemKanban_getLaneEnable(GuiItemKanban* self)
{
	return AppGui_getPropEnable(App_getGui());
}
static DbColumnString* GuiItemKanban_getLaneTitleColumn(GuiItemKanban* self)
{
	return AppGui_getName(App_getGui());
}*/


static const UNI* GuiItemKanban_getLaneTitle(GuiItemKanban* self, UBIG row)
{
	DbValue* lane = DbValues_findRow(&self->lanes, row);
	return lane ? lane->result : 0;

	//return self->lanes.values[i]
	//return AppGui_option_getName(App_getGui(), row, out);
	//return DbColumnString_get(GuiItemKanban_getLaneTitleColumn(self), row);
}




/*static DbColumn* GuiItemKanban_getDataColumn(GuiItemKanban* self)
{
	BIG columnRow = DbRows_getRow(self->column, 0);
	return (columnRow >= 0) ? DbRoot_getColumnRow(columnRow) : 0;
}


static UNI* GuiItemKanban_getDataTitle(GuiItemKanban* self, UBIG filterRow)
{
	DbColumn* column = GuiItemKanban_getDataColumn(self);
	if(column)
	{
		BIG row = DbColumnLink_getFirstRow(DbFilter_getColumnGroupRows(self->filter), filterRow);
		if(row >= 0)
			return DbColumn_getStringCopy(column, row);
	}	
	return 0;
}*/



























	
	
static BIG GuiItemKanban_findFilter(GuiItemKanban* self, const UNI* title)
{
	if(DbFilter_numGroupTable(self->filter))
	{
		UBIG row = 0;	//base
		DbColumnLink* subs = DbFilter_getColumnGroupSubs(self->filter);

		UBIG i = 0;
		while(DbColumnLink_jump(subs, row, &i, 1) >= 0)
		{
			UBIG it = DbColumnLink_getRow(subs, row, i);

			UNI* title2 = GuiItemKanban_getDataTitle(self, it);
			if(Std_cmpUNI(title2, title))
			{
				Std_deleteUNI(title2);
				return it;
			}
			Std_deleteUNI(title2);

			i++;
		}
	}

	return -1;
}

static BIG GuiItemKanban_findLane(GuiItemKanban* self, const UNI* title)
{
	BIG c;
	for(c = 0; c < self->lanes.num; c++)
	{
		if (Std_cmpUNI(self->lanes.values[c].result, title))
			return c;
	}
	return -1;
}



static void GuiItemKanban_addFilterToLane(GuiItemKanban* self)
{
	if(DbFilter_numGroupTable(self->filter))
	{
		UBIG row = 0;	//base
		DbColumnLink* subs = DbFilter_getColumnGroupSubs(self->filter);

		UBIG i = 0;
		while(DbColumnLink_jump(subs, row, &i, 1) >= 0)
		{
			UBIG it = DbColumnLink_getRow(subs, row, i);

			UNI* title = GuiItemKanban_getDataTitle(self, it);
			//Std_printlnUNI(title);	

			if(GuiItemKanban_findLane(self, title) < 0)
			{
				//UBIG laneRow = AppGui_addPropColumnBasic(App_getGui(), self->lanes->base.row);
				UBIG laneRow = AppGui_addPropSub(App_getGui(), self->lanes->base.row, -1);
				AppGui_option_setName(App_getGui(), laneRow, title);
				//DbColumnString_setCopy(GuiItemKanban_getLaneTitleColumn(self), laneRow, title);

				//DbColumnNumber_set(AppGui_getPropDbColumnWidth(App_getGui()), laneRow, 5);
				//DbColumnNumber_set(GuiItemKanban_getLaneEnable(self), laneRow, 1);
			}

			Std_deleteUNI(title);
			i++;
		}
	}
}



void GuiItemKanban_clickAddLane(GuiItem* self)
{
	GuiItemKanban* kanban = GuiItem_findParentType(self, GuiItem_KANBAN);
	if(kanban)
	{
		AppGui_addPropSub(App_getGui(), kanban->lanes->base.row, -1);
		//UBIG laneRow = AppGui_addPropColumnBasic(App_getGui(), kanban->lanes->base.row);
		//DbColumnNumber_set(AppGui_getPropDbColumnWidth(App_getGui()), laneRow, 5);
		//DbColumnNumber_set(GuiItemKanban_getLaneEnable(kanban), laneRow, 1);
		
		GuiItem_setResize(&kanban->base, TRUE);
	}
}


void GuiItemKanban_clickRemoveLane(GuiItem* self)
{
	GuiItemKanban* kanban = GuiItem_findParentType(self, GuiItem_KANBAN);
	if(kanban)
	{
		//BIG crow = GuiDbValue_getExtraBig(GuiItemButton_getTextMsg((GuiItemButton*)self));
		BIG row = GuiItem_findAttribute(self, "row");

		DbRows_removeRow(kanban->lanes, row);

		GuiItem_setResize(&kanban->base, TRUE);
	}
}
	


void GuiItemKanban_clickHideLane(GuiItem* self)
{
	GuiItemKanban* kanban = GuiItem_findParentType(self, GuiItem_KANBAN);
	if(kanban)
	{
		//BIG crow = GuiDbValue_getExtraBig(GuiItemButton_getTextMsg((GuiItemButton*)self));
		BIG row = GuiItem_findAttribute(self, "row");

		AppGui_option_setEnable(App_getGui(), row, 0);
		//DbColumnNumber_set(GuiItemKanban_getLaneEnable(kanban), crow, 0);
		GuiItem_setResize(&kanban->base, TRUE);
	}
}






void GuiItemKanban_clickMove(GuiItem* dst, GuiItem* src)
{
	BIG srcRow = GuiItem_getRow(src);
	BIG crow = GuiItem_findAttribute(dst, "crow");

	GuiItemKanban* kanban = GuiItem_findParentType(src, GuiItem_KANBAN);
	if(kanban)
	{
		DbColumn* column = GuiItemKanban_getMainColumn(kanban);
		if(column)
		{
			UNI name[64];
			DbColumn_setStringCopy(column, srcRow, GuiItemKanban_getLaneTitle(kanban, crow, name));
		}
			
		
		GuiItem_setResize(&kanban->base, TRUE);
	}
}


void GuiItemKanban_clickCardOpenEditMode(GuiItem* item)
{
	BIG recordRow = GuiItem_getRow(item);
	BIG skinRow = GuiItem_findAttribute(item, "skinRow");
	
	GuiItemLayout* skin = GuiItemKanban_buildSkin(skinRow, FALSE, FALSE);
	GuiItem_setRow((GuiItem*)skin, recordRow);
	
	GuiItemRoot_setDialog(skin);
}



GuiItemLayout* GuiItemKanban_buildSkin(BIG row, BOOL readOnly, BOOL dragDrop)
{
	//AppGui* appGui = App_getGui();
	
	GuiItemLayout* layColumn = GuiItemLayout_new(Quad2i_init());
	GuiItemLayout_addColumn(layColumn, 0, (readOnly?100:20));
	GuiItemLayout_setDrawBorder(layColumn, TRUE);
	
	UBIG propTable = AppGui_getPropTable(appGui, row);

	int size = readOnly ? 1 : 2;
	int y = 0;
	UBIG i = 0;
	BIG it;

	while((it=AppGui_getSub(appGui, propTable, i))>=0)
	//while(DbColumnLink_jump(AppGui_getPropSub(appGui), propTable, &i, 1) >= 0)
	{
		//UBIG it = AppGui_get_gui_subs(appGui, propTable, i);
		DbColumn* column = AppGui_getColumn(appGui, it);
		if(column && (!readOnly || AppGui_option_isEnable(appGui, it)))
		{
			//DbColumnTYPE type = DbColumn_getType(column);
			
			GuiItem* item = GuiItemTable_getCardSkinItem(Quad2i_init4(0, y, 1, size), column, readOnly);
			if(item)
			//if(type == DbColumn_NUMBER || type == DbColumn_STRING)
			{
				
				GuiItem_addSub((GuiItem*)layColumn, item);
				y += size;
				
				if(readOnly)
				{
					GuiItem_setCallClick(item, &GuiItemKanban_clickCardOpenEditMode);
					GuiItemText_setCursorHand((GuiItemText*)item, TRUE);
					//GuiItemText_setEnableSelect((GuiItemText*)item, FALSE);
					
					//if(dragDrop)
					//	GuiItem_setDropCallback(item, "kanbanList", 0, FALSE, &GuiItemKanban_clickMove);
				}
				GuiItem_setAttribute(item, "skinRow", row);
			}
		}
		i++;
	}

	
	if(readOnly && dragDrop)
	{
		GuiItem_setIcon((GuiItem*)layColumn, GuiImage_new1(UiIcons_init_move()));
		GuiItem_setDropCallback((GuiItem*)layColumn, "kanbanList", "", TRUE, &GuiItemKanban_clickMove);
		
		
		//GuiItem* box = GuiItem_addSub((GuiItem*)layColumn, GuiItemText_new(Quad2i_init4(1, 0, 1, y*size), g_theme.main));	//icon? ...
		//GuiItem_setDropCallback(box, "kanbanList", "", TRUE, &GuiItemKanban_clickMove);
		
//.........................................
		//GuiItem* box = GuiItem_addSub((GuiItem*)layColumn, GuiItemBox_newCd(Quad2i_init4(1, 0, 1, y*size), g_theme.main));	//icon? ...
		///GuiItem_setDropCallback(box, "kanbanList", "", TRUE, &GuiItemKanban_clickMove);
	}
	
	return layColumn;
}




void GuiItemKanban_clickAddRecord(GuiItem* self)
{
	GuiItemKanban* kanban = GuiItem_findParentType(self, GuiItem_KANBAN);
	if(kanban)
	{
		BIG crow = GuiItem_findAttribute(self, "laneRow");
		
		DbColumn* column = GuiItemKanban_getMainColumn(kanban);
		DbTable* table = GuiItemKanban_getTable(kanban);
		if(column && table && crow >= 0)
		{
			UBIG recordRow = DbTable_addRow(table);	//add line

			UNI name[64];
			DbColumn_setStringCopy(column, recordRow, GuiItemKanban_getLaneTitle(kanban, crow, name));	//set pre-set
			
			
			GuiItem_setResize(&kanban->base, TRUE);
		
			
			//open in dialog for edit
			BIG skinRow = GuiItem_findAttribute(kanban->skin, "skinRow");
			GuiItemLayout* skin = GuiItemKanban_buildSkin(skinRow, FALSE, FALSE);
			GuiItem_setRow((GuiItem*)skin, recordRow);
			GuiItemRoot_setDialog(skin);	
		}
		
		//open edit skin ...
		
		
		//UiRootCard_clickCardOpenEditMode(self);
		
		
		//BIG recordRow = GuiItem_getRow(item);
		//BIG skinRow = GuiItem_findAttribute(item, "skinRow");


		
	}
}




void GuiItemKanban_clickRemoveEmptyLanes(GuiItem* self)
{
	GuiItemKanban* kanban = GuiItem_findNameType(self, "kanban", GuiItem_KANBAN);
	if(kanban)
	{
		
		int N_LANES = DbRows_getSize(kanban->lanes);
		int c;
		for(c = N_LANES-1; c >= 0; c--)
		{
			UBIG crow = DbRows_getRow(kanban->lanes, c);


			UNI name[64];
			BIG filterRow = GuiItemKanban_findFilter(kanban, GuiItemKanban_getLaneTitle(kanban, crow, name));

			DbColumnLink* rows = DbFilter_getColumnGroupRows(kanban->filter);
			BOOL showRemove = (filterRow >= 0) ? (DbColumnLink_sizeActive(rows, filterRow)==0) : TRUE;
			
			if(showRemove)
				DbRows_removeRow(kanban->lanes, crow);	
			
			GuiItem_setResize(&kanban->base, TRUE);
		}	
	}
}





void GuiItemKanban_update(GuiItemKanban* self, Quad2i coord, Win* win)
{
	DbValues_updateText(&self->lanes);

	//? ...
	//GuiItem_setResize(self, TRUE);
	//GuiItem_setRedraw(&self->base, (GuiDbValue_isChanged(self->format) || GuiDbValue_isChanged(self->value)));
}




GuiItemLayout* GuiItemKanban_resize(GuiItemKanban* self, GuiItemLayout* layout, Win* win)
{
	//self->base.coord.size.x -= GuiScroll_widthWin(win);
	GuiItem_freeSubs(&self->base);

	
	//layout
	layout = GuiItemLayout_newCoord(&self->base, TRUE, TRUE, win);
	layout->drawBackground = FALSE;

	GuiItemLayout_addRow(layout, 2, 100);

	GuiItem_addSub(&self->base, &layout->base);
	
	
	
	
	//update filter
	DbFilter_clearShortsAndGroups(self->filter);
	DbColumn* mainColumn = GuiItemKanban_getMainColumn(self);
	//mainColumn = 0;
	if(mainColumn)
		DbFilter_addGroup(self->filter, mainColumn, TRUE, FALSE);
	
	DbFilter_execute(self->filter);
		
	//update lanes from filter
	GuiItemKanban_addFilterToLane(self);	
	



	
	int width_sum = 0;
	int N_LANES = _GuiItemKanban_getNumLanes(self);
	int c;
	for(c = 0; c < N_LANES; c++)
	{
		UBIG crow = _GuiItemKanban_getLaneEnableRow(self, c);
		
		DbColumn* column = GuiItemKanban_getDataColumn(self);
		int width = Std_max(1, AppGui_option_getWidth(App_getGui(), crow));

		UNI name[64];
		GuiItemKanban_getLaneTitle(self, crow, name);
		DbColumnLink* rows = DbFilter_getColumnGroupRows(self->filter);


		BIG filterRow = GuiItemKanban_findFilter(self, name);
		BOOL showRemove = (filterRow >= 0) ? (DbColumnLink_sizeActive(rows, filterRow)==0) : TRUE;
		
		
		//header
		{
		GuiItemLayout* header = GuiItemLayout_new(Quad2i_init4(width_sum, 0, width, 2));
		GuiItemLayout_addColumn(header, 0, 10);
		GuiItemLayout_addColumn(header, 1, 2);
		GuiItem_addSub(&layout->base, &header->base);
		GuiItem_setAttribute(&header->base, "laneRow", crow);
		
		
		//header: icon
		GuiItem_setIcon((GuiItem*)header, GuiImage_new1(GuiItem_getColumnIcon(column ? DbColumn_getType(column) : -1)));

		//header: change size
		GuiItem_setChangeSize((GuiItem*)header, TRUE, AppGui_connectWidth(App_getGui(), crow));

		//header: change order
		//DbRows dropId = DbRows_initCopy(self->lanes);
		//dropId->extraRow = crow;
		GuiItem_setDrop((GuiItem*)header, "lane", "lane", TRUE, DbRows_initCopy(self->lanes), crow);
		
		

		//title
		GuiItem_addSub((GuiItem*)header, GuiItemEdit_new(Quad2i_init4(0, 0, 1, 1), AppGui_connectName(App_getGui(), crow), DbValue_initEmpty()));

		
		//menu
		GuiItemMenu* menu = (GuiItemMenu*)GuiItem_addSub((GuiItem*)header, GuiItemMenu_new(Quad2i_init4(1, 0, 1, 1), DbValue_initStaticCopy(_UNI32("≡")), FALSE));
		GuiItemMenu_setUnderline(menu, FALSE);
		GuiItemMenu_addItem(menu, DbValue_initLang(GUI_HIDE_COLUMN), &GuiItemKanban_clickHideLane);
		if(showRemove)
			GuiItemMenu_addItem(menu, DbValue_initLang(GUI_REMOVE), &GuiItemKanban_clickRemoveLane);
		GuiItem_setAttribute((GuiItem*)menu, "row", crow);
		
		

		//#records
		GuiItemText* numRecords = GuiItem_addSub((GuiItem*)header, GuiItemText_new(Quad2i_init4(0, 1, 1, 1), FALSE, DbValue_initGET((DbColumn*)DbFilter_getColumnGroupCount(self->filter), filterRow), DbValue_initEmpty()));
		GuiItemText_setFormat(numRecords, GuiFormat_new(GuiFormat_COUNT, Lang_find(GUI_RECORDS), 0, 0));
		
		//add record
		GuiItem_addSub((GuiItem*) header, GuiItemButton_newClassicEx(Quad2i_init4(1, 1, 1, 1), DbValue_initStaticCopy(_UNI32("+")), &GuiItemKanban_clickAddRecord));
		
		}
		
		
		
		

		//list
		GuiItemList* list = (GuiItemList*)GuiItem_addSub((GuiItem*)layout, GuiItemList_new(Quad2i_init4(width_sum, 2, width, 1), DbRows_initLink(rows, filterRow), (GuiItem*)GuiItem_newCopy(self->skin), DbValue_initEmpty(), DbValue_initEmpty()));
		GuiItemList_setShowBorder(list, FALSE);
		GuiItemList_setAsGrid(list, 100);	//add space between items

		//drag & drop items ...
		GuiItem_setDropIN(&list->base, "kanbanList");
		GuiItem_setAttribute(&list->base, "crow", crow);
		
		
		
		width_sum += width;
		width_sum++;	//space between lines
	}

	//add new Lane
	GuiItem_addSub((GuiItem*) layout, GuiItemButton_newClassicEx(Quad2i_init4(width_sum, 0, 2, 1), DbValue_initStaticCopy(_UNI32("+")), &GuiItemKanban_clickAddLane));
	
	return layout;
}










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

void UiRoot_clickCloudReload(GuiItem* item)
{
	if (UiScreen_needSave())
		GuiItemRoot_addDialogLayout(UiDialogSave_new(UiDialogSave_OPEN_RELOAD));
	else
		UiScreen_reloadProject();
}
void UiRoot_clickCloudIgnore(GuiItem* item)
{
	UiScreen_setIndexChanged(FALSE);
}

void UiRoot_clickConnect(GuiItem* item)
{
	GuiItemComboStatic* self = (GuiItemComboStatic*)item;
	BOOL online = GuiItemComboStatic_getValue(self) == 0;
	MediaNetwork_run(online);

	GuiItem_setIcon((GuiItem*)self, GuiImage_new1(online ? UiIcons_init_internet_online() : UiIcons_init_internet_offline()));
}

BOOL UiRoot_isShowSub(BIG row)
{
	return DbValue_getOptionNumber(row, "show_subs", 1);
}
void UiRoot_clickSubExpand(GuiItem* item)
{
	BIG row = GuiItem_findAttribute(item, "row");
	DbValue_setOptionNumber(row, "show_subs", !UiRoot_isShowSub(row));
}

void UiRoot_updateNet(GuiItem* item)
{
	//red background
	{
		GuiItemComboStatic* onOff = GuiItem_findName(item, "onOff");
		Rgba cd = (!MediaNetwork_is() && MediaNetwork_isDownloadActive()) ? Rgba_aprox(Rgba_initWhite(), GuiItemTheme_getWarningColor(), Std_timeAprox2(0, 2)) : Rgba_initWhite();
		GuiItemComboStatic_setBackgroundColor(onOff, cd);
	}

	//updates bandwidth
	{
		GuiItemText* bandwidth = GuiItem_findName(item, "bandwidth");
		BIG time = GuiItem_findAttribute((GuiItem*)bandwidth, "time");
		if (Os_time() - time > 2)
		{
			double bytes = MediaNetwork_getBandwidth();
			const char* addStr = "B/s";
			if (bytes > 1024 * 1024) { bytes /= 1024 * 1024; addStr = "MB/s"; }
			else
				if (bytes > 1024) { bytes /= 1024; addStr = "KB/s"; }

			UNI* str = Std_newNumberPrecision(bytes, 2);
			str = Std_addAfterUNI_char(str, addStr);
			GuiItemText_setText(bandwidth, str);
			Std_deleteUNI(str);

			GuiItem_setAttribute((GuiItem*)bandwidth, "time", Os_time());
		}
	}
}

static void _UiRoot_setLeft(BIG row)
{
	DbRoot_setPanelLeft(row);
}

static void _UiRoot_setRight(BIG row)
{
	DbRoot_setPanelRight(row);
}

static void _UiRoot_removeLeft(void)
{
	_UiRoot_setLeft(-1);
}

static void _UiRoot_removeRight(void)
{
	_UiRoot_setRight(-1);
}

void UiRoot_clickSave(GuiItem* item)
{
	DbRoot_save();
}

void UiRoot_clickEditSearch(GuiItem* item)
{
	GuiItem* list = GuiItem_findName(item, "list");
	if (list)
		GuiItem_setResize(list, TRUE);
}

void UiRoot_clickSplitCloseLeft(GuiItem* item)
{
	BIG viewRight = DbRoot_getPanelRight();
	if (viewRight >= 0)
		DbRoot_setPanelLeft(viewRight);	//viewLeft = viewRight;

	_UiRoot_removeRight();
}
void UiRoot_clickSplitCloseRight(GuiItem* item)
{
	_UiRoot_removeRight();
}

void UiRoot_clickSplitSwap(GuiItem* item)
{
	DbRoot_swapPanelRight();
}

void UiRoot_clickListItem(GuiItem* item)
{
	BIG viewLeft = DbRoot_getPanelLeft();
	BIG viewRight = DbRoot_getPanelRight();

	BOOL left = (OsWinIO_getTouch_action() == Win_TOUCH_DOWN_E);

	BIG viewClick = GuiItem_findAttribute(item, "clickRow");

	if (viewLeft == viewRight && viewRight == viewClick)
	{
		_UiRoot_removeRight();
	}
	else
	{
		if (left)
			_UiRoot_setLeft(viewClick);
		else
			_UiRoot_setRight(viewClick);
	}
}

void UiRoot_clickDuplicate(GuiItem* item)
{
	BIG row = GuiItem_findAttribute(item, "row");
	DbRoot_duplicateRow(row, DbRoot_findParent(row));
}

void UiRoot_removePanel(BIG row)
{
	BIG viewLeft = DbRoot_getPanelLeft();
	BIG viewRight = DbRoot_getPanelRight();

	if (viewRight == row)
	{
		_UiRoot_removeRight();
	}
	if (viewLeft == row && viewRight >= 0)
	{
		DbRoot_setPanelLeft(viewRight);
		_UiRoot_removeRight();
	}

	if (viewLeft < 0 || viewLeft == row)
	{
		_UiRoot_removeLeft();

		BIG r = DbRoot_findParentTableRow(DbRoot_findParent(row));
		if (r < 0)
			r = DbRoot_findParentFolderRow(row);

		_UiRoot_setLeft(r);
	}

	if (viewLeft == 0)	//root
	{
		_UiRoot_removeLeft();
		_UiRoot_setLeft(1);	//pages(global)
	}

	GuiItemRoot_resizeAll();
}
void UiRoot_clickRemove(GuiItem* item)
{
	BIG row = GuiItem_findAttribute(item, "row");

	DbRoot_removeRow(row);
	UiRoot_removePanel(row);

	GuiItemRoot_closeLevels();
}

void UiRoot_clickRemoveKeepSubViews(GuiItem* item)
{
	BIG row = GuiItem_findAttribute(item, "row");

	BIG src = DbRoot_findChildType(row, "views");	//child
	BIG dst = DbRoot_findParentType(row, "views");	//parent
	if (src >= 0 && dst >= 0)
	{
		DbRoot_moveSubs(dst, src, row);
		DbRoot_removeRow(row);
	}

	UiRoot_removePanel(row);
	GuiItemRoot_closeLevels();
}

void UiRoot_clickAddSub(GuiItem* item)
{
	BIG parent = GuiItem_findAttribute(item, "row");
	BIG newType = GuiItem_findAttribute(item, "type");

	if (DbRoot_isType_root(parent))
	{
		if (newType == UI_ADD_FOLDER)
		{
			BIG r = DbRoot_createFolderRow();
			if (r >= 0)
				DbRoot_setPanelLeft(r);
		}
		if (newType == UI_ADD_REMOTE)
		{
			BIG r = DbRoot_createRemoteRow();
			if (r >= 0)
				DbRoot_setPanelLeft(r);
		}
	}
	else
		if (DbRoot_isType_folder(parent))
		{
			if (newType == UI_ADD_TABLE)
			{
				DbTable* table = DbRoot_createTableExample(parent);
				DbRoot_setPanelLeft(DbTable_getRow(table));
			}
			else
				if (newType == UI_ADD_PAGE)
				{
					//DbRoot_createPageRow(parent);
					//DbRoot_setPanelLeft(...);
				}
		}
		else
			if (DbRoot_isType_table(parent) || DbRoot_isTypeView_filter(parent) || DbRoot_isTypeView_summary(parent))
			{
				parent = DbRoot_findChildType(parent, "views");

				const UNI* name = 0;
				BIG r = -1;
				switch (newType)
				{
					case UI_ADD_VIEW_CARDS:	r = DbRoot_createView_cards(parent);		name = Lang_find("VIEW_CARDS");	break;
					case UI_ADD_VIEW_FILTER:	r = DbRoot_createView_filter(parent);	name = Lang_find("VIEW_FILTER");	break;
					case UI_ADD_VIEW_GROUP:	r = DbRoot_createView_group(parent);		name = Lang_find("VIEW_GROUP"); break;
					case UI_ADD_VIEW_KANBAN:	r = DbRoot_createView_kanban(parent);	name = Lang_find("VIEW_KANBAN");	break;
					case UI_ADD_VIEW_CALENDAR:	r = DbRoot_createView_calendar(parent);	name = Lang_find("VIEW_CALENDAR");	break;
					case UI_ADD_VIEW_TIMELINE:	r = DbRoot_createView_timeline(parent);	name = Lang_find("VIEW_TIMELINE");	break;
					case UI_ADD_VIEW_CHART:	r = DbRoot_createView_chart(parent);		name = Lang_find("VIEW_CHART"); break;
					case UI_ADD_VIEW_MAP:		r = DbRoot_createView_map(parent);		name = Lang_find("VIEW_MAP"); break;
					case UI_ADD_VIEW_SUMMARY:	r = DbRoot_createView_summary(parent);	name = Lang_find("VIEW_SUMMARY"); break;
					default:	break;
				}
				if (name)
					DbRoot_setName(r, name);

				if (r >= 0)
					DbRoot_setPanelLeft(r);
			}
}

void UiRoot_clickCreateReference(GuiItem* item)
{
	BIG row = GuiItem_findAttribute(item, "row");

	BIG origRow = DbRoot_isReference(row) ? DbRoot_getOrigReference(row) : row;

	//create Reference
	BIG refRow = DbRoot_createViewReference(DbRoot_findParent(row), origRow);

	//set name
	UNI name[64];
	DbRoot_setName(refRow, DbRoot_getName(row, name, 64));
}

void UiRoot_clickOpenReferenceSource(GuiItem* item)
{
	BIG row = GuiItem_findAttribute(item, "row");

	BIG origRow = DbRoot_isReference(row) ? DbRoot_getOrigReference(row) : row;

	//open source
	_UiRoot_setLeft(origRow);
}

void UiRoot_clickReferenceToPernament(GuiItem* item)
{
	BIG row = GuiItem_findAttribute(item, "row");
	DbRoot_pernamentReference(row);
}

static DbValue _UiRoot_getSearchValue(void)
{
	return DbValue_initOption(DbRoot_getRootRow(), "search", 0);
}
static DbValue _UiRoot_getSideValue(void)
{
	return DbValue_initOption(DbRoot_getRootRow(), "side", _UNI32("10"));
}
static DbValue _UiRoot_getSplitValue(void)
{
	return DbValue_initOption(DbRoot_getRootRow(), "split", _UNI32("10"));
}
static double _UiRoot_getSideValueNumber(void)
{
	DbValue val = _UiRoot_getSideValue();
	double v = DbValue_getNumber(&val);
	DbValue_free(&val);
	return v;
}
static double _UiRoot_getSplitValueNumber(void)
{
	DbValue val = _UiRoot_getSplitValue();
	double v = DbValue_getNumber(&val);
	DbValue_free(&val);
	return v;
}

static BIG _UiRoot_getSubsRow(BIG row)
{
	return (DbRoot_isType_root(row) || DbRoot_isType_folder(row) || DbRoot_isType_remote(row)) ? row : DbRoot_findChildType(row, "views");
}

static BOOL _UiRoot_canGoSubs(BIG row)
{
	return DbRoot_isType_folder(row) || DbRoot_isType_remote(row) || DbRoot_isType_table(row) || DbRoot_isTypeView_filter(row) || DbRoot_isTypeView_summary(row);
}

static BOOL _UiRoot_showButtonSearch(BIG row, const UNI* search)
{
	BOOL is = TRUE;
	if (Std_sizeUNI(search))
	{
		DbValue name = DbValue_initOption(row, "name", 0);
		DbValue_hasChanged(&name);
		is = Std_subUNI_small(DbValue_result(&name), search) >= 0;
		DbValue_free(&name);
	}
	return is;
}

static BOOL _UiRoot_showButtonSearchSub(const BIG row, const UNI* search)
{
	if (_UiRoot_showButtonSearch(row, search))
		return TRUE;

	if (_UiRoot_canGoSubs(row))
	{
		BIG row2 = _UiRoot_getSubsRow(row);
		BIG it;
		UBIG i = 0;
		while ((it = DbColumnN_jump(DbRoot_subs(), row2, &i, 1)) >= 0)
		{
			if (_UiRoot_showButtonSearchSub(it, search))
				return TRUE;

			i++;
		}
	}

	return FALSE;
}

static UNI* _UiRoot_getItemName(UBIG row)
{
	UNI* name = 0;

	BOOL isReference = DbRoot_isReference(row);

	UNI title[64];
	DbRoot_getName(row, title, 64);
	if (isReference)
	{
		//DbRoot_getName(DbRoot_getOrigReference(row), title, 64);

		name = Std_addAfterUNI_char(name, " << ");
		name = Std_addAfterUNI(name, title);
		name = Std_addAfterUNI_char(name, "  >>");
	}
	else
	{
		//DbRoot_getName(row, title, 64);
		name = Std_newUNI(title);
	}

	return name;
}

static void _UiRoot_buildListItem(BIG viewLeft, BIG viewRight, GuiItemLayout* parentLayout, Quad2i grid, UBIG row, UBIG parentRow, const UNI* search)
{
	BOOL isReference = DbRoot_isReference(row);
	UNI* name = _UiRoot_getItemName(row);

	char subName[64];
	Std_buildNumber(row, 0, subName);

	GuiItemButton* b = (GuiItemButton*)GuiItem_addSubName((GuiItem*)parentLayout, subName, GuiItemButton_newAlphaEx(grid, DbValue_initStatic(name), &UiRoot_clickListItem));
	GuiItemButton_setTextCenter(b, FALSE);
	GuiItem_setEnable((GuiItem*)b, _UiRoot_showButtonSearch(row, search));
	GuiItem_setAttribute((GuiItem*)b, "clickRow", row);
	GuiItem_setAttribute((GuiItem*)b, "row", row);

	if (isReference)
		GuiItem_setAlternativeIconCd((GuiItem*)b, TRUE);

	Image1 icon = GuiStruct_getIcon(row);
	if (Vec2i_is(icon.size))
		GuiItem_setIcon((GuiItem*)b, GuiImage_new1(icon));

	BOOL both = (viewLeft >= 0 && viewRight >= 0);

	GuiItemButton_setPressedEx(b, (viewLeft == row || viewRight == row), (both && viewLeft == row), (both && viewRight == row));

	BIG tableRow = DbRoot_findParentTableRow(row);
	//find original table(jump over summary tables)
	BIG r;
	while ((r = DbRoot_findParentTableRow(DbRoot_findParent(tableRow))) >= 0)
		tableRow = r;

	char desc[32];

	//drag & drop (before/after)
	{
		if (DbRoot_isType_folder(row) || DbRoot_isType_remote(row))
			snprintf(desc, sizeof(desc), "base");
		else
			if (DbRoot_isType_table(row))
			{
				BIG remoteRow = DbRoot_findParentType(row, "remote");
				if (remoteRow >= 0)
					snprintf(desc, sizeof(desc), "remote_%lld", remoteRow);	//can drag&drop Tables in/out of Remote Folder
				else
					snprintf(desc, sizeof(desc), "table");
			}
			else
				snprintf(desc, sizeof(desc), "view_%lld", tableRow);

		GuiItem_setDrop((GuiItem*)b, desc, desc, FALSE, DbRows_initLinkN(DbRoot_subs(), parentRow), row);
	}

	//drag & drop IN
	{
		desc[0] = 0;

		if (DbRoot_isType_folder(row))	snprintf(desc, sizeof(desc), "table");

		if (DbRoot_isType_table(row))	snprintf(desc, sizeof(desc), "view_%lld", tableRow);
		if (DbRoot_isTypeView_filter(row))	snprintf(desc, sizeof(desc), "view_%lld", tableRow);
		if (DbRoot_isTypeView_summary(row))	snprintf(desc, sizeof(desc), "view_%lld", tableRow);

		if (Std_sizeCHAR(desc))
		{
			BIG row2 = _UiRoot_getSubsRow(row);
			GuiItem_setDropIN((GuiItem*)b, desc, DbRows_initLinkN(DbRoot_subs(), row2));
		}
	}

	//GuiItem_setDropCallback((GuiItem*)b, &UiRoot_dropCallback);

	//GuiItem_setIconDoubleTouchCall((GuiItem*)b, &UiRoot_clickSubExpand);
}

static void _UiRoot_buildMenuTable(GuiItemMenu* menu, UBIG row)
{
	GuiItemMenu_addItemIcon(menu, GuiImage_new1(UiIcons_init_table_filter()), DbValue_initLang("VIEW_FILTER"), &UiRoot_clickAddSub, FALSE, UI_ADD_VIEW_FILTER);
	GuiItemMenu_addItemIcon(menu, GuiImage_new1(UiIcons_init_cards()), DbValue_initLang("VIEW_CARDS"), &UiRoot_clickAddSub, FALSE, UI_ADD_VIEW_CARDS);
	GuiItemMenu_addItemIcon(menu, GuiImage_new1(UiIcons_init_group()), DbValue_initLang("VIEW_GROUP"), &UiRoot_clickAddSub, FALSE, UI_ADD_VIEW_GROUP);
	GuiItemMenu_addItemIcon(menu, GuiImage_new1(UiIcons_init_map()), DbValue_initLang("VIEW_MAP"), &UiRoot_clickAddSub, FALSE, UI_ADD_VIEW_MAP);
	//GuiItemMenu_addItemIcon(menu, GuiImage_new1(UiIcons_init_kanban()), DbValue_initLang("VIEW_KANBAN"), &UiRoot_clickAddSub, FALSE, UI_ADD_VIEW_KANBAN);
	GuiItemMenu_addItemIcon(menu, GuiImage_new1(UiIcons_init_graph()), DbValue_initLang("VIEW_CHART"), &UiRoot_clickAddSub, FALSE, UI_ADD_VIEW_CHART);

	GuiItemMenu_addItemIcon(menu, GuiImage_new1(UiIcons_init_column_insight()), DbValue_initLang("VIEW_SUMMARY"), &UiRoot_clickAddSub, FALSE, UI_ADD_VIEW_SUMMARY);

	//GuiItemMenu_addItemIcon(menu, GuiImage_new1(UiIcons_init_calendar()), DbValue_initLang("VIEW_CALENDAR), &UiRoot_clickAddSub, FALSE, Gui_VIEW_CALENDAR);
	//GuiItemMenu_addItemIcon(menu, GuiImage_new1(UiIcons_init_timeline()), DbValue_initLang("VIEW_TIMELINE), &UiRoot_clickAddSub, FALSE, Gui_VIEW_TIMELINE);

	GuiItem_setAttribute((GuiItem*)menu, "row", row);
}

void UiRoot_clickImport(GuiItem* item)
{
	BIG row = GuiItem_findAttribute(item, "row");
	BIG type = GuiItem_findAttribute(item, "type");
	GuiItemRoot_addDialogLayout(UiDialogImport_new(row, type));
}

void UiRoot_clickExport(GuiItem* item)
{
	BIG row = GuiItem_findAttribute(item, "row");
	BIG type = GuiItem_findAttribute(item, "type");
	GuiItemRoot_addDialogLayout(UiDialogExport_new(row, type));
}

void UiRoot_clickExportGraphImage(GuiItem* item)
{
	BIG row = GuiItem_findAttribute(item, "row");
	GuiItemRoot_addDialogLayout(UiDialogExportImage_new(row));
}

void UiRoot_clickCreateFilter(GuiItem* item)
{
	BIG groupRow = GuiItem_findAttribute(item, "row");
	BIG filterRow = DbRoot_createView_filter(DbRoot_findParent(groupRow));

	DbRows groupF = DbRows_initFilter(groupRow);
	DbRows_hasChanged(&groupF);

	DbRoot_setName(filterRow, Lang_find("FILTER"));

	DbRows selectRows = DbRows_initLinkN(DbRoot_subs(), DbRows_findSubType(filterRow, "select"));
	DbRows shortRows = DbRows_initLinkN(DbRoot_subs(), DbRows_findSubType(filterRow, "short"));

	DbRows filterSubs = DbRows_initLinkN(DbFilter_getColumnGroupSubs(groupF.filter), 0);

	StdString strTemp = StdString_init();
	const BIG N = DbRows_getSubsNum(groupRow, "group", TRUE);
	BIG i;
	for (i = 0; i < N; i++)
	{
		DbColumn* column = DbRows_getSubsColumn(groupRow, "group", TRUE, i);
		BOOL ascending = DbRows_getSubsOptionNumber(groupRow, "group", TRUE, i, "ascending", FALSE);
		UBIG selectPos = Std_bmax(0, DbRows_getSubsOptionNumber(groupRow, "group", TRUE, i, "select", FALSE));

		//select
		{
			BIG r = DbRows_addNewRow(&selectRows);
			DbColumn1_set(DbRoot_ref(), DbRoot_findOrCreateChildType(r, "column"), DbColumn_getRow(column));

			//func
			DbValue_setOptionNumber(r, "func", 0);	//0 is "="

			//value
			BIG filterSubRow = DbRows_getRow(&filterSubs, selectPos);
			filterSubs.row = filterSubRow;
			BIG valueRow = DbColumnN_getFirstRow(DbFilter_getColumnGroupRows(groupF.filter), filterSubRow);
			DbValue_setOptionString(r, "value", DbColumn_getStringCopyLong(column, valueRow, &strTemp));
		}

		//shorting
		{
			BIG r = DbRows_addNewRow(&shortRows);
			DbColumn1_set(DbRoot_ref(), r, DbColumn_getRow(column));
			DbValue_setOptionNumber(r, "ascending", ascending);
		}
	}

	StdString_free(&strTemp);

	DbRows_free(&filterSubs);
	DbRows_free(&selectRows);
	DbRows_free(&shortRows);
	DbRows_free(&groupF);

	DbRoot_setPanelLeft(filterRow);
}

GuiItem* UiRoot_createMenu(Quad2i grid, BIG row)
{
	GuiItemMenu* menu = (GuiItemMenu*)GuiItemMenu_new(grid, DbValue_initStaticCopy(_UNI32("\u2261")), FALSE);
	GuiItem_setAttribute((GuiItem*)menu, "row", row);
	GuiItemMenu_setUnderline(menu, FALSE);

	if (DbRoot_isType_folder(row))
	{
		GuiItemMenu_addItemIcon(menu, GuiImage_new1(UiIcons_init_table()), DbValue_initLang("ADD_TABLE"), &UiRoot_clickAddSub, FALSE, UI_ADD_TABLE);
		//GuiItemMenu_addItemIcon(menu, GuiImage_new1(UiIcons_init_page()), DbValue_initLang("ADD_PAGE), &UiRoot_clickAddSub, FALSE, UI_ADD_PAGE);
		GuiItemMenu_addItemEmpty(menu);
	}

	if (DbRoot_isType_remote(row))
	{
		//? ...
		//GuiItemMenu_addItemEmpty(menu);
	}

	if (DbRoot_isTypeView_chart(row))
	{
		GuiItemMenu_addItemIcon(menu, GuiImage_new1(UiIcons_init_image()), DbValue_initLang("EXPORT_GRAPH_IMAGE"), &UiRoot_clickExportGraphImage, FALSE, 0);
		GuiItemMenu_addItemEmpty(menu);
	}

	if (DbRoot_isTypeView_group(row))
	{
		GuiItemMenu_addItemIcon(menu, GuiImage_new1(UiIcons_init_table_filter()), DbValue_initLang("EXPORT_GROUP_MAKE_FILTER"), &UiRoot_clickCreateFilter, FALSE, 0);
		GuiItemMenu_addItemEmpty(menu);
	}

	if (DbRoot_isType_folder(row))
	{
		GuiItemMenu_addItemIcon(menu, GuiImage_new1(UiIcons_init_import()), DbValue_initLang("IMPORT_CSV"), &UiRoot_clickImport, FALSE, UI_IMPORT_CSV);
		GuiItemMenu_addItemIcon(menu, GuiImage_new1(UiIcons_init_import()), DbValue_initLang("IMPORT_TSV"), &UiRoot_clickImport, FALSE, UI_IMPORT_TSV);
		GuiItemMenu_addItemIcon(menu, GuiImage_new1(UiIcons_init_import()), DbValue_initLang("IMPORT_FILES"), &UiRoot_clickImport, FALSE, UI_IMPORT_FILES);
		GuiItemMenu_addItemEmpty(menu);
	}

	if (!DbRoot_isType_folder(row) && (DbRoot_isType_table(row) || DbRoot_isTypeView_filter(row) || DbRoot_isTypeView_summary(row)))
	{
		GuiItemMenu_addItemIcon(menu, GuiImage_new1(UiIcons_init_export()), DbValue_initLang("EXPORT_CSV"), &UiRoot_clickExport, FALSE, UI_EXPORT_CSV);
		GuiItemMenu_addItemIcon(menu, GuiImage_new1(UiIcons_init_export()), DbValue_initLang("EXPORT_TSV"), &UiRoot_clickExport, FALSE, UI_EXPORT_TSV);
		GuiItemMenu_addItemIcon(menu, GuiImage_new1(UiIcons_init_export()), DbValue_initLang("EXPORT_HTML"), &UiRoot_clickExport, FALSE, UI_EXPORT_HTML);
		//GuiItemMenu_addItem(menu, DbValue_initLang("EXPORT_SQL"), GuiItemCall_newRow(&UiRoot_clickExportSql, self, row), FALSE);
		GuiItemMenu_addItemEmpty(menu);
	}

	if (DbRoot_isReference(row))
	{
		GuiItemMenu_addItemIcon(menu, GuiImage_new1(UiIcons_init_reference()), DbValue_initLang("OPEN_REFERENCE_SOURCE"), &UiRoot_clickOpenReferenceSource, FALSE, -1);
		GuiItemMenu_addItemIcon(menu, GuiImage_new1(UiIcons_init_reference()), DbValue_initLang("CONVERT_REFERENCE_PERNAMENT"), &UiRoot_clickReferenceToPernament, FALSE, -1);
		GuiItemMenu_addItemEmpty(menu);
	}
	else
		if (DbRoot_isTypeView(row))
		{
			GuiItemMenu_addItemIcon(menu, GuiImage_new1(UiIcons_init_reference()), DbValue_initLang("MAKE_REFERENCE"), &UiRoot_clickCreateReference, FALSE, -1);
			GuiItemMenu_addItemEmpty(menu);
		}

	GuiItemMenu_addItemIcon(menu, GuiImage_new1(UiIcons_init_duplicate()), DbValue_initLang("DUPLICATE"), &UiRoot_clickDuplicate, FALSE, -1);

	if (DbRoot_isTypeView_filter(row) || DbRoot_isTypeView_summary(row))
	{
		const BIG numViews = DbColumnN_sizeActive(DbRoot_subs(), DbRoot_findChildType(row, "views"));
		if (numViews)
			GuiItemMenu_addItemIcon(menu, GuiImage_new1(UiIcons_init_trash()), DbValue_initLang("REMOVE_KEEP_SUB_VIEWS"), &UiRoot_clickRemoveKeepSubViews, TRUE, -1);
	}

	GuiItemMenu_addItemIcon(menu, GuiImage_new1(UiIcons_init_trash()), DbValue_initLang("REMOVE"), &UiRoot_clickRemove, TRUE, -1);

	return (GuiItem*)menu;
}

GuiItem* UiRoot_createMenuNameHeader(Quad2i grid, BIG row)
{
	GuiItemLayout* layout = GuiItemLayout_new(grid);
	GuiItemLayout_setBackgroundWhite(layout, TRUE);
	GuiItemLayout_addColumn(layout, 1, 99);
	GuiItemLayout_addRow(layout, 0, 99);

	GuiItem_setAttribute((GuiItem*)layout, "row", row);

	//menu
	GuiItem_addSubName((GuiItem*)layout, "menu", UiRoot_createMenu(Quad2i_init4(0, 0, 1, 1), row));

	//name
	GuiItem* name;
	if (DbRoot_isReference(row))
		name = GuiItem_addSubName((GuiItem*)layout, "name", GuiItemButton_newBlackEx(Quad2i_init4(1, 0, 1, 1), DbValue_initStatic(_UiRoot_getItemName(row)), &UiRoot_clickOpenReferenceSource));

	else
	{
		name = GuiItem_addSubName((GuiItem*)layout, "name", GuiItemEdit_new(Quad2i_init4(1, 0, 1, 1), DbValue_initOption(row, "name", 0), DbValue_initLang("NAME")));
		GuiItem_setIcon(name, GuiImage_new1(UiIcons_init_name()));
	}

	return (GuiItem*)layout;
}

static void _UiRoot_buildDashboardSub(GuiItemLayout* layoutList, BIG viewLeft, BIG viewRight, UBIG row, int* y, const UNI* search, GuiItemLayout* layoutHide, int* global_y);

static GuiItemLayout* _UiRoot_buildDashboard(BIG viewLeft, BIG viewRight, UBIG row, UBIG parentRow, int* y, const UNI* search, GuiItemLayout* layoutHide, int* global_y)
{
	GuiItemLayout* layout = GuiItemLayout_new(Quad2i_init4(0, *y, 1, 1));
	GuiItemLayout_addColumn(layout, 1, 99);
	GuiItem_setAttribute((GuiItem*)layout, "row", row);

	/*	{
			GuiItem* treehide = GuiItem_addSubName((GuiItem*)layoutHide, "hide", GuiItemButton_newImage(Quad2i_init4(0, (*global_y)-1, 1, 1), GuiImage_new1(UiRoot_isShowSub(row) ? UiIcons_init_tree_hide() : UiIcons_init_tree_show()), &UiRoot_clickSubExpand));
			GuiItem_setAttribute(treehide, "row", row);
		}*/

	{
		_UiRoot_buildListItem(viewLeft, viewRight, layout, Quad2i_init4(0, 0, 2, 1), row, parentRow, search);

		if (DbRoot_isType_table(row) || DbRoot_isTypeView_filter(row) || DbRoot_isTypeView_summary(row))
		{
			GuiItemMenu* menu = (GuiItemMenu*)GuiItem_addSubName((GuiItem*)layout, "+", GuiItemMenu_newImage(Quad2i_init4(2, 0, 1, 1), GuiImage_new1(UiIcons_init_add_view()), TRUE));
			GuiItemMenu_setUnderline(menu, FALSE);
			_UiRoot_buildMenuTable(menu, row);
		}
		else
			if (DbRoot_isType_folder(row))
			{
				GuiItemButton* b;
				b = (GuiItemButton*)GuiItem_addSubName((GuiItem*)layout, "+", GuiItemButton_newImage(Quad2i_init4(2, 0, 1, 1), GuiImage_new1(UiIcons_init_add_table()), TRUE, &UiRoot_clickAddSub));
				GuiItem_setAttribute((GuiItem*)b, "row", row);
				GuiItem_setAttribute((GuiItem*)b, "type", UI_ADD_TABLE);
			}

		//menu
		GuiItem_addSubName((GuiItem*)layout, "menu", UiRoot_createMenu(Quad2i_init4(3, 0, 1, 1), row));
	}

	int yy = 0;

	BIG row2 = _UiRoot_getSubsRow(row);
	if (row2 >= 0 && UiRoot_isShowSub(row))
	{
		GuiItemLayout* layoutList = GuiItemLayout_new(Quad2i_init4(1, 1, 3, 1));
		_UiRoot_buildDashboardSub(layoutList, viewLeft, viewRight, row2, &yy, search, layoutHide, global_y);

		if (yy)
		{
			GuiItem_addSubName((GuiItem*)layout, "layoutSub", (GuiItem*)layoutList);
			GuiItemLayout_addColumn(layoutList, 0, 99);
			GuiItemLayout_addRow(layout, 1, yy);
		}
		else
			GuiItem_delete((GuiItem*)layoutList);
	}

	GuiItem_setGrid((GuiItem*)layout, Quad2i_init4(0, *y, 1, 1 + yy));
	*y += 1 + yy;

	return layout;
}

static void _UiRoot_buildDashboardSub(GuiItemLayout* layoutList, BIG viewLeft, BIG viewRight, UBIG row, int* y, const UNI* search, GuiItemLayout* layoutHide, int* global_y)
{
	BIG it;
	UBIG i = 0;
	while ((it = DbColumnN_jump(DbRoot_subs(), row, &i, 1)) >= 0)
	{
		if (!DbRoot_isType_panel(it))
		{
			const BOOL show2 = _UiRoot_showButtonSearchSub(it, search);
			if (show2)
			{
				const BOOL hasSubs = _UiRoot_canGoSubs(it);
				if (hasSubs)
				{
					GuiItem* treehide = GuiItem_addSubName((GuiItem*)layoutHide, "hide", GuiItemButton_newImage(Quad2i_init4(0, *global_y, 1, 1), GuiImage_new1(UiRoot_isShowSub(it) ? UiIcons_init_tree_hide() : UiIcons_init_tree_show()), TRUE, &UiRoot_clickSubExpand));
					GuiItem_setAttribute(treehide, "row", it);

					BIG it2 = _UiRoot_getSubsRow(it);
					GuiItem_setEnable(treehide, (it2 >= 0 && DbColumnN_sizeActive(DbRoot_subs(), it2)));
				}
				*global_y += 1;

				char subName[64];
				Std_buildNumber(it, 0, subName);

				if (hasSubs)
				{
					GuiItem_addSubName((GuiItem*)layoutList, subName, (GuiItem*)_UiRoot_buildDashboard(viewLeft, viewRight, it, row, y, search, layoutHide, global_y));
				}
				else
				{
					GuiItemLayout* lai = GuiItemLayout_new(Quad2i_init4(0, *y, 1, 1));
					GuiItemLayout_addColumn(lai, 0, 99);
					GuiItem_addSubName((GuiItem*)layoutList, subName, (GuiItem*)lai);

					_UiRoot_buildListItem(viewLeft, viewRight, lai, Quad2i_init4(0, 0, 1, 1), it, row, search);

					GuiItem_addSubName((GuiItem*)lai, "menu", UiRoot_createMenu(Quad2i_init4(1, 0, 1, 1), it));
					(*y)++;
				}
			}
		}
		i++;
	}
}

void UiRoot_clickSearchDelete(GuiItem* item)
{
	GuiItemEdit* searchEd = GuiItem_findName(item, "search");
	GuiItemEdit_setText(searchEd, 0);
}

void UiRoot_rebuildList(GuiItem* item)
{
	GuiItemLayout* list = GuiItem_findName(item, "list");
	GuiItemEdit* searchEd = GuiItem_findName(item, "search");

	if (list && searchEd)
	{
		GuiItem_freeSubs((GuiItem*)list);

		BIG viewLeft = DbRoot_getPanelLeft();
		BIG viewRight = DbRoot_getPanelRight();

		const UNI* search = GuiItemEdit_getTextOrCache(searchEd);
		int y = 0;
		int global_y = 0;
		//GuiItemLayout_addColumn(list, 0, 1);
		GuiItemLayout_addColumn(list, 1, 99);
		//GuiItemLayout_addRow(list, 0, 99);

		GuiItemLayout* listHide = GuiItemLayout_new(Quad2i_init());
		GuiItemLayout* listReal = GuiItemLayout_new(Quad2i_init());
		GuiItemLayout_addColumn(listReal, 0, 99);
		GuiItem_addSubName((GuiItem*)list, "hideLay", (GuiItem*)listHide);
		GuiItem_addSubName((GuiItem*)list, "realLay", (GuiItem*)listReal);

		_UiRoot_buildDashboardSub(listReal, viewLeft, viewRight, DbRoot_getRootRow(), &y, search, listHide, &global_y);

		GuiItem_setGrid((GuiItem*)listHide, Quad2i_init4(0, 0, 1, global_y));
		GuiItem_setGrid((GuiItem*)listReal, Quad2i_init4(1, 0, 1, global_y));

		/*		global_y++;	//space

				//+Folder
				GuiItem* b = GuiItem_addSubName((GuiItem*)list, "+folder", GuiItemButton_newAlphaEx(Quad2i_init4(0, global_y++, 2, 1), DbValue_initLang("ADD_FOLDER"), &UiRoot_clickAddSub));
				GuiItemButton_setTextCenter((GuiItemButton*)b, FALSE);
				GuiItem_setIcon(b, GuiImage_new1(UiIcons_init_add_folder()));
				GuiItem_setEnable(b, !Std_sizeUNI(search));
				GuiItem_setAttribute((GuiItem*)b, "row", DbRoot_getRootRow());	//root
				GuiItem_setAttribute((GuiItem*)b, "type", UI_ADD_FOLDER);

				//+Import
				b = GuiItem_addSubName((GuiItem*)list, "+import", GuiItemButton_newAlphaEx(Quad2i_init4(0, global_y++, 2, 1), DbValue_initLang("ADD_IMPORT"), &UiRoot_clickImport));
				GuiItemButton_setTextCenter((GuiItemButton*)b, FALSE);
				GuiItem_setIcon(b, GuiImage_new1(UiIcons_init_import()));
				GuiItem_setEnable(b, !Std_sizeUNI(search));
				GuiItem_setAttribute((GuiItem*)b, "row", -1);	//root
				GuiItem_setAttribute((GuiItem*)b, "type", UI_IMPORT_CSV);

				//+Remote
				b = GuiItem_addSubName((GuiItem*)list, "+folder", GuiItemButton_newAlphaEx(Quad2i_init4(0, global_y++, 2, 1), DbValue_initLang("ADD_REMOTE"), &UiRoot_clickAddSub));
				GuiItemButton_setTextCenter((GuiItemButton*)b, FALSE);
				GuiItem_setIcon(b, GuiImage_new1(UiIcons_init_add_remote()));
				GuiItem_setEnable(b, !Std_sizeUNI(search));
				GuiItem_setAttribute((GuiItem*)b, "row", DbRoot_getRootRow());	//root
				GuiItem_setAttribute((GuiItem*)b, "type", UI_ADD_REMOTE);*/
	}
}

void UiRoot_rebuildPanel(GuiItem* item, BIG row, BOOL left)
{
	item = GuiItem_findName(item, left ? "panelL" : "panelR");

	BIG origRow = row;

	if (DbRoot_isType_folder(origRow))
		UiRootFolder_build((GuiItemLayout*)item, row);

	if (DbRoot_isType_remote(origRow))
		UiRootRemote_build((GuiItemLayout*)item, row);

	if (DbRoot_isType_table(origRow) || DbRoot_isTypeView_filter(origRow))
		UiRootTable_build((GuiItemLayout*)item, row, DbValue_initOption(row, left ? "scrollVL" : "scrollVR", 0), DbValue_initOption(row, left ? "scrollHL" : "scrollHR", 0), DbValue_initOption(row, left ? "gridL" : "gridR", _UNI32("0 0 0 0")), DbValue_initOption(row, left ? "searchL" : "searchR", 0));

	if (DbRoot_isTypeView_summary(origRow))
		UiRootSummary_build((GuiItemLayout*)item, row, DbValue_initOption(row, left ? "scrollVL" : "scrollVR", 0), DbValue_initOption(row, left ? "scrollHL" : "scrollHR", 0), DbValue_initOption(row, left ? "gridL" : "gridR", _UNI32("0 0 0 0")), DbValue_initOption(row, left ? "searchL" : "searchR", 0));

	if (DbRoot_isTypeView_group(origRow))
		UiRootGroup_build((GuiItemLayout*)item, row, DbValue_initOption(row, left ? "scrollHL" : "scrollHR", 0));

	if (DbRoot_isTypeView_cards(origRow))
		UiRootCard_build((GuiItemLayout*)item, row, DbValue_initOption(row, left ? "scrollVL" : "scrollVR", 0), DbValue_initOption(row, left ? "scrollQL" : "scrollQR", 0));

	if (DbRoot_isTypeView_chart(origRow))
		UiRootChart_build((GuiItemLayout*)item, row, DbValue_initOption(row, left ? "scrollHL" : "scrollHR", 0), DbValue_initOption(row, left ? "scrollQL" : "scrollQR", 0), DbValue_initOption(row, left ? "scrollPL" : "scrollPR", 0));

	//	if (DbRoot_isTypeView_kanban(origRow))
	//		UiRootKanban_build((GuiItemLayout*)item, row);

	if (DbRoot_isTypeView_map(origRow))
		UiRootMap_build((GuiItemLayout*)item, row, DbValue_initOption(row, left ? "cam_latL" : "cam_latR", 0), DbValue_initOption(row, left ? "cam_longL" : "cam_longR", 0), DbValue_initOption(row, left ? "cam_zoomL" : "cam_zoomR", 0), DbValue_initOption(row, left ? "searchL" : "searchR", 0), DbValue_initOption(row, left ? "scrollQL" : "scrollQR", 0), DbValue_initOption(row, left ? "scrollPL" : "scrollPR", 0));
}

void UiRoot_rebuildPanelLeft(GuiItem* item)
{
	if (DbRoot_getPanelLeft() < 0)
		_UiRoot_setLeft(0);

	UiRoot_rebuildPanel(item, DbRoot_getPanelLeft(), TRUE);
}
void UiRoot_rebuildPanelRight(GuiItem* item)
{
	if (DbRoot_getPanelRight() < 0)
		_UiRoot_setRight(0);

	UiRoot_rebuildPanel(item, DbRoot_getPanelRight(), FALSE);
}

void UiRoot_rebuildBase(GuiItem* item)
{
	BIG viewLeft = DbRoot_getPanelLeft();
	BIG viewRight = DbRoot_getPanelRight();

	int panelsCells = (Win_getImage(UiScreen_getWin()).size.x / OsWinIO_cellSize()) - _UiRoot_getSideValueNumber();
	int leftCells = Std_max(5, _UiRoot_getSplitValueNumber());
	int rightCells = Std_max(5, panelsCells - _UiRoot_getSplitValueNumber());

	GuiItemLayout* layout = (GuiItemLayout*)item;
	GuiItemLayout_addColumn(layout, 0, Std_max(2, _UiRoot_getSideValueNumber()));
	GuiItemLayout_addColumn(layout, 1, leftCells);
	GuiItemLayout_addColumn(layout, 3, rightCells);
	GuiItemLayout_addRow(layout, 0, 100);

	//free
	GuiItem_remove(GuiItem_findName(item, "panelL"));	//left
	GuiItem_remove(GuiItem_findName(item, "panelR"));	//right
	GuiItem_remove(GuiItem_findName(item, "split"));

	//Left panel
	if (viewLeft >= 0)
	{
		GuiItemLayout* panelLayout = GuiItemLayout_new(Quad2i_init4(1, 0, (viewRight >= 0) ? 1 : 3, 1));
		GuiItemLayout_setBackgroundWhite(panelLayout, TRUE);
		GuiItem_addSubName((GuiItem*)layout, "panelL", (GuiItem*)panelLayout);
		GuiItemLayout_setResize(panelLayout, &UiRoot_rebuildPanelLeft);

		if (viewRight >= 0)
			GuiItem_setChangeSize((GuiItem*)panelLayout, TRUE, _UiRoot_getSplitValue(), TRUE);
	}

	//Right panel
	if (viewRight >= 0)
	{
		GuiItemLayout* panelLayout = GuiItemLayout_new(Quad2i_init4(3, 0, 1, 1));
		GuiItemLayout_setBackgroundWhite(panelLayout, TRUE);
		GuiItem_addSubName((GuiItem*)layout, "panelR", (GuiItem*)panelLayout);
		GuiItemLayout_setResize(panelLayout, &UiRoot_rebuildPanelRight);

		//Split (X)
		GuiItemLayout* layMid = GuiItemLayout_new(Quad2i_init4(2, 0, 1, 1));
		GuiItem_addSubName((GuiItem*)layout, "split", (GuiItem*)layMid);
		GuiItemLayout_addRow(layMid, 0, 100);
		GuiItemLayout_addRow(layMid, 2, 2);
		GuiItemLayout_addRow(layMid, 4, 100);
		GuiItem_addSubName((GuiItem*)layMid, ">>", GuiItemButton_newAlphaEx(Quad2i_init4(0, 1, 1, 1), DbValue_initStaticCopy(_UNI32(">>")), &UiRoot_clickSplitCloseRight));
		GuiItem_addSubName((GuiItem*)layMid, "<<", GuiItemButton_newAlphaEx(Quad2i_init4(0, 3, 1, 1), DbValue_initStaticCopy(_UNI32("<<")), &UiRoot_clickSplitCloseLeft));
		GuiItem_addSubName((GuiItem*)layMid, "<>", GuiItemButton_newAlphaEx(Quad2i_init4(0, 5, 1, 1), DbValue_initStaticCopy(_UNI32("<>")), &UiRoot_clickSplitSwap));
	}

	//change Window/Fullscreen mode
	GuiItemMenu* menu = GuiItem_findName(item, "logo");
	if (menu)
	{
		DbValue* itV = _GuiItemMenu_findItem(menu, Lang_find(!UiScreen_isFullScreen() ? "WINDOW_MODE" : "FULLSCREEN_MODE"));
		if (itV)
			itV->lang_id = (UiScreen_isFullScreen() ? "WINDOW_MODE" : "FULLSCREEN_MODE");
	}
}

void UiRoot_rebuildSide(GuiItem* item)
{
	GuiItem_freeSubs(item);

	GuiItemLayout* sideLayout = (GuiItemLayout*)item;
	GuiItemLayout_clearArrays(sideLayout);

	GuiItemLayout_addColumn(sideLayout, 0, 100);
	GuiItemLayout_addColumn(sideLayout, 1, 100);
	GuiItemLayout_addRow(sideLayout, 3, 100);

	//logo
	GuiItem* logo = GuiItem_addSubName((GuiItem*)sideLayout, "logo", UiRootMenu_new(Quad2i_init4(0, 0, 1, 1)));
	//save
	GuiItem* save = GuiItem_addSubName((GuiItem*)sideLayout, "save", GuiItemButton_newBlackEx(Quad2i_init4(1, 0, 1, 1), DbValue_initLang("SAVE"), &UiRoot_clickSave));

	//rewrite current
	BOOL sidePanel = _UiRoot_getSideValueNumber() > 3;
	GuiItem_setShow(save, sidePanel);
	GuiItem_setGrid(logo, Quad2i_init4(0, 0, 1 + !sidePanel, 1));

	//search
	GuiItemLayout* searchLayout = GuiItemLayout_new(Quad2i_init4(0, 1, 2, 1));
	GuiItemLayout_addColumn(searchLayout, 0, 100);
	GuiItem_addSubName((GuiItem*)sideLayout, "searchLayout", (GuiItem*)searchLayout);
	{
		GuiItemEdit* search = (GuiItemEdit*)GuiItem_addSubName((GuiItem*)searchLayout, "search", GuiItemEdit_new(Quad2i_init4(0, 0, 1, 1), _UiRoot_getSearchValue(), DbValue_initLang("SEARCH")));
		GuiItemEdit_setHighlightIfContent(search, TRUE);
		GuiItem_setIcon((GuiItem*)search, GuiImage_new1(UiIcons_init_search()));
		GuiItemEdit_setFnChanged(search, &UiRoot_clickEditSearch);

		GuiItem_addSubName((GuiItem*)searchLayout, "X", GuiItemButton_newAlphaEx(Quad2i_init4(1, 0, 1, 1), DbValue_initStaticCopy(_UNI32("X")), &UiRoot_clickSearchDelete));
	}

	//+Folder, etc.
	GuiItemLayout* addLayout = GuiItemLayout_new(Quad2i_init4(0, 2, 2, 1));
	GuiItemLayout_setBackgroundWhite(addLayout, TRUE);
	GuiItemLayout_addColumn(addLayout, 0, 20);
	GuiItemLayout_addColumn(addLayout, 1, 20);
	GuiItemLayout_addColumn(addLayout, 2, 20);
	GuiItem_addSubName((GuiItem*)sideLayout, "addLayout", (GuiItem*)addLayout);
	{
		const UNI* search = GuiItemEdit_getText(GuiItem_findName(item, "search"));

		//+Folder
		GuiItem* b = GuiItem_addSubName((GuiItem*)addLayout, "+folder", GuiItemButton_newImage(Quad2i_init4(0, 0, 1, 1), GuiImage_new1(UiIcons_init_add_folder()), TRUE, &UiRoot_clickAddSub));
		//GuiItemButton_setTextCenter((GuiItemButton*)b, FALSE);
		//GuiItem_setIcon(b, GuiImage_new1(UiIcons_init_add_folder()));
		GuiItem_setEnable(b, !Std_sizeUNI(search));
		GuiItem_setAttribute((GuiItem*)b, "row", DbRoot_getRootRow());	//root
		GuiItem_setAttribute((GuiItem*)b, "type", UI_ADD_FOLDER);

		//+Import
		b = GuiItem_addSubName((GuiItem*)addLayout, "+import", GuiItemButton_newImage(Quad2i_init4(1, 0, 1, 1), GuiImage_new1(UiIcons_init_import()), TRUE, &UiRoot_clickImport));
		//GuiItemButton_setTextCenter((GuiItemButton*)b, FALSE);
		//GuiItem_setIcon(b, GuiImage_new1(UiIcons_init_import()));
		GuiItem_setEnable(b, !Std_sizeUNI(search));
		GuiItem_setAttribute((GuiItem*)b, "row", -1);	//root
		GuiItem_setAttribute((GuiItem*)b, "type", UI_IMPORT_CSV);

		//+Remote
		b = GuiItem_addSubName((GuiItem*)addLayout, "+folder", GuiItemButton_newImage(Quad2i_init4(2, 0, 2, 1), GuiImage_new1(UiIcons_init_add_remote()), TRUE, &UiRoot_clickAddSub));
		//GuiItemButton_setTextCenter((GuiItemButton*)b, FALSE);
		//GuiItem_setIcon(b, GuiImage_new1(UiIcons_init_add_remote()));
		GuiItem_setEnable(b, !Std_sizeUNI(search));
		GuiItem_setAttribute((GuiItem*)b, "row", DbRoot_getRootRow());	//root
		GuiItem_setAttribute((GuiItem*)b, "type", UI_ADD_REMOTE);
	}

	//list panel
	{
		GuiItemLayout* projectsLayout = GuiItemLayout_new(Quad2i_init4(0, 3, 2, 1));
		GuiItem_addSubName((GuiItem*)sideLayout, "list", (GuiItem*)projectsLayout);
		GuiItemLayout_setResize(projectsLayout, &UiRoot_rebuildList);
		GuiItemLayout_setScrollV(projectsLayout, DbValue_initOption(DbRoot_getRootRow(), "scrollS", 0));
	}

	//connection info
	{
		GuiItemLayout* netLayout = GuiItemLayout_new(Quad2i_init4(0, 4, 2, 1));
		GuiItem_addSubName((GuiItem*)sideLayout, "net", (GuiItem*)netLayout);
		GuiItemLayout_addColumn(netLayout, 0, 5);
		GuiItemLayout_addColumn(netLayout, 1, 99);

		GuiItem_setLoopTouch((GuiItem*)netLayout, &UiRoot_updateNet);

		GuiItemComboStatic* newOnOff = (GuiItemComboStatic*)GuiItem_addSubName((GuiItem*)netLayout, "onOff", GuiItemComboStatic_newEx(Quad2i_init4(0, 0, 1, 1), DbValue_initNumber(!MediaNetwork_is()), 0, DbValue_initEmpty(), &UiRoot_clickConnect));
		GuiItem_setIcon((GuiItem*)newOnOff, GuiImage_new1(MediaNetwork_is() ? UiIcons_init_internet_online() : UiIcons_init_internet_offline()));
		GuiItemComboStatic_addItem(newOnOff, DbValue_initLang("NET_ONLINE"));
		GuiItemComboStatic_addItem(newOnOff, DbValue_initLang("NET_OFFLINE"));

		GuiItem* bandwidth = GuiItem_addSubName((GuiItem*)netLayout, "bandwidth", GuiItemText_new(Quad2i_init4(1, 0, 1, 1), TRUE, DbValue_initStaticCopyCHAR("0 b/s"), DbValue_initEmpty()));
		GuiItem_setAttribute(bandwidth, "time", Os_time());
	}

	//Cloud project has changed
	if (UiScreen_isIndexChanged())
	{
		GuiItemLayout* cloudLayout = GuiItemLayout_new(Quad2i_init4(0, 5, 2, 2));
		GuiItemLayout_setBackgroundError(cloudLayout, TRUE);
		GuiItem_addSubName((GuiItem*)sideLayout, "cloud", (GuiItem*)cloudLayout);
		GuiItemLayout_addColumn(cloudLayout, 0, 99);
		GuiItemLayout_addColumn(cloudLayout, 1, 99);

		GuiItem_addSubName((GuiItem*)cloudLayout, "info", GuiItemText_new(Quad2i_init4(0, 0, 2, 1), TRUE, DbValue_initLang("CLOUD_PROJECT_HAS_CHANGED"), DbValue_initEmpty()));

		GuiItem_addSubName((GuiItem*)cloudLayout, "reload", GuiItemButton_newBlackEx(Quad2i_init4(0, 1, 1, 1), DbValue_initLang("CLOUD_RELOAD"), &UiRoot_clickCloudReload));
		GuiItem_addSubName((GuiItem*)cloudLayout, "ignore", GuiItemButton_newBlackEx(Quad2i_init4(1, 1, 1, 1), DbValue_initLang("CLOUD_IGNORE"), &UiRoot_clickCloudIgnore));
	}
}

GuiItemLayout* UiRoot_new(void)
{
	GuiItemLayout* layout = GuiItemLayout_new(Quad2i_init());
	GuiItemLayout_setResize(layout, &UiRoot_rebuildBase);

	GuiItem_setShortcutKey((GuiItem*)layout, FALSE, Win_EXTRAKEY_CTRL | Win_EXTRAKEY_SAVE, 0, &UiRoot_clickSave);

	GuiItemLayout* sideLayout = GuiItemLayout_new(Quad2i_init4(0, 0, 1, 1));
	GuiItem_addSubName((GuiItem*)layout, "side", (GuiItem*)sideLayout);
	GuiItemLayout_setResize(sideLayout, &UiRoot_rebuildSide);
	GuiItem_setChangeSize((GuiItem*)sideLayout, TRUE, _UiRoot_getSideValue(), TRUE);

	return layout;
}

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

 //collaborate
#include "os.h"
#include "std.h"
#include "log.h"
#include "language.h"
#include "file.h"
#include "db.h"
#include "map.h"
#include "media.h"
#include "io.h"
#include "ui.h"	//icons


//header
#include "gui.h"

typedef struct GuiItemRoot_s GuiItemRoot;
void GuiItemRoot_showPicker(Vec2i pickerPos, BOOL pickerOpen, BOOL pickerFolder, BOOL pickerMultiple, const UNI* pickerAction, const UNI* pickerExts);
void GuiItemRoot_setDrawRectOver(Quad2i drawRectOver);
void GuiItemRoot_setDrawRectOver2(Quad2i drawRectOver);
void GuiItemRoot_addBufferRect(Quad2i rect);
void GuiItemRoot_redrawAll(void);
BOOL GuiItemRoot_hasChanges(void);
void GuiItemRoot_resetNumChanges(void);
void GuiItemRoot_addDialogRel(GuiItem* item, GuiItem* parent, Quad2i parentCoord, BOOL closeAfter);
void GuiItemRoot_addDialogRelLayout(GuiItemLayout* layout, GuiItem* parent, Quad2i parentCoord, BOOL closeAfter);
GuiItem* GuiItemRoot_findPath(const StdArr* origPath);

void GuiItem_closeParentLevel(GuiItem* self);
BOOL GuiItem_hasScrollLayoutV(const GuiItem* self);
BOOL GuiItem_hasScrollLayoutH(const GuiItem* self);

typedef struct GuiItemLevel_s GuiItemLevel;
void GuiItemLevel_tryCloseLater(GuiItemLevel* self);

GuiItemLayout* GuiItemTable_buildDialogLinks(DbColumn* column);
void GuiItemTable_clickImportFile(GuiItem* self);
void GuiItemTable_clickImportWeb(GuiItem* self);

GuiItem* GuiItem_newCopy(GuiItem* src, BOOL copySub);
void GuiItem_setDropRow(GuiItem* self, BIG row);
void GuiItem_delete(GuiItem* self);

GuiItem* GuiItemLevel_getBackChain(const GuiItemLevel* self);

GuiItemLayout* GuiItemTags_dialogFileAdd(GuiItemTags* self, DbColumn* column);

Image1 GuiItemChart_getIcon(BIG row);

#include "gui/gui_image.h"
#include "gui/gui_theme.h"
#include "gui/gui_item.h"
#include "gui/gui_scroll.h"
#include "gui/gui_edit.h"
#include "gui/gui_box.h"
#include "gui/gui_text.h"
#include "gui/gui_textMulti.h"

#include "gui/gui_level.h"

#include "gui/gui_button.h"
#include "gui/gui_check.h"
#include "gui/gui_slider.h"
#include "gui/gui_rating.h"
#include "gui/gui_particles.h"
#include "gui/gui_layout_array.h"
#include "gui/gui_layout.h"

//#include "gui/gui_design.h"
#include "gui/gui_menu.h"
#include "gui/gui_combo_static.h"
#include "gui/gui_combo_dynamic.h"
#include "gui/gui_list.h"
#include "gui/gui_struct.h"
//#include "gui/gui_todo.h"
#include "gui/gui_audio.h"
#include "gui/gui_gallery.h"

#include "gui/gui_calendarSmall.h"
#include "gui/gui_calendarBig.h"
#include "gui/gui_map.h"
#include "gui/gui_file.h"
#include "gui/gui_tags.h"
//#include "gui/gui_timeline.h"

#include "gui/gui_table.h"
#include "gui/gui_group.h"
//#include "gui/gui_kanban.h"
#include "gui/gui_chart.h"

#include "gui/gui_color.h"
#include "gui/gui_switch.h"
#include "gui/gui_item2.h"
#include "gui/gui_root.h"

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

typedef struct UiApp_s UiApp;

//collaborate
#include "os.h"
#include "std.h"
#include "language.h"
#include "file.h"
#include "db.h"
#include "gui.h"
#include "log.h"
#include "media.h"
#include "io.h"

//header
#include "ui.h"

typedef struct UiContent_s UiContent;
typedef struct UiDialog_s UiDialog;

typedef struct UiScreen_s UiScreen;
Win* UiScreen_getWin(void);
void UiScreen_switchFullScreenWindow(void);
void UiScreen_display(void);
void UiScreen_setDPI(int dpi);
BOOL UiScreen_isFullScreen(void);
void UiScreen_updateWindowTitle(void);
void UiScreen_askResize(void);
void UiScreen_askResizeHard(void);
char* UiScreen_getNameVersion(void);
UNI* UiScreen_getNameVersionUNI(void);
BOOL UiScreen_needSave(void);
void UiScreen_setWelcome(void);
void UiScreen_setStartup(void);
void UiScreen_setRoot(void);
void UiScreen_closeHard(void);
void UiScreen_closeAsk(void);
void UiScreen_updateTheme(void);
void UiScreen_openProject(const UNI* path, const UNI* password);
void UiScreen_createProject(const UNI* path, const UNI* password, BIG cycles);

void UiRoot_clickAddSub(GuiItem* item);
void UiRoot_clickImport(GuiItem* item);
void UiRoot_buildMenuImportExport(GuiItemMenu* menu, BIG row, BOOL importt, BOOL exportt, BOOL folder, BOOL addTable);

typedef enum
{
	UI_ADD_FOLDER,
	UI_ADD_TABLE,
	UI_ADD_PAGE,

	UI_ADD_VIEW_CARDS,
	UI_ADD_VIEW_FILTER,
	UI_ADD_VIEW_GROUP,
	UI_ADD_VIEW_KANBAN,
	UI_ADD_VIEW_CALENDAR,
	UI_ADD_VIEW_TIMELINE,
	UI_ADD_VIEW_CHART,
	UI_ADD_VIEW_MAP,
}UI_ADD;

#include "ui/ui_ini.h"
#include "ui/ui_update.h"

#include "ui/ui_icons.h"
#include "ui/ui_logo.h"

#include "ui/ui_dialog_license.h"
#include "ui/ui_dialog_changelog.h"
#include "ui/ui_dialog_create.h"
#include "ui/ui_dialog_password.h"
#include "ui/ui_dialog_open.h"
#include "ui/ui_dialog_save.h"
#include "ui/ui_dialog_settings_project.h"
#include "ui/ui_dialog_settings_enviroment.h"
#include "ui/ui_dialog_about.h"
#include "ui/ui_dialog_shortcuts.h"
#include "ui/ui_dialog_update.h"
#include "ui/ui_dialog_log.h"
#include "ui/ui_dialog_import.h"
#include "ui/ui_dialog_export.h"

#include "ui/ui_startup.h"
#include "ui/ui_welcome.h"

#include "ui/ui_table.h"
#include "ui/ui_cards.h"
#include "ui/ui_group.h"
//#include "ui/ui_chart.h"
//#include "ui/ui_map.h"
//#include "ui/ui_kanban.h"
#include "ui/ui_project.h"

#include "ui/ui_menu.h"
#include "ui/ui_root.h"
#include "ui/ui_screen.h"
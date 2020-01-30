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

BOOL UiIniSettings_new(void);
void UiIniSettings_delete(void);
UINT UiIniSettings_getUpdate(void);
UBIG UiIniSettings_getNumThreads(void);
BOOL UiIniSettings_isNetworkOnline(void);
void UiIniSettings_setNetworkOnline(BOOL online);

BOOL UiAutoUpdate_new(void);
void UiAutoUpdate_delete(void);
BOOL UiAutoUpdate_run(void);
BOOL UiAutoUpdate_cleanOld(void);

typedef struct UiScreen_s UiScreen;
BOOL UiScreen_new(void);
void UiScreen_delete(void);
void UiScreen_start(void);

const char* UiIniSettings_getProject(void);
const char* UiIniSettings_getLanguage(void);
OsDateTYPE UiIniSettings_getDateFormat(void);

Image1 UiLogo_init(void);

void UiIcons_convertFolder(const char* pathFolder);

Image1 UiIcons_init_add(void);
Image1 UiIcons_init_add_folder(void);
Image1 UiIcons_init_add_page(void);
Image1 UiIcons_init_add_remote(void);
Image1 UiIcons_init_add_table(void);
Image1 UiIcons_init_add_view(void);
Image1 UiIcons_init_calendar(void);
Image1 UiIcons_init_card(void);
Image1 UiIcons_init_cards(void);
Image1 UiIcons_init_chart_column_normal(void);
Image1 UiIcons_init_chart_column_stack(void);
Image1 UiIcons_init_chart_column_stack_proc(void);
Image1 UiIcons_init_chart_line(void);
Image1 UiIcons_init_chart_line_xy(void);
Image1 UiIcons_init_chart_line_xy_ordered(void);
Image1 UiIcons_init_chart_pie(void);
Image1 UiIcons_init_chart_point(void);
Image1 UiIcons_init_chart_point_xy(void);
Image1 UiIcons_init_chart_point_xy_ordered(void);
Image1 UiIcons_init_column_check(void);
Image1 UiIcons_init_column_currency(void);
Image1 UiIcons_init_column_date(void);
Image1 UiIcons_init_column_datetime(void);
Image1 UiIcons_init_column_email(void);
Image1 UiIcons_init_column_file(void);
Image1 UiIcons_init_column_formula(void);
Image1 UiIcons_init_column_insight(void);
Image1 UiIcons_init_column_link(void);
Image1 UiIcons_init_column_location(void);
Image1 UiIcons_init_column_menu(void);
Image1 UiIcons_init_column_number(void);
Image1 UiIcons_init_column_percentage(void);
Image1 UiIcons_init_column_phone(void);
Image1 UiIcons_init_column_rating(void);
Image1 UiIcons_init_column_slider(void);
Image1 UiIcons_init_column_tags(void);
Image1 UiIcons_init_column_text(void);
Image1 UiIcons_init_column_url(void);
Image1 UiIcons_init_duplicate(void);
Image1 UiIcons_init_error(void);
Image1 UiIcons_init_export(void);
Image1 UiIcons_init_folder(void);
Image1 UiIcons_init_graph(void);
Image1 UiIcons_init_group(void);
Image1 UiIcons_init_idea(void);
Image1 UiIcons_init_image(void);
Image1 UiIcons_init_import(void);
Image1 UiIcons_init_internet_offline(void);
Image1 UiIcons_init_internet_online(void);
Image1 UiIcons_init_kanban(void);
Image1 UiIcons_init_locator(void);
Image1 UiIcons_init_map(void);
Image1 UiIcons_init_map_area(void);
Image1 UiIcons_init_map_circle(void);
Image1 UiIcons_init_map_icon(void);
Image1 UiIcons_init_move(void);
Image1 UiIcons_init_name(void);
Image1 UiIcons_init_reference(void);
Image1 UiIcons_init_remote(void);
Image1 UiIcons_init_reoder(void);
Image1 UiIcons_init_search(void);
Image1 UiIcons_init_star_black(void);
Image1 UiIcons_init_star_white(void);
Image1 UiIcons_init_table(void);
Image1 UiIcons_init_table_column_height(void);
Image1 UiIcons_init_table_filter(void);
Image1 UiIcons_init_table_hide(void);
Image1 UiIcons_init_table_import_export(void);
Image1 UiIcons_init_table_row_height(void);
Image1 UiIcons_init_table_short(void);
Image1 UiIcons_init_timeline(void);
Image1 UiIcons_init_trash(void);
Image1 UiIcons_init_tree_hide(void);
Image1 UiIcons_init_tree_show(void);
Image1 UiIcons_init_unknown(void);

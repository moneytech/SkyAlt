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

void UiStartup_clickCancel(GuiItem* item)
{
	UiScreen_setWelcome();
}

GuiItemLayout* UiStartup_new(void)
{
	GuiItemLayout* layout = GuiItemLayout_new(Quad2i_init4(0, 0, 1, 1));
	GuiItemLayout_addColumn(layout, 0, 100);
	GuiItemLayout_addColumn(layout, 1, 50);
	GuiItemLayout_addColumn(layout, 2, 100);

	GuiItemLayout_addRow(layout, 0, 100);
	GuiItemLayout_addRow(layout, 1, 3);
	GuiItemLayout_addRow(layout, 2, 100);

	GuiItem_setShortcutKey((GuiItem*)layout, FALSE, Win_EXTRAKEY_ESCAPE, 0, &UiStartup_clickCancel);

	//Logo
	Image1 logo = UiLogo_init();
	GuiItemParticles* particles = GuiItemParticles_new(Quad2i_init4(1, 1, 1, 1), logo, FALSE);
	GuiItem_setCallClick((GuiItem*)particles, &UiStartup_clickCancel);
	GuiItemParticles_startAnim(particles, 5);	//5seconds
	GuiItem_addSubName((GuiItem*)layout, "effect", (GuiItem*)particles);
	Image1_free(&logo);

	return layout;
}

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

void UiDialogOpenExamples_clickOpen(GuiItem* item)
{
	const UNI* path = GuiItemButton_getText((GuiItemButton*)item);
	UiScreen_openProject(path, 0);
}

GuiItemLayout* UiDialogOpenExamples_new(const UNI* path)
{
	//layout
	GuiItemLayout* layout = GuiItemLayout_new(Quad2i_init4(0, 0, 1, 1));
	GuiItemLayout_addColumn(layout, 0, 10);

	char nameId[64];
	char** paths;
	BIG N = OsFileDir_getFileList("examples", FALSE, TRUE, TRUE, &paths);
	BIG i;
	for (i = 0; i < N; i++)
	{
		//path
		GuiItem_addSubName((GuiItem*)layout, Std_buildNumber(i, 0, nameId), GuiItemButton_newClassicEx(Quad2i_init4(0, i * 2, 1, 1), DbValue_initStaticCopyCHAR(paths[i]), &UiDialogOpenExamples_clickOpen));

		Std_deleteCHAR(paths[i]);
	}
	Os_free(paths, N * sizeof(char*));

	return GuiItemRoot_createDialogLayout(Vec2i_init2(10, 10), DbValue_initLang("OPEN_EXAMPLE"), (GuiItem*)layout, 0);
}

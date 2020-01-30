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

OsFontLetter OsFontLetter_initEmpty()
{
	OsFontLetter self;
	self.img = 0;
	self.img_w = 0;
	self.img_h = 0;
	self.move_x = 0;
	self.move_y = 0;
	self.m_bytes_in_x = 0;
	self.m_len = -1;
	return self;
}
void OsFontLetter_free(OsFontLetter* self)
{
	Os_free(self->img, self->img_w * self->img_h * sizeof(char));
	Os_memset(self, sizeof(OsFontLetter));
}
BOOL OsFontLetter_is(OsFontLetter* self)
{
	return self->m_len >= 0;
}
void OsFontLetter_alloc(OsFontLetter* self, Vec2i move, int bytes_in_x, int len, Vec2i size, UCHAR* data)
{
	self->img_w = size.x;
	self->img_h = size.y;
	self->move_x = move.x;
	self->move_y = move.y;
	self->m_bytes_in_x = bytes_in_x;
	self->m_len = len;

	const UBIG bytes = size.x * size.y * sizeof(char);
	self->img = Os_realloc(self->img, bytes);
	if (data)
		Os_memcpy(self->img, data, bytes);
}

typedef struct OsFontLetters_s
{
	OsFontLetter* m_letters;
	UINT m_num;
}OsFontLetters;
OsFontLetters OsFontLetters_init()
{
	OsFontLetters self;
	self.m_letters = 0;
	self.m_num = 0;
	return self;
}
void OsFontLetters_free(OsFontLetters* self)
{
	int i;
	for (i = 0; i < self->m_num; i++)
		OsFontLetter_free(&self->m_letters[i]);
	free(self->m_letters);
}
OsFontLetter* OsFontLetters_get(OsFontLetters* self, UINT i)
{
	if (i >= self->m_num)
	{
		UINT old = self->m_num;
		self->m_num = i + 1;
		self->m_letters = Os_realloc(self->m_letters, self->m_num * sizeof(OsFontLetter));
		for (; old < self->m_num; old++)
			self->m_letters[old] = OsFontLetter_initEmpty();	//reset new one
	}
	return &self->m_letters[i];
}

typedef struct OsFont_s
{
	UNI* m_name;
	FT_Library m_library;
	FT_Face m_face;
	int m_num_subpixels;	//1 or 3

	OsFontLetters* m_lettersH;
	UINT m_num_lettersH;

	OsLock lock;
}OsFont;

void OsFont_clear(OsFont* self)
{
	int i;
	for (i = 0; i < self->m_num_lettersH; i++)
		OsFontLetters_free(&self->m_lettersH[i]);
	free(self->m_lettersH);

	self->m_lettersH = 0;
	self->m_num_lettersH = 0;
}
const char* OsFont_free(OsFont* self)
{
	const char* err = 0;
	if (FT_Done_Face(self->m_face))
		err = "FT_Done_Face";

	if (FT_Done_FreeType(self->m_library))
		err = "FT_Done_FreeType";

	OsFont_clear(self);
	free(self->m_name);
	OsLock_free(&self->lock);
	return err;
}

static const char* OsFont_init(OsFont* self, const UNI* name)
{
	const char* err = 0;

	self->m_name = Std_newUNI(name);
	self->m_num_subpixels = 1;

	self->m_lettersH = 0;
	self->m_num_lettersH = 0;

	self->m_face = 0;

	if (FT_Init_FreeType(&self->m_library))
		err = "FT_Init_FreeType";

	if (!OsLock_init(&self->lock))
		err = "OsLock_init";

	return err;
}

const char* OsFont_initFile(OsFont* self, const UNI* name, const char* path)
{
	const char* err = OsFont_init(self, name);
	if (!err)
	{
		if (FT_New_Face(self->m_library, path, 0, &self->m_face))
		{
			err = "FT_New_Face";
			OsFont_free(self);
		}
	}
	return err;
}

const char* OsFont_initMemory(OsFont* self, const UNI* name, const UCHAR* memory, int memory_bytes)
{
	const char* err = OsFont_init(self, name);
	if (!err)
	{
		if (FT_New_Memory_Face(self->m_library, memory, memory_bytes, 0, &self->m_face))
			err = "FT_New_Memory_Face";
	}
	return err;
}

BOOL OsFont_is(const OsFont* self, const UNI* name)
{
	return Std_cmpUNI(self->m_name, name);
}

const char* OsFont_loadBitmap(OsFont* self, const UNI CH, const int Hpx, OsFontLetter* ret)
{
	const char* err = 0;
	FT_Glyph glyph;

	if (FT_Set_Pixel_Sizes(self->m_face, 0, Hpx))
		err = "FT_Set_Pixel_Sizes";

	//transform
	/*{
		const float angle = M_PI / 4;
		FT_Matrix matrix;
		matrix.xx = (FT_Fixed)(cos(angle) * 0x10000L);
		matrix.xy = (FT_Fixed)(-sin(angle) * 0x10000L);
		matrix.yx = (FT_Fixed)(sin(angle) * 0x10000L);
		matrix.yy = (FT_Fixed)(cos(angle) * 0x10000L);

		FT_Vector delta;
		delta.x = 0;
		delta.y = 0;

		FT_Set_Transform(self->m_face, &matrix, &delta);
	}*/

	if (FT_Load_Glyph(self->m_face, FT_Get_Char_Index(self->m_face, CH), FT_LOAD_DEFAULT))
		err = "FT_Load_Glyph";

	if (FT_Get_Glyph(self->m_face->glyph, &glyph))
		err = "FT_Get_Glyph";

	//transform
	{
		const double angle = 0;// M_PI / 4;	//45 degree - try 90deg to see bbox ...
		FT_Matrix matrix;
		matrix.xx = (FT_Fixed)(cos(angle) * 0x10000L);
		matrix.xy = (FT_Fixed)(-sin(angle) * 0x10000L);
		matrix.yx = (FT_Fixed)(sin(angle) * 0x10000L);
		matrix.yy = (FT_Fixed)(cos(angle) * 0x10000L);

		//FT_Vector delta;
		//delta.x = 0;
		//delta.y = 0;

		FT_Glyph_Transform(glyph, &matrix, 0);
	}

	if (FT_Glyph_To_Bitmap(&glyph, self->m_num_subpixels == 3 ? FT_RENDER_MODE_LCD : FT_RENDER_MODE_NORMAL, 0, 1))
		err = "FT_Glyph_To_Bitmap";

	if (!err)
	{
		FT_BitmapGlyph bitmap_glyph = (FT_BitmapGlyph)glyph;

		Vec2i size;
		size.x = bitmap_glyph->bitmap.width / self->m_num_subpixels;
		size.y = bitmap_glyph->bitmap.rows;

		//ret->m_bytes_in_x = bitmap_glyph->bitmap.pitch;	//can't be use only m_size, because this number is can't be device by 3(RGB-LCD)
		OsFontLetter_alloc(ret, Vec2i_init2(bitmap_glyph->left, bitmap_glyph->top - size.y), bitmap_glyph->bitmap.pitch, self->m_face->glyph->advance.x >> 6, size, bitmap_glyph->bitmap.buffer);
	}

	FT_Done_Glyph(glyph);
	return err;
}

OsFontLetter OsFont_get(OsFont* self, const UNI CH, const int Hpx)
{
	if (CH < 0)
		return OsFontLetter_initEmpty();

	OsLock_lock(&self->lock);

	if (Hpx >= self->m_num_lettersH)
	{
		UINT old = self->m_num_lettersH;
		self->m_num_lettersH = Hpx + 1;
		self->m_lettersH = Os_realloc(self->m_lettersH, self->m_num_lettersH * sizeof(OsFontLetters));
		for (; old < self->m_num_lettersH; old++)
			self->m_lettersH[old] = OsFontLetters_init();	//reset new one
	}
	OsFontLetter* letter = OsFontLetters_get(&self->m_lettersH[Hpx], CH);

	if (!OsFontLetter_is(letter))
	{
		OsFont_loadBitmap(self, CH, Hpx, letter);
	}

	OsLock_unlock(&self->lock);

	return *letter;
}

Vec2i OsFont_getTextSize(OsFont* self, const UNI* text, const int Hpx, const int betweenLinePx, int* extra_down)
{
	Vec2i size = Vec2i_init2(0, Hpx);
	int max_down_move_y = 0;
	int max_x = 0;

	while (text && *text)
	{
		if (*text != '\n')
		{
			OsFontLetter l = OsFont_get(self, *text, Hpx);
			max_x += l.m_len;
			if (-l.move_y > max_down_move_y)
				max_down_move_y = -l.move_y;
		}
		else
		{
			size.y += Hpx + betweenLinePx;
			max_x = 0;
			max_down_move_y = 0;
		}

		if (max_x > size.x)
			size.x = max_x;

		text++;
	}

	size.y += max_down_move_y;	//only for last line
	*extra_down = max_down_move_y;

	return size;
}

int OsFont_getTextSizeX(OsFont* self, const UNI* text, const int Hpx)
{
	int extra_down = 0;
	return OsFont_getTextSize(self, text, Hpx, 0, &extra_down).x;
}

int OsFont_getCharPixelPos(OsFont* self, const int Hpx, const UNI* text, int cur_pos)
{
	int pos = 0;
	int i = 0;

	while (text && text[i] && i < cur_pos)
	{
		OsFontLetter l = OsFont_get(self, text[i], Hpx);
		pos += l.m_len;
		i++;
	}

	return pos;
}

int OsFont_getCursorPos(OsFont* self, const int Hpx, const UNI* text, int pixel_x)
{
	int pos = 0;
	int i = 0;

	while (text && text[i])
	{
		OsFontLetter l = OsFont_get(self, text[i], Hpx);

		if (pixel_x < (pos + l.m_len * 0.5f))	//m_bytes_in_x
			break;

		pos += l.m_len;
		i++;
	}

	return i;
}

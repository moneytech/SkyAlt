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

FileHead FileHead_initEmpty(void)
{
	FileHead self;
	self.size = 0;
	Os_memset(self.ext, sizeof(self.ext));
	return self;
}

FileHead FileHead_init(UBIG size, const UNI* ext)
{
	FileHead self = FileHead_initEmpty();
	self.size = size;

	int i;
	for (i = 0; i < 8; i++)
		self.ext[i] = ext[i];
	self.ext[7] = 0;
	return self;
}

void FileHead_free(FileHead* self)
{
	Os_memset(self, sizeof(FileHead));
}

void FileHead_copyExt(const FileHead* self, UNI out_ext[8])
{
	int i;
	for (i = 0; i < 8; i++)
		out_ext[i] = self->ext[i];
	out_ext[7] = 0;
}

BOOL FileHead_isExt(const FileHead* self, const char* ext)
{
	return Std_cmpCHARsmall((char*)self->ext, ext);
}

void FileHead_getInfoEx(const char* ext, const UBIG size, char out[64], BOOL bytesQuotes)
{
	if (!size && !Std_sizeCHAR(ext))
	{
		out[0] = 0;
		return;
	}
	if (bytesQuotes)
	{
		if (size > 1024 * 1024 * 1024)	snprintf(out, 64, ".%s %.1fGB(%lld)", ext, ((double)size) / 1024 / 1024 / 1024, size);
		else
			if (size > 1024 * 1024)		snprintf(out, 64, ".%s %.1fMB(%lld)", ext, ((double)size) / 1024 / 1024, size);
			else
				if (size > 1024)			snprintf(out, 64, ".%s %.1fKB(%lld)", ext, ((double)size) / 1024, size);
				else
					if (size > 0)			snprintf(out, 64, ".%s %lldB", ext, size);
					else				snprintf(out, 64, "---");
	}
	else
	{
		if (size > 1024 * 1024 * 1024)	snprintf(out, 64, ".%s %.1fGB", ext, ((double)size) / 1024 / 1024 / 1024);
		else
			if (size > 1024 * 1024)		snprintf(out, 64, ".%s %.1fMB", ext, ((double)size) / 1024 / 1024);
			else
				if (size > 1024)			snprintf(out, 64, ".%s %.1fKB", ext, ((double)size) / 1024);
				else
					if (size > 0)			snprintf(out, 64, ".%s %lldB", ext, size);
					else				snprintf(out, 64, "---");
	}
}

void FileHead_getInfo(const FileHead* self, char out[64], BOOL bytesQuotes)
{
	FileHead_getInfoEx((char*)self->ext, self->size, out, bytesQuotes);
}

UNI* FileHead_getInfoUNI(const FileHead* self, BOOL bytesQuotes)
{
	char out[64];
	FileHead_getInfo(self, out, bytesQuotes);
	return Std_newUNI_char_n(out, 63);
}

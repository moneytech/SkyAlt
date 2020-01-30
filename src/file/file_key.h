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

typedef struct FileKey_s
{
	UBIG version;
	BIG cycles;
	UCHAR key[32];
	UCHAR encKey[64];

	char* projectPath;
}FileKey;

static char* FileKey_getPathKey(const char* projectPath, BOOL orig)
{
	return Std_addCHAR(projectPath, orig ? "/key" : "/key2");
}

static char* _FileKey_getProjectPath(const FileKey* self, BOOL orig)
{
	return FileKey_getPathKey(self->projectPath, orig);
}

BOOL FileKey_hasPassword(BIG cycles)
{
	return cycles > 0;
}

BOOL FileKey_hasProjectPassword(const char* projectPath)
{
	BOOL has = FALSE;

	char* keyPath = Std_addCHAR(projectPath, "/key");

	OsFile f;
	if (OsFile_init(&f, keyPath, OsFile_R))
	{
		UBIG version;
		if (OsFile_read(&f, &version, 8) == 8)
		{
			if (version == 1)
			{
				BIG cycles;
				if (OsFile_read(&f, &cycles, 8) == 8)
				{
					has = FileKey_hasPassword(cycles);
				}
			}
			//else if(version == 2)
		}
		OsFile_free(&f);
	}

	Std_deleteCHAR(keyPath);
	return has;
}

FileKey* FileKey_newCopy(FileKey* src)
{
	FileKey* self = Os_malloc(sizeof(FileKey));
	*self = *src;
	self->projectPath = Std_newCHAR(src->projectPath);
	return self;
}

void FileKey_delete(FileKey* self)
{
	Std_deleteCHAR(self->projectPath);
	Os_free(self, sizeof(FileKey));
}

void FileKey_print(FileKey* self)
{
	Std_array_print(self->key, sizeof(self->key));
}

static void _FileKey_ptrSwitch(UCHAR** a, UCHAR** b)
{
	UCHAR* c = *a;
	*a = *b;
	*b = c;
}

static BOOL _FileKey_checkHash(UCHAR arr[64])
{
	OsCryptoSha2 hash;
	OsCryptoSha2_exe(arr, 32, &hash);

	return Os_memcmp(&arr[32], &hash, 32) == 0;
}

static void _FileKey_computehash(UCHAR arr[64])
{
	OsCryptoSha2 hash;
	OsCryptoSha2_exe(arr, 32, &hash);
	Os_memcpy(&arr[32], &hash, 32);
}

BOOL FileKey_decrypt(FileKey* self, const UNI* password)
{
	if (!FileKey_hasPassword(self->cycles))
		return TRUE;

	BOOL ok = FALSE;
	OsCryptoKey pass = password ? OsCryptoKey_initFromPassword(password) : OsCryptoKey_initZero();

	BIG cycles = self->cycles;
	BIG orig_cycles = cycles;

	UCHAR in[64];
	UCHAR out[64];
	Os_memcpy(in, self->encKey, 64);
	UCHAR* a = in;
	UCHAR* b = out;

	while (cycles > 0 && StdProgress_is())
	{
		OsCryptoKey_aesDecrypt(&pass, a, b, 64);
		_FileKey_ptrSwitch(&a, &b);
		cycles--;
		StdProgress_set("DECRYPTING", 1.0f - (cycles / (float)orig_cycles));
	}
	ok = _FileKey_checkHash(a);

	Os_memcpy(self->key, a, 32);
	Os_memset(in, 64);
	OsCryptoKey_free(&pass);

	return ok;
}

BOOL FileKey_encrypt(FileKey* self, const UNI* password)
{
	if (!FileKey_hasPassword(self->cycles))
		return TRUE;

	OsCryptoKey pass = password ? OsCryptoKey_initFromPassword(password) : OsCryptoKey_initZero();

	BIG cycles = self->cycles;
	BIG orig_cycles = cycles;

	UCHAR in[64];
	Os_memcpy(in, self->key, 32);
	_FileKey_computehash(in);
	UCHAR* a = in;
	UCHAR* b = self->encKey;

	while (cycles && StdProgress_is())
	{
		OsCryptoKey_aesEncrypt(&pass, a, b, 64);
		_FileKey_ptrSwitch(&a, &b);
		cycles--;
		StdProgress_set("ENCRYPTING", 1.0f - (cycles / (float)orig_cycles));
	}

	Os_memcpy(b, a, 64);
	Os_memset(in, 64);
	OsCryptoKey_free(&pass);

	return (cycles == 0);
}

void FileKey_aesEncryptDirect(const FileKey* self, const UBIG block, unsigned char* plain_text, unsigned char* cipher_text, int size)
{
	if (!self || self->cycles <= 0)
		Os_memcpy(cipher_text, plain_text, size);
	else
		OsCryptoKey_aesEncryptDirect(self->key, block, plain_text, cipher_text, size);
}
void FileKey_aesDecryptDirect(const FileKey* self, const UBIG block, unsigned char* cipher_text, unsigned char* plain_text, int size)
{
	if (!self || self->cycles <= 0)
		Os_memcpy(plain_text, cipher_text, size);
	else
		OsCryptoKey_aesDecryptDirect(self->key, block, cipher_text, plain_text, size);
}

BOOL FileKey_readFile(FileKey* self, const char* path, const char* path2, const UNI* password)
{
	//open & compare with backup file + warning ...
	BOOL ok = FALSE;
	OsFile f;
	if (OsFile_init(&f, path, OsFile_R))
	{
		UBIG version;
		if (OsFile_read(&f, &version, 8) == 8)
		{
			if (version == self->version)
			{
				if (OsFile_read(&f, &self->cycles, 8) == 8)
				{
					if (OsFile_read(&f, self->encKey, 64) == 64)
					{
						if (FileKey_decrypt(self, password))
						{
							ok = TRUE;
						}
						else
							Logs_addError("ERR_FILE_INVALID_FILE_PASSWORD");
					}
				}
			}
		}

		OsFile_free(&f);
	}
	return ok;
}

BOOL FileKey_writeFile(FileKey* self, const char* path)
{
	OsFile f;
	BOOL ok = OsFile_init(&f, path, OsFile_W);
	if (ok)
	{
		OsFile_write(&f, &self->version, 8);
		OsFile_write(&f, &self->cycles, 8);
		OsFile_write(&f, self->encKey, 64);

		OsFile_free(&f);
	}
	return ok;
}

BOOL FileKey_writeFile2(FileKey* self, char* keyPath, char* keyPath2, const UNI* password)
{
	BOOL ok = FileKey_encrypt(self, password);
	if (ok)
	{
		ok |= FileKey_writeFile(self, keyPath);
		ok |= FileKey_writeFile(self, keyPath2);
	}

	return ok;
}

static FileKey* _FileKey_new(const char* projectPath, BIG cycles)
{
	FileKey* self = Os_calloc(1, sizeof(FileKey));

	self->projectPath = Std_newCHAR(projectPath);
	self->cycles = cycles;
	Os_memset(self->encKey, 64);
	OsCrypto_random(32, self->key);
	self->version = 1;

	return self;
}

FileKey* FileKey_newCreate(const char* projectPath, const UNI* password, BIG cycles)
{
	FileKey* self = _FileKey_new(projectPath, cycles);

	char* keyPath = _FileKey_getProjectPath(self, TRUE);
	char* keyPath2 = _FileKey_getProjectPath(self, FALSE);

	if (!FileKey_writeFile2(self, keyPath, keyPath2, password))
	{
		FileKey_delete(self);
		self = 0;
	}

	Std_deleteCHAR(keyPath);
	Std_deleteCHAR(keyPath2);

	return self;
}

FileKey* FileKey_newLoad(const char* projectPath, const UNI* password)
{
	FileKey* self = _FileKey_new(projectPath, 0);

	char* keyPath = _FileKey_getProjectPath(self, TRUE);
	char* keyPath2 = _FileKey_getProjectPath(self, FALSE);

	if (!FileKey_readFile(self, keyPath, keyPath2, password))
	{
		FileKey_delete(self);
		self = 0;
	}

	Std_deleteCHAR(keyPath);
	Std_deleteCHAR(keyPath2);

	return self;
}

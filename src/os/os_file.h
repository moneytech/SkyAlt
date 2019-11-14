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

OsFile OsFile_initEmpty(void)
{
	OsFile self;
	self.m_file = 0;
	return self;
}
BOOL OsFile_init(OsFile* self, const char* path, const char* mode)
{
	self->m_file = path ? fopen(path, mode) : 0;
	return self->m_file != 0;
}
BOOL OsFile_initTemp(OsFile* self)
{
	self->m_file = tmpfile();
	return self->m_file != 0;
}
void OsFile_free(OsFile* self)
{
	if (self->m_file)
	{
		fclose(self->m_file);
		self->m_file = 0;
	}
}

void OsFile_flush(OsFile* self)
{
	fflush(self->m_file);
}

BOOL OsFile_is(const OsFile* self)
{
	return self->m_file != 0;
}

UBIG OsFile_read(OsFile* self, void* ptr, UBIG NUM)
{
	UBIG ret = fread(ptr, 1, NUM, self->m_file);
	return ret;
}

UBIG OsFile_write(OsFile* self, const void* ptr, UBIG NUM)
{
	UBIG ret = fwrite(ptr, 1, NUM, self->m_file);
	return ret;
}
UBIG OsFile_getSeekPos(OsFile* self)
{
	return ftell(self->m_file);
}
void OsFile_seekStart(OsFile* self)
{
	rewind(self->m_file);
}
UBIG OsFile_seekEnd(OsFile* self)
{
	fseek(self->m_file, 0, SEEK_END);
	return OsFile_getSeekPos(self);
}
BOOL OsFile_seekAbs(OsFile* self, UBIG pos)
{
	return fseek(self->m_file, (long)pos, SEEK_SET) == 0;
}
BOOL OsFile_seekRel(OsFile* self, BIG pos)
{
	return fseek(self->m_file, (long)pos, SEEK_CUR) == 0;
}
BOOL OsFile_setEndOfFile(OsFile* self)
{
#ifdef _WIN32
	return SetEndOfFile(self->m_file);
#elif __linux__
	return ftruncate(fileno(self->m_file), OsFile_getSeekPos(self)) == 0;
#endif
}

UCHAR* OsFile_initRead(const char* path, UBIG* out_size, UBIG alloc_extra_bytes)
{
	UCHAR* buff = 0;
	*out_size = 0;

	OsFile f;
	if (OsFile_init(&f, path, OsFile_R))
	{
		*out_size = OsFile_seekEnd(&f);
		OsFile_seekStart(&f);

		buff = malloc(*out_size + alloc_extra_bytes);
		OsFile_read(&f, buff, *out_size);
		memset(&buff[*out_size], 0, alloc_extra_bytes);	//reset extra bytes

		OsFile_free(&f);
	}

	return buff;
}

BOOL OsFile_initWrite(const char* path, void* data, const UBIG bytes)
{
	BOOL ok = FALSE;
	OsFile f;
	if (OsFile_init(&f, path, OsFile_W))
	{
		ok = (OsFile_write(&f, data, bytes) > 0);

		OsFile_free(&f);
	}
	return ok;
}

BOOL OsFile_initHash(const char* path, OsCryptoSha2* out)
{
	UBIG n = 0;
	UCHAR* buff = OsFile_initRead(path, &n, 0);
	if (buff)
	{
		*out = OsCryptoSha2_init();
		if (OsCryptoSha2_exe(buff, n, out))
			return TRUE;
		Os_free(buff, n);
	}
	return FALSE;
}

/*BOOL OsFile_lock(OsFile* self)
{
#ifdef _WIN32
	if(self->m_file)
		return LockFile(self->m_file, 0, 0, 0xFFFFFFFF, 0xFFFFFFFF);
#elif __linux__
	if(self->m_file)
		return flock(fileno(self->m_file), LOCK_EX)==0;
#endif
	return FALSE;
}
BOOL OsFile_unlock(OsFile* self)
{
#ifdef _WIN32
	if(self->m_file)
		return UnLockFile(self->m_file, 0, 0, 0xFFFFFFFF, 0xFFFFFFFF);
#elif __linux__
	if(self->m_file)
		return flock(fileno(self->m_file), LOCK_UN)==0;
#endif
	return FALSE;
}*/

char* OsFile_readLine(OsFile* self)
{
	UBIG pos = OsFile_getSeekPos(self);

	//skip empty
	char c;
	while (OsFile_read(self, &c, 1) == 1 && (c == '\r' || c == '\n'));
	OsFile_seekRel(self, -1);

	//size
	int n = 0;
	while (OsFile_read(self, &c, 1) == 1 && c != '\r' && c != '\n')
		n++;
	if (n == 0)
		return 0;

	//alloc
	char* ret = malloc(n + 1);
	n = 0;

	//fill
	OsFile_seekAbs(self, pos);
	while (OsFile_read(self, &c, 1) == 1 && c != '\r' && c != '\n')
		ret[n++] = c;
	ret[n] = 0;

	return ret;
}

void OsFile_writeUNIch(OsFile* self, const UNI l)
{
	char ch = l;
	OsFile_write(self, &ch, 1);
}

void OsFile_writeUNI(OsFile* self, const UNI* str)
{
	while (str && *str)
	{
		OsFile_writeUNIch(self, *str);
		str++;
	}
}

void OsFile_writeNumber(OsFile* self, double value)
{
	if (value == (BIG)value)	fprintf(self->m_file, "%lld", (BIG)value);
	else			fprintf(self->m_file, "%f", value);
}

BOOL OsFile_existFile(const char* path)
{
	if (path)
	{
		struct stat sb;
#ifdef _WIN32
		return (stat(path, &sb) == 0 && sb.st_mode & S_IFREG);
#else
		return (stat(path, &sb) == 0 && S_ISREG(sb.st_mode));
#endif
	}
	return FALSE;
}

BOOL OsFile_existFolder(const char* path)
{
	if (path)
	{
		struct stat sb;
#ifdef _WIN32
		return (stat(path, &sb) == 0 && sb.st_mode & S_IFDIR);
#else
		return (stat(path, &sb) == 0 && S_ISDIR(sb.st_mode));
#endif
	}
	return FALSE;
}

BOOL OsFile_exist(const char* path)
{
	return OsFile_existFile(path) || OsFile_existFolder(path);
}

BIG OsFile_bytes(const char* path)
{
#ifdef _WIN32
	WIN32_FILE_ATTRIBUTE_DATA fad;
	if (!GetFileAttributesEx(path, GetFileExInfoStandard, &fad))
		return -1; //error
	LARGE_INTEGER size;
	size.HighPart = fad.nFileSizeHigh;
	size.LowPart = fad.nFileSizeLow;
	return size.QuadPart;
#else
	struct stat st;
	stat(path, &st);
	return st.st_size;
#endif
}

BIG OsFileDir_getFileList(const char* path, BOOL file_names, BOOL subdir_names, BOOL complete_path, char*** out)
{
	BIG num = -1;
	*out = 0;

	char* pathF = malloc(Std_sizeCHAR(path) + 1 + 1 + 1);
	strcpy(pathF, path);
	strcat(pathF, "/");

#ifdef _WIN32
	strcat(pathF, "*");
	WIN32_FIND_DATA fdFile;
	HANDLE hFind = FindFirstFile(pathF, &fdFile);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		num = 0;
		do
		{
			if (strcmp(fdFile.cFileName, ".") == 0 || strcmp(fdFile.cFileName, "..") == 0 || strcmp(fdFile.cFileName, path) == 0)	//skip
				continue;

			//if(wcscmp((wchar_t*)fdFile.cFileName, L".") && wcscmp((wchar_t*)fdFile.cFileName, L"..") && wcscmp((wchar_t*)fdFile.cFileName, (wchar_t*)path))
			{
				BOOL isDir = (fdFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
				if ((file_names && !isDir) || (subdir_names && isDir))
				{
					char* ff = malloc(strlen(path) + 1 + strlen(fdFile.cFileName) + 1);
					strcpy(ff, path);
					strcat(ff, "/");
					strcat(ff, fdFile.cFileName);

					if (!complete_path)
						strcpy(ff, fdFile.cFileName);	//only name of file/folder

					num++;
					*out = realloc(*out, num * sizeof(char*));
					(*out)[num - 1] = ff;
				}
			}
		} while (FindNextFile(hFind, &fdFile));

		FindClose(hFind);
	}
#else
	DIR* dir = opendir(pathF);
	if (dir)
	{
		num = 0;
		struct dirent* ent;
		while ((ent = readdir(dir)))
		{
			if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)	//skip
				continue;

			char* ff = malloc(Std_sizeCHAR(path) + 1 + Std_sizeCHAR(ent->d_name) + 1);
			strcpy(ff, path);
			strcat(ff, "/");
			strcat(ff, ent->d_name);

			struct stat st;
			stat(ff, &st);
			BOOL isDir = (st.st_mode & S_IFMT) == S_IFDIR;
			if (!complete_path)
				strcpy(ff, ent->d_name);	//only name of file/folder
			if ((file_names && !isDir) || (subdir_names && isDir))
			{
				num++;
				*out = realloc(*out, num * sizeof(char*));
				(*out)[num - 1] = ff;
			}
			else
				free(ff);
		}
		closedir(dir);
	}
#endif
	free(pathF);
	return num;
}

UBIG OsFileDir_getFolderBytes(const char* path, BOOL subdirs)
{
	BIG i;
	UBIG n = 0;

	char** files;
	const BIG num_files = OsFileDir_getFileList(path, TRUE, FALSE, TRUE, &files);
	for (i = 0; i < num_files; i++)
	{
		n += OsFile_bytes(files[i]);
		free(files[i]);
	}
	free(files);

	if (subdirs)
	{
		char** folders;
		const BIG num_folders = OsFileDir_getFileList(path, FALSE, TRUE, TRUE, &folders);
		for (i = 0; i < num_folders; i++)
		{
			n += OsFileDir_getFolderBytes(folders[i], subdirs);
			free(folders[i]);
		}
		free(folders);
	}
	return n;
}

BOOL OsFileDir_renameFile(const char* oldname, const char* newname)
{
	return rename(oldname, newname) == 0;
}

BOOL OsFileDir_renameDir(const char* oldname, const char* newname)
{
	return rename(oldname, newname) == 0;
}

BOOL OsFileDir_removeFile(const char* path)
{
#ifdef _WIN32
	return DeleteFile(path) != 0;
#else
	return remove(path) == 0;
#endif
}

BOOL OsFileDir_removeDirContent(const char* path);

BOOL OsFileDir_removeDir(const char* path)
{
	OsFileDir_removeDirContent(path);

#ifdef _WIN32
	return RemoveDirectory(path) != 0;
#else
	return rmdir(path) == 0;	//dir must be empty!
#endif
}

BOOL OsFileDir_removeDirContent(const char* path)
{
	BOOL ok = TRUE;
	BIG i;

	char** files;
	BIG n_files = OsFileDir_getFileList(path, TRUE, FALSE, TRUE, &files);
	for (i = 0; i < n_files; i++)
	{
		ok &= OsFileDir_removeFile(files[i]);
		free(files[i]);
	}
	free(files);

	char** folders;
	BIG n_folders = OsFileDir_getFileList(path, FALSE, TRUE, TRUE, &folders);
	for (i = 0; i < n_folders; i++)
	{
		ok &= OsFileDir_removeDir(folders[i]);
		free(folders[i]);
	}
	free(folders);

	return ok;
}

BOOL OsFileDir_makeDir(const char* path)
{
#ifdef _WIN32
	return _mkdir(path) == 0;
#else
	return mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == 0;
#endif
}

char* OsFileDir_currentDir(void)
{
#ifdef _WIN32
	UINT n = GetCurrentDirectoryA(0, NULL);
	char* path = malloc(n + 1);
	GetCurrentDirectoryA(n, path);
#else
	char* path = malloc(1024);
	if (getcwd(path, 1024))
		path = realloc(path, Std_sizeCHAR(path) + 1);	//resize down
	else
	{
		free(path);
		path = 0;
	}
#endif
	return path;
}

char* OsFileDir_currentProgramDir(void)
{
#ifdef _WIN32
	char* path = malloc(256);
	GetModuleFileName(NULL, path, 255);
#else
	char* path = malloc(256);
	int ret = readlink("/proc/self/exe", path, 255);			//Linux
	//int ret = readlink("/proc/curproc/file", path, 255);		//FreeBSD
	//int ret = readlink("/proc/self/path/a.out", path, 255);	//Solaris
	if (ret >= 0)
		path[ret] = 0;
	else
		free(path), path = 0;
#endif

	//note: On Mac OS X, use _NSGetExecutablePath.

	return path;
}

void OsFile_unlink(const char* path)
{
#ifdef __linux__
	unlink(path);
#endif
}

void OsFile_getExtension(const char* path, UCHAR ext[8])
{
	UBIG n = Std_sizeCHAR(path);

	//compute ext size
	int n2 = 0;
	while (path && n2 < 8 && n2 < n && path[n - 1 - n2] != '.')
		n2++;

	memset(ext, 0, 8);	//reset
	if (path && n > n2&& path[n - 1 - n2] == '.')
		memcpy(ext, &path[n - n2], n2);
}

void OsFile_getExtensionUNI(const char* path, UNI ext[8])
{
	UBIG n = Std_sizeCHAR(path);

	//compute ext size
	int n2 = 0;
	while (path && n2 < 8 && n2 < n && path[n - 1 - n2] != '.')
		n2++;

	memset(ext, 0, 8 * sizeof(UNI));	//reset

	if (path && n > n2&& path[n - 1 - n2] == '.')
	{
		int i;
		for (i = 0; i < n2; i++)
			ext[i] = path[n - n2 + i];
	}
}

void OsFile_getParts(const char* path, char** out_folder, char** out_name)
{
	const char* tmp;

	*out_folder = 0;
	*out_name = 0;

	const char* lastDiv = 0;
	tmp = path;
	while (tmp && *tmp)
	{
		if (*tmp == '/' || *tmp == '\\')
			lastDiv = tmp;
		tmp++;
	}

	if (lastDiv)
	{
		const UBIG N = ((UBIG)lastDiv) - (UBIG)path;
		*out_folder = calloc(1, N + 1);
		memcpy(*out_folder, path, N);
	}
	else
		lastDiv = path;

	const char* lastDot = 0;
	tmp = lastDiv;
	while (tmp && *tmp)
	{
		if (*tmp == '.')
			lastDot = tmp;
		tmp++;
	}

	//if(!lastDot)
	 //   lastDot = tmp;

	if (lastDot)
	{
		if (lastDiv != path)
			lastDiv++;  //jump after '/'
		const UBIG N = ((UBIG)lastDot) - (UBIG)lastDiv;
		*out_name = calloc(1, N + 1);
		memcpy(*out_name, lastDiv, N);
	}
}

BOOL OsFile_cmp(const char* pathA, const char* pathB, BIG maxErrors)
{
	BOOL same = FALSE;

	OsFile fa, fb;
	if (OsFile_init(&fa, pathA, OsFile_R))
	{
		if (OsFile_init(&fb, pathB, OsFile_R))
		{
			UBIG sa = OsFile_seekEnd(&fa);
			UBIG sb = OsFile_seekEnd(&fb);
			same = (sa == sb);
			//if(!same)
			//	printf("FileCompare: Different file size %lld %lld\n", sa, sb);

			//alloc
			OsFile_seekStart(&fa);
			OsFile_seekStart(&fb);
			UCHAR* ba = malloc(sa);
			UCHAR* bb = malloc(sb);

			//read
			OsFile_read(&fa, ba, sa);
			OsFile_read(&fb, bb, sb);

			//compate
			UBIG i = 0;
			while (i < sa && i < sb)
			{
				if (ba[i] != bb[i])
				{
					if (maxErrors > 0)
						;//printf("FileCompare: Different values(%d != %d) on index(%lld)\n", ba[i], bb[i], i);
					else
					{
						//printf("FileCompare: Max errors(%lld) reached. Ending test\n", maxErrors);
						break;
					}
					maxErrors--;
				}
				i++;
			}

			//free buffers
			free(ba);
			free(bb);

			OsFile_free(&fb);
		}
		OsFile_free(&fa);
	}

	return same;
}

void OsFile_testSeek(void)
{
	OsFile f;
	if (OsFile_init(&f, "really big file here", OsFile_R))
	{
		double st = Os_time();

		OsFile_seekAbs(&f, 2022172000);	//close to end of file
		UBIG value = 0;
		if (OsFile_read(&f, &value, 8) == 8)
			printf("File test done in %fsec, value: %lld\n", (Os_time() - st), value);
		OsFile_free(&f);
	}
}

void OsFile_testOpen(void)
{
	char** paths;
	const BIG N = OsFileDir_getFileList("/usr/bin", TRUE, FALSE, TRUE, &paths);

	UBIG sum = 0;
	double st = Os_time();
	int i;
	for (i = 0; i < N; i++)
	{
		OsFile f;
		if (OsFile_init(&f, paths[i], OsFile_R))
		{
			UBIG v;
			if (OsFile_read(&f, &v, 8) == 8)
				sum += v;
			OsFile_free(&f);
		}
	}
	double dt = Os_time() - st;
	printf("Files(%lld) opened in %fsec(%.0f files/sec), value: %lld\n", N, dt, N / dt, sum);

	//clear
	for (i = 0; i < N; i++)
		free(paths[i]);
	free(paths);
}

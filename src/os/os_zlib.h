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

static int _OsZlib_getType(const OsZlib_TYPE type)
{
	switch (type)
	{
		case OsZlib_NO_COMPRESS: return Z_NO_COMPRESSION;
		case OsZlib_SPEED_COMPRESS: return Z_BEST_SPEED;
		case OsZlib_BEST_COMPRESS: return Z_BEST_COMPRESSION;
	}
	return -1;
}

UBIG OsZlib_maxCompressSize(UBIG plain_n)
{
	UBIG n16kBlocks = (plain_n + 16383) / 16384;
	return plain_n + 6 + (n16kBlocks * 5);
}

BIG OsZlib_compress(const UCHAR* plain, UCHAR* compress, UBIG plain_n, UBIG compress_n, const OsZlib_TYPE compressType)
{
	BIG ret = -1;

	z_stream zInfo = { 0 };
	zInfo.total_in = zInfo.avail_in = plain_n;
	zInfo.total_out = zInfo.avail_out = compress_n;
	zInfo.next_in = (UCHAR*)plain;
	zInfo.next_out = compress;

	if (deflateInit(&zInfo, _OsZlib_getType(compressType)) == Z_OK)
	{
		if (deflate(&zInfo, Z_FINISH) == Z_STREAM_END)
			ret = zInfo.total_out;
	}
	deflateEnd(&zInfo);

	return ret;
}

BIG OsZlib_uncompress(const UCHAR* compress, UCHAR* plain, UBIG compress_n, UBIG plain_n)
{
	BIG ret = -1;

	z_stream zInfo = { 0 };
	zInfo.total_in = zInfo.avail_in = compress_n;
	zInfo.total_out = zInfo.avail_out = plain_n;
	zInfo.next_in = (UCHAR*)compress;
	zInfo.next_out = plain;

	if (inflateInit(&zInfo) == Z_OK)
	{
		if (inflate(&zInfo, Z_FINISH) == Z_STREAM_END)
			ret = zInfo.total_out;
	}
	inflateEnd(&zInfo);

	return ret;
}

void OsZlib_test(void)
{
	UBIG i;

	const int plain_n = 64 * 1024;//!!!
	UCHAR* plain = calloc(1, plain_n);
	for (i = 0; i < plain_n; i++)
		plain[i] = i + OsCrypto_random01() * 5;

	const int compress_n = OsZlib_maxCompressSize(plain_n);
	UCHAR* compress = malloc(compress_n);

	printf("Test compress ZLIB: ");
	double st = Os_time();

	const UBIG N = 1024;
	for (i = 0; i < N; i++)
	{
		OsZlib_compress(plain, compress, plain_n, compress_n, OsZlib_SPEED_COMPRESS);
	}
	double dt = Os_time() - st;
	printf("(%f) %.3f MB/S\n", dt, plain_n * N / dt / 1024 / 1024);

	printf("Test uncompress ZLIB: ");
	st = Os_time();
	for (i = 0; i < N * 3; i++)
	{
		OsZlib_uncompress(compress, plain, compress_n, plain_n);
	}
	dt = Os_time() - st;
	printf("(%f) %.3f MB/S\n", dt, plain_n * N * 3 / dt / 1024 / 1024);

	free(compress);
	free(plain);
}

void OsZlib_test2(void)
{
	OsFile f;
	if (OsFile_init(&f, "/home/milan/Desktop/ChangeOverBLOCK/names", OsFile_R))	//542x names
	{
		UBIG plain_n = OsFile_seekEnd(&f);
		plain_n /= 3;
		OsFile_seekStart(&f);
		char* plainChar = malloc(plain_n);
		OsFile_read(&f, plainChar, plain_n);
		plainChar[plain_n - 1] = 0;

		//UNI* plain = plainChar;
		UNI* plain = Std_newUNI_char(plainChar);
		plain_n *= sizeof(UNI);

		const UBIG compress_n = OsZlib_maxCompressSize(plain_n);
		UCHAR* compress = malloc(compress_n);

		BIG final_n = OsZlib_compress((UCHAR*)plain, compress, plain_n, compress_n, OsZlib_BEST_COMPRESS);	//OsZlib_SPEED_COMPRESS

		printf("Test ratio ZLIB: %lldB -> %lldB: %f\n", plain_n, final_n, final_n / (double)plain_n);

		free(plainChar);
		free(plain);
		OsFile_free(&f);
	}
}

void OsZlib_test3(void)
{
	const UNI* str = _UNI32("Milan Suk");
	const UBIG bytes = Std_bytesUNI(str);
	int mx_bytes = OsZlib_maxCompressSize(bytes);
	UCHAR compress[128];
	BIG cmBytes = OsZlib_compress((UCHAR*)str, compress, bytes, mx_bytes, OsZlib_BEST_COMPRESS);

	printf("Test: %lldB -> %lldB: %f\n", bytes, cmBytes, cmBytes / (double)bytes);
}

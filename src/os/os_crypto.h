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

const char* OsCrypto_initGlobal(void)
{
	OPENSSL_no_config();
	double t = Os_time();
	RAND_seed(&t, sizeof(t));
	return 0;
}
void OsCrypto_freeGlobal(void)
{
	CONF_modules_free();
}
BOOL OsCrypto_random(const int bytes, void* out)
{
	return RAND_bytes(out, bytes);
}

double OsCrypto_randomDouble(void)
{
	double value;
	OsCrypto_random(8, &value);
	return value;
}

double OsCrypto_random01(void)
{
	UINT value;
	OsCrypto_random(4, &value);
	return value / 4294967295.0;
}

OsCryptoSha2 OsCryptoSha2_init(void)
{
	OsCryptoSha2 self;
	memset(self.m_key, 0, 32);
	return self;
}
BOOL OsCryptoSha2_exe(const void* input, int input_size, OsCryptoSha2* hash)
{
	SHA256_CTX context;
	if (!SHA256_Init(&context))
		return FALSE;
	if (!SHA256_Update(&context, input, input_size))
		return FALSE;
	if (!SHA256_Final(hash->m_key, &context))
		return FALSE;
	return TRUE;
}
void OsCryptoSha2_free(OsCryptoSha2* self)
{
	memset(self->m_key, 0, 32);
}
BOOL OsCryptoSha2_cmp(const OsCryptoSha2* a, const OsCryptoSha2* b)
{
	return memcmp(a, b, 32) == 0;
}

BOOL OsCryptoKey_cmp(const OsCryptoKey* a, const OsCryptoKey* b)
{
	return memcmp(a, b, sizeof(OsCryptoKey)) == 0;
}

const char s_base16[] = "0123456789ABCDEF";//2^4
char _Sdk_findBase16(char ch)
{
	int i;
	for (i = 0; i < sizeof(s_base16); i++)
	{
		if (ch == s_base16[i])
			return i;
	}
	return -1;
}

OsCryptoKey OsCryptoKey_initFromPassword(const UNI* password)
{
	OsCryptoKey self;

	UNI* passwordSalt = Std_addUNI(password, _UNI32("_skyalt"));

	OsCryptoSha2 hash;
	OsCryptoSha2_exe(passwordSalt, (int)(Std_sizeUNI(passwordSalt) * sizeof(UNI)), &hash);
	memcpy(self.m_bytes, &hash, sizeof(self.m_bytes));
	memset(self.m_iv, 0, sizeof(self.m_iv));

	Std_deleteUNI(passwordSalt);

	return self;
}
OsCryptoKey OsCryptoKey_initRandom(void)
{
	OsCryptoKey self;
	OsCrypto_random(sizeof(self.m_bytes), self.m_bytes);
	memset(self.m_iv, 0, sizeof(self.m_iv));
	return self;
}

OsCryptoKey OsCryptoKey_initZero(void)
{
	OsCryptoKey self;
	memset(self.m_bytes, 0, sizeof(self.m_bytes));
	memset(self.m_iv, 0, sizeof(self.m_iv));
	return self;
}
void OsCryptoKey_free(OsCryptoKey* self)
{
	*self = OsCryptoKey_initZero();
}
void OsCryptoKey_exportString(OsCryptoKey* self, char out[65])
{
	int i;
	for (i = 0; i < 32; i++)
	{
		out[i * 2 + 0] = s_base16[self->m_bytes[i] & 0x0F];
		out[i * 2 + 1] = s_base16[self->m_bytes[i] >> 4];
	}
}
void OsCryptoKey_aesEncrypt(OsCryptoKey* self, unsigned char* plain_text, unsigned char* cipher_text, int size)
{
	AES_KEY aeskey;
	AES_set_encrypt_key(self->m_bytes, 256, &aeskey);

	unsigned char iv[16];
	memcpy(iv, self->m_iv, 16);

	AES_cbc_encrypt(plain_text, cipher_text, size, &aeskey, iv, AES_ENCRYPT);
}
void OsCryptoKey_aesDecrypt(OsCryptoKey* self, unsigned char* cipher_text, unsigned char* plain_text, int size)
{
	AES_KEY aeskey;
	AES_set_decrypt_key(self->m_bytes, 256, &aeskey);

	unsigned char iv[16];
	memcpy(iv, self->m_iv, 16);

	AES_cbc_encrypt(cipher_text, plain_text, size, &aeskey, iv, AES_DECRYPT);
}

void OsCryptoKey_aesEncryptIV(OsCryptoKey* self, unsigned char* plain_text, unsigned char* cipher_text, int size)
{
	AES_KEY aeskey;
	AES_set_encrypt_key(self->m_bytes, 256, &aeskey);
	AES_cbc_encrypt(plain_text, cipher_text, size, &aeskey, self->m_iv, AES_ENCRYPT);
}
void OsCryptoKey_aesDecryptIV(OsCryptoKey* self, unsigned char* cipher_text, unsigned char* plain_text, int size)
{
	AES_KEY aeskey;
	AES_set_decrypt_key(self->m_bytes, 256, &aeskey);
	AES_cbc_encrypt(cipher_text, plain_text, size, &aeskey, self->m_iv, AES_DECRYPT);
}

void OsCryptoKey_aesEncryptDirect(const UCHAR* key, const UBIG block, unsigned char* plain_text, unsigned char* cipher_text, int size)
{
	AES_KEY aeskey;
	AES_set_encrypt_key(key, 256, &aeskey);
	UBIG iv[2] = { block, 0 };

	AES_cbc_encrypt(plain_text, cipher_text, size, &aeskey, (UCHAR*)iv, AES_ENCRYPT);
}
void OsCryptoKey_aesDecryptDirect(const UCHAR* key, const UBIG block, unsigned char* cipher_text, unsigned char* plain_text, int size)
{
	AES_KEY aeskey;
	AES_set_decrypt_key(key, 256, &aeskey);
	UBIG iv[2] = { block, 0 };

	AES_cbc_encrypt(cipher_text, plain_text, size, &aeskey, (UCHAR*)iv, AES_DECRYPT);
}
void OsCryptoKey_moveIv(OsCryptoKey* self, unsigned char move[16])
{
	int i = 16;
	while (i--)
		self->m_iv[i] ^= move[i];
}
int OsCryptoKey_roundSizeBy16(int s)
{
	if (s % 16)
		s = (s >> 4 << 4) + 16;
	return s;
}

BOOL OsCryptoECDSAKey_cmp(const OsCryptoECDSAKey* a, const OsCryptoECDSAKey* b)
{
	return memcmp(a, b, sizeof(OsCryptoECDSAKey)) == 0;
}
BOOL OsCryptoECDSAPublic_cmp(const OsCryptoECDSAPublic* a, const OsCryptoECDSAPublic* b)
{
	return memcmp(a, b, sizeof(OsCryptoECDSAPublic)) == 0;
}
BOOL COsCryptoECDSAPrivate_cmp(const OsCryptoECDSAPrivate* a, const OsCryptoECDSAPrivate* b)
{
	return memcmp(a, b, sizeof(OsCryptoECDSAPrivate)) == 0;
}
BOOL OsCryptoECDSASign_cmp(const OsCryptoECDSASign* a, const OsCryptoECDSASign* b)
{
	return memcmp(a, b, sizeof(OsCryptoECDSASign)) == 0;
}

void OsCryptoECDSAKey_free(OsCryptoECDSAKey* self)
{
	memset(self, 0, sizeof(OsCryptoECDSAKey));
}
void OsCryptoECDSAPublic_free(OsCryptoECDSAPublic* self)
{
	memset(self, 0, sizeof(OsCryptoECDSAPublic));
}
void OsCryptoECDSAPrivate_free(OsCryptoECDSAPrivate* self)
{
	memset(self, 0, sizeof(OsCryptoECDSAPrivate));
}
void OsCryptoECDSASign_free(OsCryptoECDSASign* self)
{
	memset(self, 0, sizeof(OsCryptoECDSASign));
}

static OsCryptoECDSA _OsCryptoECDSA_init(void)
{
	OsCryptoECDSA self;
	self.key = EC_KEY_new_by_curve_name(NID_secp256k1);
	return self;
}

BOOL OsCryptoECDSA_initRandom(OsCryptoECDSA* self)
{
	*self = _OsCryptoECDSA_init();
	if (!self->key)
		return FALSE;

	if (EC_KEY_generate_key(self->key) != 1)
	{
		EC_KEY_free(self->key);
		return FALSE;
	}

	return TRUE;
}

BOOL OsCryptoECDSA_initFromKey(OsCryptoECDSA* self, const OsCryptoECDSAKey* key)
{
	*self = _OsCryptoECDSA_init();
	if (!self->key)
		return FALSE;

	//private
	BIGNUM* bigNum = BN_bin2bn(key->m_bytes, 32, 0);
	if (!EC_KEY_set_private_key(self->key, bigNum))
	{
		BN_free(bigNum);
		EC_KEY_free(self->key);
		return FALSE;
	}
	BN_free(bigNum);

	//public
	BIGNUM* x = BN_bin2bn(key->m_bytes + 32, 32, 0);
	BIGNUM* y = BN_bin2bn(key->m_bytes + 64, 32, 0);
	if (!EC_KEY_set_public_key_affine_coordinates(self->key, x, y))
	{
		BN_free(x);
		BN_free(y);
		EC_KEY_free(self->key);
		return FALSE;
	}
	BN_free(x);
	BN_free(y);

	//check
	if (!EC_KEY_check_key(self->key))
	{
		EC_KEY_free(self->key);
		return FALSE;
	}

	return TRUE;
}

BOOL OsCryptoECDSA_initFromPublic(OsCryptoECDSA* self, const OsCryptoECDSAPublic* pub)
{
	*self = _OsCryptoECDSA_init();
	if (!self->key)
		return FALSE;

	const UCHAR* p;
	p = pub->m_bytes;
	EC_KEY* key = (EC_KEY*)self->key;
	o2i_ECPublicKey(&key, &p, sizeof(OsCryptoECDSAPublic));

	return TRUE;
}

void OsCryptoECDSA_free(OsCryptoECDSA* self)
{
	EC_KEY_free((EC_KEY*)self->key);
}

BOOL OsCryptoECDSA_exportKey(OsCryptoECDSA* self, OsCryptoECDSAKey* key)
{
	BOOL ok = BN_bn2bin(EC_KEY_get0_private_key((EC_KEY*)self->key), key->m_bytes) == 32;

	if (ok)
	{
		const EC_POINT* pub = EC_KEY_get0_public_key(self->key);
		BIGNUM* x = BN_new();
		BIGNUM* y = BN_new();

		ok = EC_POINT_get_affine_coordinates_GFp(EC_KEY_get0_group((EC_KEY*)self->key), pub, x, y, NULL);
		if (ok)
		{
			ok = BN_bn2bin(x, key->m_bytes + 32) == 32;
			if (ok)
				ok = BN_bn2bin(y, key->m_bytes + 64) == 32;
		}

		BN_free(x);
		BN_free(y);
	}

	return ok;
}
BOOL OsCryptoECDSA_exportPublic(OsCryptoECDSA* self, OsCryptoECDSAPublic* pub)
{
	UCHAR* p;
	p = pub->m_bytes;
	i2o_ECPublicKey((EC_KEY*)self->key, &p);
	return TRUE;
}
BOOL OsCryptoECDSA_exportPrivate(OsCryptoECDSA* self, OsCryptoECDSAPrivate* pri)
{
	UCHAR* p;
	p = pri->m_bytes;
	i2d_ECPrivateKey((EC_KEY*)self->key, &p);
	return TRUE;
}

int OsCryptoECDSA_sign(OsCryptoECDSA* self, const UCHAR* message, int message_size, OsCryptoECDSASign* sign)
{
	int sign_size = 0;

	//sign
	ECDSA_SIG* sig = ECDSA_do_sign(message, message_size, (EC_KEY*)self->key);
	if (sig)
	{
		sign_size = i2d_ECDSA_SIG(sig, 0);
		{
			UCHAR* s = sign->m_bytes;
			i2d_ECDSA_SIG(sig, &s);
		}

		ECDSA_SIG_free(sig);
	}

	if (sign_size != sizeof(OsCryptoECDSASign))
		memset(&sign->m_bytes[sign_size], 0, sizeof(OsCryptoECDSASign) - sign_size);

	return sign_size;
}

BOOL OsCryptoECDSA_verify(OsCryptoECDSA* self, const UCHAR* message, int message_size, const OsCryptoECDSASign* sign, int sign_size)
{
	BOOL ret = FALSE;

	//sign
	const UCHAR* s = sign->m_bytes;
	ECDSA_SIG* sig = d2i_ECDSA_SIG(0, &s, sign_size);
	if (sig)
	{
		//verify
		ret = ECDSA_do_verify(message, message_size, sig, (EC_KEY*)self->key);
		ECDSA_SIG_free(sig);
	}

	return ret == 1;
}

BOOL OsCryptoECDSA_getSecret(const OsCryptoECDSA* self, const OsCryptoECDSA* selfPub, OsCryptoKey* secret)
{
	*secret = OsCryptoKey_initZero();

	int secret_len = (EC_GROUP_get_degree(EC_KEY_get0_group((EC_KEY*)self->key)) + 7) / 8;
	BOOL ok = (secret_len <= sizeof(OsCryptoKey));
	if (ok)
		secret_len = ECDH_compute_key(secret, secret_len, EC_KEY_get0_public_key(selfPub->key), (EC_KEY*)self->key, NULL);

	return ok;
}

void OsCryptoECDSA_test(void)
{
	const int msg_size = 64;//!!!
	UCHAR msg[64];	//msg_size
	OsCrypto_random(msg_size, msg);

	OsCryptoECDSA ecdsa;
	if (OsCryptoECDSA_initRandom(&ecdsa))
	{
		UBIG i;
		OsCryptoECDSASign sign;

		printf("Test ECDSA sign: ");
		double st = Os_time();
		const UBIG N = 2048;
		int sign_size = 0;
		for (i = 0; i < N; i++)
		{
			sign_size = OsCryptoECDSA_sign(&ecdsa, msg, msg_size, &sign);
		}
		double dt = Os_time() - st;
		printf("(%f) %.3f signs/sec - one takes %fms\n", dt, N / dt, dt / N * 1000);

		printf("Test ECDSA verify: ");
		st = Os_time();
		for (i = 0; i < N; i++)
		{
			OsCryptoECDSA_verify(&ecdsa, msg, msg_size, &sign, sign_size);
		}
		dt = Os_time() - st;
		printf("(%f) %.3f checks/sec - one takes %fms\n", dt, N / dt, dt / N * 1000);

		OsCryptoECDSA_free(&ecdsa);
	}
}

void OsCryptoSha2_test(void)
{
	const int msg_size = 1024;//!!!
	UCHAR* msg = malloc(msg_size);
	OsCrypto_random(msg_size, msg);

	OsCryptoSha2 sha = OsCryptoSha2_init();

	printf("Test SHA3: ");
	double st = Os_time();
	UBIG i;
	const UBIG N = 512 * 1024;
	for (i = 0; i < N; i++)
	{
		OsCryptoSha2_exe(msg, msg_size, &sha);
	}
	double dt = Os_time() - st;
	printf("(%f) %.3fK hashes/sec - one takes %fms\n", dt, N / dt / 1000, dt / N * 1000);

	OsCryptoSha2_free(&sha);
	free(msg);
}

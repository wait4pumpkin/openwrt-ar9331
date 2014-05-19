#include "ssl.h"

#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/aes.h>

#define AIRPORT_PRIVATE_KEY \
"-----BEGIN RSA PRIVATE KEY-----\n" \
"MIIEpQIBAAKCAQEA59dE8qLieItsH1WgjrcFRKj6eUWqi+bGLOX1HL3U3GhC/j0Qg90u3sG/1CUt\n" \
"wC5vOYvfDmFI6oSFXi5ELabWJmT2dKHzBJKa3k9ok+8t9ucRqMd6DZHJ2YCCLlDRKSKv6kDqnw4U\n" \
"wPdpOMXziC/AMj3Z/lUVX1G7WSHCAWKf1zNS1eLvqr+boEjXuBOitnZ/bDzPHrTOZz0Dew0uowxf\n" \
"/+sG+NCK3eQJVxqcaJ/vEHKIVd2M+5qL71yJQ+87X6oV3eaYvt3zWZYD6z5vYTcrtij2VZ9Zmni/\n" \
"UAaHqn9JdsBWLUEpVviYnhimNVvYFZeCXg/IdTQ+x4IRdiXNv5hEewIDAQABAoIBAQDl8Axy9XfW\n" \
"BLmkzkEiqoSwF0PsmVrPzH9KsnwLGH+QZlvjWd8SWYGN7u1507HvhF5N3drJoVU3O14nDY4TFQAa\n" \
"LlJ9VM35AApXaLyY1ERrN7u9ALKd2LUwYhM7Km539O4yUFYikE2nIPscEsA5ltpxOgUGCY7b7ez5\n" \
"NtD6nL1ZKauw7aNXmVAvmJTcuPxWmoktF3gDJKK2wxZuNGcJE0uFQEG4Z3BrWP7yoNuSK3dii2jm\n" \
"lpPHr0O/KnPQtzI3eguhe0TwUem/eYSdyzMyVx/YpwkzwtYL3sR5k0o9rKQLtvLzfAqdBxBurciz\n" \
"aaA/L0HIgAmOit1GJA2saMxTVPNhAoGBAPfgv1oeZxgxmotiCcMXFEQEWflzhWYTsXrhUIuz5jFu\n" \
"a39GLS99ZEErhLdrwj8rDDViRVJ5skOp9zFvlYAHs0xh92ji1E7V/ysnKBfsMrPkk5KSKPrnjndM\n" \
"oPdevWnVkgJ5jxFuNgxkOLMuG9i53B4yMvDTCRiIPMQ++N2iLDaRAoGBAO9v//mU8eVkQaoANf0Z\n" \
"oMjW8CN4xwWA2cSEIHkd9AfFkftuv8oyLDCG3ZAf0vrhrrtkrfa7ef+AUb69DNggq4mHQAYBp7L+\n" \
"k5DKzJrKuO0r+R0YbY9pZD1+/g9dVt91d6LQNepUE/yY2PP5CNoFmjedpLHMOPFdVgqDzDFxU8hL\n" \
"AoGBANDrr7xAJbqBjHVwIzQ4To9pb4BNeqDndk5Qe7fT3+/H1njGaC0/rXE0Qb7q5ySgnsCb3DvA\n" \
"cJyRM9SJ7OKlGt0FMSdJD5KG0XPIpAVNwgpXXH5MDJg09KHeh0kXo+QA6viFBi21y340NonnEfdf\n" \
"54PX4ZGS/Xac1UK+pLkBB+zRAoGAf0AY3H3qKS2lMEI4bzEFoHeK3G895pDaK3TFBVmD7fV0Zhov\n" \
"17fegFPMwOII8MisYm9ZfT2Z0s5Ro3s5rkt+nvLAdfC/PYPKzTLalpGSwomSNYJcB9HNMlmhkGzc\n" \
"1JnLYT4iyUyx6pcZBmCd8bD0iwY/FzcgNDaUmbX9+XDvRA0CgYEAkE7pIPlE71qvfJQgoA9em0gI\n" \
"LAuE4Pu13aKiJnfft7hIjbK+5kyb3TysZvoyDnb3HOKvInK7vXbKuU4ISgxB2bB3HcYzQMGsz1qJ\n" \
"2gG0N5hvJpzwwhbhXqFKA4zaaSrw622wDniAK5MlIE0tIAKKP4yxNGjoD2QYjhBGuhvkWKY=\n" \
"-----END RSA PRIVATE KEY-----"


static gint getCorrectedEncodeSize(gint size) {
	if(size % 4 == 0) {
		return size;
	} else if((size + 1) % 4 == 0) {
		return size+1;
	} else if((size + 2) % 4 == 0) {
		return size+2;
	} else {
		g_print("Unrecoverable error....base64 values are incorrectly encoded\n");
		return size;
	}
}

gchar* encodeBase64(gchar *input, gint length) {
	BIO *b64 = BIO_new(BIO_f_base64());
	BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
	BIO *bmem = BIO_new(BIO_s_mem());

	b64 = BIO_push(b64, bmem);

	BIO_write(b64, input, length);
	BIO_flush(b64);
	BUF_MEM *buffer;
	BIO_get_mem_ptr(b64, &buffer);

	gchar *data = g_malloc(buffer->length);
	memcpy(data, buffer->data, buffer->length - 1);
	data[buffer->length - 1] = 0;

	BIO_free_all(b64);

	return data;
}

gchar* decodeBase64(gchar *data, gint length, gint *actualLength) {
	gchar *input = data;
	gint correctedLength = getCorrectedEncodeSize(length);
	if(length != correctedLength) {
		input = g_malloc(correctedLength * sizeof(gchar));
		memset(input, 0, correctedLength);
		memcpy(input, data, length);
		memset(input + length, '=', correctedLength - length);
	}
	gchar *buffer = g_malloc(correctedLength);
	memset(buffer, 0, correctedLength);

	BIO *b64 = BIO_new(BIO_f_base64());
	BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);

	BIO *bmem = BIO_new_mem_buf(input, correctedLength);
	BIO_set_flags(bmem, BIO_FLAGS_BASE64_NO_NL);
	bmem = BIO_push(b64, bmem);

	BIO_set_flags(bmem, BIO_FLAGS_BASE64_NO_NL);

	*actualLength = BIO_read(bmem, buffer, correctedLength);

	BIO_free_all(bmem);

	if(length != correctedLength) {
		g_free(input);
	}
	return buffer;
}

gchar* rsaEncrypt(gchar *input, gint length, gint *encryptSize) {
	BIO *bio = BIO_new_mem_buf(AIRPORT_PRIVATE_KEY, -1);
	RSA *rsa = PEM_read_bio_RSAPrivateKey(bio, NULL, NULL, NULL);
	BIO_free(bio);

	gint size = RSA_size(rsa);
	gchar *data = g_malloc(size * sizeof(gchar));
	memset(data, 0, size * sizeof(gchar));
	RSA_private_encrypt(length, (const unsigned char *)input, (unsigned char *)data, rsa, RSA_PKCS1_PADDING);

	*encryptSize = size;
	RSA_free(rsa);
	return data;
}

gchar* rsaDecrypt(gchar *input, gint length, gint *decryptSize) {
	BIO *bio = BIO_new_mem_buf(AIRPORT_PRIVATE_KEY, -1);
	RSA *rsa = PEM_read_bio_RSAPrivateKey(bio, NULL, NULL, NULL);
	BIO_free(bio);

	gint size = RSA_size(rsa);
	gchar *data = g_malloc(size * sizeof(gchar));
	memset(data, 0, size * sizeof(gchar));
	RSA_private_decrypt(length, (const unsigned char *)input, (unsigned char *)data, rsa, RSA_PKCS1_OAEP_PADDING);

	*decryptSize = size;
	RSA_free(rsa);
	return data;
}

guint8* aesBlockDecrypt(gchar *aesKey, gchar *aesIV, gchar *src, gint length) {
	AES_KEY key;
	AES_set_decrypt_key((const unsigned char *)aesKey, 128, &key);

	guint8 iv[16];
    memcpy(iv, aesIV, sizeof(iv));

	gint aeslen = length & ~0xf;
	guint8 *data = g_malloc(2048 * sizeof(guint8));
	AES_cbc_encrypt((const unsigned char *)src, data, aeslen, &key, iv, AES_DECRYPT);
	memcpy(data + aeslen, src + aeslen, length - aeslen);

	return data;
}


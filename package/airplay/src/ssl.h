#ifndef _SSL_H
#define _SSL_H

#include <glib-object.h>

gchar* encodeBase64(gchar *input, gint length);
gchar* decodeBase64(gchar *data, gint length, gint *actualLength);
gchar* rsaEncrypt(gchar *input, gint length, gint *encryptSize);
gchar* rsaDecrypt(gchar *input, gint length, gint *decryptSize);
guint8* aesBlockDecrypt(gchar *aesKey, gchar *aesIV, gchar *src, gint length);

#endif

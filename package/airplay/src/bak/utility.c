#include "utility.h"

#include <string.h>

gchar* trim(gchar *src) {
	gint start = 0;
	gint end = strlen(src) - 1;
	
	while(src[start] == ' ' || src[start] == '\t' || src[start] == '\n' || src[start] == '\r') ++start;
	while(src[end] == ' ' || src[end] == '\t' || src[end] == '\n' || src[end] == '\r') --end;
	
	gint length = end - start + 2;
	gchar *value = g_malloc(length);
	g_strlcpy(value, src + start, length);

	return value;
}
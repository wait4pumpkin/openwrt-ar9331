#include "cJSON.h"

extern int commandGetter(cJSON **jsonRef, const char *commands[], const char *entrys[]);
extern void badRequestResponse(const char *description);
extern void serverErrorResponse(const char *description);

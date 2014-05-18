#include <uci.h>
#include <json/json.h>

#define min(x, y) x < y ? x : y

#define MAX_CONTENT 1024
#define MAX_FIELD 64

typedef struct uci_ptr UCIPtr;
typedef struct uci_context UCIContext;

typedef enum {
	UCI_GET, 
	UCI_SET
} UCICommand;

extern UCIPtr* uci(UCIContext *ctx, const UCICommand cmd, const char *key);

extern int statusGetter(json_object *json, UCIContext *ctx, const char *commands[], const char *keys[]);
extern int statusSetter(UCIContext *ctx, const char *commands[], const char *values[]);
extern void serverErrorResponse(const char *description);
extern void badRequestResponse(const char *description);

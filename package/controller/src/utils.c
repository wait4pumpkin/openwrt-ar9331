#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

extern UCIPtr* uci(UCIContext *ctx, UCICommand cmd, const char *str) {
	char *key = strdup(str);
	UCIPtr *ptr = (UCIPtr *)malloc(sizeof(UCIPtr));
	if (uci_lookup_ptr(ctx, ptr, key, true) != UCI_OK) goto fail;

	if (cmd == UCI_SET) {
		int ret = uci_set(ctx, ptr);
		if (ret != UCI_OK) goto fail;

		ret = uci_save(ctx, ptr->p);
		if (ret != UCI_OK) goto fail;	
	}

	free(key);
	return ptr;

fail:
	free(key);
	free(ptr);
	return NULL;
};

int statusGetter(json_object *json, UCIContext *ctx, const char *commands[], const char *keys[]) {
	int i = 0;
	for (; commands[i]; ++i) {
		UCIPtr *ptr = uci(ctx, UCI_GET, commands[i]);
		if (!ptr) goto fail;
		json_object_object_add(json, keys[i], json_object_new_string(ptr->o->v.string));
		free(ptr);
	}
	return 0;

fail:
	serverErrorResponse("Failed to Fetch Data");
	return -1;
}

int statusSetter(UCIContext *ctx, const char *commands[], const char *values[]) {
	int i = 0;
	char key[MAX_FIELD + MAX_FIELD];
	for (; commands[i]; ++i) {
		strlcpy(key, commands[i], MAX_FIELD << 1);
		strncat(key, values[i], MAX_FIELD);
		UCIPtr *ptr = uci(ctx, UCI_SET, key);
		if (!ptr) goto fail;
		free(ptr);
	}
	return 0;

fail:
	serverErrorResponse("Failed to Fetch Data");
	return -1;
}

void badRequestResponse(const char *description) {
	printf("Status: 400 Bad Request\n");
	printf("Content-Type: application/json\n\n");

	json_object *json = json_object_new_object();
	json_object_object_add(json, "status", json_object_new_string("fail"));
	json_object_object_add(json, "description", json_object_new_string(description));

	printf("%s\n", json_object_get_string(json));
	
	fflush(stdout);
	json_object_put(json);
}

void serverErrorResponse(const char *description) {
	printf("Status: 500 Internal Server Error\n");
	printf("Content-Type: application/json\n\n");

	json_object *json = json_object_new_object();
	json_object_object_add(json, "status", json_object_new_string("fail"));
	json_object_object_add(json, "description", json_object_new_string(description));

	printf("%s\n", json_object_get_string(json));
	
	fflush(stdout);
	json_object_put(json);
}

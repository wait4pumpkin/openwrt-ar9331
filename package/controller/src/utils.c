#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

int commandGetter(cJSON **jsonRef, const char *commands[], const char *entrys[]) {
	cJSON *json = *jsonRef;

	int i = 0;
	for (; commands[i]; ++i) {
		FILE *cmd = popen(commands[i], "r");
	    if (!cmd) {
			printf("Status: 500 Internal Server Error\n");

			if (json) cJSON_Delete(json);
			json = cJSON_CreateObject();
			cJSON_AddItemToObject(json, "status", cJSON_CreateString("fail"));
			cJSON_AddItemToObject(json, "description", cJSON_CreateString("server failed to invoke command"));

			*jsonRef = json;
			return -1;
    	} else {
			if (!json) json = cJSON_CreateObject();
	    	char str[128];
			if (fgets(str, sizeof(str), cmd) != NULL) {
				if (str[strlen(str) - 1] == '\n') {
    		        str[strlen(str) - 1] = '\0';
	    	    }
				cJSON_AddItemToObject(json, entrys[i], cJSON_CreateString(str));
			}
	    	pclose(cmd);
		}
	}
	*jsonRef = json;
	return 0;
}

void badRequestResponse(const char *description) {
	printf("Status: 400 Bad Request\n");
	printf("Content-Type:application/json\n\n");

	cJSON *json = cJSON_CreateObject();
	cJSON_AddItemToObject(json, "status", cJSON_CreateString("fail"));
	cJSON_AddItemToObject(json, "description", cJSON_CreateString(description));

	char *out = cJSON_Print(json);
	cJSON_Minify(out);
	printf("%s\n", out);
	
	fflush(stdout);
	cJSON_Delete(json);
}

void serverErrorResponse(const char *description) {
	printf("Status: 500 Internal Server Error\n");
	printf("Content-Type:application/json\n\n");

	cJSON *json = cJSON_CreateObject();
	cJSON_AddItemToObject(json, "status", cJSON_CreateString("fail"));
	cJSON_AddItemToObject(json, "description", cJSON_CreateString(description));

	char *out = cJSON_Print(json);
	cJSON_Minify(out);
	printf("%s\n", out);
	
	fflush(stdout);
	cJSON_Delete(json);
}

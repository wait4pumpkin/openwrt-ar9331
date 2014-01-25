#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

int main(int argc, char *argv[]) {

	cJSON *json = NULL;

	const char *commands[] = {
								NULL
							 };
	const char *entrys[] = { NULL };

	commandGetter(&json, commands, entrys);

	printf("Content-Type:application/json\n\n");
	char *out = cJSON_Print(json);
	cJSON_Minify(out);
	printf("%s\n", out);
	
	fflush(stdout);
	cJSON_Delete(json);
	free(out);

	return EXIT_SUCCESS;
}


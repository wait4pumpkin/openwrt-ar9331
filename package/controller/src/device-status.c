#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

int main(int argc, char *argv[]) {

	cJSON *json = NULL;

	const char *commands[] = {
								"uci show info.@info[0].hardware | awk -F '=' '{print $2}'", 
								"uci show info.@info[0].version | awk -F '=' '{print $2}'", 
								"cat /proc/uptime | awk '{print $1}'", 
								NULL
							 };
	const char *entrys[] = { "hardware", "version", "uptime", NULL };

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


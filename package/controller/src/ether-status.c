#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

int main(int argc, char *argv[]) {

	cJSON *json = NULL;

	const char *commands[] = { 
								"ifconfig eth0 | tr -s ' ' | cut -d ' ' -f5", 
								"uci show network.lan.proto | awk -F '=' '{print $2}'", 
								"uci show network.lan.ipaddr | awk -F '=' '{print $2}'", 
								"uci show network.lan.netmask | awk -F '=' '{print $2}'", 
								NULL
							 };
	const char *entrys[] = { "mac", "protocol", "ip", "netmask", NULL };

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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

int main(int argc, char *argv[]) {

	FILE *f = fopen("/sys/class/net/eth0/address", "r");
	char address[MAX_CONTENT];
	fgets(address, MAX_CONTENT, f);
	fclose(f);

	json_object *json = json_object_new_object();
	if (strlen(address) <= 0) {
		serverErrorResponse("Unable to Get MAC address");
		goto fail;
	}
	json_object_object_add(json, "mac", json_object_new_string(address));

	UCIContext *ctx = uci_alloc_context();
	const char *commands[] = { 
								"network.lan.proto", 
								"network.lan.ipaddr", 
								"network.lan.netmask", 
								NULL
							 };
	const char *keys[] = { "protocol", "ip", "netmask", NULL };
	if (statusGetter(json, ctx, commands, keys) < 0) goto uci_fail;

	printf("Content-Type:application/json\n\n");
	printf("%s\n", json_object_get_string(json));

uci_fail:	
	uci_free_context(ctx);

fail:
	fflush(stdout);
	json_object_put(json);

	return EXIT_SUCCESS;
}


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

int main(int argc, char *argv[]) {

	json_object *json = json_object_new_object();
	UCIContext *ctx = uci_alloc_context();

	const char *commands[] = { 
								"wireless.@wifi-iface[0].ssid", 
								"wireless.@wifi-iface[0].encryption", 
								"wireless.radio0.channel", 
								NULL
							 };
	const char *keys[] = { "name", "encryption", "channel", NULL };
	if (statusGetter(json, ctx, commands, keys) < 0) goto fail;

	printf("Content-Type:application/json\n\n");
	printf("%s\n", json_object_get_string(json));

fail:	
	fflush(stdout);
	uci_free_context(ctx);
	json_object_put(json);

	return EXIT_SUCCESS;
}


#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "utils.h"
#include "slre.h"

int main(int argc, char *argv[]) {
    char *lengthStr = getenv("CONTENT_LENGTH"); 
    if (!lengthStr) {
        badRequestResponse("Request Invaild");
        return EXIT_SUCCESS;
    }
    
    const size_t length = atoi(lengthStr);
    char content[MAX_CONTENT];
    fgets(content, min(length + 1, MAX_CONTENT), stdin);
    
    json_object *json = json_tokener_parse(content);
    if (!json) {
        badRequestResponse("Invaild JSON");
        goto fail;
    }

    json_object *value = NULL;
    json_bool ret = json_object_object_get_ex(json, "mode", &value);
    if (!ret) {
        badRequestResponse("Incomplete Arguments");
        goto fail;
    }
    const char *mode = json_object_get_string(value);
    if (!strcmp(mode, "sta") && !strcmp(mode, "ap")) {
        badRequestResponse("Invaild Argument: mode");
        goto fail;
    }

    ret = json_object_object_get_ex(json, "name", &value);
    if (!ret) {
        badRequestResponse("Incomplete Arguments");
        goto fail;
    }
    const char *name = json_object_get_string(value);
    struct slre_cap caps[1];
    if (!(slre_match("^([a-zA-Z0-9 -]+)", name, strlen(name), caps, 1) > 0 && caps[0].len == strlen(name))) {
        badRequestResponse("Invaild Argument: name");
        goto fail;
    }

    ret = json_object_object_get_ex(json, "encryption", &value);
    if (!ret) {
        badRequestResponse("Incomplete Arguments");
        goto fail;
    }
    const char *encryption = json_object_get_string(value);
    if (strcmp(encryption, "none") && strcmp(encryption, "psk") && strcmp(encryption, "psk2") && strcmp(encryption, "wep")) {
        badRequestResponse("Invaild Argument: encryption");
        goto fail;
    }

    ret = json_object_object_get_ex(json, "password", &value);
    if (!ret) {
        badRequestResponse("Incomplete Arguments");
        goto fail;
    }
    const char *password = json_object_get_string(value);
    if (strlen(password) > MAX_FIELD) {
        badRequestResponse("Invaild Argument: password");
        return EXIT_SUCCESS;
    }

    const char *commands[] = { 
                                "wireless.@wifi-iface[0].mode", 
                                "wireless.@wifi-iface[0].ssid", 
                                "wireless.@wifi-iface[0].encryption", 
                                "wireless.@wifi-iface[0].key", 
                                NULL
                             };
    const char *values[] = { mode, name, encryption, password };
    UCIContext *ctx = uci_alloc_context();
    if (statusSetter(ctx, commands, values) < 0) {
        goto uci_fail;
    }

    printf("Content-Type:application/json\n\n");

uci_fail:
    uci_free_context(ctx);

fail:
    fflush(stdout);
    json_object_put(json);

    return EXIT_SUCCESS;
}

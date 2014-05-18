#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "utils.h"
#include "slre.h"

int main(int argc, char *argv[]) {
    const char *lengthStr = getenv("CONTENT_LENGTH"); 
    if (!lengthStr) {
        badRequestResponse("Request Invaild");
        return EXIT_SUCCESS;
    }

    const size_t length = atoi(lengthStr);
    char content[MAX_CONTENT];
    fgets(content, min(length + 1, MAX_CONTENT), stdin);
    
    json_object *json = json_tokener_parse(content);
    UCIContext *ctx = uci_alloc_context();
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
    if (!strcmp(mode, "static")) {
        ret = json_object_object_get_ex(json, "ip", &value);
        if (!ret) {
            badRequestResponse("Incomplete Arguments");
            goto fail;
        }
        const char *ip = json_object_get_string(value);
        if (!isVaildIP(ip)) {
            badRequestResponse("Invaild Argument: ip");
            goto fail;
        }

        ret = json_object_object_get_ex(json, "netmask", &value);
        if (!ret) {
            badRequestResponse("Incomplete Arguments");
            goto fail;
        }
        const char *netmask = json_object_get_string(value);
        if (!isVaildIP(netmask)) {
            badRequestResponse("Invaild Argument: netmask");
            goto fail;
        }

        ret = json_object_object_get_ex(json, "gateway", &value);
        if (!ret) {
            badRequestResponse("Incomplete Arguments");
            goto fail;
        }
        const char *gateway = json_object_get_string(value);
        if (!isVaildIP(gateway)) {
            badRequestResponse("Invaild Argument: gateway");
            goto fail;
        }

        ret = json_object_object_get_ex(json, "dnsA", &value);
        if (!ret) {
            badRequestResponse("Incomplete Arguments");
            goto fail;
        }
        const char *dnsA = json_object_get_string(value);
        if (!isVaildIP(dnsA)) {
            badRequestResponse("Invaild Argument: dnsA");
            goto fail;
        }

        ret = json_object_object_get_ex(json, "dnsB", &value);
        if (!ret) {
            badRequestResponse("Incomplete Arguments");
            goto fail;
        }
        const char *dnsB = json_object_get_string(value);
        if (!isVaildIP(dnsB)) {
            badRequestResponse("Invaild Argument: dnsB");
            goto fail;
        }

        const char *commands[] = { 
                                    "network.lan.proto", 
                                    "network.lan.ipaddr", 
                                    "network.lan.netmask", 
                                    "network.lan.gateway", 
                                    "network.lan.dns", 
                                    NULL
                                 };
        const char *values[] = { mode, ip, netmask, gateway, dnsA, NULL };
        if (statusSetter(ctx, commands, values) < 0 || uci(ctx, UCI_COMMIT, "network.lan")) goto fail;

    } else if (!strcmp(mode, "dhcp")) {
        const char *commands[] = { 
                                    "network.lan.proto", 
                                    NULL
                                 };
        const char *values[] = { mode, NULL };
        if (statusSetter(ctx, commands, values) < 0 || uci(ctx, UCI_COMMIT, "network.lan")) goto fail;

    } else if (!strcmp(mode, "pppoe")) {
        ret = json_object_object_get_ex(json, "username", &value);
        if (!ret) {
            badRequestResponse("Incomplete Arguments");
            goto fail;
        }
        const char *username = json_object_get_string(value);
        if (strlen(username) > MAX_CONTENT) {
            badRequestResponse("Invaild Argument: username");
            goto fail;
        }

        ret = json_object_object_get_ex(json, "password", &value);
        if (!ret) {
            badRequestResponse("Incomplete Arguments");
            goto fail;
        }
        const char *password = json_object_get_string(value);
        if (strlen(password) > MAX_CONTENT) {
            badRequestResponse("Invaild Argument: password");
            goto fail;
        }

        const char *commands[] = { 
                                    "network.lan.proto", 
                                    "network.wan.username", 
                                    "network.wan.password", 
                                    NULL
                                 };
        const char *values[] = { mode, username, password, NULL };
        if (statusSetter(ctx, commands, values) < 0 || uci(ctx, UCI_COMMIT, "network.wan")) goto fail;

    } else {
        badRequestResponse("Invaild Argument: mode");
        goto fail;
    }
    
    printf("Content-Type:application/json\n\n");
    fflush(stdout);

fail:
    uci_free_context(ctx);
    json_object_put(json);

    return EXIT_SUCCESS;
}

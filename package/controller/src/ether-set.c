#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "utils.h"
#include "slre.h"
#include "ether.h"

int main(int argc, char *argv[]) {

	printf("Content-Type:application/json\n\n");
	
	char *lengthStr = getenv("CONTENT_LENGTH");	
	if (lengthStr != NULL) {
		int length = atoi(lengthStr);
		char content[256];
		fgets(content, length + 1, stdin);
	
		cJSON *json = cJSON_Parse(content);
		if (!json) {
			badRequestResponse("Invaild JSON");
			return EXIT_SUCCESS;
		} else {
			EtherRequest request;

			const char *protocol = cJSON_GetObjectItem(json, "mode")->valuestring;
			
			if (!strcmp(protocol, "static") || !strcmp(protocol, "dhcp") || !strcmp(protocol, "pppoe")) {
				strncpy(request.protocol, protocol, sizeof(request.protocol));
			} else {
				badRequestResponse("Invaild Argument: protocol");
				return EXIT_SUCCESS;
			}

			int sock = socket(PF_UNIX, SOCK_STREAM, 0);
			if (sock < 0) {
				serverErrorResponse("Service Not Response");
				return EXIT_SUCCESS;
			}

			struct sockaddr_un addr;
			bzero(&addr, sizeof(struct sockaddr_un));
			addr.sun_family = AF_UNIX;
			snprintf(addr.sun_path, UNIX_PATH_MAX, ETHER_SOCKET_DOMAIN);
		
			if (connect(sock, (struct sockaddr *)&addr, sizeof(struct sockaddr_un))) {
				serverErrorResponse("Service Not Response");
				return EXIT_SUCCESS;
			}

			if (!strcmp(request.protocol, "dhcp")) {
				write(sock, (char *)&request, sizeof(EtherRequest));
			} else if (!strcmp(request.protocol, "static")) {
				const char *ip = cJSON_GetObjectItem(json, "ip")->valuestring;
				const char *netmask = cJSON_GetObjectItem(json, "netmask")->valuestring;
				const char *gateway = cJSON_GetObjectItem(json, "gateway")->valuestring;
				const char *dnsA = cJSON_GetObjectItem(json, "dnsA")->valuestring;
				const char *dnsB = cJSON_GetObjectItem(json, "dnsB")->valuestring;

				if (!ip || !netmask || !gateway || !dnsA || !dnsB) {
					badRequestResponse("Arguments Not Complete");
					return EXIT_SUCCESS;
				}

				struct slre_cap caps[1];
				if (slre_match("^(\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3})", ip, strlen(ip), caps, 1) > 0 && caps[0].len == strlen(ip)) {
					strncpy(request.ip, ip, sizeof(request.ip));
				} else {
					badRequestResponse("Invaild Argument: ip");
					return EXIT_SUCCESS;
				}

				if (slre_match("^(\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3})", netmask, strlen(netmask), caps, 1) > 0 && caps[0].len == strlen(netmask)) {
					strncpy(request.netmask, netmask, sizeof(request.netmask));
				} else {
					badRequestResponse("Invaild Argument: netmask");
					return EXIT_SUCCESS;
				}

				if (slre_match("^(\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3})", gateway, strlen(gateway), caps, 1) > 0 && caps[0].len == strlen(gateway)) {
					strncpy(request.gateway, gateway, sizeof(request.gateway));
				} else {
					badRequestResponse("Invaild Argument: gateway");
					return EXIT_SUCCESS;
				}

				if (slre_match("^(\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3})", dnsA, strlen(dnsA), caps, 1) > 0 && caps[0].len == strlen(dnsA)) {
					strncpy(request.dnsA, dnsA, sizeof(request.dnsA));
				} else {
					badRequestResponse("Invaild Argument: dnsA");
					return EXIT_SUCCESS;
				}

				if (slre_match("^(\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3})", dnsB, strlen(dnsB), caps, 1) > 0 && caps[0].len == strlen(dnsB)) {
					strncpy(request.dnsB, dnsB, sizeof(request.dnsB));
				} else {
					badRequestResponse("Invaild Argument: dnsB");
					return EXIT_SUCCESS;
				}

				write(sock, (char *)&request, sizeof(EtherRequest));
			} else if (!strcmp(request.protocol, "pppoe")) {
				const char *username = cJSON_GetObjectItem(json, "username")->valuestring;
				const char *password = cJSON_GetObjectItem(json, "password")->valuestring;

				if (!username || !password) {
					badRequestResponse("Arguments Not Complete");
					return EXIT_SUCCESS;
				}

				struct slre_cap caps[1];
				if (slre_match("^([a-zA-Z0-9 -]+)", username, strlen(username), caps, 1) > 0 && caps[0].len == strlen(username)) {
					strncpy(request.username, username, sizeof(request.username));
				} else {
					badRequestResponse("Invaild Argument: username");
					return EXIT_SUCCESS;
				}

				// Passwork without check
				if (strlen(password) < sizeof(request.password) - 2) {
					strncpy(request.password, password, sizeof(request.password));
				} else {
					badRequestResponse("Invaild Argument: password");
					return EXIT_SUCCESS;
				}

				write(sock, (char *)&request, sizeof(EtherRequest));
			}

			close(sock);
			cJSON_Delete(json);
		}
	}

	fflush(stdout);

	return EXIT_SUCCESS;
}

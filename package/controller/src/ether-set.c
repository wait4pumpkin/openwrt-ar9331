#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "utils.h"
#include "slre.h"
#include "ether.h"

EtherProtocol checkProtocol(const char *str);

int main(int argc, char *argv[]) {
	static const int MAX_CONTENT = 256;
	
	const char *lengthStr = getenv("CONTENT_LENGTH");
	if (!lengthStr) {
		badRequestResponse("Invaild Request");
		return EXIT_SUCCESS;
	}

	const size_t length = atoi(lengthStr);
	char content[MAX_CONTENT];
	fgets(content, min(MAX_CONTENT, length + 1), stdin);

	cJSON *json = cJSON_Parse(content);
	if (!json) {
		badRequestResponse("Invaild JSON");
		return EXIT_SUCCESS;
	}

	EtherRequest request;

	const char *protocol = cJSON_GetObjectItem(json, "mode")->valuestring;
	request.protocol = checkProtocol(protocol);
	if (request.protocol < PROTOCOL_BASE || request.protocol >= NUM_PROTOCOL) {
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

	if (request.protocol == PROTOCOL_DHCP) {
		write(sock, (char *)&request, sizeof(EtherRequest));
	} else if (request.protocol == PROTOCOL_STATIC) {
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
	} else if (request.protocol == PROTOCOL_PPPOE) {
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

	printf("Content-Type:application/json\n\n");
	close(sock);
	cJSON_Delete(json);

	fflush(stdout);

	return EXIT_SUCCESS;
}

EtherProtocol checkProtocol(const char *str) {
	const char *vaild[] = { "static", "dhcp", "pppoe" };
	int i = PROTOCOL_BASE;
	for (; i<NUM_PROTOCOL; ++i) {
		if (!strcmp(vaild[i], str)) return i;
	}
	return PROTOCOL_BASE - 1;
}


#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "utils.h"
#include "slre.h"
#include "wifi.h"

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
			WifiRequest request;

			const char *mode = cJSON_GetObjectItem(json, "mode")->valuestring;
			const char *name = cJSON_GetObjectItem(json, "name")->valuestring;
			const char *encryption = cJSON_GetObjectItem(json, "encryption")->valuestring;
			const char *pws = cJSON_GetObjectItem(json, "password")->valuestring;

			if (!strcmp(mode, "sta")) {
				request.cmd = CMD_STA;
			} else if (!strcmp(mode, "ap")) {
				request.cmd = CMD_AP;
			} else {
				badRequestResponse("Invaild Argument: mode");
				return EXIT_SUCCESS;
			}

			if (!name || !encryption || !pws) {
				badRequestResponse("Arguments Not Complete");
				return EXIT_SUCCESS;
			}

			struct slre_cap caps[1];
    		if (slre_match("^([a-zA-Z0-9 -]+)", name, strlen(name), caps, 1) > 0 && caps[0].len == strlen(name)) {
             	strncpy(request.name, name, sizeof(request.name));
			} else {
				badRequestResponse("Invaild Argument: name");
				return EXIT_SUCCESS;
			}
			
			if (!strcmp(encryption, "none") || !strcmp(encryption, "psk") || !strcmp(encryption, "psk2") || !strcmp(encryption, "wep")) {
				strncpy(request.encryption, encryption, sizeof(request.encryption));
			} else {
				badRequestResponse("Invaild Argument: encryption");
				return EXIT_SUCCESS;
			}

			if (strlen(pws) < sizeof(request.pws)) {
				strncpy(request.pws, pws, sizeof(request.pws));
			} else {
				badRequestResponse("Invaild Argument: password");
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
			snprintf(addr.sun_path, UNIX_PATH_MAX, SOCKET_DOMAIN);
		
			if (connect(sock, (struct sockaddr *)&addr, sizeof(struct sockaddr_un))) {
				serverErrorResponse("Service Not Response");
				return EXIT_SUCCESS;
			}
			write(sock, (char *)&request, sizeof(WifiRequest));
		
			close(sock);
			cJSON_Delete(json);
		}
	}

	fflush(stdout);

	return EXIT_SUCCESS;
}

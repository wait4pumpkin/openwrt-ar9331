#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "ssid.h"

int main(int argc, char *argv[]) {
	Request request;
	
	struct sockaddr_un addr;
	int sock;
	
	char postStr[128];
	char *lenStr;
	int length;
	char mode[5];

	bzero(&request, sizeof(Request));

	printf("Content-Type:text/html\n\n");
	printf("<HTML>\n");
	printf("<HEAD>\n<TITLE >Get Method</TITLE>\n</HEAD>\n");
	printf("<BODY>\n");
	printf("<div style=\"font-size:12px\">\n");

	lenStr = getenv("CONTENT_LENGTH");	
	if (lenStr != NULL) {
		length = atoi(lenStr);
		fgets(postStr, length + 1, stdin);

		if (sscanf(postStr, "wifimode=%[^&]&ssid=%[^&]&encryption=%[^&]&pwd=%[^&]&submit=%%E4%%BF%%9D%%E5%%AD%%98",
				mode, request.name, request.encryption, request.pws) && !strcmp(mode, "home")) {
			request.cmd = CMD_STA;

			printf("<DIV STYLE=\"COLOR:GREEN; font-size:15px;font-weight:bold\">Connect %s succeed</DIV>\n", request.name);
			printf("<DIV STYLE=\"COLOR:GREEN; font-size:15px;font-weight:bold\">SSID: %s</DIV>\n", request.name);
			printf("<DIV STYLE=\"COLOR:GREEN; font-size:15px;font-weight:bold\">Password: %s</DIV>\n", request.pws);
			printf("<DIV STYLE=\"COLOR:GREEN; font-size:15px;font-weight:bold\">Encryption: %s</DIV>\n", request.encryption);
		} else if (sscanf(postStr, "wifimode=%[^&]", mode) == 1 && !strcmp(mode, "ap")) {
			request.cmd = CMD_AP;

			printf("<DIV STYLE=\"COLOR:GREEN; font-size:15px;font-weight:bold\">Change to ap mode</DIV>\n");
		}
	}

	printf("</div>\n");
	printf("</BODY>\n");
	printf("</HTML>\n");

	fflush(stdout);

	if (request.cmd > 0) {
		sock = socket(PF_UNIX, SOCK_STREAM, 0);
		if (sock < 0) exit(EXIT_FAILURE);

		bzero(&addr, sizeof(struct sockaddr_un));
		addr.sun_family = AF_UNIX;
		snprintf(addr.sun_path, UNIX_PATH_MAX, SOCKET_DOMAIN);
		
		if (connect(sock, (struct sockaddr *)&addr, sizeof(struct sockaddr_un))) exit(EXIT_FAILURE);
		write(sock, (char *)&request, sizeof(Request));
		
		close(sock);
	}

	return EXIT_SUCCESS;
}


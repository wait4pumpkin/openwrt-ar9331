#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <uci.h>

#include "ether.h"

static int initServer(void);
static void handleClient(int sockfd, EtherRequest *request);

int main(int argc, char *argv[]) {
	int server, conn;
	EtherRequest request;

	server = initServer();

	while (1) {
		conn = accept(server, NULL, NULL);
		if (conn < 0) continue;
		handleClient(conn, &request);
	}

	return EXIT_SUCCESS;
}


static int initServer(void) {
	int sock;
	struct sockaddr_un addr;

	sock = socket(PF_UNIX, SOCK_STREAM, 0);
	if (sock < 0) exit(EXIT_FAILURE);

	unlink(ETHER_SOCKET_DOMAIN);

	bzero(&addr, sizeof(struct sockaddr_un));
	addr.sun_family = AF_UNIX;
	snprintf(addr.sun_path, UNIX_PATH_MAX, ETHER_SOCKET_DOMAIN);
	
	if (bind(sock, (struct sockaddr *)&addr, sizeof(struct sockaddr_un)) < 0) exit(EXIT_FAILURE);
	if (listen(sock, 5) < 0) exit(EXIT_FAILURE);

	return sock;
}

static void handleClient(int conn, EtherRequest *request) {	
	int length = 0;
	char cmdline[128];

	memset(cmdline, 0, sizeof(cmdline));	
	memset(request, 0, sizeof(EtherRequest));

	length = read(conn, (char *)request, sizeof(EtherRequest));
	if (length < 0) return;

	char *key = strdup("wireless.@wifi-iface[0].ssid");
	struct uci_context *context = uci_alloc_context();
	struct uci_ptr p;
	if (uci_lookup_ptr(context, &p, key, true) != UCI_OK) return;

	printf("%s\n", p.value);

	uci_free_context(context);
	free(key);

	close(conn);
}


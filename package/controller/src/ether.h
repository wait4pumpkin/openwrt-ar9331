#ifndef __ETHER_H__
#define __ETHER_H__

#define UNIX_PATH_MAX	128

#define CMD_AP			1
#define CMD_STA			2
#define ETHER_SOCKET_DOMAIN	"/tmp/ether.socket"

typedef struct {
	char protocol[64];
	char username[64];
	char password[64];
	char ip[64];
	char netmask[64];
	char gateway[64];
	char dnsA[64];
	char dnsB[64];
} EtherRequest;

#endif

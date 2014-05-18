#ifndef __ETHER_H__
#define __ETHER_H__

#define UNIX_PATH_MAX	128

#define CMD_AP			1
#define CMD_STA			2
#define ETHER_SOCKET_DOMAIN	"/tmp/ether.socket"

#define PROTOCOL_BASE 0

typedef enum {
	PROTOCOL_STATIC = PROTOCOL_BASE, 
	PROTOCOL_DHCP, 
	PROTOCOL_PPPOE, 
	NUM_PROTOCOL
} EtherProtocol;

typedef struct {
	EtherProtocol protocol;
	char username[64];
	char password[64];
	char ip[64];
	char netmask[64];
	char gateway[64];
	char dnsA[64];
	char dnsB[64];
} EtherRequest;

#endif

#ifndef __SSID_H__
#define __SSID_H__

#define UNIX_PATH_MAX	128

#define CMD_AP			1
#define CMD_STA			2
#define SOCKET_DOMAIN	"/tmp/wifi.socket"

typedef struct {
	int cmd;
	char name[64];
	char pws[64];
	char encryption[16];
} WifiRequest;

#endif

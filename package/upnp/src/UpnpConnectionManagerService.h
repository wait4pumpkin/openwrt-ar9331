#ifndef _UPNP_CONNECTION_MANAGER_SERVICE
#define _UPNP_CONNECTION_MANAGER_SERVICE

#include <libgupnp/gupnp.h>

#define UPNP_TYPE_CONNECTION_MANAGER_SERVICE (upnpConnectionManagerService_get_type())
#define UPNP_CONNECTION_MANAGER_SERVICE(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), UPNP_TYPE_CONNECTION_MANAGER_SERVICE, \
			UpnpConnectionManagerService))
#define UPNP_IS_CONNECTION_MANAGER_SERVICE(obj) G_TYPE_CHECK_INSTANCE_TYPE((obj), UPNP_TYPE_CONNECTION_MANAGER_SERVICE))
#define UPNP_CONNECTION_MANAGER_SERVICE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), UPNP_TYPE_CONNECTION_MANAGER_SERVICE, \
			UpnpConnectionManagerServiceClass))
#define UPNP_IS_CONNECTION_MANAGER_SERVICE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), UPNP_CONNECTION_MANAGER_SERVICE))
#define UPNP_CONNECTION_MANAGER_SERVICE_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), UPNP_TYPE_CONNECTION_MANAGER_SERVICE, \ 				UpnpConnectionManagerServiceClass))

typedef struct _UpnpConnectionManagerService UpnpConnectionManagerService;
struct _UpnpConnectionManagerService {
	GObject parent;
};
 
typedef struct _UpnpConnectionManagerServiceClass UpnpConnectionManagerServiceClass;
struct _UpnpConnectionManagerServiceClass {
	GObjectClass parentClass;
};

GType upnpConnectionManagerService_get_type(void);
void upnpConnectionManagerServiceConnect(UpnpConnectionManagerService *self);

#endif

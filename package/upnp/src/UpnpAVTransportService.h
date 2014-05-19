#ifndef _UPNP_AVTRANSPORT_SERVICE_H
#define _UPNP_AVTRANSPORT_SERVICE_H

#include <libgupnp/gupnp.h>
#include <string.h>
#include <stdio.h>

#define UPNP_TYPE_AVTRANSPORT_SERVICE (upnpAVTransportService_get_type())
#define UPNP_AVTRANSPORT_SERVICE(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), UPNP_TYPE_AVTRANSPORT_SERVICE, UpnpAVTransportService))
#define UPNP_IS_AVTRANSPORT_SERVICE(obj) G_TYPE_CHECK_INSTANCE_TYPE((obj), UPNP_TYPE_AVTRANSPORT_SERVICE))
#define UPNP_AVTRANSPORT_SERVICE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), UPNP_TYPE_AVTRANSPORT_SERVICE, UpnpAVTransportServiceClass))
#define UPNP_IS_AVTRANSPORT_SERVICE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), UPNP_AVTRANSPORT_SERVICE))
#define UPNP_AVTRANSPORT_SERVICE_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), UPNP_TYPE_AVTRANSPORT_SERVICE, UpnpAVTransportServiceClass))

typedef struct _UpnpAVTransportService UpnpAVTransportService;
struct _UpnpAVTransportService {
	GObject parent;
};
 
typedef struct _UpnpAVTransportServiceClass UpnpAVTransportServiceClass;
struct _UpnpAVTransportServiceClass {
	GObjectClass parentClass;
};

GType upnpAVTransportService_get_type(void);
void upnpAVTransportServiceConnect(UpnpAVTransportService *self);
gint64 time_from_string (gchar* str);

#endif

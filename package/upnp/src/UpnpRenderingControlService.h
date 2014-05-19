#ifndef _UPNP_RENDERING_CONTROL_SERVICE
#define _UPNP_RENDERING_CONTROL_SERVICE

#include <libgupnp/gupnp.h>

#define UPNP_TYPE_RENDERING_CONTROL_SERVICE (upnpRenderingControlService_get_type())
#define UPNP_RENDERING_CONTROL_SERVICE(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), UPNP_TYPE_RENDERING_CONTROL_SERVICE, UpnpRenderingControlService))
#define UPNP_IS_RENDERING_CONTROL_SERVICE(obj) G_TYPE_CHECK_INSTANCE_TYPE((obj), UPNP_TYPE_RENDERING_CONTROL_SERVICE))
#define UPNP_RENDERING_CONTROL_SERVICE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), UPNP_TYPE_RENDERING_CONTROL_SERVICE, UpnpRenderingControlServiceClass))
#define UPNP_IS_RENDERING_CONTROL_SERVICE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), UPNP_RENDERING_CONTROL_SERVICE))
#define UPNP_RENDERING_CONTROL_SERVICE_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), UPNP_TYPE_RENDERING_CONTROL_SERVICE, UpnpRenderingControlServiceClass))

typedef struct _UpnpRenderingControlService UpnpRenderingControlService;
struct _UpnpRenderingControlService {
	GObject parent;
};
 
typedef struct _UpnpRenderingControlServiceClass UpnpRenderingControlServiceClass;
struct _UpnpRenderingControlServiceClass {
	GObjectClass parentClass;
};

GType upnpRenderingControlService_get_type(void);
void upnpRenderingControlServiceConnect(UpnpRenderingControlService *self);
void storecurrentvolume(gpointer *self);

#endif


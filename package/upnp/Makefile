#
# Copyright (C) 2008-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=upnp
PKG_VERSION:=1.0
PKG_RELEASE:=1
PKG_BUILD_PARALLEL:=0

PKG_BUILD_DIR:= $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/upnpdevice
  SECTION:=utils
  CATEGORY:=Sound
  DEPENDS:=+libglib +libgobject +libgupnp +alsa-lib +libao
  TITLE:=UPnP MediaRenderer
endef

TARGET_CFLAGS += -I$(STAGING_DIR)/usr/include \
		 -I$(STAGING_DIR)/usr/include/gupnp-1.0 \
		 -I$(STAGING_DIR)/usr/include/gssdp-1.0 \
		 -I$(STAGING_DIR)/usr/include/glib-2.0 \
		 -I$(STAGING_DIR)/usr/lib/glib-2.0/include \
		 -I$(STAGING_DIR)/usr/include/libsoup-2.4 \
		 -I$(STAGING_DIR)/usr/include/libxml2 \
		 -I$(STAGING_DIR)/usr/include/gstreamer-0.10

TARGET_LDFLAGS += -L$(STAGING_DIR)/usr/lib \
		  -L$(STAGING_DIR)/usr/lib/gstreamer-0.10

define Package/upnpdevice/description
upnpdevice is a upnp mediarenderer.
you can use's the upnp ControlPoint to make it play music
from the cloud compute or your phone or any network multimedia file..
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef


define Package/upnpdevice/install
	$(INSTALL_DIR) $(1)/etc/webxml
	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_DIR) $(1)/etc
	#$(INSTALL_DIR) $(1)/root/.gstreamer-0.10
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/upnpdevice $(1)/sbin/
	$(CP) ./files/upnpdevice.init $(1)/etc/init.d/upnpdevice
#	$(CP) ./files/udprecv.init $(1)/etc/init.d/udprecv
	$(CP) ./files/up.sh $(1)/etc/up.sh
	$(CP) ./files/webxml/* $(1)/etc/webxml
	chmod 1777 $(1)/etc/webxml/
	chmod 1777 $(1)/etc/webxml/* -R
	#$(CP) ./files/root/.gstreamer-0.10/* $(1)/root/.gstreamer-0.10
	#chmod 1777 $(1)/root/.gstreamer-0.10/* -R
endef

#define Build/InstallDev
#	$(INSTALL_DIR) $(1)/usr/include
#	$(CP) $(PKG_BUILD_DIR)/uci{,_config,map}.h $(1)/usr/include
#	$(INSTALL_DIR) $(1)/usr/lib
#	$(CP) $(PKG_BUILD_DIR)/libuci.so* $(1)/usr/lib
#	$(CP) $(PKG_BUILD_DIR)/libucimap.a $(1)/usr/lib
#endef

$(eval $(call BuildPackage,upnpdevice))

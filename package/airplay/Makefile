#
# Copyright (C) 2008-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=Airplay
PKG_VERSION:=1.0
PKG_RELEASE:=1
PKG_BUILD_PARALLEL:=0

PKG_BUILD_DIR:= $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/Airplay
  SECTION:=utils
  CATEGORY:=Sound
  DEPENDS:=+libglib +libgstreamer +libopenssl +libgstapp +libcrypto + libgstapp +libavahi-client +avahi-utils
  TITLE:=C MediaRenderer Device(UCI)
endef

TARGET_CFLAGS += -I$(STAGING_DIR)/usr/include \
		 -I$(STAGING_DIR)/usr/include/libxml2 \
		 -I$(STAGING_DIR)/usr/include/glib-2.0 \
		 -I$(STAGING_DIR)/usr/lib/glib-2.0/include \
		 -I$(STAGING_DIR)/usr/include/gstreamer-0.10

TARGET_LDFLAGS += -L$(STAGING_DIR)/usr/lib \
		  -L$(STAGING_DIR)/usr/lib/gstreamer-0.10

define Package/Airplay/description
Airplay is a Apple mediarenderer.
you can use's the Apple device to make it play music
from the cloud compute or your phone or any network multimedia file..
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef


define Package/Airplay/install
	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/Airplay $(1)/sbin/
	$(CP) ./files/Airplay.init $(1)/etc/init.d/Airplay
endef

#define Build/InstallDev
#	$(INSTALL_DIR) $(1)/usr/include
#	$(CP) $(PKG_BUILD_DIR)/uci{,_config,map}.h $(1)/usr/include
#	$(INSTALL_DIR) $(1)/usr/lib
#	$(CP) $(PKG_BUILD_DIR)/libuci.so* $(1)/usr/lib
#	$(CP) $(PKG_BUILD_DIR)/libucimap.a $(1)/usr/lib
#endef

$(eval $(call BuildPackage,Airplay))

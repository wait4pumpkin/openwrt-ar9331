#
# Copyright (C) 2008-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=controller
PKG_VERSION:=1.0
PKG_RELEASE:=1
PKG_BUILD_PARALLEL:=0

PKG_BUILD_DIR:= $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/controller
  SECTION:=base
  CATEGORY:=Base system
  TITLE:=RESTful api for device control
endef

TARGET_CFLAGS += -I$(STAGING_DIR)/usr/include

TARGET_LDFLAGS += -L$(STAGING_DIR)/usr/lib

define Package/ssid/description
RESTful api for device control, including device info, wifi and ether configuration.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef


define Package/ssid/install
	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/wifiController $(1)/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/etherController $(1)/sbin/

	$(INSTALL_DATA) ./files/info $(1)/etc/config/
	$(INSTALL_DIR) $(1)/www
	$(INSTALL_DIR) $(1)/www/cgi-bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/wifi-status $(1)/www/cgi-bin/wifi-status.cgi
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/wifi-set $(1)/www/cgi-bin/wifi-set.cgi
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/ether-status $(1)/www/cgi-bin/ether-status.cgi
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/ether-set $(1)/www/cgi-bin/ether-set.cgi
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/device-status $(1)/www/cgi-bin/device-status.cgi
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/device-reset $(1)/www/cgi-bin/device-reset.cgi
endef

$(eval $(call BuildPackage,ssid))

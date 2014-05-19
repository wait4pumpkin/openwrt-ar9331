cmake_minimum_required(VERSION 2.6)

PROJECT(upnpdevice C)

SET(CMAKE_INSTALL_PREFIX /usr)

ADD_DEFINITIONS(-Os -Wall -Werror --std=gnu99 -I. -DUCI_PREFIX="${CMAKE_INSTALL_PREFIX}")

FILE(GLOB SOURCE "./*.c")
ADD_EXECUTABLE(upnpdevice ${SOURCE})  

INCLUDE_DIRECTORIES("/usr/include")
INCLUDE_DIRECTORIES("/usr/include/gupnp-1.0")
INCLUDE_DIRECTORIES("/usr/include/gssdp-1.0")
#INCLUDE_DIRECTORIES("/usr/lib/glib-2.0/include")
INCLUDE_DIRECTORIES("/usr/lib/i386-linux-gnu/glib-2.0/include/")
INCLUDE_DIRECTORIES("/usr/include/glib-2.0")
INCLUDE_DIRECTORIES("/usr/include/libsoup-2.4")
INCLUDE_DIRECTORIES("/usr/include/libxml2")

TARGET_LINK_LIBRARIES(upnpdevice)

INSTALL(TARGETS upnpdevice
	ARCHIVE DESTINATION lib
	LIBRARY DESTINATION lib
	RUNTIME DESTINATION sbin
)

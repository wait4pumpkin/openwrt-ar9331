cmake_minimum_required(VERSION 2.6)

PROJECT(Airplay)

SET(CMAKE_INSTALL_PREFIX /usr)

ADD_DEFINITIONS(-Os -Wall -Werror --std=gnu99 -g3 -I. -DUCI_PREFIX="${CMAKE_INSTALL_PREFIX}")

FILE(GLOB SOURCE "./*.c")
ADD_EXECUTABLE(Airplay ${SOURCE})  

INCLUDE_DIRECTORIES("/usr/include/glib-2.0")
INCLUDE_DIRECTORIES("/usr/lib/x86_64-linux-gnu/glib-2.0/include")
INCLUDE_DIRECTORIES("/usr/local/include/gstreamer-0.10")

TARGET_LINK_LIBRARIES(Airplay gobject-2.0 glib-2.0 gio-2.0 ssl crypto gstreamer-0.10 gstapp-0.10)

INSTALL(TARGETS Airplay
	ARCHIVE DESTINATION lib
	LIBRARY DESTINATION lib
	RUNTIME DESTINATION sbin
)

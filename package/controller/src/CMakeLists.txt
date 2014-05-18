cmake_minimum_required(VERSION 2.6)


PROJECT(ssid C)

SET(CMAKE_INSTALL_PREFIX /usr)

ADD_DEFINITIONS(-Os -Wall -Werror --std=gnu99 -I. -DUCI_PREFIX="${CMAKE_INSTALL_PREFIX}")

FILE(GLOB SOURCE "./slre.c")
ADD_LIBRARY(slre SHARED ${SOURCE})

FILE(GLOB SOURCE "./utils.c")
ADD_LIBRARY(control SHARED ${SOURCE})

FILE(GLOB SOURCE "./test.c" "./utils.c")
ADD_EXECUTABLE(test ${SOURCE})
TARGET_LINK_LIBRARIES(test uci json-c)
INSTALL(TARGETS test
	ARCHIVE DESTINATION lib
	LIBRARY DESTINATION lib
	RUNTIME DESTINATION sbin
)

FILE(GLOB SOURCE "./wifi-status.c")
ADD_EXECUTABLE(wifi-status ${SOURCE})
TARGET_LINK_LIBRARIES(wifi-status control uci json-c)

FILE(GLOB SOURCE "./wifi-set.c")
ADD_EXECUTABLE(wifi-set ${SOURCE})
TARGET_LINK_LIBRARIES(wifi-set control slre uci json-c)

#FILE(GLOB SOURCE "./wifi.c")
#ADD_EXECUTABLE(wifiController ${SOURCE})
#INSTALL(TARGETS wifiController
#	ARCHIVE DESTINATION lib
#	LIBRARY DESTINATION lib
#	RUNTIME DESTINATION sbin
#)#

#FILE(GLOB SOURCE "./ether.c")
#ADD_EXECUTABLE(etherController ${SOURCE})
#TARGET_LINK_LIBRARIES(etherController uci)
#INSTALL(TARGETS etherController
#	ARCHIVE DESTINATION lib
#	LIBRARY DESTINATION lib
#	RUNTIME DESTINATION sbin
#)#
#

#FILE(GLOB SOURCE "./ether-status.c" "./utils.c")
#ADD_EXECUTABLE(ether-status ${SOURCE})
#TARGET_LINK_LIBRARIES(ether-status m)
#INSTALL(TARGETS ether-status
#	ARCHIVE DESTINATION lib
#	LIBRARY DESTINATION lib
#	RUNTIME DESTINATION /www/cgi-bin
#)#

#FILE(GLOB SOURCE "./ether-set.c" "./slre.c" "./utils.c")
#ADD_EXECUTABLE(ether-set ${SOURCE})
#TARGET_LINK_LIBRARIES(ether-set m)
#INSTALL(TARGETS ether-set
#	ARCHIVE DESTINATION lib
#	LIBRARY DESTINATION lib
#	RUNTIME DESTINATION /www/cgi-bin
#)#

#FILE(GLOB SOURCE "./device-status.c" "./utils.c")
#ADD_EXECUTABLE(device-status ${SOURCE})
#TARGET_LINK_LIBRARIES(device-status m)
#INSTALL(TARGETS device-status
#	ARCHIVE DESTINATION lib
#	LIBRARY DESTINATION lib
#	RUNTIME DESTINATION /www/cgi-bin
#)#

#FILE(GLOB SOURCE "./device-reset.c" "./utils.c")
#ADD_EXECUTABLE(device-reset ${SOURCE})
#TARGET_LINK_LIBRARIES(device-reset m)
#INSTALL(TARGETS device-status
#	ARCHIVE DESTINATION lib
#	LIBRARY DESTINATION lib
#	RUNTIME DESTINATION /www/cgi-bin
#)#


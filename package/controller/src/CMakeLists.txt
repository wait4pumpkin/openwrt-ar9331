cmake_minimum_required(VERSION 2.6)

PROJECT(controller C)

SET(CMAKE_INSTALL_PREFIX /usr)

ADD_DEFINITIONS(-Os -Wall -Werror --std=gnu99 -I. -DUCI_PREFIX="${CMAKE_INSTALL_PREFIX}")

FILE(GLOB SOURCE "./utils.c" "./slre.c")
ADD_LIBRARY(control SHARED ${SOURCE})

FILE(GLOB SOURCE "./wifi-status.c")
ADD_EXECUTABLE(wifi-status ${SOURCE})
TARGET_LINK_LIBRARIES(wifi-status control uci json-c)

FILE(GLOB SOURCE "./wifi-set.c")
ADD_EXECUTABLE(wifi-set ${SOURCE})
TARGET_LINK_LIBRARIES(wifi-set control uci json-c)

FILE(GLOB SOURCE "./ether-status.c")
ADD_EXECUTABLE(ether-status ${SOURCE})
TARGET_LINK_LIBRARIES(ether-status control uci json-c)

FILE(GLOB SOURCE "./ether-set.c")
ADD_EXECUTABLE(ether-set ${SOURCE})
TARGET_LINK_LIBRARIES(ether-set control uci json-c)

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


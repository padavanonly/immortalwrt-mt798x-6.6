cmake_minimum_required(VERSION 3.10)

PROJECT(rpcd-mod-mtk-clients C)

ADD_DEFINITIONS(-Os -Wall -Werror --std=gnu99 -D_GNU_SOURCE)

IF (NOT DEFINED LIBNL_LIBS)
  FIND_LIBRARY(libnl NAMES libnl-3 libnl nl-3 nl-tiny nl)
  SET(LIBNL_LIBS ${libnl})
ENDIF()

ADD_LIBRARY(mtk_clients MODULE discover.c traffic.c webprobe.c)
TARGET_LINK_LIBRARIES(mtk_clients ubox ubus uci ${LIBNL_LIBS})
SET_TARGET_PROPERTIES(mtk_clients PROPERTIES OUTPUT_NAME mtk_clients PREFIX "")

INSTALL(TARGETS mtk_clients LIBRARY DESTINATION lib)

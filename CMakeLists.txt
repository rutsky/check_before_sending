cmake_minimum_required (VERSION 2.6)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

project(pidgin-check-before-sending)

find_package(Pidgin REQUIRED)
add_definitions(${PIDGIN_DEFINITIONS})
include_directories(${PIDGIN_INCLUDE_DIRS})

find_package(Purple REQUIRED)
add_definitions(${PURPLE_DEFINITIONS})
include_directories(${PURPLE_INCLUDE_DIRS})

find_package(GTK2 REQUIRED)
add_definitions(${GTK2_DEFINITIONS})
include_directories(${GTK2_INCLUDE_DIRS})

add_library(check_before_sending SHARED check_before_sending.c)
install(TARGETS check_before_sending LIBRARY DESTINATION .purple/plugins NAMELINK_SKIP)

target_link_libraries(check_before_sending ${LIBPURPLE_LIBRARIES} ${GTK2_LIBRARIES} ${PIDGIN_LIBRARIES})

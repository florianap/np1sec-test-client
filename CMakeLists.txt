cmake_minimum_required (VERSION 2.8)
################################################################################
set(CMAKE_BUILD_TYPE Debug)
set(GLOB BOOST_VERSION 1.58)

################################################################################
project (np1sec-pidgin-plugin)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y -Wall -ggdb")
set(PIDGIN_INC_DIR "/usr/include" CACHE FILEPATH "pidgin dir")

find_package(Boost ${BOOST_VERSION} COMPONENTS REQUIRED)
# Use the package PkgConfig to detect GTK+ headers/library files
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK2 REQUIRED gtk+-2.0)

link_directories(
    ${NP1SEC_LIB_DIR}
)

include_directories(
  "${Boost_INCLUDE_DIR}"
  "${CMAKE_SOURCE_DIR}/src"
  "${PIDGIN_INC_DIR}"
  "${PIDGIN_INC_DIR}/libpurple" # works around a pidgin bug
  "${NP1SEC_INC_DIR}"
  "${GTK2_INCLUDE_DIRS}"
)

file(GLOB sources
  "${CMAKE_SOURCE_DIR}/src/*.cpp"
)

add_library(np1sec-plugin SHARED ${sources})
target_link_libraries(np1sec-plugin libnp1sec.so)

################################################################################

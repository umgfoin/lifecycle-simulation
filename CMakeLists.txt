cmake_minimum_required (VERSION 2.8)

set (CMAKE_INSTALL_PREFIX "/" CACHE PATH "Global installation prefix. Works for relocating the complete installation. Normally / ")

project(LifecycleSimulator)
include(FindPkgConfig)

# Cmake module directory
set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/")

#---------------------------------
#--------------------------------
# needed for Boost 2.8
set (Boost_NO_BOOST_CMAKE ON)
#---------------------------------
# ---------------- Find Boost 1.58
set (BOOST_INCLUDEDIR "/usr/include/boost148") # EPEL5
set (BOOST_LIBRARYDIR "/usr/lib${LIB_SUFFIX}/boost148/") # EPEL5
set (Boost_ADDITIONAL_VERSIONS "1.59" "1.59.0")

#needed for GCC6
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

find_package (Boost 1.59 COMPONENTS random date_time system thread regex REQUIRED)

if (Boost_FOUND)
  # Hack for CentOS6 with EPEL enabled
  if (Boost_INCLUDE_DIR STREQUAL "/usr/include/boost148")
    message ("INFO: Overriding Boost library location")
    set (Boost_LIBRARY_DIRS "/usr/lib${LIB_SUFFIX}/boost148/")
  endif ()

  message ("Boost headers under: ${Boost_INCLUDE_DIR}")
  message ("Boost libraries: ${Boost_LIBRARY_DIRS}")
endif (Boost_FOUND)

set(simulatorSrcs stdafx.cpp main.cpp gpd-x303s-control.cpp asio_serial_comm.cpp files.cpp)

add_executable(simulator ${simulatorSrcs})

target_link_libraries(simulator libboost_system-mt.so libboost_random-mt.so libboost_filesystem-mt.so pthread)

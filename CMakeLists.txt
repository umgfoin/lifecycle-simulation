cmake_minimum_required (VERSION 3.7)

set (CMAKE_INSTALL_PREFIX "/" CACHE PATH "Global installation prefix. Works for relocating the complete installation. Normally / ")

project(LifecycleSimulator)
include(FindPkgConfig)

# Cmake module directory
set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/")
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/build)
# set (BOOST_ROOT "/usr/include/boost")
#---------------------------------
#--------------------------------
# needed for Boost 2.8BOOST_ROOT
# set (Boost_NO_BOOST_CMAKE ON)

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

#---------------------------------
# ---------------- Find Boost - start search here
set (BOOST_INCLUDEDIR "/usr/include/boost")
set (BOOST_LIBRARYDIR "/usr/lib${LIB_SUFFIX}/boost/")
set (Boost_ADDITIONAL_VERSIONS "1.48" "1.48.0")

find_package (Boost 1.4 COMPONENTS filesystem random date_time system thread regex REQUIRED)

# switch to additional alt. boot-lib, if desired
if (Boost_FOUND)
  # Hack for CentOS6 with EPEL enabled
  if (Boost_INCLUDE_DIR STREQUAL "/usr/include/boost148")
    message ("INFO: Overriding Boost library location")
    set (Boost_LIBRARY_DIRS "/usr/lib${LIB_SUFFIX}/boost148/")
  endif ()

  message ("Boost headers under: ${Boost_INCLUDE_DIR}")
  message ("Boost libraries: ${Boost_LIBRARY_DIRS}")
endif (Boost_FOUND)



#set(simulatorSrcs stdafx.cpp main.cpp gpd-x303s-control.cpp asio_serial_comm.cpp files.cpp)

# compile all cpp-files
file(GLOB_RECURSE cpp_sources src/*.cpp)

set(TARGET_NAME "simulator")

add_executable(${TARGET_NAME} ${cpp_sources})

# append additional compiler-flags
# set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

# compile as C++14 (current CMake-style)
set_property(TARGET ${TARGET_NAME} PROPERTY CXX_STANDARD 14)
set_property(TARGET ${TARGET_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)

# compile as C++14 (newest CMake only)
# target_compile_features(${TARGET_NAME} PUBLIC cxx_std_14)

target_link_libraries(${TARGET_NAME} ${Boost_LIBRARIES} Threads::Threads)

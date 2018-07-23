#
# Copyright 2014-2018 Neueda Ltd.
#
cmake_minimum_required(VERSION 2.8.0)
project(logger CXX)

set(CMAKE_C_COMPILER gcc)
set(CMAKE_CXX_COMPILER g++)
set(SUBMODULE_FLAGS -DTESTS=off)

if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set (CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "default install path" FORCE)
endif()

if(APPLE)
  set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")
  set(CMAKE_XCODE_ATTRIBUTE_GCC_VERSION "com.apple.compilers.llvm.clang.1_0")
  set(CMAKE_SHARED_LINKER_FLAGS "-Wl,-undefined,dynamic_lookup")
  set(CMAKE_MACOSX_RPATH 1)
endif()

# options
option(DEBUG "Enable debug build" OFF)
option(JAVA "Enable Java bindings" OFF)
option(PYTHON "Enable Python bindings" OFF)
option(CSHARP "Enable C# bindings" OFF)
option(COVERAGE "Enable gcov coverage" OFF)

# set version info
set(LOGGER_MAJOR_VERSION 0)
set(LOGGER_MINOR_VERSION 3)
set(LOGGER_PATCH_VERSION 2)
set(LOGGER_VERSION ${LOGGER_MAJOR_VERSION}.${LOGGER_MINOR_VERSION}.${LOGGER_PATCH_VERSION})

if (UNIX)
  macro (add_library _name)
    _add_library(${ARGV})
    if (NOT APPLE AND TARGET ${_name})
      SET_TARGET_PROPERTIES(${_name}
        PROPERTIES
            VERSION ${LOGGER_VERSION}
            # SOVERSION 0.0.0
        )
    endif()
  endmacro()
endif (UNIX)

add_compile_options(-Wall -c -W -g)
if (DEBUG)
  message(STATUS "logger DEBUG: ON")
  set(SUBMODULE_FLAGS ${SUBMODULE_FLAGS} -DDEBUG=on)
else ()
  message(STATUS "logger DEBUG: OFF")
  add_compile_options(-O3)
endif (DEBUG)

if (JAVA)
  message(STATUS "JAVA: ON")
  set(SUBMODULE_FLAGS ${SUBMODULE_FLAGS} -DJAVA=ON)
else ()
  message(STATUS "JAVA: OFF")
  set(SUBMODULE_FLAGS ${SUBMODULE_FLAGS} -DJAVA=OFF)
endif()
if (PYTHON)
  message(STATUS "PYTHON: ON")
  set(SUBMODULE_FLAGS ${SUBMODULE_FLAGS} -DPYTHON=ON)
else ()
  message(STATUS "PYTHON: OFF")
  set(SUBMODULE_FLAGS ${SUBMODULE_FLAGS} -DPYTHON=OFF)
endif()
if (CSHARP)
  message(STATUS "CSHARP: ON")
  set(SUBMODULE_FLAGS ${SUBMODULE_FLAGS} -DCSHARP=ON)
else ()
  message(STATUS "CSHARP: OFF")
  set(SUBMODULE_FLAGS ${SUBMODULE_FLAGS} -DCSHARP=OFF)
endif()

# need flex
find_package(FLEX REQUIRED)

# allow external projects
include(${CMAKE_ROOT}/Modules/ExternalProject.cmake)

# extra include and links
include_directories(
  ${PROJECT_SOURCE_DIR}/src
  ${CMAKE_INSTALL_PREFIX}/include/
  ${CMAKE_INSTALL_PREFIX}/include/sbf
  ${CMAKE_INSTALL_PREFIX}/include/utils
  ${CMAKE_INSTALL_PREFIX}/include/event2
  ${CMAKE_INSTALL_PREFIX}/include/properties)
link_directories(${CMAKE_INSTALL_PREFIX}/lib)

# coverage
if (COVERAGE)
    set(COVERAGE_COMPILE_FLAGS "-g -O0 -coverage -fprofile-arcs -ftest-coverage")
    set(COVERAGE_LINK_FLAGS    "-coverage -lgcov")
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${COVERAGE_COMPILE_FLAGS}" )
    set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${COVERAGE_LINK_FLAGS}" )
endif(COVERAGE)

# add source
add_subdirectory(src)
add_subdirectory(examples)

# deps
find_library(EVENT_LIB
  NAMES event
  HINTS "${CMAKE_INSTALL_PREFIX}/lib"
  )
find_library(EVENT_PTHREAD_LIB
  NAMES event_pthreads
  HINTS "${CMAKE_INSTALL_PREFIX}/lib"
  )
if (EVENT_LIB AND EVENT_PTHREAD_LIB)
  message(STATUS "event-found: " ${EVENT_LIB})
  message(STATUS "event-pthread-found: " ${EVENT_PTHREAD_LIB})
else()
  set(LIBEVENT_CMAKE ${PROJECT_SOURCE_DIR}/ext/libevent CACHE PATH "Location of libevent-cmake" FORCE)
  message(STATUS "libevent-location: " ${LIBEVENT_CMAKE})
  ExternalProject_Add(
    LIBEVENT_PROJECT
    SOURCE_DIR ${LIBEVENT_CMAKE}
    CMAKE_ARGS -DEVENT__DISABLE_TESTS=ON -DEVENT__DISABLE_REGRESS=ON -DEVENT__DISABLE_OPENSSL=ON -DEVENT__DISABLE_SAMPLES=ON -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
    )
endif()

# sbf
set(sbf_DIR ${CMAKE_INSTALL_PREFIX}/lib/cmake/sbf)
find_package(sbf QUIET)
if(NOT ${sbf_FOUND})
  set(SBF_CMAKE ${PROJECT_SOURCE_DIR}/ext/sbf CACHE PATH "Location of sbf-cmake" FORCE)
  message(STATUS "sbf-location: " ${SBF_CMAKE})
  ExternalProject_Add(
    SBF_PROJECT
    SOURCE_DIR ${SBF_CMAKE}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX} ${SUBMODULE_FLAGS}
    )
  ExternalProject_Get_Property(SBF_PROJECT binary_dir)
  set(SBF_PROJECT_DIR ${binary_dir}/lib/cmake/sbf CACHE PATH "location of sbf cmake files" FORCE)
  if (NOT EVENT_LIB)
    add_dependencies(SBF_PROJECT LIBEVENT_PROJECT)
  endif()
endif()

# utils
set(utils_DIR ${CMAKE_INSTALL_PREFIX}/lib/cmake/utils)
find_package(utils QUIET)
if(NOT ${utils_FOUND})
  set(UTILS_CMAKE ${PROJECT_SOURCE_DIR}/ext/utils CACHE PATH "Location of utils-cmake" FORCE)
  message(STATUS "utils-location: " ${UTILS_CMAKE})
  ExternalProject_Add(
    UTILS_PROJECT
    SOURCE_DIR ${UTILS_CMAKE}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX} ${SUBMODULE_FLAGS}
    )
  ExternalProject_Get_Property(UTILS_PROJECT binary_dir)
  set(UTILS_PROJECT_DIR ${binary_dir}/lib/cmake/utils CACHE PATH "location of utils cmake files" FORCE)
  add_dependencies(UTILS_PROJECT SBF_PROJECT)
endif()

# properties
set(properties_DIR ${CMAKE_INSTALL_PREFIX}/lib/cmake/properties)
find_package(properties QUIET)
if(NOT ${properties_FOUND})
  set(PROPERTIES_CMAKE ${PROJECT_SOURCE_DIR}/ext/properties CACHE PATH "Location of properties-cmake" FORCE)
  message(STATUS "properties-location: " ${PROPERTIES_CMAKE})
  ExternalProject_Add(
      PROPERTIES_PROJECT
      SOURCE_DIR ${PROPERTIES_CMAKE}
      CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX} ${SUBMODULE_FLAGS}
  )
  ExternalProject_Get_Property(PROPERTIES_PROJECT binary_dir)
  set(PROPERTIES_PROJECT_DIR ${binary_dir}/lib/cmake/properties CACHE PATH "location of properties cmake files" FORCE)
  if(NOT ${sbf_FOUND})
      add_dependencies(PROPERTIES_PROJECT SBF_PROJECT UTILS_PROJECT)
  endif()
endif()

# unit-tests
option(TESTS "Enable unit-tests" OFF)
if(TESTS)
  ExternalProject_Add(
    googletest
    SOURCE_DIR ${PROJECT_SOURCE_DIR}/ext/googletest
    CMAKE_ARGS -DBUILD_GMOCK=ON -DBUILD_GTEST=ON -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/googletest -DCMAKE_INSTALL_LIBDIR=${CMAKE_BINARY_DIR}/googletest/lib
  )
  
  include(CTest)
  enable_testing()
  
  add_subdirectory(test)
endif()

INCLUDE(CPack)
SET(CPACK_GENERATOR "TGZ")

# add examples
INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/examples/cpp/ DESTINATION
        ${CMAKE_INSTALL_PREFIX}/examples/cpp/)

if(JAVA)
    INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/examples/java/ DESTINATION
            ${CMAKE_INSTALL_PREFIX}/examples/java/)
endif()

if(PYTHON)
    INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/examples/python/ DESTINATION
            ${CMAKE_INSTALL_PREFIX}/examples/python/)
endif()

#
# Copyright 2014-2018 Neueda Ltd.
#
# Include Java
find_package(Java REQUIRED COMPONENTS Runtime Development)
find_package(JNI REQUIRED)
include(UseJava)

include_directories(${JNI_INCLUDE_DIRS})

# set the flags for swig
set(CMAKE_SWIG_FLAGS -package com.neueda.logger)
set_source_files_properties(../logger.java.i PROPERTIES CPLUSPLUS ON)

# Add swig module
add_compile_options(-fno-strict-aliasing)
swig_add_module(Logger java ../logger.java.i)
swig_link_libraries(Logger logger ${JNI_LIBRARIES})

set(CMAKE_JAR_CLASSES_PREFIX com/neueda/logger)
set(JNI_SOURCES
  ${CMAKE_CURRENT_BINARY_DIR}/Logger.java
  ${CMAKE_CURRENT_BINARY_DIR}/LogHandlerFactory.java
  ${CMAKE_CURRENT_BINARY_DIR}/LogHandler.java
  ${CMAKE_CURRENT_BINARY_DIR}/loggerBindings.java
  ${CMAKE_CURRENT_BINARY_DIR}/loggerBindingsJNI.java
  ${CMAKE_CURRENT_BINARY_DIR}/LogService.java
  ${CMAKE_CURRENT_BINARY_DIR}/LogSeverity.java
  ${CMAKE_CURRENT_BINARY_DIR}/SWIGTYPE_p_bool.java
  ${CMAKE_CURRENT_BINARY_DIR}/SWIGTYPE_p_std__string.java
  ${CMAKE_CURRENT_BINARY_DIR}/SWIGTYPE_p_tm.java
  ${CMAKE_CURRENT_BINARY_DIR}/SWIGTYPE_p_setT_neueda__logHandler_p_t.java
  ${CMAKE_CURRENT_BINARY_DIR}/SWIGTYPE_p_timeval.java
  ${CMAKE_CURRENT_BINARY_DIR}/SWIGTYPE_p_void.java)

add_jar(LoggerJNI
  SOURCES
  ${JNI_SOURCES}
  INCLUDE_JARS
  ${CMAKE_INSTALL_PREFIX}/lib/java/PropertiesJNI.jar
  )
if(TARGET _Logger)
  add_dependencies(LoggerJNI _Logger)
else()
  add_dependencies(LoggerJNI Logger)
endif()

# Files to install with Java
if(APPLE)
  set(JAVA_DYLIB_SUFFIX ".jnilib")
else()
  set(JAVA_DYLIB_SUFFIX ".so")
endif()

# cmake 2.8 swig has issues on figuring out naming libs
if(TARGET _Logger)
  set_target_properties(_Logger PROPERTIES OUTPUT_NAME "Logger")
  set_target_properties(_Logger PROPERTIES SUFFIX ${JAVA_DYLIB_SUFFIX})
endif()

set(JAVA_INSTALL_FILES
  # ${CMAKE_CURRENT_SOURCE_DIR}/run.sh
  # ${CMAKE_CURRENT_SOURCE_DIR}/build.sh
  # ${CMAKE_CURRENT_SOURCE_DIR}/example.java
  ${CMAKE_CURRENT_BINARY_DIR}/LoggerJNI.jar)

# Declare install target for java
install(FILES ${JAVA_INSTALL_FILES}
  DESTINATION lib/java)

if(TARGET _Logger)
  install(TARGETS _Logger
    DESTINATION lib/java)
else()
  install(TARGETS Logger
    DESTINATION lib/java)
endif()

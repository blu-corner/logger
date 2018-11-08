#
# Copyright 2014-2018 Neueda Ltd.
#
if(NOT CSHARP_COMPILER)
  if(NOT WIN32)
    find_program(CSHARP_COMPILER mcs)
  else()
    find_program(CSHARP_COMPILER csc)
  endif()

  if(NOT CSHARP_COMPILER)
    message(FATAL_ERROR "Failed to find csharp compiler")
  endif()
endif()
message(STATUS "Found csharp compiler: ${CSHARP_COMPILER}")
set(CSHARP_OUTPUT_NAME "LoggerBindings.dll")

# set the flags for swig
set(CMAKE_SWIG_FLAGS -namespace Neueda.Log -dllimport LogCsharp -outfile Logger.cs)
set_source_files_properties(../logger.csharp.i PROPERTIES CPLUSPLUS ON)

# Add swig module
add_compile_options(-fno-strict-aliasing)
swig_add_module(LogCsharp csharp ../logger.csharp.i)
swig_link_libraries(LogCsharp logger)

# Files to install with csharp
add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${CSHARP_OUTPUT_NAME}
    COMMAND ${CSHARP_COMPILER} -target:library -out:${CMAKE_CURRENT_BINARY_DIR}/${CSHARP_OUTPUT_NAME} -reference:${CMAKE_INSTALL_PREFIX}/lib/csharp/PropertiesBindings.dll ${CMAKE_CURRENT_BINARY_DIR}/*.cs
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS ${CSHARP_SRC_FILES}
  COMMENT "Generating Csharp DLL"
  VERBATIM
  )
add_custom_target(CsharpDLL ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${CSHARP_OUTPUT_NAME})
add_dependencies(CsharpDLL ${SWIG_MODULE_LogCsharp_REAL_NAME} PROPERTIES_PROJECT)

if(APPLE)
  set(CSHARP_DYLIB_PREFIX "")
  set(CSHARP_DYLIB_SUFFIX ".so")
elseif(WIN32)
  set(CSHARP_DYLIB_PREFIX "")
  set(CSHARP_DYLIB_SUFFIX ".dll")
else()
  set(CSHARP_DYLIB_PREFIX "lib")
  set(CSHARP_DYLIB_SUFFIX ".so")
endif()

# cmake 2.8 swig has issues on figuring out naming libs
set_target_properties(${SWIG_MODULE_LogCsharp_REAL_NAME} PROPERTIES OUTPUT_NAME "LogCsharp")
if(${CSHARP_DYLIB_SUFFIX})
  set_target_properties(${SWIG_MODULE_LogCsharp_REAL_NAME} PROPERTIES PREFIX ${CSHARP_DYLIB_PREFIX})
endif()
set_target_properties(${SWIG_MODULE_LogCsharp_REAL_NAME} PROPERTIES SUFFIX ${CSHARP_DYLIB_SUFFIX})

set(CSHARP_INSTALL_FILES
    ${CMAKE_CURRENT_BINARY_DIR}/${CSHARP_OUTPUT_NAME})

# Declare install target for csharp
install(FILES ${CSHARP_INSTALL_FILES}
        DESTINATION lib/csharp)

install(TARGETS ${SWIG_MODULE_LogCsharp_REAL_NAME}
        DESTINATION lib/csharp)

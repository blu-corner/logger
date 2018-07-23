#
# Copyright 2014-2018 Neueda Ltd.
#
# Include python
find_package(PythonInterp REQUIRED)
find_package(PythonLibs REQUIRED)
include_directories(${PYTHON_INCLUDE_PATH})

# set the flags for swig
set(CMAKE_SWIG_FLAGS -module logger -threads)
set_source_files_properties(../logger.py.i PROPERTIES CPLUSPLUS ON)

# Add swig module
swig_add_module(logger python ../logger.py.i)
swig_link_libraries(logger logger ${PYTHON_LIBRARIES})
set_target_properties(_logger PROPERTIES OUTPUT_NAME "_logger")

# Files to install with Python
set(PYTHON_INSTALL_FILES
  # ${CMAKE_CURRENT_SOURCE_DIR}/example-log-handler.py
  ${CMAKE_CURRENT_BINARY_DIR}/logger.py)

# Declare install target for python
install(TARGETS _logger
  DESTINATION lib/python)
install(FILES ${PYTHON_INSTALL_FILES}
  DESTINATION lib/python)


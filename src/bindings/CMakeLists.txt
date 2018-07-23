#
# Copyright 2014-2018 Neueda Ltd.
#
if(PYTHON OR JAVA OR CSHARP)
  find_package(SWIG REQUIRED)
  include(${SWIG_USE_FILE})
endif()

if(PYTHON)
  add_subdirectory(python)
endif()

if(JAVA)
  add_subdirectory(java)
endif()

if(CSHARP)
  add_subdirectory(csharp)
endif()

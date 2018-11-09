#
# Copyright 2014-2018 Neueda Ltd.
#
include_directories(
  ${CMAKE_BINARY_DIR}/googletest/include
  ${CMAKE_INSTALL_PREFIX}/include/config
  ${CMAKE_INSTALL_PREFIX}/include/sbf
  ${CMAKE_INSTALL_PREFIX}/include/utils
  ${CMAKE_INSTALL_PREFIX}/include
  ${PROJECT_SOURCE_DIR}/test
  ${PROJECT_SOURCE_DIR}/src
  )
link_directories(${CMAKE_INSTALL_PREFIX}/lib)
link_directories(${CMAKE_BINARY_DIR}/googletest/lib)

add_executable(unittest
  testrunner.cc
  testLogService.cc
  testLogHandler.cc
  testFormatScanner.cc
  testStreamInterface.cc
  testCoreOnMessageLength.cc
  )

target_link_libraries(unittest
  logger
  gtest
  gmock
  )
add_dependencies(unittest googletest)

add_test(NAME unittest
  COMMAND unittest --gtest_output=xml:../test.xml
)

install(TARGETS unittest
    RUNTIME DESTINATION bin)

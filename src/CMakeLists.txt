#
# Copyright 2014-2018 Neueda Ltd.
#
set(LOGGER_HEADERS
  logSeverity.h
  logger.h
  logHandler.h
  consoleLogHandler.h
  syslogLogHandler.h  
  fileLogHandler.h
  sharedMemoryLogHandler.h
  loggerDaemonServer.h
  ITransportDelegate.h
  sharedMemoryRingBuffer.h
  unixSocketClient.h
  )

install(FILES ${LOGGER_HEADERS}
  DESTINATION include/logger
  )

ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_SOURCE_DIR}/src/FormatScanner.cpp ${CMAKE_SOURCE_DIR}/src/FormatScanner.h
  SOURCE ${PROJECT_SOURCE_DIR}/src/FormatScanner.l
  DEPENDS ${PROJECT_SOURCE_DIR}/src/FormatScanner.l
  COMMAND ${FLEX_EXECUTABLE}
  ARGS --c++ --outfile=${CMAKE_SOURCE_DIR}/src/FormatScanner.cpp --header-file=${CMAKE_SOURCE_DIR}/src/FormatScanner.h ${CMAKE_SOURCE_DIR}/src/FormatScanner.l
  COMMENT "Generating FormatScanner.cpp"
  )
add_custom_target(GenerateLexer ALL
  DEPENDS ${CMAKE_SOURCE_DIR}/src/FormatScanner.cpp ${CMAKE_SOURCE_DIR}/src/FormatScanner.h
  )

add_library(logger SHARED
  logger.cpp
  loggerDaemonServer.cpp
  sharedMemoryRingBuffer.cpp
  logHandler.cpp
  consoleLogHandler.cpp
  syslogLogHandler.cpp
  fileLogHandler.cpp
  sharedMemoryLogHandler.cpp
  unixSocketClient.cpp
  FormatScanner.cpp
  )

add_dependencies(logger GenerateLexer)
add_executable(logger-daemon daemon.cpp)
target_link_libraries(logger event pthread properties sbfcore sbfcommon)
add_dependencies (logger SBF_PROJECT PROPERTIES_PROJECT)
target_link_libraries(logger-daemon logger)

install(TARGETS logger-daemon
  RUNTIME DESTINATION bin)
install(TARGETS logger
  EXPORT ${PROJECT_NAME}
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib)

install(
  EXPORT ${PROJECT_NAME}
  DESTINATION lib/cmake/${PROJECT_NAME} COMPONENT dev
  PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
  )

set(${PROJECT_NAME}_INCLUDE_DIR ${CMAKE_INSTALL_PREFIX}/include)
set(${PROJECT_NAME}_LIBRARIES logger)
configure_file(
  ${PROJECT_SOURCE_DIR}/${PROJECT_NAME}.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config-install.cmake
  @ONLY
  )
install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config-install.cmake
  DESTINATION lib/cmake/${PROJECT_NAME} COMPONENT dev
  RENAME ${PROJECT_NAME}Config.cmake
  PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
  )

add_subdirectory(bindings)

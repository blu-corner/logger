#
# Copyright 2014-2018 Neueda Ltd.
#
set(EXAMPLE_INSTALL_FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/logger-client-config.ini
  ${CMAKE_CURRENT_SOURCE_DIR}/logger-daemon-config.ini)

# Declare install target for python
install(FILES ${EXAMPLE_INSTALL_FILES}
  DESTINATION etc)

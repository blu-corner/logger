#
# Copyright 2014-2018 Neueda Ltd.
#
# A Python example showing how to configure the console handler, then log output
# to the console

from properties import Properties
from logger import LogSeverity
from logger import LogService
from logger import LogHandler

props = Properties()

# set the log level of the console handler to debug
props["lh.console.level"] = "debug"

# enable colored output
props["lh.console.color"] = "true"

# retrieve and then configure the LogService
service = LogService.get ()
service.configure(props)

# retrieve a named logger
logger = service.getLogger ("PYTHON_TEST_LOGGER")
logger.setLevel(LogSeverity.DEBUG)

logger.debug("Hello world")
logger.info("Hello world")
logger.warn("Hello world")
logger.err("Hello world")

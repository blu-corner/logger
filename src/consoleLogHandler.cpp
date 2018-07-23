/*
 * Copyright 2014-2018 Neueda Ltd.
 */


#include "consoleLogHandler.h"

#include <iostream>
#include <cstdio>

# define CONSOLE_HANDLER_BLACK   "30"
# define CONSOLE_HANDLER_RED     "31"
# define CONSOLE_HANDLER_GREEN   "32"
# define CONSOLE_HANDLER_YELLOW  "33"
# define CONSOLE_HANDLER_BLUE    "34"
# define CONSOLE_HANDLER_MAGENTA "35"
# define CONSOLE_HANDLER_CYAN    "36"
# define CONSOLE_HANDLER_WHITE   "37"

namespace neueda
{

consoleLogHandler::consoleLogHandler ()
    : logHandler (),
      mOutput (stdout),
      mColorEnabled (false)
{
}

void
consoleLogHandler::handle (logSeverity::level severity,
                           const char* name,
                           const struct ::tm* tm_time,
                           const ::timeval *tv,
                           const char* message,
                           size_t message_len)
{
    std::string logMessage = toString (mFormat,
                                       severity,
                                       name,
                                       tm_time,
                                       tv,
                                       message,
                                       message_len);

    if (mColorEnabled)
        logMessage = colorize (severity, logMessage);

    fprintf (mOutput, "%s\n", logMessage.c_str ());
}

std::string
consoleLogHandler::colorize (logSeverity::level severity,
                             const std::string& message)
{

    switch (severity)
    {
    case logSeverity::INFO:
        return message;

    case logSeverity::WARN:
        return "\033[1;" CONSOLE_HANDLER_YELLOW "m" + message + "\033[0m";

    case logSeverity::ERROR:
        return "\033[1;" CONSOLE_HANDLER_RED "m" + message + "\033[0m";

    case logSeverity::DEBUG:
        return message;

    case logSeverity::FATAL:
        return "\033[1;" CONSOLE_HANDLER_RED "m" + message + "\033[0m";
    }
    return message;
}

}

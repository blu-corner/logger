/*
 * Copyright 2014-2018 Neueda Ltd.
 */

#include "syslogLogHandler.h"
#include <syslog.h>

namespace neueda
{

syslogLogHandler::syslogLogHandler ()
    : logHandler ()
{
}

void
syslogLogHandler::handle (logSeverity::level severity,
                          const char* name,
			  uint64_t time,
                          const char* message,
                          size_t message_len)
{
    string logString = toString (mFormat,
                                 severity,
                                 name,
                                 time,
                                 message,
                                 message_len);
    syslog (severityToSyslogPriority (severity),
            "%s",
            logString.c_str ());
}

int
syslogLogHandler::severityToSyslogPriority (logSeverity::level level)
{
    switch (level)
    {
    case logSeverity::DEBUG:
        return LOG_DEBUG;
    case logSeverity::INFO:
        return LOG_INFO;
    case logSeverity::WARN:
        return LOG_WARNING;
    case logSeverity::ERROR:
        return LOG_ERR;
    case logSeverity::FATAL:
        return LOG_CRIT;
    }

    return LOG_ERR;
}

}

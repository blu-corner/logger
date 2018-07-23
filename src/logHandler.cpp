/*
 * Copyright 2014-2018 Neueda Ltd.
 */

#include "logHandler.h"
#include "fileLogHandler.h"
#include "consoleLogHandler.h"
#include "syslogLogHandler.h"
#include "sharedMemoryLogHandler.h"

#include "FormatScanner.h"
#include "tokens.h"

#include "utils.h"

#include <cstdio>
#include <cstring>
#include <sstream>

#define DEFAULT_CONFIG_NAMESPACE "logger"
#define DEFAULT_CONSOLE_OUTPUT   "stdout"
#define DEFAULT_LOG_LEVEL        "info"
#define DEFAULT_LOG_SIZE         "0"
#define DEFAULT_FILE_COUNT       "0"
#define DEFAULT_LOG_FORMAT       "{severity} {time} {name} {message}"

// needed by flex
int inscribe_FlexLexer::yywrap () { return 1; }

namespace neueda
{

logHandler::logHandler () : 
    mLevel (logSeverity::INFO),
    mFormat (DEFAULT_LOG_LEVEL)
{
}

string
logHandler::toString (const string& format,
                      logSeverity::level severity,
                      const char* name,
                      const struct tm* tm_time,
                      const timeval *tv,
                      const char* message,
                      size_t message_len)
{
    ostringstream oss;
    istringstream iss (format);
    yyFlexLexer lexer (&iss, &oss);

    int yytoken = 0;
    while ((yytoken = lexer.yylex ()) != 0)
    {
        if (yytoken == TIME)
        {
            char dateTimeBuffer[64];
            memset (dateTimeBuffer, 0, sizeof dateTimeBuffer);
            size_t nBytes = snprintf (dateTimeBuffer,
                                      sizeof dateTimeBuffer,
                                      "%04u-%02u-%02u %02u:%02u:%02u.%06u",
                                      tm_time->tm_year + 1900,
                                      tm_time->tm_mon + 1,
                                      tm_time->tm_mday,
                                      tm_time->tm_hour,
                                      tm_time->tm_min,
                                      tm_time->tm_sec,
                                      (unsigned int)tv->tv_usec);
            oss << string (dateTimeBuffer, nBytes);
        }
        else if (yytoken == SEVERITY)
        {
            oss << logHandler::severityToString (severity);
        }
        else if (yytoken == NAME)
        {
            oss << name;
        }
        else if (yytoken == MESSAGE)
        {
            oss << string (message, message_len);
        }
    }
    return oss.str ();
}

string
logHandler::severityToString (const logSeverity::level severity)
{
    switch (severity)
    {
    case logSeverity::INFO:
        return "INFO";
    case logSeverity::WARN:
        return "WARN";
    case logSeverity::ERROR:
        return "ERROR";
    case logSeverity::DEBUG:
        return "DEBUG";
    case logSeverity::FATAL:
        return "FATAL";
    default:
        return "UNKNOWN";
    }
}

bool
logHandler::isLevelEnabled (logSeverity::level level) const
{
    return mLevel <= level;
}

set<logHandler*>
logHandlerFactory::getHandlers (properties& props,
                                bool& ok,
                                string& errorMessage)
{
    set<logHandler*> handlers;

    ok = configureShmHandler (props, handlers, errorMessage);
    if (!ok)
        return handlers;

    ok = configureConsoleHandler (props, handlers, errorMessage);
    if (!ok)
        return handlers;

    ok = configureFileHandler (props, handlers, errorMessage);
    if (!ok)
        return handlers;

    ok = configureSyslogHandler (props, handlers, errorMessage);
    if (!ok)
        return handlers;

    return handlers;
}

bool
logHandlerFactory::configureShmHandler (properties& props,
                                        set<logHandler*>& handlers,
                                        string& errorMessage)
{
    bool enabled;
    if (!getHandlerEnabled (props, "shm", false, enabled))
    {
        // failed to parse bool from a user configured value
        errorMessage.assign ("failed parsing property: enabled for shm");
        return false;
    }

    if (enabled)
    {
        logSeverity::level logLevel;
        if (!getHandlerLevel (props, "shm", DEFAULT_LOG_LEVEL, logLevel))
        {
            errorMessage.assign ("failed to parse value for file.level");
            return false;
        }

        string sockPath;
        if (props.get ("lh.shm.sock", sockPath))
        {
            sharedMemoryLogHandler* handler = new sharedMemoryLogHandler (sockPath);
            handler->setLevel (logLevel);
            handlers.insert (handler);
            return true;
        }
        errorMessage.assign ("failed configuring shm handler");
        return false;
    }

    return true;
}

bool
logHandlerFactory::configureConsoleHandler (properties& props,
                                            set<logHandler*>& handlers,
                                            string& errorMessage)
{
    bool enabled;
    bool valid;
    if (!getHandlerEnabled (props, "console", true, enabled))
    {
        // failed to parse bool from a user configured value
        errorMessage.assign ("failed parsing property: enabled for console");
        return false;
    }

    string format;
    string output;

    props.get ("lh.console.format", DEFAULT_LOG_FORMAT, format);
    props.get ("lh.console.output", DEFAULT_CONSOLE_OUTPUT, output);

    if (enabled)
    {
        logSeverity::level logLevel;
        if (!getHandlerLevel (props, "console", DEFAULT_LOG_LEVEL, logLevel))
        {
            errorMessage.assign ("failed to parse value for console.level");
            return false;
        }

        FILE* outputFd;
        if (!propertyValueToConsoleFd (output, outputFd, errorMessage))
        {
            errorMessage.assign ("failed to parser value for output");
            return false;
        }

        bool color;
        props.get ("lh.console.color", false, color, valid);

        if (!valid)
        {
            errorMessage.assign ("failed to parse color enabled for console");
            return false;
        }

        consoleLogHandler* handler = new consoleLogHandler ();
        handler->setLevel (logLevel);
        handler->setFormat (format);
        handler->setOutput (outputFd);
        handler->setColorEnabled (color);
        handlers.insert (handler);
    }

    return true;
}

bool
logHandlerFactory::configureFileHandler (properties& props,
                                         set<logHandler*>& handlers,
                                         string& errorMessage)
{
    bool enabled;
    if (!getHandlerEnabled (props, "file", false, enabled))
    {
        // failed to parse bool from a user configured value
        errorMessage.assign ("failed parsing property: enabled for file");
        return false;
    }

    string format;
    string sizeLimit;
    string fileCount;

    props.get ("lh.file.format", DEFAULT_LOG_FORMAT, format);
    props.get ("lh.file.size", DEFAULT_LOG_SIZE, sizeLimit);
    props.get ("lh.file.count", DEFAULT_FILE_COUNT, fileCount);

    if (enabled)
    {
        logSeverity::level logLevel;
        if (!getHandlerLevel (props, "file", DEFAULT_LOG_LEVEL, logLevel))
        {
            errorMessage.assign ("failed to parse value for file.level");
            return false;
        }

        string path;
        if (!props.get ("lh.file.path", path))
        {
            errorMessage.assign ("missing parameter file.path");
            return false;
        }

        int size = 0;
        if (!utils_parseNumber (sizeLimit, size))
        {
            errorMessage.assign ("failed to parse value for file.size");
            return false;
        }

        int fileCountLimit = 0;
        if (!utils_parseNumber (fileCount, fileCountLimit))
        {
            errorMessage.assign ("failed to parse value for file.count");
            return false;
        }

        fileLogHandler* handler = new fileLogHandler (path, size, fileCountLimit);
        handler->setLevel (logLevel);
        handler->setFormat (format);
        handlers.insert (handler);
    }

    return true;
}

bool
logHandlerFactory::configureSyslogHandler (properties& props,
                                           set<logHandler*>& handlers,
                                           string& errorMessage)
{
    bool enabled;
    if (!getHandlerEnabled (props, "syslog", false, enabled))
    {
        // failed to parse bool from a user configured value
        errorMessage.assign ("failed parsing property: enabled for syslog");
        return false;
    }
    
    string format;
    props.get ("lh.syslog.format", DEFAULT_LOG_FORMAT, format);

    if (enabled)
    {
        logSeverity::level logLevel;
        if (!getHandlerLevel (props, "syslog", DEFAULT_LOG_LEVEL, logLevel))
        {
            errorMessage.assign ("failed to parse value for file.level");
            return false;
        }

        syslogLogHandler* handler = new syslogLogHandler ();
        handler->setLevel (logLevel);
        handler->setFormat (format);
        handlers.insert (handler);
    }

    return true;
}

bool
logHandlerFactory::propertyValueToConsoleFd (const string& value,
                                             FILE*& fdValue,
                                             string& errorMessage)
{
    if (value == "stdout")
    {
        fdValue = stdout;
        return true;
    }
    else if (value == "stderr")
    {
        fdValue = stderr;
        return true;
    }

    errorMessage.assign ("unable to parse fd from: " + value);
    return false;
}

bool
logHandlerFactory::propertyValueToSeverity (const string& value,
                                            logSeverity::level& severityValue,
                                            string& errorMessage)
{
    if (value == "info")
    {
        severityValue = logSeverity::INFO;
        return true;
    }
    else if (value == "warn")
    {
        severityValue = logSeverity::WARN;
        return true;
    }
    else if (value == "err")
    {
        severityValue = logSeverity::ERROR;
        return true;
    }
    else if (value == "debug")
    {
        severityValue = logSeverity::DEBUG;
        return true;
    }
    else if (value == "fatal")
    {
        severityValue = logSeverity::FATAL;
        return true;
    }

    errorMessage.assign ("unable to parse severity from: " + value);
    return false;
}

bool
logHandlerFactory::getHandlerEnabled (const properties& props,
                                      const string& handler,
                                      bool defaultVal,
                                      bool& enabled)
{
    bool valid = true;

    stringstream propStr;
    propStr << "lh." << handler << ".enabled";

    props.get (propStr.str (), defaultVal, enabled, valid);

    return valid;
}

bool
logHandlerFactory::getHandlerLevel (const properties& props,
                                    const string& handler,
                                    string defaultVal,
                                    logSeverity::level& lvl)
{
    string err;
    string slvl;
    stringstream propStr;

    propStr << "lh." << handler << ".level";

    props.get (propStr.str (), defaultVal, slvl);

    return propertyValueToSeverity (slvl, lvl, err);
}

}

/*
 * Copyright 2014-2018 Neueda Ltd.
 */

#pragma once

#include "logSeverity.h"
#include "properties.h"
#include <string>
#include <set>
#include <sbfCommon.h>

#ifdef WIN32
#undef ERROR
#endif

using namespace std;

namespace neueda
{

class logHandler
{
public:
    logHandler ();

    virtual ~logHandler() { }
    
    virtual void setLevel (logSeverity::level severity) { mLevel = severity; }

    virtual logSeverity::level getLevel () const { return mLevel; }

    virtual const string& getFormat () const { return mFormat; }

    virtual void setFormat (string& format) { mFormat = format; }

    virtual bool setup () { return true; }

    virtual void teardown () { }

    virtual void handle (logSeverity::level severity,
                         const char* name,
                         const struct tm* tm_time,
                         const struct timeval *tv,
                         const char* message,
                         size_t message_len) { }

    virtual bool isLevelEnabled (logSeverity::level level) const;

    static string toString (const string& format,
                            logSeverity::level severity,
                            const char* name,
                            const struct tm* tm_time,
                            const struct timeval *tv,
                            const char* message,
                            size_t message_len);

    static string severityToString (const logSeverity::level severity);

    void setLastError (const string& err) { mError.assign (err); }

    string getLastError () { return mError; }

protected:
    logSeverity::level  mLevel;
    string              mFormat;
    string              mError;
};

class logHandlerFactory
{
public:
    static set<logHandler*> getHandlers (properties& props,
                                         bool& status,
                                         string& errorMessage);

private:

    static bool configureConsoleHandler (properties& properties,
                                         set<logHandler*>& handlers,
                                         string& errorMessage);

    static bool configureFileHandler (properties& properties,
                                      set<logHandler*>& handlers,
                                      string& errorMessage);
#ifndef WIN32
    static bool configureShmHandler (properties& properties,
                                     set<logHandler*>& handlers,
                                     string& errorMessage);

    static bool configureSyslogHandler (properties& properties,
                                        set<logHandler*>& handlers,
                                        string& errorMessage);
#endif

    static bool propertyValueToConsoleFd (const string& value,
                                          FILE*& fdValue,
                                          string& errorMessage);

    static bool propertyValueToSeverity (const string& value,
                                         logSeverity::level& boolValue,
                                         string& errorMessage);

    static bool getHandlerEnabled (const properties& props,
                                   const string& handler,
                                   bool defaultVal,
                                   bool& enabled);

    static bool getHandlerLevel (const properties& props,
                                 const string& handler,
                                 string defaultVal,
                                 logSeverity::level& lvl);
};

};

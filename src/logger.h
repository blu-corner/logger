/*
 * Copyright 2014-2018 Neueda Ltd.
 */

#pragma once

#include "logSeverity.h"
#include "logHandler.h"
#include <properties.h>
#include <sbfCommon.h>
#include <sbfMw.h>
#include <sbfQueue.h>

#include <cstdarg>
#include <string>
#include <set>
#include <map>
#include <sstream>

#ifdef __GNUC__
# define PRINTF_LIKE(_x, _y) __attribute__ ((format(printf,_x,_y)))
#else
# define PRINTF_LIKE(_x, _y)
#endif

namespace neueda
{
static const size_t defaultLogMessageChunkSize = 2048;

class logService;

class logger
{
    friend class logService;

public:
    void err (const char* fmt, ...) PRINTF_LIKE(2,3);
    void warn (const char* fmt, ...) PRINTF_LIKE(2,3);
    void info (const char* fmt, ...) PRINTF_LIKE(2,3);
    void debug (const char* fmt, ...) PRINTF_LIKE(2,3);
    void trace (const char* fmt, ...) PRINTF_LIKE(2,3);
    void fatal (const char* fmt, ...) PRINTF_LIKE(2,3);
    void log (logSeverity::level, const char* fmt, ...) PRINTF_LIKE(3,4);

    void setLevel (logSeverity::level lvl);
    logSeverity::level getLevel () const;

    static void endl (logger& l);

    template<class T>
    logger& operator<< (const T& t)
    {
        mStream << t;
        return *this;
    }

    void operator<< (void (*f) (logger&))
    {
        f (*this);
    }

    logger& err ();
    logger& warn ();
    logger& info ();
    logger& trace ();
    logger& debug ();

    const std::string& getName () const { return mName; }

private:
    logger (logger const&);
    logger (const std::string name, logService* const service, logSeverity::level lvl);
    ~logger ();

    void operator= (logger const &);

    void vlog (logSeverity::level lvl, const char* fmt, va_list ap);
    logger& log (logSeverity::level lvl);

    bool isLevelEnabled (logSeverity::level lvl) const;

    const std::string   mName;
    logService* const   mService;
    std::ostringstream  mStream;
    sbfMutex            mStreamMutex;
    logSeverity::level  mStreamLevel;
    logSeverity::level  mLevel;
};

class logService
{
    friend class logger;

public:
    ~logService ();

    static logService& get();

    static logger* getLogger (const std::string& name);

    void setLevel (logSeverity::level lvl) { mLevel = lvl; }

    bool addHandler (logHandler* handler,
                     std::string& errorMessage)
    {
        return addHandler (handler, errorMessage, true);
    }

    bool addHandler (logHandler* handler,
                     std::string& errorMessage,
                     bool owned);

    void removeHandler (logHandler* handler);

    // configure to config specification
    bool configure (properties& props, std::string& errorMessage);

    void clearHandlers ();

    void handle (const std::string& logger,
                 logSeverity::level severity,
                 uint64_t time,
                 const char* message,
                 size_t messageLen);

private:
    logService ();
    logService (const logService& that);

    bool findLogger (std::string name, logger*& logger);

    bool init (std::string& errorMessage);

    void stopDispatching ();

    static int sbfLogCb (sbfLog log,
                         sbfLogLevel level,
                         const char* message,
                         void* closure);
    static void* dispatchCb (void* closure);
    static void asyncHandle (sbfQueueItem item, void* closure);

    sbfMutex                        mMutex;
    sbfLog                          mSbfLog;
    sbfMw                           mMw;
    sbfQueue                        mQueue;
    sbfThread                       mThread;
    bool                            mDispatching;
    bool                            mIsAsync;
    logSeverity::level              mLevel;
    std::map<std::string, logger*>  mloggers;
    std::set<logHandler*>           mHandlers;
    std::map<logHandler*, bool>     mHandlerOwnedTable;

    static logService*              mInstance;
};

};

#undef PRINTF_LIKE

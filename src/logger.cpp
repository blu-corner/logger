/*
 * Copyright 2014-2018 Neueda Ltd.
 */

#include <sharedMemoryLogHandler.h>
#include <logger.h>

#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <sys/time.h>

namespace neueda
{
struct logWorkItem
{
    logService*        mService;
    logSeverity::level mSeverity;
    char               mName[64];
    struct tm          mTime;
    struct timeval     mTv;
    char               mMessage[defaultLogMessageChunkSize];
    size_t             mMessageLen;
};


static const logSeverity::level defaultLoggerSeverity = logSeverity::INFO;
static const string defaultRootSBFLoogerName = "SBF";

logService::~logService ()
{
    sbfMutex_lock (&mMutex);

    std::map<std::string, logger*>::iterator lit;
    for (lit = mloggers.begin (); lit != mloggers.end (); ++lit)
    {
        logger* l = lit->second;
        delete l;
    }
    mloggers.clear ();

    sbfMutex_unlock (&mMutex);

    stopDispatching ();

    clearHandlers ();

    sbfMutex_destroy (&mMutex);
}

logService::logService ()
    : mMw (NULL),
      mQueue (NULL),
      mDispatching (false),
      mIsAsync (false),
      mLevel (defaultLoggerSeverity)
{
    sbfMutex_init (&mMutex, 1);
    mSbfLog = sbfLog_create (NULL, "sbf"); // can't fail
    sbfLog_setHook (mSbfLog, SBF_LOG_INFO, sbfLogCb, this);
    sbfLog_setLevel (mSbfLog, SBF_LOG_INFO);
}

logger*
logService::getLogger (const std::string& name)
{
    logService& service = logService::get ();
    logger* l = NULL;

    if (!service.findLogger (name, l))
    {
        l = new logger (name, &service, service.mLevel);
        service.mloggers.insert (std::pair<std::string, logger*> (name, l));
    }

    return l;
}

bool
logService::addHandler (logHandler* handler,
                        std::string& errorMessage,
                        bool owned)
{
    sbfMutex_lock (&mMutex);

    bool ok = handler->setup ();
    if (ok)
    {
        mHandlers.insert (handler);
        mHandlerOwnedTable.insert (std::pair<logHandler*, bool>(handler, owned));
    }
    else
        errorMessage.assign("failed to setup handler: " + handler->getLastError ());

    sbfMutex_unlock (&mMutex);

    return ok;
}

void
logService::removeHandler (logHandler* handler)
{
    sbfMutex_lock (&mMutex);

    handler->teardown ();

    std::map<logHandler*,bool>::iterator it = mHandlerOwnedTable.find(handler);
    bool isOwned = it->second;

    mHandlers.erase (handler);
    mHandlerOwnedTable.erase (it);

    if (isOwned)
        delete handler;

    sbfMutex_unlock (&mMutex);
}

bool
logService::init (std::string& errorMessage)
{
    sbfKeyValue kv = sbfKeyValue_create ();
    mMw = sbfMw_create (mSbfLog, kv);
    sbfKeyValue_destroy (kv);
    if (mMw == NULL)
    {
        errorMessage.assign("failed to create mw");
        return false;
    }
    
    mQueue = sbfQueue_create (mMw, "default");
    if (mQueue == NULL)
    {
        errorMessage.assign ("failed to create queue");
        return false;
    }
    
    // start to dispatch 
    if (sbfThread_create (&mThread, logService::dispatchCb, this) != 0)
    {
        errorMessage.assign ("failed to start dispatch queue");
        return false;
    }

    mDispatching = true;
    return true;
}

void
logService::stopDispatching ()
{
    if (mQueue)
        sbfQueue_destroy (mQueue);
    if (mDispatching)
        sbfThread_join (mThread);
    if (mMw)
        sbfMw_destroy (mMw);

    mDispatching = false;
}

bool
logService::configure (properties& props,
                       std::string& errorMessage)
{
    sbfMutex_lock (&mMutex);

    // clear the slate
    clearHandlers ();

    bool ok;
    std::set<logHandler*> handlers = logHandlerFactory::getHandlers (props,
                                                                     ok,
                                                                     errorMessage);
    if (!ok)
    {
        std::set<logHandler*>::iterator it;
        for (it = mHandlers.begin (); it != mHandlers.end (); ++it)
            delete *it;

        return false;
    }

    std::set<logHandler*>::iterator it;
    for (it = handlers.begin (); it != handlers.end (); ++it)
    {
        logHandler* handle = *it;

        if (!handle->setup ())
        {
            // delete all pending handlers and fall back to caller
            std::set<logHandler*>::iterator itt;
            for (itt = mHandlers.begin (); itt != mHandlers.end (); ++itt)
                delete *itt;

            mHandlerOwnedTable.clear ();
            return false;
        }

        mHandlerOwnedTable.insert (std::pair<logHandler*, bool>(handle, true));
    }

    // set the handlers
    mHandlers = handlers;

    // is aync mode
    string value;
    props.get ("logger.service.async", "false", value);

    if (!utils_parseBool (value, mIsAsync))
    {
        // failed to parse bool from a user configured value
        errorMessage.assign ("failed parsing property: enabled for async");
        return false;
    }

    if (mIsAsync && !mDispatching)
    {
        // failed to init service
        if (!init (errorMessage))
            return false;
    }
    else if (!mIsAsync && mDispatching)
    {
        // teardown async mode
        stopDispatching ();
    }

    sbfMutex_unlock (&mMutex);
    
    return true;
}

void
logService::clearHandlers ()
{
    sbfMutex_lock (&mMutex);

    std::set<logHandler*>::iterator it;
    for (it = mHandlers.begin (); it != mHandlers.end (); ++it)
    {
        logHandler* handler = *it;

        std::map<logHandler*,bool>::iterator it = mHandlerOwnedTable.find(handler);
        bool isOwned = it->second;
        
        handler->teardown ();
        if (isOwned)
            delete handler;
    }
    mHandlers.clear ();
    mHandlerOwnedTable.clear ();

    sbfMutex_unlock (&mMutex);
}

void
logService::handle (const std::string& logger,
                    logSeverity::level severity,
                    const struct tm* time,
                    const timeval* tv,
                    const char* message,
                    size_t messageLen)
{    
    logWorkItem* item = new logWorkItem();
    
    item->mService = this;
    item->mSeverity = severity;
    memcpy (&item->mTime, time, sizeof (struct ::tm));
    memcpy (&item->mTv, tv, sizeof (::timeval));

    size_t nameCopySize = (logger.size () > sizeof (item->mName)) ?
        sizeof (item->mName) :
        logger.size ();
    strncpy (item->mName, logger.c_str (), nameCopySize);

    size_t msgCopySize = (messageLen > sizeof (item->mMessage)) ?
        sizeof (item->mMessage) :
        messageLen;
    strncpy (item->mMessage, message, msgCopySize);
    item->mMessageLen = msgCopySize;

    if (mIsAsync && mQueue != NULL)
    {
        sbfQueue_enqueue (mQueue, logService::asyncHandle, item);
    }
    else
        logService::asyncHandle (NULL, item);
}

void*
logService::dispatchCb (void* closure)
{
    logService* self = reinterpret_cast<logService*>(closure);
    sbfQueue_dispatch (self->mQueue);
    return NULL;
}
    
void
logService::asyncHandle (sbfQueueItem item, void* closure)
{
    logWorkItem* workItem = static_cast<logWorkItem*>(closure);
    logService* self = workItem->mService;

    sbfMutex_lock (&self->mMutex);

    std::set<logHandler*>::iterator it;
    for (it = self->mHandlers.begin (); it != self->mHandlers.end (); ++it)
    {
        logHandler* const handle = *it;
        if (!handle->isLevelEnabled (workItem->mSeverity))
            continue;

        handle->handle (workItem->mSeverity,
                        workItem->mName,
                        &workItem->mTime,
                        &workItem->mTv,
                        workItem->mMessage,
                        workItem->mMessageLen);
    }

    delete workItem;
    
    sbfMutex_unlock (&self->mMutex);
}
    

bool
logService::findLogger (std::string name, logger*& l)
{
    std::map<std::string, logger*>::iterator foundIterator = mloggers.find (name);
    bool found = foundIterator != mloggers.end ();

    if (found) 
    {
        l = foundIterator->second;
        return true;
    }

    return false;
}

int
logService::sbfLogCb (sbfLog l, sbfLogLevel level, const char* message, void* closure)
{
    logService* self = reinterpret_cast<logService*>(closure);
    logger* log = self->getLogger (defaultRootSBFLoogerName);

    switch (level)
    {
    case SBF_LOG_DEBUG:
        log->debug ("%s", message);
    case SBF_LOG_INFO:
        log->info ("%s", message);
        break;
    case SBF_LOG_WARN:
        log->warn ("%s", message);
        break;
    case SBF_LOG_ERROR:
        log->err ("%s", message);
        break;
    default:
        break;
    }
    return 1; // don't let sbf log it as well
}

logger::logger (const std::string name,
                logService* const service,
                logSeverity::level level) : 
    mName (name),
    mService (service),
    mLevel (level)
{
    sbfMutex_init (&mStreamMutex, 0);
}

bool
logger::isLevelEnabled (logSeverity::level level) const
{
    return mLevel <= level;
}

void
logger::setLevel (logSeverity::level level)
{
    mLevel = level;
}

logSeverity::level
logger::getLevel () const
{
    return mLevel;
}

void
logger::err (const char* fmt, ...)
{
    va_list ap;

    va_start (ap, fmt);
    vlog (logSeverity::ERROR, fmt, ap);
    va_end (ap);
}

void
logger::warn (const char* fmt, ...)
{
    va_list ap;

    va_start (ap, fmt);
    vlog (logSeverity::WARN, fmt, ap);
    va_end (ap);
}

void
logger::info (const char* fmt, ...)
{
    va_list ap;

    va_start (ap, fmt);
    vlog (logSeverity::INFO, fmt, ap);
    va_end (ap);
}

void
logger::debug (const char* fmt, ...)
{
    va_list ap;

    va_start (ap, fmt);
    vlog (logSeverity::DEBUG, fmt, ap);
    va_end (ap);
}

void
logger::fatal (const char* fmt, ...)
{
    va_list ap;

    va_start (ap, fmt);
    vlog (logSeverity::FATAL, fmt, ap);
    va_end (ap);

    exit (-1);
}

void
logger::log (logSeverity::level level,
             const char* fmt, ...)
{
    va_list ap;

    va_start (ap, fmt);
    vlog (level, fmt, ap);
    va_end (ap);
}

void
logger::vlog (logSeverity::level level,
              const char* fmt,
              va_list ap)
{
    if (!isLevelEnabled (level))
        return;
  
    struct tm time;
    timeval tv;

    gettimeofday (&tv, NULL);
    time_t t = tv.tv_sec;
    gmtime_r (&t, &time);

    va_list cp;
    va_copy (cp, ap);

    size_t bufsize = vsnprintf (NULL, 0, fmt, cp) + 1;
    char* s        = new char[bufsize];
    size_t length  = vsnprintf (s, bufsize, fmt, ap);
    size_t offset  = 0;

    if (length <= defaultLogMessageChunkSize)
    {
        mService->handle (getName (),
                          level,
                          &time,
                          &tv,
                          s,
                          length);
    }
    else
    {
        do
        {
            size_t chunkSize = std::min (length - offset,
                                         defaultLogMessageChunkSize);
            mService->handle (getName (),
                              level,
                              &time,
                              &tv,
                              s+offset,
                              chunkSize);
            offset += chunkSize;
        } while (offset < length);
    }

    delete [] s;
}

void
logger::endl (logger& l)
{
    sbfMutex_lock (&l.mStreamMutex);
    l.log (l.mStreamLevel, "%s", l.mStream.str ().c_str ());

    l.mStream.str ("");
    l.mStream.clear ();

    sbfMutex_unlock (&l.mStreamMutex);
}

logger&
logger::err ()
{
    return log (logSeverity::ERROR);
}

logger&
logger::warn ()
{
    return log (logSeverity::WARN);
}

logger&
logger::info ()
{
    return log (logSeverity::INFO);
}

logger&
logger::debug ()
{
    return log (logSeverity::DEBUG);
}

logger&
logger::log (logSeverity::level severity)
{
    mStreamLevel = severity;
    return *this;
}

logger::~logger()
{
    sbfMutex_destroy (&mStreamMutex);
}

}

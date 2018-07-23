/*
 * Copyright 2014-2018 Neueda Ltd.
 */

#include "loggerDaemonServer.h"

#include <sys/socket.h>
#include <sys/un.h>

#include <cstring>
#include <cstdlib>
#include <csignal>
#include <cstdio>

#define LOOP_INTERUPT_SECONDS 3

namespace neueda
{

loggerStream::loggerStream (struct bufferevent* event,
                            ITransportDelegate* delegate,
                            loggerDaemonServer* parent) : 
    mEvent (event),
    mDelegate (delegate),
    mParent (parent)
{
    mDelegate->onConnect (this);
}

loggerStream::~loggerStream ()
{
    bufferevent_free (mEvent);
}

void
loggerStream::close ()
{
    mParent->streamDidClose (this);
}

void
loggerStream::send (const uint8_t *buffer, size_t length)
{
    struct evbuffer* output = bufferevent_get_output (mEvent);
    evbuffer_add (output, buffer, length);
}

void
loggerStream::handleRead (const uint8_t* buf, size_t len)
{
    vector<uint8_t> readBuf;
    if (mBuffer.size () > 0)
    {
        readBuf.resize (mBuffer.size ());
        copy (mBuffer.begin (),
              mBuffer.end (),
              readBuf.begin());
    }

    size_t offset = readBuf.size ();
    readBuf.resize (offset + len);
    copy (buf, buf + len, readBuf.begin () + offset);

    size_t totalSize = readBuf.size ();
    size_t read = mDelegate->onData (this,
                                     &(readBuf[0]),
                                     totalSize);

    mBuffer.clear ();
    mBuffer.resize (0);
    if (read < totalSize)
    {
        size_t trailingSize = totalSize - read;
        mBuffer.resize (trailingSize);
        copy (readBuf.begin () + read,
              readBuf.end (),
              mBuffer.begin ());
    }
}

void
loggerStream::readCb (struct bufferevent* bev, void* ctx)
{
    loggerStream* instance = static_cast<loggerStream*>(ctx);
    struct evbuffer* input = bufferevent_get_input (bev);

    size_t len = evbuffer_get_length (input);
    uint8_t* data = new uint8_t[len];
    evbuffer_copyout (input, data, len);

    instance->handleRead (data, len);

    delete [] data;
}

void
loggerStream::eventCb (struct bufferevent* bev, short events, void* ctx)
{
    loggerStream* instance = static_cast<loggerStream*>(ctx);
    if (events & BEV_EVENT_EOF)
    {
        instance->mDelegate->onDisconnect (instance);
        instance->close ();
    }
    else if (events & BEV_EVENT_ERROR)
    {
        instance->mDelegate->onError (instance);
        instance->close ();
    }
}

loggerDaemonServer::loggerDaemonServer (const string& path,
                                        ITransportDelegate* delegate) : 
    mPath (path),
    mDelegate (delegate)
{
    mLogger = logService::getLogger ("LOG_DAEMON_SERVER");
}

loggerDaemonServer::~loggerDaemonServer ()
{
}

void
loggerDaemonServer::attachSignalHandler (loggerSignalHandler handler)
{
    mSignalHandler = handler;
}

void
loggerDaemonServer::dispatch ()
{
    event_set_log_callback (&eventloggerCallback);
    mBase = event_base_new ();
    if (!mBase)
    {
        mLogger->err ("failed to create event base");
        return;
    }

    struct sockaddr_un sin;
    memset (&sin, 0, sizeof (sin));
    sin.sun_family = AF_LOCAL;
    strcpy (sin.sun_path, mPath.c_str ());

    /* Create a new listener */
    mListener = evconnlistener_new_bind (mBase,
                                         &acceptConnCb,
                                         this,
                                         LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE,
                                         -1,
                                         (struct sockaddr *) &sin,
                                         sizeof(sin));
    if (!mListener)
    {
        mLogger->err ("failed to create listener");
        return;
    }

    evconnlistener_set_error_cb (mListener, &acceptErrorCb);

    // signal handler
    mSignal = evsignal_new (mBase, SIGINT, signalCb, this);
    event_add (mSignal, NULL);

    // timer
    mTimerValue.tv_sec = LOOP_INTERUPT_SECONDS;
    mTimerValue.tv_usec = 0;
    
    mTimerEvent = evtimer_new (mBase, &timerCallback, this);
    evtimer_add (mTimerEvent, &mTimerValue);

    event_base_dispatch (mBase);

    // cleanup
    event_free (mSignal);
    event_free (mTimerEvent);
    evconnlistener_free (mListener);
    event_base_free (mBase);

    // remove sock path
    remove (mPath.c_str ());

    // free streams
    set<loggerStream*>::iterator it;
    for (it = mStreams.begin (); it != mStreams.end (); ++it)
    {
        loggerStream* s = *it;
        delete s;
    }
    mStreams.clear ();
    mCloseStreams.clear ();
}

void
loggerDaemonServer::stop ()
{
    event_base_loopexit (mBase, NULL);
}

void
loggerDaemonServer::streamDidClose (loggerStream* stream)
{
    mCloseStreams.insert (stream);
}

void
loggerDaemonServer::acceptConnCb (struct evconnlistener* listener,
                                  evutil_socket_t fd,
                                  struct sockaddr* address,
                                  int socklen,
                                  void* ctx)
{
    loggerDaemonServer* instance = static_cast<loggerDaemonServer*>(ctx);

    struct event_base* base = evconnlistener_get_base (listener);
    struct bufferevent* bev = bufferevent_socket_new (base, fd, BEV_OPT_CLOSE_ON_FREE);

    loggerStream* st = new loggerStream (bev, instance->mDelegate, instance);
    bufferevent_setcb (bev, loggerStream::readCb, NULL, loggerStream::eventCb, st);
    bufferevent_enable (bev, EV_READ | EV_WRITE);

    instance->mStreams.insert (st);
}

void
loggerDaemonServer::acceptErrorCb (struct evconnlistener* listener,
                                   void* ctx)
{
    // TODO
}

void
loggerDaemonServer::signalCb (evutil_socket_t fd, short event, void* arg)
{
    loggerDaemonServer* instance = static_cast<loggerDaemonServer*> (arg);

    struct event* signal = static_cast<struct event*>(arg);
    int signalValue = event_get_signal (signal);

    instance->mSignalHandler (instance, signalValue);
}

void
loggerDaemonServer::timerCallback (int fd, short event, void *arg)
{
    loggerDaemonServer* instance = static_cast<loggerDaemonServer*>(arg);

    set<loggerStream*>::iterator it;
    for (it = instance->mCloseStreams.begin ();
         it != instance->mCloseStreams.end ();
         ++it)
    {
        loggerStream* s = *it;
        instance->mStreams.erase (s);
        delete s;
    }
    instance->mCloseStreams.clear ();
}

void
loggerDaemonServer::eventloggerCallback (int severity, const char* msg)
{
    logger* logger = logService::getLogger ("LIBEVENT");
    switch (severity)
    {
    case _EVENT_LOG_DEBUG:
        logger->debug ("%s", msg);
        break;
    case _EVENT_LOG_MSG:
        logger->info ("%s", msg);
        break;
    case _EVENT_LOG_WARN:
        logger->warn ("%s", msg);
        break;
    case _EVENT_LOG_ERR:
        logger->err ("%s", msg);
        break;
    default:
        // ignore
        break;
    }
}

}

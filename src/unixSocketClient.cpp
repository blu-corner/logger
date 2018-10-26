/*
 * Copyright 2014-2018 Neueda Ltd.
 */

#include "unixSocketClient.h"

#include <sys/socket.h>
#include <sys/un.h>

#include <cstring>
#include <cstdlib>
#include <csignal>
#include <cstdio>

static const int kDefaultLoopInteruptSeconds = 1;

namespace neueda
{

unixClient::unixClient (const std::string& sockPath,
                        unixClientDelegate* delegate)
    : mSockPath (sockPath),
      mDelegate (delegate),
      mShouldStop (false),
      mBase(NULL),
      mTimerEvent(NULL),
      mEvent(NULL)
{
    mlogger = logService::getLogger ("UNIX_CLIENT");
}

unixClient::~unixClient ()
{
    
}

void
unixClient::stop ()
{
    mShouldStop = true;
}

void
unixClient::dispatch ()
{
    event_set_log_callback (&eventloggerCallback);
    mBase = event_base_new ();
    if (!mBase) {
        mlogger->err ("failed to create event base");
        return;
    }

    struct sockaddr_un sin;
    memset (&sin, 0, sizeof (sin));
    sin.sun_family = AF_LOCAL;
    strcpy (sin.sun_path, mSockPath.c_str ());

    mEvent = bufferevent_socket_new (mBase, -1, BEV_OPT_CLOSE_ON_FREE);
    bufferevent_setcb (mEvent, readCb, NULL, eventCb, this);
    bufferevent_enable (mEvent, EV_READ | EV_WRITE);

    int code = bufferevent_socket_connect (mEvent,
                                           (struct sockaddr *)&sin,
                                           sizeof(sin));
    bool failedToConnect = code < 0;
    if (failedToConnect)
        mlogger->err ("failed to connect to [%s]", mSockPath.c_str ());

    // timer
    mTimerValue.tv_sec = kDefaultLoopInteruptSeconds;
    mTimerValue.tv_usec = 0;
    
    mTimerEvent = evtimer_new (mBase, &timerCallback, this);
    evtimer_add (mTimerEvent, &mTimerValue);

    if (!failedToConnect)
    {
        mDelegate->onConnect (this);
        event_base_dispatch (mBase);
    }

    event_free (mTimerEvent);
    bufferevent_free (mEvent);
    event_base_free (mBase);
}

void
unixClient::sendBuffer (const unsigned char* buffer, size_t length)
{
    struct evbuffer* output = bufferevent_get_output (mEvent);
    evbuffer_add (output, buffer, length);
}

void
unixClient::readCb (struct bufferevent* bev, void* ctx)
{
    unixClient* instance = static_cast<unixClient*>(ctx);
    struct evbuffer* input = bufferevent_get_input (bev);

    size_t len = evbuffer_get_length (input);
    unsigned char* data = new unsigned char[len];
    evbuffer_copyout (input, data, len);

    instance->handleRead (data, len);

    delete[] data;
}

void
unixClient::eventCb (struct bufferevent* bev, short events, void* ctx)
{
    unixClient* instance = static_cast<unixClient*>(ctx);
    if (events & BEV_EVENT_EOF)
    {
        instance->mDelegate->onDisconnect (instance);
        instance->stop ();
    }
    else if (events & BEV_EVENT_ERROR)
    {
        instance->mDelegate->onError (instance);
        instance->stop ();
    }
}

void
unixClient::handleRead (const unsigned char* buf, size_t len)
{
    std::vector<unsigned char> readBuf;
    if (mBuffer.size () > 0)
    {
        readBuf.resize (mBuffer.size ());
        std::copy (mBuffer.begin (),
                   mBuffer.end (),
                   readBuf.begin());
    }

    size_t offset = readBuf.size ();
    readBuf.resize (offset + len);
    std::copy (buf, buf + len, readBuf.begin () + offset);

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
        std::copy (readBuf.begin () + read,
                   readBuf.end (),
                   mBuffer.begin ());
    }
}

void
unixClient::eventloggerCallback (int severity, const char* msg)
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

void
unixClient::timerCallback (int fd, short event, void *arg)
{
    unixClient* instance = static_cast<unixClient*>(arg);
    if (instance->mShouldStop)
    {
        event_base_loopbreak (instance->mBase);
    }
    else if (!evtimer_pending (instance->mTimerEvent, NULL))
    {
        evtimer_del (instance->mTimerEvent);
        evtimer_add (instance->mTimerEvent, &instance->mTimerValue);
    }
}

}

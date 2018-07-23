/*
 * Copyright 2014-2018 Neueda Ltd.
 */

#pragma once

#include "logger.h"

#include "ITransportDelegate.h"

#include <event2/bufferevent.h>
#include <event2/listener.h>
#include <event2/buffer.h>

#include <string>
#include <vector>
#include <set>

#include <sys/time.h>

using namespace std;

namespace neueda
{

class loggerDaemonServer;

class loggerStream
{
    friend class loggerDaemonServer;

public:
    void close ();

    void send (const uint8_t* buffer, size_t length);

private:
    loggerStream (struct bufferevent* event,
                  ITransportDelegate* delegate,
                  loggerDaemonServer* parent);

    ~loggerStream ();

    void handleRead (const uint8_t* buf, size_t len);

    static void readCb (struct bufferevent* bev, void* ctx);

    static void eventCb (struct bufferevent* bev, short events, void* ctx);

    struct bufferevent*         mEvent;
    ITransportDelegate*         mDelegate;
    loggerDaemonServer*         mParent;
    vector<uint8_t>             mBuffer;
};


class loggerDaemonServer;
typedef void (*loggerSignalHandler)(loggerDaemonServer* server, int signal);

class loggerDaemonServer
{
    friend class loggerStream;

public:
    loggerDaemonServer (const string& path,
                        ITransportDelegate* delegate);

    ~loggerDaemonServer ();

    void attachSignalHandler (loggerSignalHandler handler);

    void dispatch ();

    void stop ();

private:
    void streamDidClose (loggerStream* stream);

    static void acceptConnCb (struct evconnlistener* listener,
                              evutil_socket_t fd,
                              struct sockaddr* address,
                              int socklen,
                              void* ctx);

    static void acceptErrorCb (struct evconnlistener* listener,
                               void* ctx);

    static void signalCb (evutil_socket_t fd, short event, void *arg);

    static void timerCallback (int fd, short event, void *arg);

    static void eventloggerCallback (int severity, const char* msg);

    string                      mPath;
    ITransportDelegate*         mDelegate;
    logger*                     mLogger;

    struct event_base*          mBase;
    struct evconnlistener*      mListener;
    struct ::timeval            mTimerValue;
    struct event*               mTimerEvent;
    struct event*               mSignal;
    loggerSignalHandler         mSignalHandler;

    set<loggerStream*>          mStreams;
    set<loggerStream*>          mCloseStreams;
};

};

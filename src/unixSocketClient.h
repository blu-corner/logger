/*
 * Copyright 2014-2018 Neueda Ltd.
 */

#pragma once

#include "logger.h"

#include <event2/bufferevent.h>
#include <event2/listener.h>
#include <event2/buffer.h>

#include <string>
#include <vector>


namespace neueda
{

class unixClient;
class unixClientDelegate
{
public:
    virtual void onConnect (unixClient* client) = 0;
    
    virtual void onDisconnect (unixClient* client) = 0;
    
    virtual size_t onData (unixClient* client,
                           const void* buf,
                           size_t len) = 0;
    
    virtual void onError (unixClient* client) = 0;
};

class unixClient
{
public:
    unixClient (const std::string& sockPath,
                unixClientDelegate* delegate);

    ~unixClient ();

    void stop ();

    void dispatch ();

    void sendBuffer (const unsigned char* buffer, size_t length);
    
private:
    void handleRead (const unsigned char* buf, size_t len);
    
    static void readCb (struct bufferevent* bev, void* ctx);

    static void eventCb (struct bufferevent* bev, short events, void* ctx);

    static void timerCallback (int fd, short event, void *arg);

    static void eventloggerCallback (int severity, const char* msg);
    
    const std::string&          mSockPath;
    unixClientDelegate*         mDelegate;
    bool                        mShouldStop;
    logger*                     mlogger;
    std::vector<unsigned char>  mBuffer;

    struct event_base*          mBase;
    struct timeval              mTimerValue;
    struct event*               mTimerEvent;
    struct bufferevent*         mEvent;
};

};

#pragma once

#include "sharedMemoryRingBuffer.h"
#include "loggerDaemonServer.h"
#include "logger.h"
#include "sbfCommon.h"

#include "properties.h"

#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <string>

#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>

using namespace std;

namespace neueda
{

class DaemonConfiguration
{
public:
    DaemonConfiguration (properties& props) :
        mProps (props, "logger", "daemon", "shm")
    {
    }

    string getSockPath () const
    {
        string defaultVal = "/tmp/logger.sock";
        string value;
        mProps.get ("sock", defaultVal, value);
        return value;
    }

    key_t getSharedMemoryKey () const
    {
        string defaultVal = "1234";
        string value;
        mProps.get ("key", defaultVal, value);
        key_t key = (key_t)atoi (value.c_str ());
        return key;
    }

private:
    properties mProps;
};


class MessageHandler : public ITransportDelegate
{
public:
    MessageHandler (DaemonConfiguration& config)
        : mConfig (config)
    { }
    
    virtual void onConnect (loggerStream* const stream)
    {
        key_t key = mConfig.getSharedMemoryKey ();
        stream->send ((unsigned char*)&key, sizeof (key));
    }

    virtual void onDisconnect (loggerStream* const stream)
    {
    }

    virtual void onError (loggerStream* const stream)
    {
    }

    virtual size_t onData (loggerStream* const stream, const void* buf, size_t len)
    {
        // ignore bytes
        return len;
    }

private:
    DaemonConfiguration& mConfig;
};

}

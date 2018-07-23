/*
 * Copyright 2014-2018 Neueda Ltd.
 */

#pragma once

#include <cstddef>


namespace neueda
{

class loggerStream;

class ITransportDelegate
{
public:
    virtual void onConnect (loggerStream* stream) = 0;
    
    virtual void onDisconnect (loggerStream* stream) = 0;
    
    virtual void onError (loggerStream* stream) = 0;
    
    virtual size_t onData (loggerStream* stream, const void* buf, size_t len) = 0;
};

};

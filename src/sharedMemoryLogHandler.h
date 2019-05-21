/*
 * Copyright 2014-2018 Neueda Ltd.
 */

#pragma once

#include "sharedMemoryRingBuffer.h"
#include "unixSocketClient.h"
#include "logHandler.h"
#include "sbfCommon.h"

using namespace std;

namespace neueda
{   

class sharedMemoryLogHandler : public logHandler,
                               public unixClientDelegate
{
public:
    sharedMemoryLogHandler (const string& sockPath);

    ~sharedMemoryLogHandler ();

    bool setup ();

    void teardown ();

    void handle (logSeverity::level severity,
                 const char* name,
		 uint64_t time,
                 const char* message,
                 size_t message_len);

    // delegate
    virtual void onConnect (unixClient* client);

    virtual void onDisconnect (unixClient* client);

    virtual size_t onData (unixClient* client, const void* buf, size_t len);

    virtual void onError (unixClient* client);

    bool isReady () const;

private:
    void openHandle (key_t key);

    void closeHandle ();

    static void* dispatchClient (void* closure);

    const string            mSockPath;
    sharedMemoryRingBuffer* mBuffer;
    unixClient              mClient;
    sbfThread               mClientThread;
    sbfMutex                mMutex;
    sbfCondVar              mReadyCond;
    key_t                   mKey;
    bool                    mFailed;
    string                  mLastError;
};

};

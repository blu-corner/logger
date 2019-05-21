/*
 * Copyright 2014-2018 Neueda Ltd.
 */

#include "sharedMemoryLogHandler.h"

#include <iostream>
#include <cstring>
#include <unistd.h>

static const struct timespec kDefaultTimeout = { 3, 0};

namespace neueda
{

sharedMemoryLogHandler::sharedMemoryLogHandler (const string& sockPath) :
    mSockPath (sockPath),
    mBuffer (NULL),
    mClient (mSockPath, this),
    mKey (-1),
    mFailed (false),
    mLastError ()
{
    sbfMutex_init (&mMutex, 0);
    sbfCondVar_init (&mReadyCond);
}

sharedMemoryLogHandler::~sharedMemoryLogHandler ()
{
}

bool
sharedMemoryLogHandler::setup ()
{
    sbfThread_create (&mClientThread, &dispatchClient, (void*)&mClient);

    sbfMutex_lock (&mMutex);
    while (!isReady())
    {
        pthread_cond_timedwait (&mReadyCond, &mMutex, &kDefaultTimeout);
        sbfMutex_unlock (&mMutex);
    }

    if (!isReady ())
        setLastError ("shared memory log handler failed to become ready");

    return isReady ();
}

void
sharedMemoryLogHandler::teardown ()
{
    mClient.stop ();
    sbfThread_join (mClientThread);
}

void
sharedMemoryLogHandler::handle (logSeverity::level severity,
                                const char* name,
                                uint64_t time,
                                const char* message,
                                size_t message_len)
{
    sbfMutex_lock (&mMutex);

    if (isReady ())
    {
        struct logEntry entry;
        memset (&entry, 0, sizeof entry);
	
        entry.severity = severity;
	entry.mTime = time;

        size_t nameCopySize = (strlen(name) > sizeof (entry.mName))
            ? sizeof (entry.mName)
            : strlen(name);
        strncpy (entry.mName, name, nameCopySize);

        size_t msgCopySize = (message_len > sizeof (entry.mMessage))
            ? sizeof (entry.mMessage)
            : message_len;
        strncpy (entry.mMessage, message, msgCopySize);

        mBuffer->blockingEnqueue (&entry);
    }

    sbfMutex_unlock (&mMutex);
}

void
sharedMemoryLogHandler::onConnect (unixClient* client)
{
}

void
sharedMemoryLogHandler::onDisconnect (unixClient* client)
{
    closeHandle ();
}

size_t
sharedMemoryLogHandler::onData (unixClient* client,
                                const void* buf,
                                size_t len)
{
    // dont care about endianess on unix-socket
    key_t key;
    if (len >= sizeof (key_t))
    {
        memcpy (&key, buf, sizeof (key_t));
        openHandle (key);
    }
    return len;
}

void
sharedMemoryLogHandler::onError (unixClient* client)
{
    closeHandle ();
}

bool
sharedMemoryLogHandler::isReady () const
{
    return mBuffer != NULL;
}

void
sharedMemoryLogHandler::openHandle (key_t key)
{
    sbfMutex_lock (&mMutex);

    if (key != mKey)
    {
        mKey = key;
        if (mBuffer != NULL)
            delete mBuffer;

        string error;
        mBuffer = sharedMemoryRingBuffer::attach (mKey, error);
        if (mBuffer == NULL)
        {
            mFailed = true;
            mLastError = error;
            goto openHandleDone;
        }

        // signal
        sbfCondVar_signal (&mReadyCond);
    }

    mFailed = false;
    mLastError.clear ();

openHandleDone:
    sbfMutex_unlock (&mMutex);
}

void
sharedMemoryLogHandler::closeHandle ()
{
    sbfMutex_lock (&mMutex);
    delete mBuffer;
    mBuffer = NULL;
    sbfMutex_unlock (&mMutex);
}

void*
sharedMemoryLogHandler::dispatchClient (void* closure)
{
    unixClient* client = static_cast<unixClient*>(closure);
    client->dispatch ();
    return NULL;
}

}

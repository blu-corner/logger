/*
 * Copyright 2014-2018 Neueda Ltd.
 */

#pragma once

#include "logSeverity.h"
#include "sbfCommon.h"

#include <string>
#include <cstring>
#include <semaphore.h>

using namespace std;

namespace neueda
{

struct logEntry
{
    logSeverity::level      severity;
    char                    mName[64];
    struct tm               mTime;
    struct timeval          mTv;
    char                    mMessage[512];
};

struct shmLogEntryHeader
{
    sbfMutex            mMutex;
    sbfCondVar          mCond;
    sem_t               mSema;
    size_t              mHead;
    size_t              mTail;
    size_t              mSize;
    struct logEntry*    mEntries;
};

class sharedMemoryRingBuffer
{
public:
    static sharedMemoryRingBuffer* create (key_t key, string& error);

    static sharedMemoryRingBuffer* attach (key_t key, string& error);

    void blockingEnqueue (const struct logEntry* entry);

    bool blockingDequeue (struct logEntry* entry);

    ~sharedMemoryRingBuffer ();

    void signal ();

private:
    sharedMemoryRingBuffer (int shmid,
                            uint8_t* handle,
                            bool mOwned,
                            key_t key,
                            size_t elementSize,
                            size_t length,
                            size_t offset);

    void detach ();

    void clearSharedMemory ();

    void insertElement (const struct logEntry* entry);

    bool takeElement (struct logEntry* entry);

    shmLogEntryHeader* getHeader ();

    uint8_t* getEntriesTable ();

    size_t getSize ();

    int             mShmid;
    uint8_t*        mHandle;
    bool            mOwned;
    key_t           mKey;
    size_t          mElementSize;
    size_t          mLength;
    size_t          mOffset;
    bool            mDidSignal;
};

};

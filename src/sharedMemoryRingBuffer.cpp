/*
 * Copyright 2014-2018 Neueda Ltd.
 */

#include "sharedMemoryRingBuffer.h"

#include <cerrno>
#include <cstdlib>
#include <stddef.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/shm.h>

static const unsigned int kDefaultMaxRingBufferLength = 512;

namespace neueda
{

sharedMemoryRingBuffer::sharedMemoryRingBuffer (int shmid,
                                                uint8_t* handle,
                                                bool owned,
                                                key_t key,
                                                size_t elementSize,
                                                size_t length,
                                                size_t offset) :
    mShmid (shmid),
    mHandle (handle),
    mOwned (owned),
    mKey (key),
    mElementSize (elementSize),
    mLength (length),
    mOffset (offset),
    mDidSignal (false)
{
}

sharedMemoryRingBuffer::~sharedMemoryRingBuffer ()
{
    detach ();
    if (mOwned)
        clearSharedMemory ();
}


sharedMemoryRingBuffer*
sharedMemoryRingBuffer::create (key_t key, string& error)
{
    int shmFlags = 0644 | IPC_CREAT | IPC_EXCL;
    size_t size = sizeof (struct shmLogEntryHeader)
        + (kDefaultMaxRingBufferLength * sizeof (struct logEntry));

    int shmid = shmget (key, size, shmFlags);
    if (shmid == -1)
    {
        error.assign ("failed to get shared memory id from key");
        return NULL;
    }

    uint8_t* handle = (uint8_t*)shmat (shmid, NULL, 0);
    if (handle == (void *)-1)
    {
        string e (strerror (errno));
        error.assign ("failed to attach to region: " + e);
        return NULL;
    }

    struct shmLogEntryHeader* handleEntry = (struct shmLogEntryHeader*)handle;

    // init the mutex
    sbfMutexAttr attr;
    sbfMutexAttr_init (&attr);
    sbfMutexAttr_setpshared (&attr, PTHREAD_PROCESS_SHARED);
    sbfMutex_init_attr (&(handleEntry->mMutex), &attr);

    // init cond
    sbfCondAttr condAttr;
    sbfCondAttr_init (&condAttr);
    sbfCondAttr_setpshared (&condAttr, PTHREAD_PROCESS_SHARED);
    sbfCondVar_init_attr (&(handleEntry->mCond), &condAttr);

    // init sema
    int pshared = 1;
    bool ok = sem_init (&(handleEntry->mSema),
                        pshared,
                        kDefaultMaxRingBufferLength) == 0;
    if (!ok)
    {
        string e (strerror (errno));
        error.assign ("failed to init semaphore: " + e);
        return NULL;
    }

    // init state
    handleEntry->mHead = 0;
    handleEntry->mTail = 0;
    handleEntry->mSize = 0;

    return new sharedMemoryRingBuffer (shmid,
                                       handle,
                                       true,
                                       key,
                                       sizeof (struct logEntry),
                                       kDefaultMaxRingBufferLength,
                                       offsetof (struct shmLogEntryHeader, mEntries));
}

sharedMemoryRingBuffer*
sharedMemoryRingBuffer::attach (key_t key, string& error)
{
    int shmFlags = 0644;
    size_t size = sizeof (struct shmLogEntryHeader)
        + (kDefaultMaxRingBufferLength * sizeof (struct logEntry));

    int shmid = shmget (key, size, shmFlags);
    if (shmid == -1)
    {
        error.assign ("failed to get shared memory id from key");
        return NULL;
    }

    unsigned char* handle = (unsigned char*)shmat (shmid, NULL, 0);
    if (handle == NULL)
    {
        string e (strerror (errno));
        error.assign ("failed to attach to region: " + e);
        return NULL;
    }

    return new sharedMemoryRingBuffer (shmid,
                                       handle,
                                       false,
                                       key,
                                       sizeof (struct logEntry),
                                       kDefaultMaxRingBufferLength,
                                       offsetof (struct shmLogEntryHeader, mEntries));
}

void
sharedMemoryRingBuffer::detach ()
{
    shmdt (mHandle);
}

void
sharedMemoryRingBuffer::clearSharedMemory ()
{
    shmctl (mShmid, IPC_RMID, NULL);
}

void
sharedMemoryRingBuffer::blockingEnqueue (const logEntry* entry)
{
    sem_wait (&(getHeader ()->mSema));
    sbfMutex_lock (&(getHeader ()->mMutex)); // lock table

    insertElement (entry);

    sbfCondVar_signal (&(getHeader ()->mCond));
    sbfMutex_unlock (&(getHeader ()->mMutex));
}

bool
sharedMemoryRingBuffer::blockingDequeue (logEntry* entry)
{
    sbfMutex_lock (&(getHeader ()->mMutex)); // lock table
    while (getSize () == 0 and !mDidSignal)
        sbfCondVar_wait (&(getHeader ()->mCond), &(getHeader ()->mMutex));

    bool ok = takeElement (entry);
    if (ok)
        sem_post (&(getHeader ()->mSema)); // release resource

    sbfMutex_unlock (&(getHeader ()->mMutex));

    return ok;
}

void
sharedMemoryRingBuffer::insertElement (const logEntry* entry)
{
    struct shmLogEntryHeader* hdr = getHeader ();

    size_t offs = hdr->mHead * mElementSize;
    uint8_t* table = getEntriesTable ();

    memcpy (table + offs, entry, sizeof (struct logEntry));
    hdr->mSize += 1;

    // update indexes
    hdr->mHead += 1;
    if (hdr->mHead == mLength)
    {
        hdr->mHead = 0;
    }
}

// assumes caller has acquired locks correctly
bool
sharedMemoryRingBuffer::takeElement (logEntry* entry)
{
    struct shmLogEntryHeader* hdr = getHeader ();
    if (hdr->mSize == 0)
    {
        return false;
    }

    size_t offs = hdr->mTail * mElementSize;
    uint8_t* table = getEntriesTable ();

    memcpy (entry, table + offs, sizeof (struct logEntry));
    hdr->mSize -= 1;

    hdr->mTail += 1;
    if (hdr->mTail == mLength)
    {
        hdr->mTail = 0;
    }

    return true;
}

shmLogEntryHeader*
sharedMemoryRingBuffer::getHeader ()
{
    return (struct shmLogEntryHeader*)mHandle;
}

uint8_t*
sharedMemoryRingBuffer::getEntriesTable ()
{
    return mHandle + mOffset;
}

size_t
sharedMemoryRingBuffer::getSize ()
{
    return getHeader ()->mSize;
}

void
sharedMemoryRingBuffer::signal ()
{
    sbfMutex_lock (&(getHeader ()->mMutex));

    // signal end
    mDidSignal = true;
    sbfCondVar_signal (&(getHeader ()->mCond));
    
    sbfMutex_unlock (&(getHeader ()->mMutex));
}

}

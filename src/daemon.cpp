/*
 * Copyright 2014-2018 Neueda Ltd.
 */

#include "daemon.h"
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


using namespace neueda;

static bool gConsumerRunning = true;

static void*
consumeSharedMemory (void* closure)
{
    logService& logService = logService::get ();
    sharedMemoryRingBuffer* sharedMemory = (sharedMemoryRingBuffer*)closure;

    while (gConsumerRunning)
    {
        logEntry entry;
        if (sharedMemory->blockingDequeue (&entry))
        {
            string message (entry.mMessage);
            logService.handle (string (entry.mName),
                               entry.severity,
                               &entry.mTime,
                               &entry.mTv,
                               message.c_str (),
                               message.size ());
        }
    }
    return NULL;
}

static void
serverSignalHandler (loggerDaemonServer* server, int signal)
{
    server->stop ();
}

static bool
setupDaemon (DaemonConfiguration& config)
{
    string error;
    sharedMemoryRingBuffer* sharedMemory =
        sharedMemoryRingBuffer::create (config.getSharedMemoryKey (), error);
    
    if (sharedMemory == NULL)
    {
        cerr << "error creating shared memory buffer: " << error << endl;
        return false;
    }
    
    MessageHandler handler (config);
    loggerDaemonServer server (config.getSockPath (), &handler);
    server.attachSignalHandler (&serverSignalHandler);

    // start the consumer
    sbfThread consumer;
    sbfThread_create (&consumer, &consumeSharedMemory, (void*)sharedMemory);

    // this blocks
    server.dispatch ();

    // close shared memory
    gConsumerRunning = false;
    sharedMemory->signal ();
    sbfThread_join (consumer);
    delete sharedMemory;

    return true;
}

int
main (int argc, char** argv)
{
    char *cvalue = NULL;
    int c;
    while ((c = getopt (argc, argv, "c:")) != -1)
        switch (c)
        {
        case 'c':
            cvalue = optarg;
            break;
        case '?':
            if (optopt == 'c')
                fprintf (stderr, "Option -%c requires an argument.\n", optopt);
            else if (isprint (optopt))
                fprintf (stderr, "Unknown option `-%c'.\n", optopt);
            else
                fprintf (stderr,
                         "Unknown option character `\\x%x'.\n",
                         optopt);
            return 1;
        default:
            abort ();
        }

    if (cvalue == NULL)
    {
        cerr << "no configuration specified see --help" << endl;
        return 1;
    }
    
    string configPath (cvalue);
    string errorMessage;
    properties props;
    if (!props.loadFromFile (configPath, errorMessage))
    {
        cerr << "failed to load config: " << errorMessage << endl;
        return 1;
    }

    logService& logService = logService::get ();
    if (!logService.configure (props, errorMessage))
    {
        cerr << "failed to configure handlers: " << errorMessage << endl;
        return 1;
    }

    DaemonConfiguration config (props);
    if (!setupDaemon (config))
        return 1; 

    return 0;
}

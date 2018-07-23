/*
 * Copyright 2014-2018 Neueda Ltd.
 */

#include "fileLogHandler.h"

#include <cstdio>
#include <iostream>
#include <sstream>
#include <cerrno>
#include <cstring>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/time.h>

namespace neueda
{

fileLogHandler::fileLogHandler (const string& path,
                                const size_t limit,
                                const int fileCount) :
    logHandler (),
    mFile (NULL),
    mPath (path),
    mSize (0),
    mSizeLimit (limit),
    mCountLimit (fileCount),
    mCount (0)
{
}

fileLogHandler::~fileLogHandler ()
{
}

bool
fileLogHandler::setup ()
{
    errno = 0;

    FILE* f = fopen (mPath.c_str (), "w");
    if (f == NULL)
    {
        setLastError (strerror (errno));
        return false;
    }

    // no buffering
    setbuf (f, NULL);

    errno = 0;
    // get current size
    struct stat sb;
    if (fstat (fileno (f), &sb) != 0)
    {
        setLastError (strerror (errno));
        fclose (f);
        return false;
    }

    mFile = f;
    mSize = sb.st_size;

    return true;
}

void
fileLogHandler::teardown ()
{
    if (mFile != NULL)
        fclose (mFile);
}

void
fileLogHandler::flush ()
{
    fflush (mFile);
}

void
fileLogHandler::roll ()
{
    string errorMessage;

    mCount++;
    if (mCountLimit > 0 && mCount >= mCountLimit)
        mCount = 1;

    stringstream to;
    stringstream from;
    for (int i = mCount - 1; i >= 0; i--)
    {
        to.str ("");
        from.str ("");

        to << mPath << "." << i + 1;

        if (i == 0)
            from << mPath;
        else
            from << mPath << "." << i;

        if (rename (from.str ().c_str (), to.str ().c_str ()) != 0)
        {
            errorMessage.assign ("failed to rename");
            return;
        }
    }

    fclose (mFile);
    if (!setup ())
    {
        cerr << getLastError () << endl;
        return;
    }
}


void
fileLogHandler::handle (logSeverity::level severity,
                        const char* name,
                        const struct tm* tm_time,
                        const timeval *tv,
                        const char* message,
                        size_t message_len)
{
    string outStr = toString (mFormat,
                              severity,
                              name,
                              tm_time,
                              tv,
                              message,
                              message_len);

    mSize += fprintf (mFile, "%s\n", outStr.c_str ());

    if (mSizeLimit != 0 && mSize > mSizeLimit)
        roll ();
}

}

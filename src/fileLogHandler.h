/*
 * Copyright 2014-2018 Neueda Ltd.
 */

#pragma once

#include "logHandler.h"
#include <string>

using namespace std;

namespace neueda
{

class fileLogHandler: public logHandler
{
public:
    fileLogHandler (const std::string& path,
                    const size_t limit,
                    const int fileCount);
    
    ~fileLogHandler ();

    bool setup ();

    void teardown ();

    void handle (logSeverity::level severity,
                 const char* name,
		 uint64_t time,
                 const char* message,
                 size_t message_len);

private:
    void flush ();
    void roll ();

    FILE*               mFile;
    std::string         mPath;
    size_t              mSize;
    size_t              mSizeLimit;
    int                 mCountLimit;
    int                 mCount;
};

};

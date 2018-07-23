/*
 * Copyright 2014-2018 Neueda Ltd.
 */

#pragma once

#include "logHandler.h"


namespace neueda
{

class consoleLogHandler: public logHandler
{
public:
    consoleLogHandler ();

    void teardown () { }

    void handle (logSeverity::level severity,
                 const char* name,
                 const struct tm* tm_time,
                 const timeval *tv,
                 const char* message,
                 size_t message_len);

    void setOutput (FILE* o) { mOutput = o; }

    void setColorEnabled (bool enabled) { mColorEnabled = enabled; }

private:
    std::string colorize (logSeverity::level severity, const std::string& message);

    FILE*   mOutput;
    bool    mColorEnabled;
};

};

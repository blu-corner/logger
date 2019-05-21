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
		 uint64_t time,
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

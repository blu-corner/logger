/*
 * Copyright 2014-2018 Neueda Ltd.
 */

#pragma once

#include "logHandler.h"

using namespace std;

namespace neueda
{

class syslogLogHandler: public logHandler
{
public:
    syslogLogHandler ();
    
    void teardown () { }

    void handle (logSeverity::level severity,
                 const char* name,
		 uint64_t time,
                 const char* message,
                 size_t message_len);

    static int severityToSyslogPriority (logSeverity::level level);
};

};

/*
 * Copyright 2014-2018 Neueda Ltd.
 */

#pragma once
#ifdef WIN32
#undef ERROR
#endif


namespace neueda
{

class logSeverity
{
public:
    enum level { DEBUG = 0, INFO, WARN, ERROR, FATAL };
};

}

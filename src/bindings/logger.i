/*
 * Copyright 2014-2018 Neueda Ltd.
 */
%module(directors="1", thread="1") loggerBindings

%{
#include "logger.h"

#include <stdexcept>
#include <sstream>
#include <string>
%}

%include "std_string.i"
%include "stdint.i"
%include "std_vector.i"
%include "cdata.i"
%include "typemaps.i"

// macros
%define __attribute__(x)
%enddef

%rename(Properties) neueda::properties;
%rename(LogService) neueda::logService;
%rename(Logger) neueda::logger;
%rename(LogHandler) neueda::logHandler;
%rename(LogSeverity) neueda::logSeverity;
%rename(LogHandlerFactory) neueda::logHandlerFactory;

%ignore neueda::logService::addHandler (logHandler* handler, std::string& err, bool owned);
%ignore neueda::logService::removeHandler (logHandler* handler);

/* %feature("director") neueda::logHandler; */

%import(module="properties") "properties.h"

%extend neueda::logService {
    /* 
     * TODO add support for custom logHandlers in language bindings
     *
     void addHandler (logHandler* const handler)
     {
        std::string errorMessage;
        bool ok = self->addHandler (handler, errorMessage, false);

        if (!ok)
        {
            std::ostringstream oss;
            oss << "failed to add handler: "
                << errorMessage;
            throw std::runtime_error (oss.str ());
        }
     }
     */

    void configure (neueda::properties& props)
    {
        std::string errorMessage;
        bool ok = self->configure (props, errorMessage);
        
        if (!ok)
        {
            std::ostringstream oss;
            oss << "failed to configure: "
                << errorMessage;
            throw std::runtime_error (oss.str ());
        }
    }
}

%extend neueda::logger {
    void debug (std::string msg)
    {
        self->log (neueda::logSeverity::DEBUG, "%s", msg.c_str ()); 
    }

    void info (std::string msg)
    {
        self->log (neueda::logSeverity::INFO, "%s", msg.c_str ()); 
    }

    void warn (std::string msg)
    {
        self->log (neueda::logSeverity::WARN, "%s", msg.c_str ()); 
    }

    void err (std::string msg)
    {
        self->log (neueda::logSeverity::ERROR, "%s", msg.c_str ()); 
    }

    void fatal (std::string msg)
    {
        self->log (neueda::logSeverity::FATAL, "%s", msg.c_str ()); 
    }
}

%include "logSeverity.h"
%include "logHandler.h"
%include "logger.h"

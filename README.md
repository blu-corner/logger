logger
======

- [Overview](#overview)
- [Getting Started](#getting-started)
    * [Dependencies](#dependencies)
    * [Example Usage](#example-usage)
    * [Running the Tests](#running-the-tests)
- [Architecture](#architecture)
    * [Levels](#levels)
- [Configuration](#configuration)
- [Recipes](docs/recipes.md)
    * [Format](docs/recipes.md#format)
    * [Console](docs/recipes.md#console)
    * [Shared Memory](docs/recipes.md#shared-memory)
    * [File Roll](docs/recipes.md#file-roll)

# Overview

logger is an asynchronous logging library designed to offload logging from the
main execution thread to minimise performance impact. Bindings for Java and
Python are provided via SWIG.

The module currently supports the following log handlers:

* Console
* File
* Shared Memory
* syslog

Users can currently implement their own custom handlers in C++. Support for custom
handlers is currently planned for bindings.

# Getting Started

To compile the installation:

```bash
$ git submodule update --init --recursive
$ mkdir build
$ cd build
$ cmake -DTESTS=ON ../
$ make
$ make install
```

Language bindings can be enabled by passing -DJAVA=on, -DPYTHON=on
to CMake. It is possible to build all bindings in the same language.

## Dependencies

The only external dependency is SWIG, and is only required when building the
Java, C# or Python bindings. For information on installing SWIG please visit the
[SWIG website](http://www.swig.org). All other dependencies are managed through 
git submodules.

## Example Usage

Given a configuration file:

```bash
oms.gw.lse.lh.console.enabled=true
oms.gw.lse.lh.console.color=true
oms.gw.lse.lh.console.level=debug
```

The following code will load the configuration, create the logService and log to
the console.

```cpp
#include "logger.h"
#include "properties.h"
#include <stdint.h>
#include <string>
#include <iostream>

using namespace neueda;

int main ()
{
    properties p ("oms", "gw", "lse");
    std::string err;
    if (!p.loadFromFile ("config.file", err))
    {
        std::cout << err << std::endl;
        return 1;
    }

    logService& service = logService::get ();
    service.configure (p);

    logger& logger = service.getLogger ('root');

    logger.debug('debug msg');
    logger.info('info msg');
    logger.warn('warn msg');
    logger.err('err msg');
}
```

Examples have been provided in each language within the examples/ folder of the
repository.

## Running the Tests

To run the unit tests:

```bash
$ make test
```

# Architecture

There are 3 main components to logger.

* service
* handlers
* loggers

The service creates and manages loggers/handlers. loggers and handlers have 
assigned log levels that can differ, events below this log level will not 
be processed.

When an event is logged the logger will send this to the service if it is
greater than or equal to the assigned level. The service iterates the configured
handlers and passes the event to them for 'handling'. If the service is
configured to be asynchronous the event is placed on a queue, then processed on 
a thread separate from the main execution thread.

The following displays example configurations and available configurations for
each of the built in log handlers.

## Levels

There are 5 levels:

* FATAL
* ERROR
* WARN
* INFO
* DEBUG

# Configuration

| Scope | Parameter Name | Options | Default | Description |
| :---: | :---: | :---: | :---: | :--- |
| Global | logger.service.async | true/false | false | A new thread is created on which all log handlers are executed. |
| Global | lh.*HANDLER*.enabled | true/false | false (except console which is enabled by default) | Enables the specified log handler. |
| Global | lh.*HANDLER*.level | debug/info/warn/err | info | Define the log level for the handler. |
| Global | lh.*HANDLER*.format | {severity}, {time}, {name}, {message} | {severity} {time} {name} {message} | Format for log messages from this handler. Does not apply to shared memory |
| Console | lh.console.color | true/false | true | Enables colored console output. |
| Console | lh.console.output | stdout/stderr | stdout | Where to output console messages to. |
| File | lh.file.path | /path/to/log/file | None | The log file to output to |
| File | lh.file.size | X bytes | 0 | Log file will be rolled when it has exceeded this size |
| File | lh.file.count | # Files | 0 | The maximum number of files that will exist before overwriting the first file |
| Shared Memory | lh.shm.sock | /path/to/shm/socket | None | The location of the shared memory file. |

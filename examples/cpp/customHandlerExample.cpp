#include "logger.h"
#include "properties.h"
#include <stdint.h>
#include <iostream>
#include <iomanip>

using namespace std;
using namespace neueda;

class customLogHandler  : public logHandler
{
    bool setup ()
    {
        setLevel (logSeverity::DEBUG);
        return true;
    }

    void handle (logSeverity::level severity,
                 const char* name,
                 const struct tm* tm_time,
                 const struct timeval *tv,
                 const char* message,
                 size_t message_len) 
    {
        cout << "customLogHandler: " << setw(5)
             << logHandler::severityToString (severity)
             << ": " << message << endl;
    }
};

int main (int argc, char** argv)
{
    string err;
    properties p = properties ();
    p.setProperty ("lh.console.enabled", "false");

    logService& service = logService::get ();
    if (!service.configure (p, err))
    {
        cout << "failed to configure logService" << endl;
        return 1;
    }

    customLogHandler* handler = new customLogHandler ();
    service.addHandler (handler, err, true);

    logger* logger = service.getLogger("example");
    logger->setLevel (logSeverity::DEBUG);

    logger->debug ("Hello World!");
    logger->info ("Hello World!");
    logger->warn ("Hello World!");
    logger->err ("Hello World!");

    return 0;
}

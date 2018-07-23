#include "logger.h"
#include "properties.h"
#include <stdint.h>
#include <iostream>

using namespace std;
using namespace neueda;

int main (int argc, char** argv)
{
    string err;
    properties p = properties ();
    p.setProperty ("lh.console.color", "true");
    p.setProperty ("lh.console.level", "debug");

    logService& service = logService::get ();
    if (!service.configure (p, err))
    {
        cout << "failed to configure logService" << endl;
        return 1;
    }

    logger* logger = service.getLogger("example");
    logger->setLevel (logSeverity::DEBUG);

    logger->debug ("Hello World!");
    logger->info ("Hello World!");
    logger->warn ("Hello World!");
    logger->err ("Hello World!");

    return 0;
}

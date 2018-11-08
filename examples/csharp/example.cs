/*
 * Copyright 2014-2018 Neueda Ltd.
 */
using System;
using Neueda.Properties;
using Neueda.Log;

class example
{
    static void Main(string[] args)
    {
        Properties p = new Properties ();
        p.setProperty ("lh.console.level", "debug");
        p.setProperty ("lh.console.color", "true");

        LogService service = LogService.get ();
        service.configure (p);

        Logger logger = LogService.getLogger ("CSHARP_TEST_LOGGER");
        logger.setLevel (LogSeverity.level.DEBUG);

        logger.debug("Hello world");
        logger.info("Hello world");
        logger.warn("Hello world");
        logger.err("Hello world");
    }
}

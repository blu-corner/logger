/*
 * Copyright 2014-2018 Neueda Ltd.
 */
package com.neueda.logger.example;

import com.neueda.logger.Logger;
import com.neueda.logger.LogSeverity;
import com.neueda.logger.LogService;
import com.neueda.logger.LogHandler;
import com.neueda.properties.Properties;

public class App
{
    public static void main( String[] args )
    {
        Properties p = new Properties();
        p.setProperty("lh.console.level", "debug");
        p.setProperty("lh.console.color", "true");

        LogService service = LogService.get();
        try {
            service.configure(p);
        } catch (Exception e) {
            System.out.println("failed to configure LogService");
            return;
        }

        Logger logger = service.getLogger("main");
        logger.setLevel(LogSeverity.level.DEBUG);

        logger.debug("Hello world");
        logger.info("Hello world");
        logger.warn("Hello world");
        logger.err("Hello world");
    }
}

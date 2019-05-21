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
    public class CustomHandler extends LogHandler {

	public CustomHandler () { super(); }

	public LogSeverity.level getLevel() {
	    return LogSeverity.level.DEBUG;
	}

	public boolean setup() {
	    return true;
	}

	public String getFormat() {
	    return "{severity} {name} {time} {message}";
	}

	public void handle (LogSeverity.level severity,
			    String name,
			    java.math.BigInteger time,
			    String message,
			    long message_len) 
	{
	    System.out.println ("customLogHandler: " +
				LogHandler.toString (getFormat(),
						     severity,
						     name,
						     time,
						     message,
						     message_len));
	}
    }

    private CustomHandler mHandler = new CustomHandler();

    public void run()
    {
	Properties p = new Properties();
        p.setProperty("lh.console.level", "debug");
        p.setProperty("lh.console.color", "true");

        LogService service = LogService.get();
        try {
            service.configure(p);
	    service.addHandler(mHandler);
        } catch (Exception e) {
            System.out.println("failed to configure or add handler LogService");
	    e.printStackTrace();
            return;
        }
	
        Logger logger = service.getLogger("main");
        logger.setLevel(LogSeverity.level.DEBUG);

        logger.debug("Hello world");
        logger.info("Hello world");
        logger.warn("Hello world");
        logger.err("Hello world");
    }
    
    public static void main(String[] args)
    {
        // get out of static
	new App().run();
    }
}

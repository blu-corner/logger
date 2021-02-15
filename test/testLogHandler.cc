/*
 * Copyright 2014-2018 Neueda Ltd.
 */

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "logger.h"

using namespace neueda;
using namespace std;

class testLogHandler : public logHandler
{
public:

    MOCK_METHOD0 (setup, bool ());

    MOCK_METHOD0 (teardown, void ());

    MOCK_METHOD5 (handle, void (logSeverity::level severity,
				const char* name,
				uint64_t time,
				const char* message,
				size_t message_len));
};


class logHandlerTestHarness : public ::testing::Test
{
protected:
    virtual void SetUp()
    {
        mLogger = logService::getLogger ("TEST_LOG_HANDLER");
        mLogger->setLevel (logSeverity::TRACE);
    }

    virtual void TearDown()
    {
    }

    logger* mLogger;
};


TEST_F(logHandlerTestHarness, TEST_SETUP_FAILURE)
{
    logService& logService = logService::get ();
    testLogHandler* handler = new testLogHandler ();

    ON_CALL (*handler, setup ())
        .WillByDefault (::testing::Return (false));
    EXPECT_CALL (*handler, setup ()).Times (1);

    string errorMessage;
    bool ok = logService.addHandler (handler, errorMessage, false);

    ASSERT_FALSE (ok);
    delete handler;
}

TEST_F(logHandlerTestHarness, TEST_SETUP_OK)
{
    logService& logService = logService::get ();
    testLogHandler* handler = new testLogHandler ();

    ON_CALL (*handler, setup ())
        .WillByDefault (::testing::Return (true));
    EXPECT_CALL (*handler, setup ()).Times(1);

    string errorMessage;
    bool ok = logService.addHandler (handler, errorMessage, true);

    ASSERT_TRUE (ok);

    EXPECT_CALL (*handler, teardown ()).Times (1);
    logService.removeHandler (handler);
}

TEST_F(logHandlerTestHarness, TEST_HANDLER_LOGS_TRACE_LEVEL_SET_TO_TRACE)
{
    logService& logService = logService::get ();
    testLogHandler* handler = new testLogHandler ();
    handler->setLevel (logSeverity::TRACE);

    ON_CALL (*handler, setup ())
        .WillByDefault (::testing::Return (true));
    EXPECT_CALL (*handler, setup ()).Times (1);

    string errorMessage;
    bool ok = logService.addHandler (handler, errorMessage, true);
    ASSERT_TRUE (ok);

    EXPECT_CALL (*handler, handle (::testing::_,
                                   ::testing::_,
                                   ::testing::_,
                                   ::testing::_,
                                   ::testing::_))
        .Times (3);

    mLogger->trace ("HELLO WORLD 1");
    mLogger->trace ("HELLO WORLD 2");
    mLogger->trace ("HELLO WORLD 3");

    EXPECT_CALL (*handler, teardown ()).Times (1);
    logService.removeHandler (handler);
}

TEST_F(logHandlerTestHarness, TEST_HANDLER_LOGS_DEBUG_LEVEL_SET_TO_DEBUG)
{
    logService& logService = logService::get ();
    testLogHandler* handler = new testLogHandler ();
    handler->setLevel (logSeverity::DEBUG);

    ON_CALL (*handler, setup ())
        .WillByDefault (::testing::Return (true));
    EXPECT_CALL (*handler, setup ()).Times (1);

    string errorMessage;
    bool ok = logService.addHandler (handler, errorMessage, true);
    ASSERT_TRUE (ok);

    EXPECT_CALL (*handler, handle (::testing::_,
                                   ::testing::_,
                                   ::testing::_,
                                   ::testing::_,
                                   ::testing::_))
        .Times (3);

    mLogger->debug ("HELLO WORLD 1");
    mLogger->debug ("HELLO WORLD 2");
    mLogger->debug ("HELLO WORLD 3");

    EXPECT_CALL (*handler, teardown ()).Times (1);
    logService.removeHandler (handler);
}

TEST_F(logHandlerTestHarness, TEST_HANDLER_IGNORES_DEBUG_LEVEL_SET_TO_INFO)
{
    logService& logService = logService::get ();
    testLogHandler* handler = new testLogHandler ();
    handler->setLevel (logSeverity::INFO);

    ON_CALL (*handler, setup ())
        .WillByDefault (::testing::Return (true));
    EXPECT_CALL (*handler, setup ()).Times (1);

    string errorMessage;
    bool ok = logService.addHandler (handler, errorMessage, true);
    ASSERT_TRUE (ok);

    EXPECT_CALL (*handler, handle (::testing::_,
                                   ::testing::_,
                                   ::testing::_,
                                   ::testing::_,
                                   ::testing::_))
        .Times (0);

    mLogger->debug ("HELLO WORLD 1");
    mLogger->debug ("HELLO WORLD 2");
    mLogger->debug ("HELLO WORLD 3");

    EXPECT_CALL (*handler, teardown ()).Times (1);
    logService.removeHandler (handler);
}

TEST_F(logHandlerTestHarness, TEST_HANDLER_IGNORES_DEBUG_LEVEL_SET_TO_WARNING)
{
    logService& logService = logService::get ();
    testLogHandler* handler = new testLogHandler ();
    handler->setLevel (logSeverity::WARN);

    ON_CALL (*handler, setup ())
        .WillByDefault (::testing::Return (true));
    EXPECT_CALL (*handler, setup ()).Times (1);

    string errorMessage;
    bool ok = logService.addHandler (handler, errorMessage, true);
    ASSERT_TRUE (ok);

    EXPECT_CALL (*handler, handle (::testing::_,
                                   ::testing::_,
                                   ::testing::_,
                                   ::testing::_,
                                   ::testing::_))
        .Times (0);

    mLogger->debug ("HELLO WORLD 1");
    mLogger->debug ("HELLO WORLD 2");
    mLogger->debug ("HELLO WORLD 3");

    EXPECT_CALL (*handler, teardown ()).Times (1);
    logService.removeHandler (handler);
}

TEST_F(logHandlerTestHarness, TEST_HANDLER_IGNORES_DEBUG_LEVEL_SET_TO_ERROR)
{
    logService& logService = logService::get ();
    testLogHandler* handler = new testLogHandler ();
    handler->setLevel (logSeverity::ERROR);

    ON_CALL (*handler, setup ())
        .WillByDefault (::testing::Return (true));
    EXPECT_CALL (*handler, setup ()).Times (1);

    string errorMessage;
    bool ok = logService.addHandler (handler, errorMessage, true);
    ASSERT_TRUE (ok);

    EXPECT_CALL (*handler, handle (::testing::_,
                                   ::testing::_,
                                   ::testing::_,
                                   ::testing::_,
                                   ::testing::_))
        .Times (0);

    mLogger->debug ("HELLO WORLD 1");
    mLogger->debug ("HELLO WORLD 2");
    mLogger->debug ("HELLO WORLD 3");

    EXPECT_CALL (*handler, teardown ()).Times (1);
    logService.removeHandler (handler);
}

TEST_F(logHandlerTestHarness, TEST_HANDLER_IGNORES_DEBUG_LEVEL_SET_TO_FATAL)
{
    logService& logService = logService::get ();
    testLogHandler* handler = new testLogHandler ();
    handler->setLevel (logSeverity::FATAL);

    ON_CALL (*handler, setup ())
        .WillByDefault (::testing::Return (true));
    EXPECT_CALL (*handler, setup ()).Times (1);

    string errorMessage;
    bool ok = logService.addHandler (handler, errorMessage, true);
    ASSERT_TRUE (ok);

    EXPECT_CALL (*handler, handle (::testing::_,
                                   ::testing::_,
                                   ::testing::_,
                                   ::testing::_,
                                   ::testing::_))
        .Times (0);

    mLogger->debug ("HELLO WORLD 1");
    mLogger->debug ("HELLO WORLD 2");
    mLogger->debug ("HELLO WORLD 3");

    EXPECT_CALL (*handler, teardown ()).Times (1);
    logService.removeHandler (handler);
}

TEST_F(logHandlerTestHarness, TEST_HANDLER_LOGS_INFO_LEVEL_SET_TO_DEBUG)
{
    logService& logService = logService::get ();
    testLogHandler* handler = new testLogHandler ();
    handler->setLevel (logSeverity::DEBUG);

    ON_CALL (*handler, setup ())
        .WillByDefault (::testing::Return (true));
    EXPECT_CALL (*handler, setup ()).Times (1);

    string errorMessage;
    bool ok = logService.addHandler (handler, errorMessage, true);
    ASSERT_TRUE (ok);

    EXPECT_CALL (*handler, handle (::testing::_,
                                   ::testing::_,
                                   ::testing::_,
                                   ::testing::_,
                                   ::testing::_))
        .Times (3);

    mLogger->info ("HELLO WORLD 1");
    mLogger->info ("HELLO WORLD 2");
    mLogger->info ("HELLO WORLD 3");

    EXPECT_CALL (*handler, teardown ()).Times (1);
    logService.removeHandler (handler);
}

TEST_F(logHandlerTestHarness, TEST_HANDLER_LOGS_INFO_LEVEL_SET_TO_INFO)
{
    logService& logService = logService::get ();
    testLogHandler* handler = new testLogHandler ();
    handler->setLevel (logSeverity::INFO);

    ON_CALL (*handler, setup ())
        .WillByDefault (::testing::Return (true));
    EXPECT_CALL (*handler, setup ()).Times (1);

    string errorMessage;
    bool ok = logService.addHandler (handler, errorMessage, true);
    ASSERT_TRUE (ok);

    EXPECT_CALL (*handler, handle (::testing::_,
                                   ::testing::_,
                                   ::testing::_,
                                   ::testing::_,
                                   ::testing::_))
        .Times (3);

    mLogger->info ("HELLO WORLD 1");
    mLogger->info ("HELLO WORLD 2");
    mLogger->info ("HELLO WORLD 3");

    EXPECT_CALL (*handler, teardown ()).Times (1);
    logService.removeHandler (handler);
}

TEST_F(logHandlerTestHarness, TEST_HANDLER_IGNORES_INFO_LEVEL_SET_TO_WARNING)
{
    logService& logService = logService::get ();
    testLogHandler* handler = new testLogHandler ();
    handler->setLevel (logSeverity::WARN);

    ON_CALL (*handler, setup ())
        .WillByDefault (::testing::Return (true));
    EXPECT_CALL (*handler, setup ()).Times (1);

    string errorMessage;
    bool ok = logService.addHandler (handler, errorMessage, true);
    ASSERT_TRUE (ok);

    EXPECT_CALL (*handler, handle (::testing::_,
                                   ::testing::_,
                                   ::testing::_,
                                   ::testing::_,
                                   ::testing::_))
        .Times (0);

    mLogger->info ("HELLO WORLD 1");
    mLogger->info ("HELLO WORLD 2");
    mLogger->info ("HELLO WORLD 3");

    EXPECT_CALL (*handler, teardown ()).Times (1);
    logService.removeHandler (handler);
}

TEST_F(logHandlerTestHarness, TEST_HANDLER_IGNORES_INFO_LEVEL_SET_TO_ERROR)
{
    logService& logService = logService::get ();
    testLogHandler* handler = new testLogHandler ();
    handler->setLevel (logSeverity::ERROR);

    ON_CALL (*handler, setup ())
        .WillByDefault (::testing::Return (true));
    EXPECT_CALL (*handler, setup ()).Times (1);

    string errorMessage;
    bool ok = logService.addHandler (handler, errorMessage, true);
    ASSERT_TRUE (ok);

    EXPECT_CALL (*handler, handle (::testing::_,
                                   ::testing::_,
                                   ::testing::_,
                                   ::testing::_,
                                   ::testing::_))
        .Times (0);

    mLogger->info ("HELLO WORLD 1");
    mLogger->info ("HELLO WORLD 2");
    mLogger->info ("HELLO WORLD 3");

    EXPECT_CALL (*handler, teardown ()).Times (1);
    logService.removeHandler (handler);
}

TEST_F(logHandlerTestHarness, TEST_HANDLER_IGNORES_INFO_LEVEL_SET_TO_FATAL)
{
    logService& logService = logService::get ();
    testLogHandler* handler = new testLogHandler ();
    handler->setLevel (logSeverity::FATAL);

    ON_CALL (*handler, setup ())
        .WillByDefault (::testing::Return (true));
    EXPECT_CALL (*handler, setup ()).Times (1);

    string errorMessage;
    bool ok = logService.addHandler (handler, errorMessage, true);
    ASSERT_TRUE (ok);

    EXPECT_CALL (*handler, handle (::testing::_,
                                   ::testing::_,
                                   ::testing::_,
                                   ::testing::_,
                                   ::testing::_))
        .Times (0);

    mLogger->info ("HELLO WORLD 1");
    mLogger->info ("HELLO WORLD 2");
    mLogger->info ("HELLO WORLD 3");

    EXPECT_CALL (*handler, teardown ()).Times (1);
    logService.removeHandler (handler);
}

TEST_F(logHandlerTestHarness, TEST_HANDLER_LOGS_WARNING_LEVEL_SET_TO_DEBUG)
{
    logService& logService = logService::get ();
    testLogHandler* handler = new testLogHandler ();
    handler->setLevel (logSeverity::DEBUG);

    ON_CALL (*handler, setup ())
        .WillByDefault (::testing::Return (true));
    EXPECT_CALL (*handler, setup ()).Times (1);

    string errorMessage;
    bool ok = logService.addHandler (handler, errorMessage, true);
    ASSERT_TRUE (ok);

    EXPECT_CALL (*handler, handle (::testing::_,
                                   ::testing::_,
                                   ::testing::_,
                                   ::testing::_,
                                   ::testing::_))
        .Times (3);

    mLogger->warn ("HELLO WORLD 1");
    mLogger->warn ("HELLO WORLD 2");
    mLogger->warn ("HELLO WORLD 3");

    EXPECT_CALL (*handler, teardown()).Times (1);
    logService.removeHandler (handler);
}

TEST_F(logHandlerTestHarness, TEST_HANDLER_LOGS_WARNING_LEVEL_SET_TO_INFO)
{
    logService& logService = logService::get ();
    testLogHandler* handler = new testLogHandler ();
    handler->setLevel (logSeverity::INFO);

    ON_CALL (*handler, setup ())
        .WillByDefault (::testing::Return (true));
    EXPECT_CALL (*handler, setup ()).Times (1);

    string errorMessage;
    bool ok = logService.addHandler (handler, errorMessage, true);
    ASSERT_TRUE (ok);

    EXPECT_CALL (*handler, handle (::testing::_,
                                   ::testing::_,
                                   ::testing::_,
                                   ::testing::_,
                                   ::testing::_))
        .Times (3);

    mLogger->warn ("HELLO WORLD 1");
    mLogger->warn ("HELLO WORLD 2");
    mLogger->warn ("HELLO WORLD 3");

    EXPECT_CALL (*handler, teardown ()).Times (1);
    logService.removeHandler (handler);
}

TEST_F(logHandlerTestHarness, TEST_HANDLER_LOGS_WARNING_LEVEL_SET_TO_WARNING)
{
    logService& logService = logService::get ();
    testLogHandler* handler = new testLogHandler ();
    handler->setLevel (logSeverity::WARN);

    ON_CALL (*handler, setup ())
        .WillByDefault (::testing::Return (true));
    EXPECT_CALL (*handler, setup ()).Times (1);

    string errorMessage;
    bool ok = logService.addHandler (handler, errorMessage, true);
    ASSERT_TRUE (ok);

    EXPECT_CALL (*handler, handle (::testing::_,
                                   ::testing::_,
                                   ::testing::_,
                                   ::testing::_,
                                   ::testing::_))
        .Times(3);

    mLogger->warn ("HELLO WORLD 1");
    mLogger->warn ("HELLO WORLD 2");
    mLogger->warn ("HELLO WORLD 3");

    EXPECT_CALL (*handler, teardown ()).Times (1);
    logService.removeHandler (handler);
}

TEST_F(logHandlerTestHarness, TEST_HANDLER_IGNORES_WARNING_LEVEL_SET_TO_ERROR)
{
    logService& logService = logService::get ();
    testLogHandler* handler = new testLogHandler ();
    handler->setLevel (logSeverity::ERROR);

    ON_CALL (*handler, setup ())
        .WillByDefault (::testing::Return (true));
    EXPECT_CALL (*handler, setup ()).Times (1);

    string errorMessage;
    bool ok = logService.addHandler (handler, errorMessage, true);
    ASSERT_TRUE (ok);

    EXPECT_CALL (*handler, handle (::testing::_,
                                   ::testing::_,
                                   ::testing::_,
                                   ::testing::_,
                                   ::testing::_))
        .Times(0);

    mLogger->warn ("HELLO WORLD 1");
    mLogger->warn ("HELLO WORLD 2");
    mLogger->warn ("HELLO WORLD 3");

    EXPECT_CALL (*handler, teardown ()).Times (1);
    logService.removeHandler (handler);
}

TEST_F(logHandlerTestHarness, TEST_HANDLER_IGNORES_WARNING_LEVEL_SET_TO_FATAL)
{
    logService& logService = logService::get ();
    testLogHandler* handler = new testLogHandler ();
    handler->setLevel (logSeverity::FATAL);

    ON_CALL (*handler, setup ())
        .WillByDefault (::testing::Return (true));
    EXPECT_CALL (*handler, setup ()).Times (1);

    string errorMessage;
    bool ok = logService.addHandler (handler, errorMessage, true);
    ASSERT_TRUE (ok);

    EXPECT_CALL (*handler, handle (::testing::_,
                                   ::testing::_,
                                   ::testing::_,
                                   ::testing::_,
                                   ::testing::_))
        .Times (0);

    mLogger->warn ("HELLO WORLD 1");
    mLogger->warn ("HELLO WORLD 2");
    mLogger->warn ("HELLO WORLD 3");

    EXPECT_CALL (*handler, teardown ()).Times (1);
    logService.removeHandler (handler);
}

TEST_F(logHandlerTestHarness, TEST_HANDLER_LOGS_ERROR_LEVEL_SET_TO_DEBUG)
{
    logService& logService = logService::get ();
    testLogHandler* handler = new testLogHandler ();
    handler->setLevel (logSeverity::DEBUG);

    ON_CALL (*handler, setup ())
        .WillByDefault (::testing::Return (true));
    EXPECT_CALL (*handler, setup ()).Times (1);

    string errorMessage;
    bool ok = logService.addHandler (handler, errorMessage, true);
    ASSERT_TRUE (ok);

    EXPECT_CALL (*handler, handle (::testing::_,
                                   ::testing::_,
                                   ::testing::_,
                                   ::testing::_,
                                   ::testing::_))
        .Times (3);

    mLogger->err ("HELLO WORLD 1");
    mLogger->err ("HELLO WORLD 2");
    mLogger->err ("HELLO WORLD 3");

    EXPECT_CALL (*handler, teardown ()).Times (1);
    logService.removeHandler (handler);
}

TEST_F(logHandlerTestHarness, TEST_HANDLER_LOGS_ERROR_LEVEL_SET_TO_INFO)
{
    logService& logService = logService::get ();
    testLogHandler* handler = new testLogHandler ();
    handler->setLevel (logSeverity::INFO);

    ON_CALL (*handler, setup ())
        .WillByDefault (::testing::Return (true));
    EXPECT_CALL (*handler, setup ()).Times (1);

    string errorMessage;
    bool ok = logService.addHandler (handler, errorMessage, true);
    ASSERT_TRUE (ok);

    EXPECT_CALL (*handler, handle (::testing::_,
                                   ::testing::_,
                                   ::testing::_,
                                   ::testing::_,
                                   ::testing::_))
        .Times (3);

    mLogger->err ("HELLO WORLD 1");
    mLogger->err ("HELLO WORLD 2");
    mLogger->err ("HELLO WORLD 3");

    EXPECT_CALL (*handler, teardown ()).Times (1);
    logService.removeHandler (handler);
}

TEST_F(logHandlerTestHarness, TEST_HANDLER_LOGS_ERROR_LEVEL_SET_TO_WARNING)
{
    logService& logService = logService::get ();
    testLogHandler* handler = new testLogHandler ();
    handler->setLevel (logSeverity::WARN);

    ON_CALL (*handler, setup ())
        .WillByDefault (::testing::Return (true));
    EXPECT_CALL (*handler, setup ()).Times (1);

    string errorMessage;
    bool ok = logService.addHandler (handler, errorMessage, true);
    ASSERT_TRUE (ok);

    EXPECT_CALL (*handler, handle (::testing::_,
                                   ::testing::_,
                                   ::testing::_,
                                   ::testing::_,
                                   ::testing::_))
        .Times (3);

    mLogger->err ("HELLO WORLD 1");
    mLogger->err ("HELLO WORLD 2");
    mLogger->err ("HELLO WORLD 3");

    EXPECT_CALL (*handler, teardown ()).Times (1);
    logService.removeHandler (handler);
}

TEST_F(logHandlerTestHarness, TEST_HANDLER_LOGS_ERROR_LEVEL_SET_TO_ERROR)
{
    logService& logService = logService::get ();
    testLogHandler* handler = new testLogHandler ();
    handler->setLevel (logSeverity::ERROR);

    ON_CALL (*handler, setup ())
        .WillByDefault (::testing::Return (true));
    EXPECT_CALL (*handler, setup ()).Times (1);

    string errorMessage;
    bool ok = logService.addHandler (handler, errorMessage, true);
    ASSERT_TRUE (ok);

    EXPECT_CALL (*handler, handle (::testing::_,
                                   ::testing::_,
                                   ::testing::_,
                                   ::testing::_,
                                   ::testing::_))
        .Times (3);

    mLogger->err ("HELLO WORLD 1");
    mLogger->err ("HELLO WORLD 2");
    mLogger->err ("HELLO WORLD 3");

    EXPECT_CALL (*handler, teardown ()).Times (1);
    logService.removeHandler (handler);
}

TEST_F(logHandlerTestHarness, TEST_HANDLER_IGNORES_ERROR_LEVEL_SET_TO_FATAL)
{
    logService& logService = logService::get ();
    testLogHandler* handler = new testLogHandler ();
    handler->setLevel (logSeverity::FATAL);

    ON_CALL (*handler, setup ())
        .WillByDefault (::testing::Return (true));
    EXPECT_CALL (*handler, setup ()).Times (1);

    string errorMessage;
    bool ok = logService.addHandler (handler, errorMessage, true);
    ASSERT_TRUE (ok);

    EXPECT_CALL (*handler, handle (::testing::_,
                                   ::testing::_,
                                   ::testing::_,
                                   ::testing::_,
                                   ::testing::_))
        .Times (0);

    mLogger->err ("HELLO WORLD 1");
    mLogger->err ("HELLO WORLD 2");
    mLogger->err ("HELLO WORLD 3");

    EXPECT_CALL (*handler, teardown ()).Times (1);
    logService.removeHandler (handler);
}

/*
    Copyright 2018 Neueda Ltd.
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

class LoggerHandlerTestHarness : public ::testing::Test
{
protected:
    virtual void SetUp()
    {
        mLogger = logService::getLogger ("TEST_LOG_HANDLER");
    }

    virtual void TearDown()
    {
    }

    logger* mLogger;
};

TEST_F(LoggerHandlerTestHarness, TEST_EXCEED_BUFFER_SIZE)
{
    char* testBuffer = new char[defaultLogMessageChunkSize * 2];
    memset (testBuffer, 0, defaultLogMessageChunkSize * 2);
    memset (testBuffer, '1', defaultLogMessageChunkSize + (defaultLogMessageChunkSize / 2));

    logService& service = logService::get ();
    testLogHandler* handler = new testLogHandler ();
    handler->setLevel (logSeverity::INFO);

    ON_CALL (*handler, setup ())
        .WillByDefault (::testing::Return (true));
    EXPECT_CALL (*handler, setup ()).Times (1);

    string errorMessage;
    bool ok = service.addHandler (handler, errorMessage, true);
    ASSERT_TRUE (ok);

    EXPECT_CALL (*handler, handle (::testing::_,
                                   ::testing::_,
                                   ::testing::_,
                                   ::testing::_,
                                   ::testing::_))
        .Times (2);

    mLogger->info ("%s", testBuffer);
    EXPECT_CALL (*handler, teardown ()).Times (1);
    service.removeHandler (handler);

    delete [] testBuffer;
}

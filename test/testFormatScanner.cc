/*
 * Copyright 2014-2018 Neueda Ltd.
 */

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "logger.h"

using namespace neueda;
using namespace std;

class formatScannerTestHarness : public ::testing::Test
{
protected:
    virtual void SetUp ()
    {
        gettimeofday (&mTv, NULL);
        time_t t = mTv.tv_sec;
        gmtime_r (&t, &mTimee);

        mTime = 1000000 * mTv.tv_sec + mTv.tv_usec;

        char dateTimeBuffer[64];
        memset (dateTimeBuffer, 0, sizeof dateTimeBuffer);
        size_t nBytes = snprintf (dateTimeBuffer,
                                  sizeof dateTimeBuffer,
                                  "%04u-%02u-%02u %02u:%02u:%02u.%06u",
                                  mTimee.tm_year + 1900,
                                  mTimee.tm_mon + 1,
                                  mTimee.tm_mday,
                                  mTimee.tm_hour,
                                  mTimee.tm_min,
                                  mTimee.tm_sec,
                                  (unsigned int)mTv.tv_usec);
        
        timeString = string (dateTimeBuffer, nBytes);
    }

    struct tm mTimee;
    struct timeval mTv;
    uint64_t mTime;
    string timeString;
};


TEST_F(formatScannerTestHarness, TEST_SCANNER_HANDLES_TIME_TOKEN)
{
    string format = "{time}";
    string message = "message";
    
    string log = logHandler::toString (format,
                                       logSeverity::INFO,
                                       "TEST",
                                       mTime,
                                       message.c_str(),
                                       message.size ());

    ASSERT_STREQ(log.c_str(), timeString.c_str());
}

TEST_F(formatScannerTestHarness, TEST_SCANNER_HANDLES_SEVERITY_TOKEN_DEBUG)
{
    string format = "{severity}";
    string message = "message";
    
    string log = logHandler::toString (format,
                                       logSeverity::DEBUG,
                                       "TEST",
                                       mTime,
                                       message.c_str(),
                                       message.size ());

    ASSERT_STREQ(log.c_str(), "DEBUG");
}

TEST_F(formatScannerTestHarness, TEST_SCANNER_HANDLES_SEVERITY_TOKEN_INFO)
{
    string format = "{severity}";
    string message = "message";
    
    string log = logHandler::toString (format,
                                       logSeverity::INFO,
                                       "TEST",
                                       mTime,
                                       message.c_str(),
                                       message.size ());

    ASSERT_STREQ(log.c_str(), "INFO");
}

TEST_F(formatScannerTestHarness, TEST_SCANNER_HANDLES_SEVERITY_TOKEN_WARNING)
{
    string format = "{severity}";
    string message = "message";
    
    string log = logHandler::toString (format,
                                       logSeverity::WARN,
                                       "TEST",
                                       mTime,
                                       message.c_str(),
                                       message.size ());

    ASSERT_STREQ(log.c_str(), "WARN");
}

TEST_F(formatScannerTestHarness, TEST_SCANNER_HANDLES_SEVERITY_TOKEN_ERROR)
{
    string format = "{severity}";
    string message = "message";
    
    string log = logHandler::toString (format,
                                       logSeverity::ERROR,
                                       "TEST",
                                       mTime,
                                       message.c_str(),
                                       message.size ());

    ASSERT_STREQ(log.c_str(), "ERROR");
}

TEST_F(formatScannerTestHarness, TEST_SCANNER_HANDLES_SEVERITY_TOKEN_FATAL)
{
    string format = "{severity}";
    string message = "message";
    
    string log = logHandler::toString (format,
                                       logSeverity::FATAL,
                                       "TEST",
                                       mTime,
                                       message.c_str(),
                                       message.size ());

    ASSERT_STREQ(log.c_str(), "FATAL");
}

TEST_F(formatScannerTestHarness, TEST_SCANNER_HANDLES_TOKEN_NAME)
{
    string format = "{name}";
    string message = "message";
    
    string log = logHandler::toString (format,
                                       logSeverity::FATAL,
                                       "TEST",
                                       mTime,
                                       message.c_str(),
                                       message.size ());

    ASSERT_STREQ(log.c_str(), "TEST");
}

TEST_F(formatScannerTestHarness, TEST_SCANNER_HANDLES_TOKEN_MESSAGE)
{
    string format = "{message}";
    string message = "message";
    
    string log = logHandler::toString (format,
                                       logSeverity::FATAL,
                                       "TEST",
                                       mTime,
                                       message.c_str(),
                                       message.size ());

    ASSERT_STREQ(log.c_str(), "message");
}

TEST_F(formatScannerTestHarness, TEST_SCANNER_HANDLES_EMPTY_FORMAT)
{
    string format = "";
    string message = "message";
    
    string log = logHandler::toString (format,
                                       logSeverity::FATAL,
                                       "TEST",
                                       mTime,
                                       message.c_str(),
                                       message.size ());

    ASSERT_STREQ(log.c_str(), "");
}

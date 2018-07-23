/*
 * Copyright 2014-2018 Neueda Ltd.
 */
#include <logger/logger.h>

#include <gtest/gtest.h>

using namespace neueda;
using namespace std;

class localWriter : public ::testing::Test i
{
protected:
    virtual void SetUp()
    {
        string error;
        LogService& logService = LogService::get ();

        RawProperties props;
        props.setProperty ("logger.handler.shm.sock", "/tmp/logger.sock");

        string errorMessage;
        bool ok = logService.configure (props, errorMessage);
        if (not ok)
        {
            cerr << "logger-config-error: " << errorMessage << endl;
            ASSERT_TRUE(ok);
        }
        
        mLogger = logService.getLogger ("TEST_LOCAL");
    }

    virtual void TearDown ()
    {
        // TODO
    }
    
    Logger* mLogger;
};

TEST_F(localWriter, WRITE_TEST_1)
{
    size_t i;
    for (i = 0; i < 100; ++i)
    {
        ostringstream oss;
        oss << "log-message: " << i;
        string m = oss.str ();
        mLogger->info ("%s", m.c_str ());
    }
}

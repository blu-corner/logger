/*
 * Copyright 2014-2018 Neueda Ltd.
 */

#include "logger.h"
#include "properties.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <iostream>
#include <fstream>
#include <cstdio>
#include <string>

using namespace neueda;

class logServiceTestHarness : public ::testing::Test
{
protected:
    virtual void SetUp ()
    {
        mService = &(logService::get ());
    }

    logService* mService;
};


TEST_F(logServiceTestHarness, TEST_CONFIGURE_SUCCEEDS)
{
    properties p;
    p.setProperty ("lh.console.level", "debug");
    p.setProperty ("lh.console.color", "true");

    string err ("");
    bool ok = mService->configure (p, err);
    ASSERT_TRUE (ok);
    ASSERT_TRUE (err.empty ());
}

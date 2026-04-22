#pragma once

#include <gmock/gmock.h>
#include "src/streamer/DX_LinkStreamer.h"

class MockDX_LinkStreamer : public DX_LinkStreamer
{
public:
    MockDX_LinkStreamer(TastyWorksClient& client)
        : DX_LinkStreamer(client)
    {
    }

    MOCK_METHOD(void, set_on_quote, (QuoteCallback qcb), (override));
    MOCK_METHOD(void, run, (), (override));
};
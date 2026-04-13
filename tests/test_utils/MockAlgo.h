#pragma once
#include <gmock/gmock.h>
#include "../../src/algo/algo/Algo.h"

class MockAlgo : public Algo {
public:
    MOCK_METHOD(Signal, generate_trading_signal, (const MarketQuote& qt), (override));
    MOCK_METHOD(void, process_quote, (), (override));
};
#pragma once
#include <gmock/gmock.h>
#include "src/tastyworks/TastyWorks.h"

class MockTastyWorksClient : public TastyWorksClient {
public:
    // Pass false to the constructor to prevent actual network calls
    MockTastyWorksClient() : TastyWorksClient(false) {}

    MOCK_METHOD(void, getSessionToken, (), (override));
    MOCK_METHOD(void, confirmUserAccountActive, (), (override));
    MOCK_METHOD(void, defineQuoteTokenStreamUrl, (), (override));
    MOCK_METHOD(std::string, getAPI_QuoteToken, (), (override));
    MOCK_METHOD(std::string, getDX_LinkUrl, (), (override));
    MOCK_METHOD(double, getAccountBalance, (), (override));
    MOCK_METHOD(double, getTradeableAmount, (), (override));
    MOCK_METHOD(size_t, getNumberAccountPositions, (), (override));
    MOCK_METHOD(void, submitOrder, (const nlohmann::json& body), (override));
    MOCK_METHOD(std::string, getAccountNumber, (), (override));
};
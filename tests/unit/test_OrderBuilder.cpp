#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include "../../src/log/Log.h"
#include "../../src/order/OrderBuilder.h"

class test_OrderBuilder : public ::testing::Test {
protected:
    void SetUp() override {
        Log::init("src/log/");
    }
};

TEST_F(test_OrderBuilder, BuildAllOrderComponentsJson_EmptyLegsProducesEmptyArray)
{
    static constexpr double ORDER_AMOUNT = 100.0;

    OrderBuilder builder;

    auto result = builder.timeInForce("GTC")
                         .orderType("Notional Market")
                         .amount(ORDER_AMOUNT)
                         .transactionType("Debit")
                         .dryRun(OrderBuilder::Mode::DryRun)
                         .buildAllOrderComponentsJson();

    EXPECT_EQ(result.at("time-in-force"), "GTC");
    EXPECT_EQ(result.at("order-type"), "Notional Market");
    EXPECT_DOUBLE_EQ(result.at("value").get<double>(), ORDER_AMOUNT);
    EXPECT_EQ(result.at("value-effect"), "Debit");

    ASSERT_TRUE(result.contains("legs"));
    ASSERT_TRUE(result.at("legs").is_array());
    EXPECT_EQ(result.at("legs").size(), 0U);
}

TEST_F(test_OrderBuilder, AddLeg_AddsOneLegWithExpectedFields)
{
    static constexpr double ORDER_AMOUNT = 50.0;

    OrderBuilder builder;

    builder.timeInForce("GTC")
           .orderType("Notional Market")
           .amount(ORDER_AMOUNT)
           .transactionType("Debit")
           .addLeg("Cryptocurrency", "BTC/USD", "Buy to Open")
           .dryRun(OrderBuilder::Mode::DryRun);

    auto result = builder.buildAllOrderComponentsJson();

    ASSERT_TRUE(result.at("legs").is_array());
    ASSERT_EQ(result.at("legs").size(), 1U);

    const auto& leg0 = result.at("legs").at(0);

    EXPECT_EQ(leg0.at("instrument-type"), "Cryptocurrency");
    EXPECT_EQ(leg0.at("symbol"), "BTC/USD");
    EXPECT_EQ(leg0.at("action"), "Buy to Open");
}

TEST_F(test_OrderBuilder, AddLeg_AddsMultipleLegsInOrder)
{
    static constexpr double ORDER_AMOUNT = 123.0;

    OrderBuilder builder;

    builder.timeInForce("GTC")
           .orderType("Notional Market")
           .amount(ORDER_AMOUNT)
           .transactionType("Debit")
           .addLeg("Equity", "AAPL", "Buy to Open")
           .addLeg("Equity", "MSFT", "Sell to Close")
           .dryRun(OrderBuilder::Mode::DryRun);

    auto result = builder.buildAllOrderComponentsJson();

    ASSERT_EQ(result.at("legs").size(), 2U);
    EXPECT_EQ(result.at("legs").at(0).at("symbol"), "AAPL");
    EXPECT_EQ(result.at("legs").at(1).at("symbol"), "MSFT");
}

TEST_F(test_OrderBuilder, BuildJsonToSubmitOrder_SetsDefaultsAndBuildsExpectedJson)
{
    static constexpr double ORDER_AMOUNT = 200.0;

    OrderBuilder builder;

    auto result = builder.buildJsonToSubmitOrder(
        ORDER_AMOUNT,
        "Cryptocurrency",
        "BTC/USD",
        "Buy to Open",
        OrderBuilder::Mode::DryRun
    );

    EXPECT_EQ(result.at("time-in-force"), "GTC");
    EXPECT_EQ(result.at("order-type"), "Notional Market");
    EXPECT_DOUBLE_EQ(result.at("value").get<double>(), ORDER_AMOUNT);
    EXPECT_EQ(result.at("value-effect"), "Debit");

    ASSERT_TRUE(result.at("legs").is_array());
    ASSERT_EQ(result.at("legs").size(), 1U);

    const auto& leg0 = result.at("legs").at(0);
    EXPECT_EQ(leg0.at("instrument-type"), "Cryptocurrency");
    EXPECT_EQ(leg0.at("symbol"), "BTC/USD");
    EXPECT_EQ(leg0.at("action"), "Buy to Open");
}

TEST_F(test_OrderBuilder, BuildJsonToSubmitOrder_ClearsExistingLegs)
{
    static constexpr double SEED_AMOUNT  = 1.0;
    static constexpr double ORDER_AMOUNT = 999.0;

    OrderBuilder builder;

    builder.timeInForce("OLD")
           .orderType("OLD")
           .amount(SEED_AMOUNT)
           .transactionType("OLD")
           .addLeg("Equity", "AAPL", "Buy to Open");

    auto result = builder.buildJsonToSubmitOrder(
        ORDER_AMOUNT,
        "Cryptocurrency",
        "BTC/USD",
        "Sell to Close",
        OrderBuilder::Mode::DryRun
    );

    ASSERT_TRUE(result.at("legs").is_array());
    ASSERT_EQ(result.at("legs").size(), 1U);

    const auto& leg0 = result.at("legs").at(0);
    EXPECT_EQ(leg0.at("instrument-type"), "Cryptocurrency");
    EXPECT_EQ(leg0.at("symbol"), "BTC/USD");
    EXPECT_EQ(leg0.at("action"), "Sell to Close");

    EXPECT_EQ(result.at("time-in-force"), "GTC");
    EXPECT_EQ(result.at("order-type"), "Notional Market");
    EXPECT_EQ(result.at("value-effect"), "Debit");
    EXPECT_DOUBLE_EQ(result.at("value").get<double>(), ORDER_AMOUNT);
}

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
    OrderBuilder b;

    auto j = b.timeInForce("GTC")
              .orderType("Notional Market")
              .amount(100.0)
              .transactionType("Debit")
              .dryRun(OrderBuilder::Mode::DryRun)
              .buildAllOrderComponentsJson();

    EXPECT_EQ(j.at("time-in-force"), "GTC");
    EXPECT_EQ(j.at("order-type"), "Notional Market");
    EXPECT_DOUBLE_EQ(j.at("value").get<double>(), 100.0);
    EXPECT_EQ(j.at("value-effect"), "Debit");

    ASSERT_TRUE(j.contains("legs"));
    ASSERT_TRUE(j.at("legs").is_array());
    EXPECT_EQ(j.at("legs").size(), 0u);
}

TEST_F(test_OrderBuilder, AddLeg_AddsOneLegWithExpectedFields)
{
    OrderBuilder b;

    b.timeInForce("GTC")
     .orderType("Notional Market")
     .amount(50.0)
     .transactionType("Debit")
     .addLeg("Cryptocurrency", "BTC/USD", "Buy to Open")
     .dryRun(OrderBuilder::Mode::DryRun);

    auto j = b.buildAllOrderComponentsJson();

    ASSERT_TRUE(j.at("legs").is_array());
    ASSERT_EQ(j.at("legs").size(), 1u);

    const auto& leg0 = j.at("legs").at(0);

    EXPECT_EQ(leg0.at("instrument-type"), "Cryptocurrency");
    EXPECT_EQ(leg0.at("symbol"), "BTC/USD");
    EXPECT_EQ(leg0.at("action"), "Buy to Open");
}

TEST_F(test_OrderBuilder, AddLeg_AddsMultipleLegsInOrder)
{
    OrderBuilder b;

    b.timeInForce("GTC")
     .orderType("Notional Market")
     .amount(123.0)
     .transactionType("Debit")
     .addLeg("Equity", "AAPL", "Buy to Open")
     .addLeg("Equity", "MSFT", "Sell to Close")
     .dryRun(OrderBuilder::Mode::DryRun);

    auto j = b.buildAllOrderComponentsJson();

    ASSERT_EQ(j.at("legs").size(), 2u);
    EXPECT_EQ(j.at("legs").at(0).at("symbol"), "AAPL");
    EXPECT_EQ(j.at("legs").at(1).at("symbol"), "MSFT");
}

TEST_F(test_OrderBuilder, BuildJsonToSubmitOrder_SetsDefaultsAndBuildsExpectedJson)
{
    OrderBuilder b;

    auto j = b.buildJsonToSubmitOrder(
        200.0,
        "Cryptocurrency",
        "BTC/USD",
        "Buy to Open",
        OrderBuilder::Mode::DryRun
    );

    EXPECT_EQ(j.at("time-in-force"), "GTC");
    EXPECT_EQ(j.at("order-type"), "Notional Market");
    EXPECT_DOUBLE_EQ(j.at("value").get<double>(), 200.0);
    EXPECT_EQ(j.at("value-effect"), "Debit");

    ASSERT_TRUE(j.at("legs").is_array());
    ASSERT_EQ(j.at("legs").size(), 1u);

    const auto& leg0 = j.at("legs").at(0);
    EXPECT_EQ(leg0.at("instrument-type"), "Cryptocurrency");
    EXPECT_EQ(leg0.at("symbol"), "BTC/USD");
    EXPECT_EQ(leg0.at("action"), "Buy to Open");
}

TEST_F(test_OrderBuilder, BuildJsonToSubmitOrder_ClearsExistingLegs)
{
    OrderBuilder b;

    // Seed the builder with a leg that should be cleared
    b.timeInForce("OLD")
     .orderType("OLD")
     .amount(1.0)
     .transactionType("OLD")
     .addLeg("Equity", "AAPL", "Buy to Open");

    // Now build a new order; this should clear legs_ and rebuild with exactly 1 leg
    auto j = b.buildJsonToSubmitOrder(
        999.0,
        "Cryptocurrency",
        "BTC/USD",
        "Sell to Close",
        OrderBuilder::Mode::DryRun
    );

    ASSERT_TRUE(j.at("legs").is_array());
    ASSERT_EQ(j.at("legs").size(), 1u);

    const auto& leg0 = j.at("legs").at(0);
    EXPECT_EQ(leg0.at("instrument-type"), "Cryptocurrency");
    EXPECT_EQ(leg0.at("symbol"), "BTC/USD");
    EXPECT_EQ(leg0.at("action"), "Sell to Close");

    // Confirm defaults override prior state
    EXPECT_EQ(j.at("time-in-force"), "GTC");
    EXPECT_EQ(j.at("order-type"), "Notional Market");
    EXPECT_EQ(j.at("value-effect"), "Debit");
    EXPECT_DOUBLE_EQ(j.at("value").get<double>(), 999.0);
}
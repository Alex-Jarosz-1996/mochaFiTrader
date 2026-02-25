#include <gtest/gtest.h>
#include <string_view>

#include "../../src/order/OrderType.h"

TEST(test_OrderType, ExecuteConstants_HaveExpectedValues)
{
    EXPECT_EQ(Execute::BuyToOpen,  std::string_view("Buy to Open"));
    EXPECT_EQ(Execute::SellToClose, std::string_view("Sell to Close"));
}

TEST(test_OrderType, ExecuteConstants_AreDistinct)
{
    EXPECT_NE(Execute::BuyToOpen, Execute::SellToClose);
}
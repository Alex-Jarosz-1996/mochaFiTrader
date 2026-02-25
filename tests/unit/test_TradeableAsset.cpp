#include <gtest/gtest.h>
#include "../../src/config/Config.h"
#include "../../src/model/TradeableAsset.h"

TEST(test_TradeableAsset, CanConstructTradeableAsset)
{
    EXPECT_NO_THROW({
        TradeableAsset a = TradeableAsset::constructTradeableAsset();
        EXPECT_FALSE(a.symbol.empty());
        EXPECT_FALSE(a.instrument.empty());
    });
}

#include <gtest/gtest.h>
#include "src/config/Config.h"
#include "src/model/TradeableAsset.h"

TEST(test_TradeableAsset, CanConstructTradeableAsset)
{
    EXPECT_NO_THROW({
        TradeableAsset asset = TradeableAsset::constructTradeableAsset();
        EXPECT_FALSE(asset.symbol.empty());
        EXPECT_FALSE(asset.instrument.empty());
    });
}

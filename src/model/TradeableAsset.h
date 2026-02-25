#pragma once
#include <string>

#include "../../src/config/Config.h"

struct TradeableAsset {
    std::string symbol;
    std::string instrument;

    static TradeableAsset constructTradeableAsset()
    {
        TradeableAsset trAsset;

        trAsset.symbol = Config::get_config_value("TRADEABLE_ASSETS");
        trAsset.instrument = Config::get_config_value("INSTRUMENT");

        if (trAsset.symbol.empty())
        {
            throw std::runtime_error("SYMBOL missing in config");
        }

        if (trAsset.instrument.empty())
        {
            throw std::runtime_error("INSTRUMENT missing in config");
        }

        return trAsset;
    }
};

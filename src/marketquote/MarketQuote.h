#pragma once

#include <string>
#include <optional>
#include <chrono>
#include <iostream>
#include <typeinfo>

struct MarketQuote
{
    std::string symbol;

    std::optional<double> price;
    std::optional<double> bidPrice;
    std::optional<double> askPrice;

    std::optional<double> dayVolume;
    std::optional<double> size;

    std::optional<double> bidSize;
    std::optional<double> askSize;
};

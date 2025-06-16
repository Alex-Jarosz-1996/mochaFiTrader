#pragma once

#include <string>
#include <optional>
#include <chrono>

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

    std::chrono::system_clock::time_point timestamp;

    MarketQuote() : timestamp(std::chrono::system_clock::now()) {}

    void print_summary() const
    {
        std::cout << "=== Market Quote Summary ===" << std::endl;
        std::cout << "Symbol     : " << symbol << std::endl;
        std::cout << "Price      : " << (price     ? std::to_string(*price)     : "n/a") << std::endl;
        std::cout << "Bid Price  : " << (bidPrice  ? std::to_string(*bidPrice)  : "n/a") << std::endl;
        std::cout << "Ask Price  : " << (askPrice  ? std::to_string(*askPrice)  : "n/a") << std::endl;
        std::cout << "Volume     : " << (dayVolume ? std::to_string(*dayVolume) : "n/a") << std::endl;
        std::cout << "Size       : " << (size      ? std::to_string(*size)      : "n/a") << std::endl;
        std::cout << "Bid Size   : " << (bidSize   ? std::to_string(*bidSize)   : "n/a") << std::endl;
        std::cout << "Ask Size   : " << (askSize   ? std::to_string(*askSize)   : "n/a") << std::endl;
        std::cout << "=============================" << std::endl;
    }
};

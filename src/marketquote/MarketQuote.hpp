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

    void print_summary() const
    {
        std::cout << "=== Market Quote Summary ===" << std::endl;
        std::cout << "Symbol        : " << symbol << std::endl;
        std::cout << "Symbol (T)    : " << typeid(symbol).name() << std::endl;
        std::cout << "Price         : " << (price ? std::to_string(*price) : "N/A") << std::endl;
        std::cout << "Price (T)     : " << typeid(price).name() << std::endl;
        std::cout << "Bid Price     : " << (bidPrice ? std::to_string(*bidPrice) : "N/A") << std::endl;
        std::cout << "Bid Price (T) : " << typeid(bidPrice).name() << std::endl;
        std::cout << "Ask Price     : " << (askPrice ? std::to_string(*askPrice) : "N/A") << std::endl;
        std::cout << "Ask Price (T) : " << typeid(askPrice).name() << std::endl;
        std::cout << "Volume        : " << (dayVolume ? std::to_string(*dayVolume) : "N/A") << std::endl;
        std::cout << "Volume (T)    : " << typeid(dayVolume).name() << std::endl;
        std::cout << "Size          : " << (size ? std::to_string(*size) : "N/A") << std::endl;
        std::cout << "Size (T)      : " << typeid(size).name() << std::endl;
        std::cout << "Bid Size      : " << (bidSize ? std::to_string(*bidSize) : "N/A") << std::endl;
        std::cout << "Bid Size (T)  : " << typeid(bidSize).name() << std::endl;
        std::cout << "Ask Size      : " << (askSize ? std::to_string(*askSize) : "N/A") << std::endl;
        std::cout << "Ask Size (T)  : " << typeid(askSize).name() << std::endl;
        std::cout << "=============================" << std::endl;
    }
};

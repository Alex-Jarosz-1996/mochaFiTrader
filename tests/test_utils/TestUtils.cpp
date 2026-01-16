#include <iostream>
#include <string>

#include "TestUtils.h"

MarketQuote make_unknown(const std::string& sym)
{
    MarketQuote q;
    q.symbol = sym;
    return q;
}

MarketQuote make_trade(const std::string& sym, double price)
{
    MarketQuote q;
    q.symbol = sym;
    q.price = price;
    // bid/ask intentionally empty
    return q;
}

MarketQuote make_quote(const std::string& sym,
                       double bid,
                       double ask,
                       double bidSize,
                       double askSize)
{
    MarketQuote q;
    q.symbol = sym;
    q.bidPrice = bid;
    q.askPrice = ask;
    q.bidSize = bidSize;
    q.askSize = askSize;
    // price intentionally empty
    return q;
}

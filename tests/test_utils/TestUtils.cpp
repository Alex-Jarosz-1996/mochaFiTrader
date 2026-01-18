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

MarketQuote make_quote_with_imbalance(const std::string& sym,
                                      double midPrice,
                                      double spread,
                                      double totalSize,
                                      double imbalance_norm)
{
    // clamp imbalance into [-0.999, 0.999] to keep sizes positive
    const double I = std::max(-0.999, std::min(0.999, imbalance_norm));

    // b+a=T and (b-a)/(b+a)=I => b=T*(1+I)/2, a=T*(1-I)/2
    const double bidSize = totalSize * (1.0 + I) * 0.5;
    const double askSize = totalSize * (1.0 - I) * 0.5;

    const double bid = midPrice - spread * 0.5;
    const double ask = midPrice + spread * 0.5;

    return make_quote(sym, bid, ask, bidSize, askSize);
}

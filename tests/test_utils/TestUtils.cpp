#include <iostream>
#include <string>
#include <algorithm>
#include <cmath>

#include "TestUtils.h"

auto make_unknown(const std::string& sym) -> MarketQuote
{
    MarketQuote mkt;
    mkt.symbol = sym;
    return mkt;
}

auto make_trade(const std::string& sym, double price) -> MarketQuote
{
    MarketQuote mkt;
    mkt.symbol = sym;
    mkt.price = price;
    // bid/ask intentionally empty
    return mkt;
}

auto make_quote(const std::string& sym, QuoteParams params) -> MarketQuote
{
    MarketQuote mkt;
    mkt.symbol   = sym;
    mkt.bidPrice = params.bid;
    mkt.askPrice = params.ask;
    mkt.bidSize  = params.bidSize;
    mkt.askSize  = params.askSize;
    // price intentionally empty
    return mkt;
}

auto make_quote_with_imbalance(const std::string& sym, ImbalanceQuoteParams params) -> MarketQuote
{
    // clamp imbalance into [-0.999, 0.999] to keep sizes positive
    static constexpr double IMB_MIN  = -0.999;
    static constexpr double IMB_MAX  =  0.999;
    static constexpr double HALF     =  0.5;

    const double imb_clamped = std::max(IMB_MIN, std::min(IMB_MAX, params.imbalanceNorm));

    // b+a=T and (b-a)/(b+a)=I => b=T*(1+I)/2, a=T*(1-I)/2
    const double bid_size = params.totalSize * (1.0 + imb_clamped) * HALF;
    const double ask_size = params.totalSize * (1.0 - imb_clamped) * HALF;

    const double bid = params.midPrice - params.spread * HALF;
    const double ask = params.midPrice + params.spread * HALF;

    return make_quote(sym, QuoteParams{bid, ask, bid_size, ask_size});
}

auto make_mid_quote(const std::string& sym, MidQuoteParams params) -> MarketQuote
{
    static constexpr double HALF = 0.5;
    const double bid = params.mid - params.spread * HALF;
    const double ask = params.mid + params.spread * HALF;
    return make_quote(sym, QuoteParams{bid, ask, params.bidSize, params.askSize});
}

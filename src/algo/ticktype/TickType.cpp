#include "TickType.h"

#include "../../../src/marketquote/MarketQuote.h"

auto get_tick_type(const MarketQuote& mkt_quote) -> TickType
{
    // considering TickType::Trade; price
    if (mkt_quote.price.has_value())
    {
        return TickType::Trade;
    }

    // considering TickType::Quote; bid_price, ask_price, bid_size, ask_size
    if (
        mkt_quote.bidPrice.has_value()
        && mkt_quote.askPrice.has_value()
        && mkt_quote.bidSize.has_value()
        && mkt_quote.askSize.has_value()
    )
    {
        return TickType::Quote;
    }

    return TickType::Unknown;
}
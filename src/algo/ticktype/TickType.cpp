#include "TickType.h"

#include "../../../src/marketquote/MarketQuote.h"

TickType get_tick_type(const MarketQuote& quote)
{
    // considering TickType::Trade; price
    if (quote.price.has_value())
    {
        return TickType::Trade;
    }
    
    // considering TickType::Quote; bid_price, ask_price, bid_size, ask_size
    if (
        quote.bidPrice.has_value() 
        && quote.askPrice.has_value() 
        && quote.bidSize.has_value() 
        && quote.askSize.has_value()
    )
    {
        return TickType::Quote;
    }
    
    return TickType::Unknown;
}
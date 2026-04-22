#ifndef TICKTYPE_H
#define TICKTYPE_H

#include "src/marketquote/MarketQuote.h"

enum class TickType
{
    Trade,
    Quote,
    Unknown
};

TickType get_tick_type(const MarketQuote& mkt_quote);


#endif // TICKTYPE_H
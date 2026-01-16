#ifndef TICKTYPE_H
#define TICKTYPE_H

#include "../marketquote/MarketQuote.h"

enum class TickType
{
    Trade,
    Quote,
    Unknown
};

TickType get_tick_type(const MarketQuote& quote);


#endif // TICKTYPE_H
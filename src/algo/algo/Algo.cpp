#include "Algo.h"
#include "../Signal.h"
#include "../TickType.h"

#include <optional>
#include <vector>

void Algo::ingest_quote(const std::vector<MarketQuote>& qt)
{
    buffer = qt;
}

bool Algo::is_genuine_transition(const MarketQuote& prev,
                                 const MarketQuote& curr)
{
    
    TickType Q = TickType::Quote;
    TickType T = TickType::Trade;
    TickType U = TickType::Unknown;
    
    TickType prevT = get_tick_type(prev);
    TickType currT = get_tick_type(curr);

    // reject TickType::Unknown
    if (currT == U)
    {
        return false;
    }

    // reject bid > ask
    if (curr.bidPrice && curr.askPrice &&
        curr.bidPrice.value() > curr.askPrice.value())
    {
        return false;
    }

    // map TickType::Trade -> TickType::Trade and validate
    // based on change in price values <= 10%
    if (prevT == T && currT == T)
    {
        double p_prev = prev.price.value();
        double p_curr = curr.price.value();

        double pct_change = std::abs(p_curr - p_prev) / p_prev;
        if (pct_change > 0.10)  // reject: 10% price jump
        {
            return false;
        }
    }
    
    // map TickType::Quote -> TickType::Quote and validate
    if (prevT == Q && currT == Q)
    {
        double bid_ask_spread = curr.askPrice.value() - curr.bidPrice.value();
        // checking that askPrice > bidPrice
        if (bid_ask_spread <= 0)
        {
            return false;
        }
        
        // checking askPrice and bidPrice spread
        if (bid_ask_spread > curr.bidPrice.value() * 0.20)
        {
            return false;
        }

        // checking that askSize > 0 and bidSize > 0
        if (curr.bidSize <= 0 || curr.askSize <= 0)
        {
            return false;
        }
    }

    return true;
}

void Algo::updated_buffer()
{
    std::vector<MarketQuote> cleaned;
    cleaned.reserve(buffer.size());

    cleaned.push_back(buffer[0]);

    for (size_t i = 1; i < buffer.size(); i++)
    {
        const auto& prev = cleaned.back();
        const auto& curr = buffer[i];

        if (is_genuine_transition(prev, curr))
            cleaned.push_back(curr);
        else
            std::cout << "Removed abnormal tick at index: " << i << "\n";
    }

    buffer = cleaned;
}

void Algo::clear_buffer()
{
    buffer.clear();
}
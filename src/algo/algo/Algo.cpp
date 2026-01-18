#include "Algo.h"
#include "../Signal.h"
#include "../ticktype/TickType.h"

#include <optional>
#include <vector>

void Algo::ingest_quote(const MarketQuote& qt)
{
    if (curr_raw) prev_raw = curr_raw;
    curr_raw = qt;
}

bool Algo::is_genuine_transition(const MarketQuote& prev,
                                 const MarketQuote& curr)
{
    
    const TickType Q = TickType::Quote;
    const TickType T = TickType::Trade;
    const TickType U = TickType::Unknown;
    
    const TickType prevT = get_tick_type(prev);
    const TickType currT = get_tick_type(curr);

    // reject TickType::Unknown
    if (currT == U) return false;

    // reject bid > ask
    if (curr.bidPrice && curr.askPrice &&
        curr.bidPrice.value() > curr.askPrice.value()) return false;

    // map TickType::Trade -> TickType::Trade and validate
    // based on change in price values <= 10%
    if (prevT == T && currT == T)
    {
        double p_prev = prev.price.value();
        double p_curr = curr.price.value();

        if (p_prev == 0.0 || p_curr == 0.0) return false;

        double pct_change = std::abs(p_curr - p_prev) / std::abs(p_prev);
        
        // reject: 15% price jump
        if (pct_change > 0.15) return false;
    }
    
    // map TickType::Quote -> TickType::Quote and validate
    if (prevT == Q && currT == Q)
    {
        // ensuring we have bidPrice / askPrice, and bidSize / askSize
        if (!curr.bidPrice || !curr.askPrice) return false;
        if (curr.bidPrice.value() < 0 || curr.askPrice.value() < 0) return false;
        
        if (!curr.bidSize || !curr.askSize) return false;
        if (curr.bidSize.value() < 0 || curr.askSize.value() < 0) return false;
        
        // considering bid / ask spread
        double bid_ask_spread = curr.askPrice.value() - curr.bidPrice.value();
        
        // checking that askPrice > bidPrice
        if (bid_ask_spread <= 0) return false;
        
        // checking askPrice and bidPrice spread
        if (bid_ask_spread > curr.bidPrice.value() * 0.20) return false;

    }

    return true;
}


Signal Algo::generate_trading_signal(const MarketQuote& qt)
{
    ingest_quote(qt);
    
    // need at least one raw packet to start
    if (!curr_raw)
    {
        sig = Signal::HOLD;
        return sig;
    }

    // seeding prev_valid if it has not been set yet
    if (!prev_valid)
    {
        prev_valid = curr_raw;
        valid_count = 0;
        sig = Signal::HOLD;
        return sig;
    }

    // validate current raw packet against last accepted (prev_valid)
    if (!is_genuine_transition(*prev_valid, *curr_raw)) {
        sig = Signal::HOLD;
        return sig;
    }

    // accepted new packet, update counter
    ++valid_count;

    // processing quote
    process_quote();

    // now that we've updated state based on curr_raw, promote to prev_valid
    prev_valid = curr_raw;

    return sig;
}
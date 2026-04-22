#include "Algo.h"
#include "src/algo/Signal.h"
#include "src/algo/ticktype/TickType.h"

#include <optional>
#include <vector>

static constexpr double PRICE_JUMP_THRESHOLD = 0.15;
static constexpr double MAX_BID_ASK_SPREAD_RATIO = 0.20;

static auto is_valid_trade_transition(const MarketQuote& prev, const MarketQuote& curr) -> bool
{
    if (!prev.price || !curr.price) return false;

    const double p_prev = prev.price.value();
    const double p_curr = curr.price.value();

    if (p_prev == 0.0 || p_curr == 0.0) return false;

    const double pct_change = std::abs(p_curr - p_prev) / std::abs(p_prev);
    return pct_change <= PRICE_JUMP_THRESHOLD;
}

static auto is_valid_quote_transition(const MarketQuote& curr) -> bool
{
    if (!curr.bidPrice || !curr.askPrice) return false;
    if (curr.bidPrice.value() < 0 || curr.askPrice.value() < 0) return false;

    if (!curr.bidSize || !curr.askSize) return false;
    if (curr.bidSize.value() < 0 || curr.askSize.value() < 0) return false;

    const double bid_ask_spread = curr.askPrice.value() - curr.bidPrice.value();
    if (bid_ask_spread <= 0) return false;

    return bid_ask_spread <= curr.bidPrice.value() * MAX_BID_ASK_SPREAD_RATIO;
}

void Algo::ingest_quote(const MarketQuote& mkt_quote)
{
    if (curr_raw) prev_raw = curr_raw;
    curr_raw = mkt_quote;
}

auto Algo::is_genuine_transition(const MarketQuote& prev,
                                 const MarketQuote& curr) -> bool
{
    const TickType prevT = get_tick_type(prev);
    const TickType currT = get_tick_type(curr);

    if (currT == TickType::Unknown) return false;

    if (curr.bidPrice && curr.askPrice &&
        curr.bidPrice.value() > curr.askPrice.value()) return false;

    if (prevT == TickType::Trade && currT == TickType::Trade)
        return is_valid_trade_transition(prev, curr);

    if (prevT == TickType::Quote && currT == TickType::Quote)
        return is_valid_quote_transition(curr);

    return true;
}


auto Algo::generate_trading_signal(const MarketQuote& mkt_quote) -> Signal
{
    ingest_quote(mkt_quote);
    
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

    // processing mkt_quote
    process_quote();

    // now that we've updated state based on curr_raw, promote to prev_valid
    prev_valid = curr_raw;

    return sig;
}
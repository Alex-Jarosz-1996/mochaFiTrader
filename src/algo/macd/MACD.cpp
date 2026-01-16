#include "MACD.h"
#include "../Signal.h"
#include "../../marketquote/MarketQuote.h"
#include "../../algo/ticktype/TickType.h"

#include <optional>
#include <vector>

MACD::MACD(int fast, int slow, int signal)
{
    k_fast = 2.0 / (fast + 1);
    k_slow = 2.0 / (slow + 1);
    k_signal = 2.0 / (signal + 1);
    
    trigger_window = 3 * slow;
}

std::optional<double> MACD::extract_price_for_macd(const MarketQuote& q) const
{
    const TickType t = get_tick_type(q);

    if (t == TickType::Quote)
    {
        if (q.bidPrice && q.askPrice)
        {
            return (q.bidPrice.value() + q.askPrice.value()) * 0.5; // mid
        }
        return std::nullopt;
    }

    if (t == TickType::Trade)
    {
        if (q.price) return q.price.value();
        return std::nullopt;
    }

    return std::nullopt;
}

void MACD::process_quote()
{
    if (!curr_raw)
    {
        sig = Signal::HOLD;
        return;
    }

    std::optional<double> pxOpt = extract_price_for_macd(*curr_raw);
    if (!pxOpt)
    {
        sig = Signal::HOLD;
        return;
    }

    const double price = *pxOpt;

    // initialise EMA seeds on first accepted usable price
    if (!initialised)
    {
        ema_fast = price;
        ema_slow = price;

        macd = 0.0;
        ema_signal = 0.0;

        prev_macd = macd;
        prev_signal = ema_signal;

        initialised = true;
        sig = Signal::HOLD;
    }

    // incremental ema updates
    ema_fast = k_fast * price + (1.0 - k_fast) * ema_fast;
    ema_slow = k_slow * price + (1.0 - k_slow) * ema_slow;

    prev_macd = macd;
    macd = ema_fast - ema_slow;

    prev_signal = ema_signal;
    ema_signal = k_signal * macd + (1.0 - k_signal) * ema_signal;

    if (valid_count < trigger_window)
    {
        sig = Signal::HOLD;
        return;
    }

    // crossover logic
    const bool was_below = prev_macd < prev_signal;
    const bool now_above = macd > ema_signal;

    const bool was_above = prev_macd > prev_signal;
    const bool now_below = macd < ema_signal;

    if (was_below && now_above) {
        sig = Signal::BUY;
    } else if (was_above && now_below) {
        sig = Signal::SELL;
    } else {
        sig = Signal::HOLD;
    }
}

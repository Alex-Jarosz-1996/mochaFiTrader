#include "MACD.h"
#include "src/algo/Signal.h"
#include "src/marketquote/MarketQuote.h"
#include "src/algo/ticktype/TickType.h"

#include <optional>
#include <vector>

static constexpr double EMA_MULTIPLIER = 2.0;
static constexpr double MID_PRICE_FACTOR = 0.5;

MACD::MACD(MACDParams params)
    : k_fast(EMA_MULTIPLIER / (params.fast + 1)),
      k_slow(EMA_MULTIPLIER / (params.slow + 1)),
      k_signal(EMA_MULTIPLIER / (params.signal + 1)),
      trigger_window(3 * params.slow)
{
}

auto MACD::extract_price_for_macd(const MarketQuote& mkt_quote) -> std::optional<double>
{
    const TickType t_type = get_tick_type(mkt_quote);

    if (t_type == TickType::Quote)
    {
        if (mkt_quote.bidPrice && mkt_quote.askPrice)
        {
            return (mkt_quote.bidPrice.value() + mkt_quote.askPrice.value()) * MID_PRICE_FACTOR; // mid
        }
        return std::nullopt;
    }

    if (t_type == TickType::Trade)
    {
        if (mkt_quote.price) return mkt_quote.price.value();
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

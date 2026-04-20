#include <cmath>
#include "RSI.h"

static constexpr double MID_PRICE_FACTOR = 0.5;
static constexpr double RSI_NEUTRAL = 50.0;

RSI::RSI(RSIParams params)
    : period(params.period), oversold(params.oversold), overbought(params.overbought)
{
}

auto RSI::extract_trade_price(const MarketQuote& mkt_quote) -> std::optional<double>
{
    if (mkt_quote.price && std::isfinite(mkt_quote.price.value()))
        return mkt_quote.price.value();
    return std::nullopt;
}

auto RSI::extract_mid_price(const MarketQuote& mkt_quote) -> std::optional<double>
{
    if (mkt_quote.bidPrice && mkt_quote.askPrice)
    {
        const double bid_price = mkt_quote.bidPrice.value();
        const double ask_price = mkt_quote.askPrice.value();
        if (std::isfinite(bid_price) && std::isfinite(ask_price) && ask_price >= bid_price)
            return (ask_price + bid_price) * MID_PRICE_FACTOR;
    }
    return std::nullopt;
}

auto RSI::extract_price_for_rsi(const MarketQuote& mkt_quote) -> std::optional<double>
{
    if (mkt_quote.price && std::isfinite(mkt_quote.price.value()))
    {
        return extract_trade_price(mkt_quote);
    }

    if (mkt_quote.bidPrice && mkt_quote.askPrice)
    {
        return extract_mid_price(mkt_quote);
    }

    return std::nullopt;
}

void RSI::update_rsi(double price)
{
    if (!prev_price)
    {
        prev_price = price;
        prev_rsi = rsi;
        rsi = RSI_NEUTRAL;
        return;
    }

    const double change = price - prev_price.value();
    const double gain = (change > 0.0) ? change : 0.0;
    const double loss = (change < 0.0) ? -change : 0.0;

    prev_rsi = rsi;

    if (!seeded)
    {
        seed_gain_sum += gain;
        seed_loss_sum += loss;
        seed_count++;

        // Need period_ samples of changes, which requires period_+1 prices
        if (seed_count >= period)
        {
            avg_gain = seed_gain_sum / period;
            avg_loss = seed_loss_sum / period;
            seeded = true;
        }
    }
    else
    {
        // Wilder smoothing
        avg_gain = (avg_gain * (period - 1) + gain) / period;
        avg_loss = (avg_loss * (period - 1) + loss) / period;
    }

    if (seeded)
    {
        if (avg_loss == 0.0)
        {
            rsi = 100.0; // no losses
        }
        else
        {
            const double rs_ratio = avg_gain / avg_loss;
            rsi = 100.0 - (100.0 / (1.0 + rs_ratio));
        }
    }
    else
    {
        rsi = RSI_NEUTRAL; // during seed
    }

    prev_price = price;
}

auto RSI::crossed_up(double level) const -> bool
{
    return (prev_rsi < level) && (rsi >= level);
}

auto RSI::crossed_down(double level) const -> bool
{
    return (prev_rsi > level) && (rsi <= level);
}

void RSI::process_quote()
{
    if (!curr_raw)
    {
        sig = Signal::HOLD;
        return;
    }

    const MarketQuote& mkt_quote = *curr_raw;

    // only update RSI if we can extract a usable price
    auto price = extract_price_for_rsi(mkt_quote);
    if (!price)
    {
        sig = Signal::HOLD;
        return;
    }

    update_rsi(*price);

    // Warmup gating:
    // - valid_count is Algo’s accepted packet count
    // - RSI needs at least period changes seeded => roughly period+1 accepted price updates
    // We'll be conservative: require valid_count >= period + 1
    if (valid_count < period + 1)
    {
        sig = Signal::HOLD;
        return;
    }

    if (!seeded)
    {
        sig = Signal::HOLD;
        return;
    }

    // Signal logic: crossing thresholds
    if (crossed_up(oversold))
    {
        sig = Signal::BUY;
        return;
    }

    if (crossed_down(overbought))
    {
        sig = Signal::SELL;
        return;
    }

    sig = Signal::HOLD;
}
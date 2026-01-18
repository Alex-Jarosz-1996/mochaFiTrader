#include <cmath>
#include "RSI.h"

RSI::RSI(int period, double oversold, double overbought)
    : period(period), oversold(oversold), overbought(overbought)
{
}

std::optional<double> RSI::extract_trade_price(const MarketQuote& q) const
{
    if (q.price && std::isfinite(q.price.value()))
        return q.price.value();
    return std::nullopt;
}

std::optional<double> RSI::extract_mid_price(const MarketQuote& q) const
{
    if (q.bidPrice && q.askPrice)
    {
        const double b = q.bidPrice.value();
        const double a = q.askPrice.value();
        if (std::isfinite(b) && std::isfinite(a) && a >= b)
            return (a + b) * 0.5;
    }
    return std::nullopt;
}

std::optional<double> RSI::extract_price_for_rsi(const MarketQuote& q) const
{
    if (q.price && std::isfinite(q.price.value()))
    {
        return extract_trade_price(q);
    }

    if (q.bidPrice && q.askPrice)
    {
        return extract_mid_price(q);
    }

    return std::nullopt;
}

void RSI::update_rsi(double price)
{
    if (!prev_price)
    {
        prev_price = price;
        prev_rsi = rsi;
        rsi = 50.0;
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
            const double rs = avg_gain / avg_loss;
            rsi = 100.0 - (100.0 / (1.0 + rs));
        }
    }
    else
    {
        rsi = 50.0; // during seed
    }

    prev_price = price;
}

bool RSI::crossed_up(double level) const
{
    return (prev_rsi < level) && (rsi >= level);
}

bool RSI::crossed_down(double level) const
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

    const MarketQuote& q = *curr_raw;

    // only update RSI if we can extract a usable price
    auto px = extract_price_for_rsi(q);
    if (!px)
    {
        sig = Signal::HOLD;
        return;
    }

    update_rsi(*px);

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
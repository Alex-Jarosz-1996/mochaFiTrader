#include <cmath>
#include "VMACD.h"

VMACD::VMACD(int fast, int slow, int signal)
{
    k_fast = 2.0 / (fast + 1.0);
    k_slow = 2.0 / (slow + 1.0);
    k_signal = 2.0 / (signal + 1.0);

    trigger_window = 3 * slow;
}

std::optional<double> VMACD::extract_trade_price(const MarketQuote& q) const
{
    if (q.price && std::isfinite(q.price.value()))
    {
        return q.price.value();
    }
    return std::nullopt;
}

std::optional<double> VMACD::extract_mid_price(const MarketQuote& q) const
{
    if (q.bidPrice && q.askPrice)
    {
        const double b = q.bidPrice.value();
        const double a = q.askPrice.value();
        if (std::isfinite(b) && std::isfinite(a) && a >= b)
        {
            return (a + b) * 0.5;
        }
    }
    return std::nullopt;
}

std::optional<double> VMACD::extract_price_for_macd(const MarketQuote& q) const
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

std::optional<double> VMACD::extract_normalized_imbalance(const MarketQuote& q) const
{
    if (!q.bidSize || !q.askSize) return std::nullopt;

    const double bid = q.bidSize.value();
    const double ask = q.askSize.value();

    if (!std::isfinite(bid) || !std::isfinite(ask)) return std::nullopt;
    if (bid < 0.0 || ask < 0.0) return std::nullopt;

    const double denominator = bid + ask;
    if (denominator <= 0.0) return std::nullopt;

    const double numerator = bid - ask;

    return numerator / denominator; // [-1, 1]
}

void VMACD::update_price_macd(double price)
{
    if (!price_init)
    {
        ema_fast_p = price;
        ema_slow_p = price;
        macd_p = 0.0;
        ema_sig_p = 0.0;
        prev_macd_p = macd_p;
        prev_sig_p = ema_sig_p;
        price_init = true;
        return;
    }

    ema_fast_p = k_fast * price + (1.0 - k_fast) * ema_fast_p;
    ema_slow_p = k_slow * price + (1.0 - k_slow) * ema_slow_p;

    prev_macd_p = macd_p;
    macd_p = ema_fast_p - ema_slow_p;

    prev_sig_p = ema_sig_p;
    ema_sig_p = k_signal * macd_p + (1.0 - k_signal) * ema_sig_p;
}

void VMACD::update_imbalance_macd(double imb)
{
    if (!imb_init)
    {
        ema_fast_i = imb;
        ema_slow_i = imb;
        macd_i = 0.0;
        ema_sig_i = 0.0;
        prev_macd_i = macd_i;
        prev_sig_i = ema_sig_i;
        imb_init = true;
        return;
    }

    ema_fast_i = k_fast * imb + (1.0 - k_fast) * ema_fast_i;
    ema_slow_i = k_slow * imb + (1.0 - k_slow) * ema_slow_i;

    prev_macd_i = macd_i;
    macd_i = ema_fast_i - ema_slow_i;

    prev_sig_i = ema_sig_i;
    ema_sig_i = k_signal * macd_i + (1.0 - k_signal) * ema_sig_i;
}

bool VMACD::price_bull_cross() const
{
    // Cross bullish: was below, now above
    return (prev_macd_p < prev_sig_p) && (macd_p > ema_sig_p);
}

bool VMACD::price_bear_cross() const
{
    // Cross bearish: was above, now below
    return (prev_macd_p > prev_sig_p) && (macd_p< ema_sig_p);
}

bool VMACD::imb_bullish() const
{
    return macd_i > ema_sig_i && std::abs(macd_i) > imb_strength;
}

bool VMACD::imb_bearish() const
{
    return macd_i < ema_sig_i && std::abs(macd_i) > imb_strength;
}

void VMACD::process_quote()
{
    if (!curr_raw)
    {
        sig = Signal::HOLD;
        return;
    }

    const MarketQuote& q = *curr_raw;

    // update the imbalance MACD only when sizes exist
    if (std::optional<double> imb = extract_normalized_imbalance(q))
    {
        update_imbalance_macd(*imb);
    }

    // update the price MACD when we can extract a price
    if (std::optional<double> px = extract_price_for_macd(q))
    {
        update_price_macd(*px);
    }

    // need both series initialised before we act
    if (!price_init || !imb_init)
    {
        sig = Signal::HOLD;
        return;
    }

    // warmup based on accepted/genuine ticks (streaming)
    if (valid_count < trigger_window)
    {
        sig = Signal::HOLD;
        return;
    }

    // decision rule: price crossover confirmed by imbalance regime
    if (price_bull_cross() && imb_bullish())
    {
        sig = Signal::BUY;
        return;
    }

    if (price_bear_cross() && imb_bearish())
    {
        sig = Signal::SELL;
        return;
    }

    sig = Signal::HOLD;
}
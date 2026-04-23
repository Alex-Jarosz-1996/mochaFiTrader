#include <cmath>
#include "VMACD.h"
#include "src/config/Config.h"

static constexpr double EMA_MULTIPLIER = 2.0;
static constexpr double MID_PRICE_FACTOR = 0.5;

VMACD::VMACD(VMACDParams params)
    : k_fast(EMA_MULTIPLIER / (params.fast + 1.0)),
      k_slow(EMA_MULTIPLIER / (params.slow + 1.0)),
      k_signal(EMA_MULTIPLIER / (params.signal + 1.0)),
      trigger_window(3 * params.slow)
{
    imb_strength = Config::get_strategy_value("vmacd", "IMB_STRENGTH");
}

auto VMACD::extract_trade_price(const MarketQuote& mkt_quote) -> std::optional<double>
{
    if (mkt_quote.price && std::isfinite(mkt_quote.price.value()))
    {
        return mkt_quote.price.value();
    }
    return std::nullopt;
}

auto VMACD::extract_mid_price(const MarketQuote& mkt_quote) -> std::optional<double>
{
    if (mkt_quote.bidPrice && mkt_quote.askPrice)
    {
        const double bid = mkt_quote.bidPrice.value();
        const double ask = mkt_quote.askPrice.value();
        if (std::isfinite(bid) && std::isfinite(ask) && ask >= bid)
        {
            return (ask + bid) * MID_PRICE_FACTOR;
        }
    }
    return std::nullopt;
}

auto VMACD::extract_price_for_macd(const MarketQuote& mkt_quote) -> std::optional<double>
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

auto VMACD::extract_normalized_imbalance(const MarketQuote& mkt_quote) -> std::optional<double>
{
    if (!mkt_quote.bidSize || !mkt_quote.askSize) return std::nullopt;

    const double bid = mkt_quote.bidSize.value();
    const double ask = mkt_quote.askSize.value();

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

auto VMACD::price_bull_cross() const -> bool
{
    // Cross bullish: was below, now above
    return (prev_macd_p < prev_sig_p) && (macd_p > ema_sig_p);
}

auto VMACD::price_bear_cross() const -> bool
{
    // Cross bearish: was above, now below
    return (prev_macd_p > prev_sig_p) && (macd_p< ema_sig_p);
}

auto VMACD::imb_bullish() const -> bool
{
    return macd_i > ema_sig_i && std::abs(macd_i) > imb_strength;
}

auto VMACD::imb_bearish() const -> bool
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

    const MarketQuote& mkt_quote = *curr_raw;

    // update the imbalance MACD only when sizes exist
    if (std::optional<double> imb = extract_normalized_imbalance(mkt_quote))
    {
        update_imbalance_macd(*imb);
    }

    // update the price MACD when we can extract ask price
    if (std::optional<double> price = extract_price_for_macd(mkt_quote))
    {
        update_price_macd(*price);
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
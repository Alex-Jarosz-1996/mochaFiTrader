#ifndef VMACD_H
#define VMACD_H

#include "../algo/Algo.h"
#include "../../marketquote/MarketQuote.h"
#include "../Signal.h"

#include <optional>
#include <vector>

class VMACD : public Algo
{
    public:
        VMACD(int fast = 12, int slow = 26, int signal = 9);

        ~VMACD() override = default;

    protected:
        void process_quote() override;
    
    private:
        // warmup (counts accepted ticks, not packets)
        int trigger_window = 0;
        
        // imb strength threshold indicator
        double imb_strength = 0.12;

        // smoothing factors
        double k_fast = 0.0;
        double k_slow = 0.0;
        double k_signal = 0.0;

        // --- price MACD state ---
        bool price_init = false;
        double ema_fast_p = 0.0;
        double ema_slow_p = 0.0;
        double ema_sig_p = 0.0;
        double macd_p = 0.0;
        double prev_macd_p = 0.0;
        double prev_sig_p = 0.0;

        // --- imbalance MACD state ---
        bool imb_init = false;
        double ema_fast_i = 0.0;
        double ema_slow_i = 0.0;
        double ema_sig_i = 0.0;
        double macd_i = 0.0;
        double prev_macd_i = 0.0;
        double prev_sig_i = 0.0;

        // extractors
        std::optional<double> extract_trade_price(const MarketQuote& q) const;
        std::optional<double> extract_mid_price(const MarketQuote& q) const;
        std::optional<double> extract_price_for_macd(const MarketQuote& q) const;

        // normalised imbalance: (bidSize-askSize)/(bidSize+askSize)
        std::optional<double> extract_normalized_imbalance(const MarketQuote& q) const;

        // macd update helpers
        void update_price_macd(double price);
        void update_imbalance_macd(double imb);

        // helpers for crossovers / state
        bool price_bull_cross() const; // last step crossed bullish
        bool price_bear_cross() const; // last step crossed bearish
        bool imb_bullish() const; // current bullish state (macd > signal)
        bool imb_bearish() const; // current bearish state (macd < signal)
};

#endif // VMACD_H
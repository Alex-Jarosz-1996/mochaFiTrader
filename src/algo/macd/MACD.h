#ifndef MACD_H
#define MACD_H

#include "../algo/Algo.h"
#include "../../marketquote/MarketQuote.h"
#include "../Signal.h"

#include <optional>
#include <vector>

class MACD : public Algo {
    private:
        // signals
        int fast_ema = 0;
        int slow_ema = 0;
        int signal_ema = 0;

        // signal init
        bool fast_init = false;
        bool slow_init = false;
        bool signal_init = false;

        // smoothing factors
        double k_fast;
        double k_slow;
        double k_signal;

        // trading logic
        double macd = 0.0;
        double signal = 0.0;
    
        double prevMacd = 0.0;
        double prevSignal = 0.0;

        std::optional<double> extract_latest_price(const std::vector<MarketQuote>& quotes);
    
    public:
        MACD(int fast = 12, int slow = 26, int signal = 9);

        // over-ride methods of Algo class
        void process_quote() override;
        Signal generate_trading_signal() override;
}

#endif // MACD_H
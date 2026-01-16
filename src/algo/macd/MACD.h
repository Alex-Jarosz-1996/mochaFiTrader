#ifndef MACD_H
#define MACD_H

#include "../algo/Algo.h"
#include "../../marketquote/MarketQuote.h"
#include "../Signal.h"

#include <optional>
#include <vector>

class MACD : public Algo {
    private:
        // ema smoothing
        double k_fast = 0.0;
        double k_slow = 0.0;
        double k_signal = 0.0;

        // trigger threshold
        int trigger_window = 0;

        // ema state
        bool initialised = false;
        double ema_fast = 0.0;
        double ema_slow = 0.0;
        double ema_signal = 0.0;

        // crossover state
        double macd = 0.0;
        double prev_macd = 0.0;
        double prev_signal = 0.0;

        // handling Quote and Trade packets
        std::optional<double> extract_price_for_macd(const MarketQuote& q) const;
    
    protected:
        void process_quote() override;
    
    public:
        MACD(int fast = 12, int slow = 26, int signal = 9);
        ~MACD() override = default;
};

#endif // MACD_H
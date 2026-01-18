#ifndef RSI_H
#define RSI_H

#include "../algo/Algo.h"
#include "../../marketquote/MarketQuote.h"
#include "../Signal.h"

#include <optional>
#include <vector>

class RSI : public Algo
{
    public:
        RSI(int period = 14,
            double oversold = 30.0,
            double overbought = 70.0);

        ~RSI() override = default;

    protected:
        void process_quote() override;

    private:
        int period;
        double oversold;
        double overbought;

        // price extraction
        std::optional<double> extract_trade_price(const MarketQuote& q) const;
        std::optional<double> extract_mid_price(const MarketQuote& q) const;
        std::optional<double> extract_price_for_rsi(const MarketQuote& q) const;

        // rsi state
        bool seeded = false;
        std::optional<double> prev_price;

        // seeding buffers
        int seed_count = 0;
        double seed_gain_sum = 0.0;
        double seed_loss_sum = 0.0;

        // Wilder averages
        double avg_gain = 0.0;
        double avg_loss = 0.0;

        // RSI values
        double rsi = 50.0;
        double prev_rsi = 50.0;

        void update_rsi(double price);

        bool crossed_up(double level) const;    // prev < level && now >= level
        bool crossed_down(double level) const;  // prev > level && now <= level
};

#endif // RSI_H
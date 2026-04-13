#ifndef ALGO_H
#define ALGO_H

#include <iostream>
#include <optional>
#include <vector>
#include <string>

#include "../../marketquote/MarketQuote.h"
#include "../Signal.h"

class Algo {
    protected:
        // used for warmup
        std::optional<MarketQuote> prev_raw;
        std::optional<MarketQuote> curr_raw;

        // most recent accepted packet
        std::optional<MarketQuote> prev_valid;

        // usage for warmup
        int valid_count = 0;

        // signal indicator
        Signal sig = Signal::HOLD;

        // MarketQuote raw packet tracking
        virtual void ingest_quote(const MarketQuote& qt);
        
        // processing quote
        virtual bool is_genuine_transition(const MarketQuote& prev,
                                           const MarketQuote& curr);
        
        // strategy specific logic
        virtual void process_quote() = 0;
    
    public:
        Algo() {};
        virtual ~Algo() {};
        
        // trading signal to determine BUY / SELL / HOLD actions
        virtual Signal generate_trading_signal(const MarketQuote& qt);

        int get_valid_count() { return valid_count; }
        Signal get_signal() { return sig; }
};

#endif // ALGO_H
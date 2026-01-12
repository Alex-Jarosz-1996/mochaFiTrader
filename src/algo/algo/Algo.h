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
        std::vector<MarketQuote> buffer;
    
    public:
        Algo() {};
        virtual ~Algo() {};
        
        virtual void ingest_quote(const std::vector<MarketQuote>& qt);
        
        // processing quote
        virtual bool is_genuine_transition(
            const MarketQuote& prev,
            const MarketQuote& curr
        );
        virtual void updated_buffer();
        virtual void clear_buffer();
        
        // strategy specific logic
        virtual void process_quote() = 0;
        virtual Signal generate_trading_signal() = 0;
};

#endif // ALGO_H
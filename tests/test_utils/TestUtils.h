#include "../../src/marketquote/MarketQuote.h"
#include "../../src/algo/Signal.h"
#include "../../src/algo/algo/Algo.h"

class AlgoHarness : public Algo {
    public:
        using Algo::is_genuine_transition; // expose protected for tests
    protected:
        void process_quote() override { sig = Signal::HOLD; }
};

MarketQuote make_unknown(const std::string& sym);


MarketQuote make_trade(const std::string& sym, double price);


MarketQuote make_quote(const std::string& sym,
                       double bid,
                       double ask,
                       double bidSize = 1.0,
                       double askSize = 1.0);

MarketQuote make_quote_with_imbalance(const std::string& sym,
                                      double midPrice,
                                      double spread,
                                      double totalSize,
                                      double imbalance_norm);
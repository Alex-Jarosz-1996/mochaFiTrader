#include "../../src/marketquote/MarketQuote.h"
#include "../../src/algo/Signal.h"
#include "../../src/algo/algo/Algo.h"

class AlgoHarness : public Algo {
    public:
        using Algo::is_genuine_transition; // expose protected for tests
    protected:
        void process_quote() override { sig = Signal::HOLD; }
};

struct QuoteParams {
    double bid;
    double ask;
    double bidSize = 1.0;
    double askSize = 1.0;
};

struct ImbalanceQuoteParams {
    double midPrice;
    double spread;
    double totalSize;
    double imbalanceNorm;
};

struct MidQuoteParams {
    double mid;
    double spread  = 2.0;
    double bidSize = 1.0;
    double askSize = 1.0;
};

auto make_unknown(const std::string& sym) -> MarketQuote;

auto make_trade(const std::string& sym, double price) -> MarketQuote;

auto make_quote(const std::string& sym, QuoteParams params) -> MarketQuote;

auto make_quote_with_imbalance(const std::string& sym, ImbalanceQuoteParams params) -> MarketQuote;

auto make_mid_quote(const std::string& sym, MidQuoteParams params) -> MarketQuote;

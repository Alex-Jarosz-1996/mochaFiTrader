#include <functional>
#include <vector>

#include "src/marketquote/MarketQuote.h"

class MockStreamer {
public:
    using QuoteHandler = std::function<void(const MarketQuote&)>;

    void set_on_quote(QuoteHandler cb) { on_quote_ = std::move(cb); }

    // Emit a sequence synchronously (deterministic)
    void emit(const std::vector<MarketQuote>& quotes) {
        for (const auto& q : quotes) {
            if (on_quote_) on_quote_(q);
        }
    }

private:
    QuoteHandler on_quote_;
};

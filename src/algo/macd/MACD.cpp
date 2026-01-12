#include "MACD.h"
#include "../Signal.h"
#include "../../marketquote/MarketQuote.h"

#include <optional>

MACD::MACD(int fast, int slow, int signal)
{
    k_fast = 2.0 / (fast + 1);
    k_slow = 2.0 / (slow + 1);
    k_signal = 2.0 / (signal + 1);
}

std::optional<double> MACD::extract_latest_price(const std::vector<MarketQuote>& quotes)
{
    for (MarketQuote it = quotes.rbegin(); it != quotes.rend(); ++it)
        if (it->price)
            return it->price.value();

    return std::nullopt;
}

void MACD::process_quote()
{

}

Signal MACD::generate_trading_signal()
{

}

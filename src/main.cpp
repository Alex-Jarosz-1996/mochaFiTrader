#include <iostream>
#include <vector>

using namespace std;

int main()
{
    // TODO; placeholder, incorporate where needed
    std::vector<std::string> symbols = {"SPY", "QQQ", "BTCUSD"};

    for (const auto& symbol: symbols) 
    {
        cout << symbol << endl;
    }

    return 0;
}
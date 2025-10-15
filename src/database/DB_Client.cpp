#include "DB_Client.h"
#include <pqxx/pqxx>
#include <iostream>
#include <chrono>
#include <nlohmann/json.hpp>

#include "../marketquote/MarketQuote.hpp"
#include "../config/Config.h"

DB_Client::DB_Client()
{
    loadConfig();
    create_db_if_not_exists();
};

DB_Client::~DB_Client(){ };

void DB_Client::loadConfig()
{
    CONNECTION_STR = Config::get_config_value("DB_CREDENTIALS");
}

void DB_Client::create_db_if_not_exists()
{
    try 
    {
        pqxx::connection c(CONNECTION_STR);
        
        if (c.is_open())
        {
            std::string sql = "CREATE TABLE IF NOT EXISTS market_quotes(" \
                              "id SERIAL PRIMARY KEY," \
                              "symbol TEXT NOT NULL," \
                              "price DOUBLE PRECISION," \
                              "bid_price DOUBLE PRECISION," \
                              "ask_price DOUBLE PRECISION," \
                              "day_volume DOUBLE PRECISION," \
                              "size DOUBLE PRECISION," \
                              "bid_size DOUBLE PRECISION," \
                              "ask_size DOUBLE PRECISION," \
                              "timestamp TIMESTAMP NOT NULL );";
            pqxx::work work(c);

            work.exec(sql);
            work.commit();

            std::cout << "Table created." << std::endl;
        }
        else
        {
            std::cout << "Did not create table." << std::endl;
        }

    } catch (const std::exception& e)
    {
        std::cerr << "Failed to create DB table: " << e.what() << std::endl;
    }
}

void DB_Client::insert_quote(const MarketQuote& quote)
{
    try
    {
        pqxx::connection c(CONNECTION_STR);
        pqxx::work txn{c};
        
        std::string sql = R"(
            INSERT INTO market_quotes (
                symbol, price, bid_price, ask_price,
                day_volume, size, bid_size, ask_size, timestamp
            ) VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9);
        )";

        pqxx::params p;
        
        p.append(quote.symbol);

        if (quote.price.has_value())
            p.append(quote.price.value());
        else
            p.append(nullptr);

        if (quote.bidPrice.has_value())
            p.append(quote.bidPrice.value());
        else
            p.append(nullptr);

        if (quote.askPrice.has_value())
            p.append(quote.askPrice.value());
        else
            p.append(nullptr);

        if (quote.dayVolume.has_value())
            p.append(quote.dayVolume.value());
        else
            p.append(nullptr);

        if (quote.size.has_value())
            p.append(quote.size.value());
        else
            p.append(nullptr);

        if (quote.bidSize.has_value())
            p.append(quote.bidSize.value());
        else
            p.append(nullptr);

        if (quote.askSize.has_value())
            p.append(quote.askSize.value());
        else
            p.append(nullptr);

        // Timestamp string
        std::string date_timestamp = generate_current_timestamp();
        p.append(date_timestamp);

        txn.exec(sql, p);
        txn.commit();
        
        std::cout << "Quote inserted for: " << quote.symbol << " Current timestamp: " << date_timestamp << std::endl;

    } catch (const std::exception& e)
    {
        std::cerr << "Failed to insert quote: " << e.what() << std::endl;
    }
}

std::string DB_Client::generate_current_timestamp()
{
    auto now = std::chrono::system_clock::now();

    // Convert to time_t for calendar parts
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::gmtime(&t); // can use *std::localtime(&t) instead

    // Extract milliseconds
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    // Format into string
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S")
        << '.' << std::setfill('0') << std::setw(3) << ms.count();

    return oss.str();
}

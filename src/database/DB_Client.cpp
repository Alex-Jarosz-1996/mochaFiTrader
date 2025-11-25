#include "DB_Client.h"
#include <pqxx/pqxx>
#include <iostream>
#include <chrono>
#include <nlohmann/json.hpp>
#include <optional>
#include <map>

#include "../marketquote/MarketQuote.hpp"
#include "../config/Config.h"
#include "../log/Log.h"

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
                              "timestamp TIMESTAMP NOT NULL," \
                              "symbol TEXT NOT NULL," \
                              "price DOUBLE PRECISION," \
                              "bid_price DOUBLE PRECISION," \
                              "ask_price DOUBLE PRECISION," \
                              "day_volume DOUBLE PRECISION," \
                              "size DOUBLE PRECISION," \
                              "bid_size DOUBLE PRECISION," \
                              "ask_size DOUBLE PRECISION );";
            pqxx::work work(c);

            work.exec(sql);
            work.commit();

            LOG_INFO("Table created.", "DB_CLIENT");
        }
        else
        {
            LOG_INFO("Did not create table.", "DB_CLIENT");
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
                timestamp, symbol, price, bid_price, ask_price,
                day_volume, size, bid_size, ask_size
            ) VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9);
        )";

        pqxx::params p;

        std::string date_timestamp = generate_current_timestamp();
        p.append(date_timestamp);        
        p.append(quote.symbol);

        write_to_db(p, quote.price);
        write_to_db(p, quote.bidPrice);
        write_to_db(p, quote.askPrice);
        write_to_db(p, quote.dayVolume);
        write_to_db(p, quote.size);
        write_to_db(p, quote.bidSize);
        write_to_db(p, quote.askSize);

        txn.exec(sql, p);
        txn.commit();
        
        LOG_INFO("Quote inserted for: " + quote.symbol, "DB_CLIENT");
        LOG_INFO("Current timestamp: " + date_timestamp, "DB_CLIENT");

    } catch (const std::exception& e)
    {
        std::cerr << "Failed to insert quote: " << e.what() << std::endl;
    }
}

std::optional<std::vector<MarketQuote>> DB_Client::get_quote()
{
    try
    {
        pqxx::connection c(CONNECTION_STR);
        pqxx::work txn{c};

        pqxx::result count_result = txn.exec("SELECT COUNT(*) FROM market_quotes;");
        size_t row_count = count_result[0][0].as<size_t>();

        LOG_INFO("Number of rows discovered: " + std::to_string(row_count), "DB_CLIENT");

        if (row_count < 100)
        {
            txn.commit();
            return std::nullopt;
        }

        pqxx::result res = txn.exec(
            "SELECT symbol, price, bid_price, ask_price, day_volume, size, bid_size, ask_size "
            "FROM market_quotes ORDER BY id DESC LIMIT 100;"
        );
        txn.commit();

        std::vector<MarketQuote> mkt_qts;
        mkt_qts.reserve(res.size());
        for (const auto& row : res)
        {
            MarketQuote q;
            q.symbol    = row["symbol"].c_str();
            q.price     = get_from_db<double>(row, "price");
            q.bidPrice  = get_from_db<double>(row, "bid_price");
            q.askPrice  = get_from_db<double>(row, "ask_price");
            q.dayVolume = get_from_db<double>(row, "day_volume");
            q.size      = get_from_db<double>(row, "size");
            q.bidSize   = get_from_db<double>(row, "bid_size");
            q.askSize   = get_from_db<double>(row, "ask_size");
            mkt_qts.push_back(q);
        }

        return mkt_qts;
    }
    catch (const std::exception& e)
    {
        std::cerr << "[DB_Client::get_quote] Error: " << e.what() << std::endl;
        return std::nullopt;
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

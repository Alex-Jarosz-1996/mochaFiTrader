#pragma once

#include <pqxx/pqxx>
#include <optional>
#include <vector>
#include <map>

#include "../marketquote/MarketQuote.h"

class DB_Client
{
    private:
        std::string CONNECTION_STR;

        void loadConfig();
    
    public:
        DB_Client();

        ~DB_Client();

        void create_db_if_not_exists();
        void insert_quote(const MarketQuote& quote);
        std::optional<std::vector<MarketQuote>> get_quote();
        std::string generate_current_timestamp();

        template<typename T>
        inline std::optional<T> get_from_db(const pqxx::row& row, const std::string& col) {
            return !row[col].is_null() ? std::optional<T>(row[col].as<T>()) : std::nullopt;
        }

        template<typename T>
        inline void write_to_db(pqxx::params& p, const std::optional<T>& value) {
            if (value.has_value())
                p.append(value.value());
            else
                p.append(nullptr);
        }
};
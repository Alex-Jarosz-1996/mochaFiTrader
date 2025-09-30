#pragma once

#include <pqxx/pqxx>

#include "../marketquote/MarketQuote.hpp"

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
        std::string generate_current_timestamp();
};
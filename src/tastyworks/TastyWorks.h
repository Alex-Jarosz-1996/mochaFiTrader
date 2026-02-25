#ifndef TASTYWORKSCLIENT_H
#define TASTYWORKSCLIENT_H

#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include <string>

#include "../../src/algo/Signal.h"

class TastyWorksClient {
private:
  std::string BASE_URL;
  std::string LOGIN;
  std::string PASSWORD;
  std::string ACCOUNT_NUMBER;
  bool REMEMBER_ME;

  cpr::Header _h;
  cpr::Header _h_auth;

  const double ACCOUNT_MIN = 100;
  const double TRADE_FACTOR = 0.5;

  void loadConfig();
  void constructHeader();
  void constructAuthHeader();
  void logout();

public:
  std::string _session_token;
  std::string _api_quote_token;
  std::string _dx_link_url;
  
  TastyWorksClient();
  
  ~TastyWorksClient();
  
  // order submission parameters
  std::string getBaseUrl();
  std::string getAccountNumber();
  
  // required for token and feed handling
  void getSessionToken();
  void confirmUserAccountActive();
  void defineQuoteTokenStreamUrl();
  std::string getAPI_QuoteToken();
  std::string getDX_LinkUrl();

  // account balance and status
  double getAccountBalance();
  double getTradeableAmount();
  size_t getNumberAccountPositions();

  // order submission
  void submitOrder(const nlohmann::json& body);
};

#endif // TASTYWORKSCLIENT_H

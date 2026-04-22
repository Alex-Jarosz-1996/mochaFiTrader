#ifndef TASTYWORKSCLIENT_H
#define TASTYWORKSCLIENT_H

#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include <string>

#include "src/algo/Signal.h"

class TastyWorksClient {
private:
  std::string BASE_URL;
  std::string LOGIN;
  std::string PASSWORD;
  std::string ACCOUNT_NUMBER;
  bool REMEMBER_ME = false;

  cpr::Header _h;
  cpr::Header _h_auth;

  const double ACCOUNT_MIN = 100;
  const double TRADE_FACTOR = 0.5;

  static constexpr int HTTP_TIMEOUT_MS = 5000;
  static constexpr int HTTP_OK = 200;
  static constexpr int HTTP_CREATED = 201;
  static constexpr int HTTP_NO_CONTENT = 204;

  void loadConfig();
  void constructHeader();
  void constructAuthHeader();
  void logout();

public:
  std::string _session_token;
  std::string _api_quote_token;
  std::string _dx_link_url;
  
  TastyWorksClient(bool auto_init = true);
  
  virtual ~TastyWorksClient();
  
  // order submission parameters
  virtual std::string getBaseUrl();
  virtual std::string getAccountNumber();
  
  // required for token and feed handling
  virtual void getSessionToken();
  virtual void confirmUserAccountActive();
  virtual void defineQuoteTokenStreamUrl();
  virtual std::string getAPI_QuoteToken();
  virtual std::string getDX_LinkUrl();

  // account balance and status
  virtual double getAccountBalance();
  virtual double getTradeableAmount();
  virtual size_t getNumberAccountPositions();

  // order submission
  virtual void submitOrder(const nlohmann::json& body);
};

#endif // TASTYWORKSCLIENT_H

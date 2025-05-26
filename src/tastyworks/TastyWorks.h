#ifndef TASTYWORKSCLIENT_H
#define TASTYWORKSCLIENT_H

#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include <string>

class TastyWorksClient {
private:
  std::string BASE_URL;
  std::string LOGIN;
  std::string PASSWORD;
  std::string ACCOUNT_NUMBER;
  bool REMEMBER_ME;

  void loadConfig();
  void logout();

public:
  std::string _session_token;
  std::string _api_quote_token;
  std::string _dx_link_url;
  
  TastyWorksClient();
  
  ~TastyWorksClient();
  
  void getSessionToken();
  void confirmSessionTokenGenerated();
  void confirmUserAccountActive();
  void getAPI_QuoteToken();
};

#endif // TASTYWORKSCLIENT_H

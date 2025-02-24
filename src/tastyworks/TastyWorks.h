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
  std::string _session_token;

  void loadConfig();

  void logout();

public:
  TastyWorksClient();

  ~TastyWorksClient();

  std::string getSessionToken();

  void confirmSessionTokenGenerated();

  void confirmAccountActive();

};

#endif // TASTYWORKSCLIENT_H

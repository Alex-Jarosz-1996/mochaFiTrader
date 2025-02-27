#include <gtest/gtest.h>
#include <httplib.h>
#include "TastyWorks.h"
#include <thread>
#include <iostream>
#include <memory>

TEST(TastyWorksClientTest, HandlesSessionTokenGeneration) {
  TastyWorksClient* twClient = new TastyWorksClient();;
  std::string _session_token = twClient->getSessionToken();
  ASSERT_TRUE(!_session_token.empty());
  delete twClient;
}

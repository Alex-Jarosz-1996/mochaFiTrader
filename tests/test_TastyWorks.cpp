#include <gtest/gtest.h>
#include <httplib.h>
#include "TastyWorks.h"
#include <thread>
#include <iostream>
#include <memory>

TEST(TastyWorksClientTest, HandlesSessionTokenGeneration) {
  TastyWorksClient* twClient = new TastyWorksClient();;
  twClient->getSessionToken();
  ASSERT_TRUE(!twClient->_session_token.empty());
  delete twClient;
}

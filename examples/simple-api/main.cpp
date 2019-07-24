#include "apee.hpp"

using namespace apee;

struct Handler : public AbstractRequestHandler {
  Response on_request(Request const &req) override {
      Response r;
      r.m_body = "Hi";
      return r;
      
  }
};

int main() {
  auto handler = std::make_shared<Handler>();
  Service s(handler);
  s.run();
}

#include "apee.hpp"

using namespace apee;

struct Handler : public AbstractRequestHandler {
  Response on_request(Request const &req) override {
    return Response(StatusLine("HTTP/1.1 200 OK"),
                    MessageBody("Hello from Handler!\n"));
  }
};

int main() {
  auto handler = std::make_shared<Handler>();
  Service s(handler);
  s.run();
}

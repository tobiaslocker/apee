#include "apee.hpp"

using namespace apee;

struct Handler : public AbstractRequestHandler {
  Response on_request(Request const &req) override {
    if (req.request_line().uri() == "/moop") {
      return Response(StatusCode::OK, MessageBody("Hello from Handler!\n"));
    }
    return Response(StatusCode::NotFound,
                    MessageBody("Hello from Handler! Target not found!\n"));
  }
};

int main() {
  auto handler = std::make_shared<Handler>();
  Service s(handler);
  s.run();
}

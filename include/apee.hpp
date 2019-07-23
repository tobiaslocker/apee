#ifndef APEE_H
#define APEE_H

#include <memory>
#include <string>

namespace apee {

enum class Method { GET, HEAD, POST, PUT, DELETE, CONNECT, OPTIONS, TRACE };

struct Body {};

struct Request {
  Method method;
  std::string request_uri;
  std::string version;
  Body body;
};

struct Response {};

struct Config {
  std::string address;
  unsigned short port;
};

struct AbstractRequestHandler {
  virtual Response on_request(Request const &) = 0;
};

class Service {
  class impl;
  std::unique_ptr<impl> d_ptr;

 public:
  Service();
  ~Service();
  Service(Service &&) noexcept;
  Service &operator=(Service &&) noexcept;

  Service(std::shared_ptr<AbstractRequestHandler> handler);
  void run();
};

}  // namespace apee

#endif  // APEE_H

#ifndef APEE_H
#define APEE_H

#include <iostream>
#include <memory>
#include <string>

namespace apee {

enum class Method {
  UNKNOWN,
  DELETE,
  GET,
  HEAD,
  POST,
  PUT,
  CONNECT,
  OPTIONS,
  TRACE
};

inline std::ostream &operator<<(std::ostream &out, Method const &op) {
  switch (op) {
    case Method::UNKNOWN:
      out << "UNKNOWN";
      break;
    case Method::DELETE:
      out << "DELETE";
      break;
    case Method::GET:
      out << "GET";
      break;
    case Method::HEAD:
      out << "HEAD";
      break;
    case Method::POST:
      out << "POST";
      break;
    case Method::PUT:
      out << "PUT";
      break;
    case Method::CONNECT:
      out << "CONNECT";
      break;
    case Method::OPTIONS:
      out << "OPTIONS";
      break;
    case Method::TRACE:
      out << "TRACE";
      break;
  }
  return out;
}

struct Body {};

struct Request {
  std::string_view m_request_uri;
  Method m_method;
  std::string m_body;

  Request(std::string_view const &request_uri,
          Method method,
          std::string const &body)
      : m_request_uri{request_uri}, m_method{method}, m_body{body} {}

  Method method() const;
  std::string_view request_uri() const;
  std::string_view version;
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

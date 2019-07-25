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

struct StatusLine {
  StatusLine(std::string_view const &line) {}
};

struct Uri {
  Uri(std::string_view const &uri);
};

class Version {
  unsigned int m_major;
  unsigned int m_minor;
public:
  Version(unsigned int http_version)
      : m_major{http_version / 10}, m_minor{http_version % 10} {}
  unsigned int major_version() const { return m_major; }
  unsigned int minor_version() const { return m_minor; }
};

class RequestLine {
  Method m_method;
  Uri m_request_uri;
  Version m_version;

  //  RequestLine(std::string_view const &line) {
  //    // TODO split to types
  //  }

 public:
  RequestLine(Method method,
              Uri const &request_uri,
              Version const &http_version)
      : m_method{method}, m_request_uri{request_uri}, m_version{http_version} {}
  Method method() const { return m_method; }
  Uri uri() const { return m_request_uri; }
  Version version() const { return m_version; }
};

struct MessageBody {
  std::string_view m_body;
  MessageBody(std::string_view const &body) : m_body{body} {}
};

inline std::ostream &operator<<(std::ostream &out, MessageBody const &op) {
  out << op.m_body;
  return out;
}

struct Request {
  RequestLine m_request_line;
  MessageBody m_body;

  Request(RequestLine const &request_line, MessageBody const &body)
      : m_request_line{request_line}, m_body{body} {}

  RequestLine request_line() const { return m_request_line; }
  MessageBody body() const { return m_body; }

  //  std::string_view m_request_uri;
  //  Method m_method;
  //  std::string m_body;

  //  Request(std::string_view const &request_uri,
  //          Method method,
  //          std::string const &body)
  //      : m_request_uri{request_uri}, m_method{method}, m_body{body} {}
};

class Response {
  StatusLine m_status_line;
  MessageBody m_body;

 public:
  Response(StatusLine const &status_line, MessageBody const &body)
      : m_status_line{status_line}, m_body{body} {}

  MessageBody body() const { return m_body; }
  StatusLine status_line() const { return m_status_line; }
};

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

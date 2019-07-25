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

enum class StatusCode {
  Continue = 100,
  SwitchingProtocols = 101,
  OK = 200,
  Created = 201,
  Accepted = 202,
  NonAuthoritativeInformation = 203,
  NoContent = 204,
  ResetContent = 205,
  PartialContent = 206,
  MultipleChoices = 300,
  MovedPermanently = 301,
  Found = 302,
  SeeOther = 303,
  NotModified = 304,
  UseProxy = 305,
  TemporaryRedirect = 307,
  BadRequest = 400,
  Unauthorized = 401,
  PaymentRequired = 402,
  Forbidden = 403,
  NotFound = 404,
  MethodNotAllowed = 405,
  NotAcceptable = 406,
  ProxyAuthenticationRequired = 407,
  RequestTimeOut = 408,
  Conflict = 409,
  Gone = 410,
  LengthRequired = 411,
  PreconditionFailed = 412,
  RequestEntityTooLarge = 413,
  RequestUriTooLarge = 414,
  UnsupportedMediaType = 415,
  RequestedRangeNotSatisfiable = 416,
  ExpectationFailed = 417,
  InternalServerError = 500,
  NotImplemented = 501,
  BadGateway = 502,
  ServiceUnavailable = 503,
  GatewayTimeOut = 504,
  HttpVersionNotSupported = 505,
};

struct StatusLine {
  StatusLine(std::string_view const &line) {}
  StatusLine(StatusCode status) {}
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

inline std::ostream &operator<<(std::ostream &out, Version const &op) {
  out << "HTTP/" << op.major_version() << "." << op.minor_version();
  return out;
}

class RequestLine {
  Method m_method;
  std::string_view m_request_uri;
  Version m_version;

  //  RequestLine(std::string_view const &line) {
  //    // TODO split to types
  //  }

 public:
  RequestLine(Method method,
              std::string_view const &request_uri,
              Version const &http_version)
      : m_method{method}, m_request_uri{request_uri}, m_version{http_version} {}
  Method method() const { return m_method; }
  std::string_view uri() const { return m_request_uri; }
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

class Request {
  RequestLine m_request_line;
  MessageBody m_body;

 public:
  Request(RequestLine const &request_line, MessageBody const &body)
      : m_request_line{request_line}, m_body{body} {}

  RequestLine request_line() const { return m_request_line; }
  MessageBody body() const { return m_body; }
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

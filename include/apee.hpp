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

std::ostream &operator<<(std::ostream &out, Method const &op);

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

std::ostream &operator<<(std::ostream &out, StatusCode const &op);

class Version {
  unsigned int m_major;
  unsigned int m_minor;

 public:
  Version() : m_major{1}, m_minor{1} {}
  Version(unsigned int http_version)
      : m_major{http_version / 10}, m_minor{http_version % 10} {}
  unsigned int major_version() const { return m_major; }
  unsigned int minor_version() const { return m_minor; }
};

std::ostream &operator<<(std::ostream &out, Version const &op);

class StatusLine {
  StatusCode m_status;
  Version m_version;

 public:
  StatusLine(StatusCode status);
  StatusLine(StatusCode status, Version const &version);
  StatusCode status_code() const;
  Version version() const;
};

class RequestLine {
  Method m_method;
  std::string_view m_request_uri;
  Version m_version;

 public:
  RequestLine(Method method,
              std::string_view const &request_uri,
              Version const &http_version);
  Method method() const;
  std::string_view uri() const;
  Version version() const;
};

class MessageBody {
  std::string_view m_data;

 public:
  MessageBody(std::string_view const &data);
  std::string_view str() const { return m_data; }
};

std::ostream &operator<<(std::ostream &out, MessageBody const &op);

class Request {
  RequestLine m_request_line;
  MessageBody m_body;

 public:
  Request(RequestLine const &request_line, MessageBody const &body);

  RequestLine request_line() const;
  MessageBody body() const;
};

class Response {
  StatusLine m_status_line;
  MessageBody m_body;

 public:
  Response(StatusLine const &status_line, MessageBody const &body);

  MessageBody body() const;
  StatusLine status_line() const;
};

struct Config {
  std::string address;
  unsigned short port;
};

struct AbstractRequestHandler {
  virtual ~AbstractRequestHandler();
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

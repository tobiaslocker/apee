#ifndef OPCUA_CLIENT_REST_API_H
#define OPCUA_CLIENT_REST_API_H
#include "log.hpp"

// Boost
#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>

namespace ip = boost::asio::ip;       // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;     // from <boost/asio.hpp>
namespace http = boost::beast::http;  // from <boost/beast/http.hpp>

// Dependencies
#include <nlohmann/json.hpp>

namespace api {
using namespace nlohmann;
using namespace logger;

class AbstractRequestHandler {
 protected:
  src::severity_channel_logger<severity_level, std::string> m_lg;
  std::string m_channel = "request_handler";

 public:
  virtual ~AbstractRequestHandler();
  virtual json on_get_request(boost::beast::string_view const &target) = 0;
  virtual json on_post_request(
      boost::beast::string_view const &target,
      boost::beast::basic_multi_buffer<std::allocator<char>> const
          &request_body) = 0;
};

class Config {
  boost::asio::ip::address m_address;
  unsigned short m_port;

 public:
  Config(std::string const &address, unsigned short port);
  boost::asio::ip::address address() const;
  unsigned short port() const;
};

class HttpConnection : public std::enable_shared_from_this<HttpConnection> {
  src::severity_channel_logger<severity_level, std::string> m_lg;
  std::string m_channel = "http_connection";
  tcp::socket m_socket;
  boost::beast::flat_buffer m_buffer{8192};
  http::request<http::dynamic_body> m_request;
  http::response<http::dynamic_body> m_response;
  boost::asio::basic_waitable_timer<std::chrono::steady_clock> m_deadline{
      m_socket.get_executor().context(), std::chrono::seconds(60)};
  std::shared_ptr<AbstractRequestHandler> m_request_handler;

 public:
  HttpConnection(tcp::socket socket,
                 std::shared_ptr<AbstractRequestHandler> request_handler);
  void start();
  void read_request();
  void process_request();
  void handle_options_request();
  void handle_post_request();
  void handle_get_request();
  void handle_target_not_found();
  void write_response();
  void check_deadline();
};

class HttpServer {
  src::severity_channel_logger<severity_level, std::string> m_lg;
  boost::asio::io_context m_ioc{1};
  tcp::acceptor m_acceptor;
  tcp::socket m_socket;
  std::string m_channel = "http_server";
  std::shared_ptr<AbstractRequestHandler> m_request_handler;

 public:
  HttpServer(Config const &config);
  void add_connection();
  void run();
  void install_request_handler(std::shared_ptr<AbstractRequestHandler> handler);
};

}  // namespace api

#endif  // OPCUA_CLIENT_REST_API_H

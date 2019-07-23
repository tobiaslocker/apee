#include "apee.hpp"
#include "log.hpp"

// Boost
#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>

namespace ip = boost::asio::ip;       // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;     // from <boost/asio.hpp>
namespace http = boost::beast::http;  // from <boost/beast/http.hpp>

// api::Config::Config(const std::string &address, unsigned short port)
//    : m_address{boost::asio::ip::make_address(address)}, m_port(port) {}

// ip::address api::Config::address() const { return m_address; }

// unsigned short api::Config::port() const { return m_port; }

namespace apee {

using namespace logger;

class Connection : public std::enable_shared_from_this<Connection> {
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
  Connection(tcp::socket socket,
             std::shared_ptr<AbstractRequestHandler> request_handler)
      : m_socket(std::move(socket)),
        m_request_handler(std::move(request_handler)) {}

  void start() {
    BOOST_LOG_CHANNEL_SEV(m_lg, m_channel, debug) << "Started";
    read_request();
    check_deadline();
  }

  void read_request() {
    BOOST_LOG_CHANNEL_SEV(m_lg, m_channel, debug) << "Received request";
    auto self = shared_from_this();
    http::async_read(
        m_socket,
        m_buffer,
        m_request,
        [self](boost::beast::error_code ec, std::size_t bytes_transferred) {
          boost::ignore_unused(bytes_transferred);
          if (!ec) {
            self->process_request();
          } else {
            BOOST_LOG_CHANNEL_SEV(self->m_lg, self->m_channel, error) << ec;
          }
        });
  }

  void process_request() {
    BOOST_LOG_CHANNEL_SEV(m_lg, m_channel, info)
        << "Processing " << m_request.method() << " request";
    BOOST_LOG_CHANNEL_SEV(m_lg, m_channel, trace) << "Request:\n" << m_request;
    m_response.version(m_request.version());
    m_response.keep_alive(false);
    m_response.set(http::field::access_control_allow_origin, "*");

    switch (m_request.method()) {
      case http::verb::get:
        handle_get_request();
        break;
      case http::verb::post:
        handle_post_request();
        break;
      case http::verb::options:
        handle_options_request();
        break;

      default:
        BOOST_LOG_CHANNEL_SEV(m_lg, m_channel, error)
            << "Invalid request-method";
        m_response.result(http::status::bad_request);
        m_response.set(http::field::content_type, "text/plain");
        boost::beast::ostream(m_response.body())
            << "Invalid request-method '"
            << m_request.method_string().to_string() << "'";
        break;
    }
    write_response();
  }

  void handle_options_request() {
    BOOST_LOG_CHANNEL_SEV(m_lg, m_channel, debug) << "Handling OPTIONS request";
    m_response.result(http::status::ok);
    m_response.set(http::field::server, "Beast");
    m_response.set(http::field::access_control_allow_origin, "*");
    m_response.set(http::field::access_control_request_method, "GET, POST");
    m_response.set(http::field::access_control_allow_headers,
                   "Origin, Content-Type, X-Auth-Token");
  }

  void handle_post_request() {
    BOOST_LOG_CHANNEL_SEV(m_lg, m_channel, info)
        << "POST " << m_request.target();
    m_response.result(http::status::ok);
    m_response.set(http::field::server, "Beast");

    if (m_request_handler) {
      //    auto response =
      //    m_request_handler->on_post_request(m_request.target(),
      //                                                       m_request.body());
      //    if (!response.is_null()) {
      //      m_response.set(http::field::content_type, "application/json");
      //      m_response.set(http::field::access_control_allow_origin, "*");
      //      boost::beast::ostream(m_response.body()) << response;
      //      return;
      //    }
    }
    handle_target_not_found();
  }

  void handle_get_request() {
    BOOST_LOG_CHANNEL_SEV(m_lg, m_channel, info)
        << "GET " << m_request.target();
    m_response.result(http::status::ok);
    m_response.set(http::field::server, "Beast");

    if (m_request_handler) {
      //    auto response =
      //    m_request_handler->on_get_request(m_request.target());
      //    m_response.set(http::field::content_type, "application/json");
      //    m_response.set(http::field::access_control_allow_origin, "*");
      //    boost::beast::ostream(m_response.body()) << response;

    } else {
      handle_target_not_found();
    }
  }

  void handle_target_not_found() {
    BOOST_LOG_CHANNEL_SEV(m_lg, m_channel, error) << "Target not found!";
    m_response.result(http::status::not_found);
    m_response.set(http::field::content_type, "text/plain");
    boost::beast::ostream(m_response.body()) << "File not found\r\n";
  }

  void write_response() {
    BOOST_LOG_CHANNEL_SEV(m_lg, m_channel, debug) << "Writing response";
    auto self = shared_from_this();
    m_response.set(http::field::content_length, m_response.body().size());
    http::async_write(
        m_socket, m_response, [self](boost::beast::error_code ec, std::size_t) {
          self->m_socket.shutdown(tcp::socket::shutdown_send, ec);
          self->m_deadline.cancel();
        });
    BOOST_LOG_CHANNEL_SEV(m_lg, m_channel, trace) << "Response:\n"
                                                  << m_response;
  }

  void check_deadline() {
    BOOST_LOG_CHANNEL_SEV(m_lg, m_channel, debug) << "Checking deadline";
    auto self = shared_from_this();
    m_deadline.async_wait([self](boost::beast::error_code ec) {
      if (!ec) {
        self->m_socket.close(ec);
      }
    });
  }
};

class Service::impl {
  src::severity_channel_logger<severity_level, std::string> m_lg;
  boost::asio::io_context m_ioc{1};
  tcp::acceptor m_acceptor;
  tcp::socket m_socket;
  std::string m_channel = "http_server";
  std::shared_ptr<AbstractRequestHandler> m_handler;
public:
//  impl()
//      : m_acceptor{m_ioc, {boost::asio::ip::make_address("0.0.0.0"), 80}},
//        m_socket{m_ioc} {}

  impl(std::shared_ptr<AbstractRequestHandler> handler)
      : m_acceptor{m_ioc, {boost::asio::ip::make_address("0.0.0.0"), 80}},
        m_socket{m_ioc},
        m_handler{handler} {}

  impl(std::string const &address, uint16_t port)
      : m_acceptor{m_ioc, {boost::asio::ip::make_address(address), port}},
        m_socket{m_ioc} {}

  //  impl(Config const &config)
  //      : m_acceptor{m_ioc, {config.address(), config.port()}},
  //      m_socket{m_ioc} {}

  void add_connection() {
    BOOST_LOG_CHANNEL_SEV(m_lg, m_channel, debug) << "Accepting requests";
    m_acceptor.async_accept(m_socket, [&](boost::beast::error_code ec) {
      if (!ec) {
        std::make_shared<Connection>(std::move(m_socket), m_handler)->start();
      } else {
        BOOST_LOG_CHANNEL_SEV(m_lg, m_channel, error) << ec;
      }
      add_connection();
    });
  }

  void run() {
    BOOST_LOG_CHANNEL_SEV(m_lg, m_channel, info) << "Running";
    add_connection();
    m_ioc.run();
    BOOST_LOG_CHANNEL_SEV(m_lg, m_channel, warning) << "Stopped";
  }
};

Service::Service(std::shared_ptr<AbstractRequestHandler> handler)
    : d_ptr{std::make_unique<impl>(handler)} {}

void Service::run() {
    d_ptr->run();
}

Service::~Service() = default;

Service::Service(Service &&) noexcept = default;

Service &Service::operator=(Service &&) noexcept = default;

}  // namespace apee

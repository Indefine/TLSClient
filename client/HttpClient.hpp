#include <memory>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <functional>
#include <string>
#include <iostream>

#include "Session.hpp"
namespace Sas
{
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>
namespace ssl = boost::asio::ssl;       // from <boost/asio/ssl.hpp>
namespace http = boost::beast::http;    // from <boost/beast/http.hpp>

inline void fail(boost::system::error_code ec, char const* what)
{
    std::cerr << what << ": " << ec.message() << "\n";
}
class Client : public std::enable_shared_from_this<Client>
{
public:
    Client(
        boost::asio::io_context& ioc, ssl::context& ctx,
        std::string host, std::string port)
        : resolver_(ioc)
        , stream_(ioc, ctx)
        , host_(host)
        , port_(port)
        , version_(1)
        {}

    void init()
    {
        std::cout << "-----init-----\n";
        if(! SSL_set_tlsext_host_name(stream_.native_handle(), host_.c_str()))
        {
            boost::system::error_code ec{static_cast<int>(::ERR_get_error()), boost::asio::error::get_ssl_category()};
            std::cerr << ec.message() << "\n";
            return;
        }

        doResolver();
    }

    void doResolver()
    {
        std::cout << "-----doResolver-----\n";
        resolveResults_ = resolver_.resolve(host_.c_str(), port_.c_str());
        std::cout << resolveResults_->host_name();
    }

    void on_resolve(boost::system::error_code ec, tcp::resolver::results_type results)
    {
        std::cout << "-----on_resolve-----\n";
        if(ec)
            return fail(ec, "resolve");
        resolveResults_ = results;
    }

    std::shared_ptr<Session> getShortSession()
    {
        std::cout << "-----getShortSession-----\n";
        return std::make_shared<Session>(stream_, resolveResults_);
    }

private:
    tcp::resolver resolver_;
    ssl::stream<tcp::socket> stream_;
    std::string host_;
    std::string port_;
    int version_;

    tcp::resolver::results_type resolveResults_;

};
}
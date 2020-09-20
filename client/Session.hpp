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

void fail(boost::system::error_code ec, char const* what)
{
    std::cerr << what << ": " << ec.message() << "\n";
}

using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>
namespace ssl = boost::asio::ssl;       // from <boost/asio/ssl.hpp>
namespace http = boost::beast::http;    // from <boost/beast/http.hpp>

class Session : public std::enable_shared_from_this<Session>
{

public:
    using Handler = std::function<void(boost::system::error_code, http::response<http::string_body>)>;

    Session(ssl::stream<tcp::socket>& stream, tcp::resolver::results_type& endPoint)
        : stream_(stream)
        , endPoint_(endPoint)
    {
    }

    void async_send(const http::request<http::string_body>& req, Handler handler)
    {
        std::cout << "-----async_send-----\n";
        handler_ = handler;
        req_ = std::move(req);

        boost::asio::async_connect(
            stream_.next_layer(),
            endPoint_.begin(),
            endPoint_.end(),
            std::bind(
                &Session::on_connect,
                shared_from_this(),
                std::placeholders::_1));
    };
    
    void on_connect(boost::system::error_code ec)
    {
        std::cout << "-----on_connect-----\n";
        if(ec)
            return fail(ec, "connect");

        // Perform the SSL handshake
        stream_.async_handshake(
            ssl::stream_base::client,
            std::bind(
                &Session::on_handshake,
                shared_from_this(),
                std::placeholders::_1));
    }

    void on_handshake(boost::system::error_code ec)
    {
        std::cout << "-----on_handshake-----\n";
        if(ec)
            return fail(ec, "handshake");

        // Send the HTTP request to the remote host
        http::async_write(stream_, req_,
            std::bind(
                &Session::on_write,
                shared_from_this(),
                std::placeholders::_1,
                std::placeholders::_2));
    }

    void on_write(
        boost::system::error_code ec,
        std::size_t bytes_transferred)
    {
        std::cout << "-----on_write-----\n";
        boost::ignore_unused(bytes_transferred);

        if(ec)
            return fail(ec, "write");
        
        // Receive the HTTP response
        http::async_read(stream_, buffer_, res_,
            std::bind(
                &Session::on_read,
                shared_from_this(),
                std::placeholders::_1,
                std::placeholders::_2));
    }

    void on_read(
        boost::system::error_code ec,
        std::size_t bytes_transferred)
    {
        std::cout << "-----on_read-----\n";
        boost::ignore_unused(bytes_transferred);

        if(ec)
            return fail(ec, "read");

        // Write the message to standard out
        std::cout << res_ << std::endl;
        std::cout << "-----on_read done1-----\n";
        handler_(ec, res_);   
        // Gracefully close the stream
        std::cout << "-----on_read done-----\n";
        stream_.async_shutdown(
            std::bind(
                &Session::on_shutdown,
                shared_from_this(),
                std::placeholders::_1));
    }

    void on_shutdown(boost::system::error_code ec)
    {
        std::cout << "-----on_shutdown-----\n";
        if(ec == boost::asio::error::eof)
        {
            ec.assign(0, ec.category());
        }
        if(ec)
            return fail(ec, "shutdown");

        // If we get here then the connection is closed gracefully
    }

private:
    ssl::stream<tcp::socket>& stream_;
    tcp::resolver::results_type& endPoint_;
    http::request<http::string_body> req_;
    http::response<http::string_body> res_;
    Handler handler_;
    boost::beast::flat_buffer buffer_;
};


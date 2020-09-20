#include <memory>
#include <string>
#include <iostream>
#include "../root_certificates.hpp"
#include "HttpClient.hpp"

int main(int argc, char const *argv[])
{
    auto const host = "127.0.0.1";
    auto const port = "8080";

    auto ioc = std::make_shared<boost::asio::io_context>();
    ssl::context ctx{ssl::context::tlsv12};
    load_root_certificates(ctx);
    ctx.set_verify_mode(ssl::verify_peer);

    auto client = std::make_shared<Sas::Client>(*ioc, ctx, host, port);
    client->init();
    auto session = client->getShortSession();
    http::request<http::string_body> req;
    // req.version(1);
    req.method(http::verb::post);
    req.target("/index.html");
    std::string str = "{123123213123213213}";
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    req.body() = str;
    req.set(http::field::host, "127.0.0.1");
    req.prepare_payload();
    
    auto func = [](boost::system::error_code ec, http::response<http::string_body> resp)
        {
            if (ec){
                std::cerr << ": " << ec.message() << "\n";
            }

            std::cout << "resp is ===== :  " << resp << "\n";
        };
    
    session->async_send(std::move(req), func);
    ioc->run();
    sleep(1);
    return 0;
}

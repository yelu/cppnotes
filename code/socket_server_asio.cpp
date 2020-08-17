// https://www.boost.org/doc/libs/1_74_0/doc/html/boost_asio/example/cpp11/echo/async_tcp_echo_server.cpp
// async_tcp_echo_Server.cpp
//
// Copyright (c) 2003-2020 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>
#include <boost/asio.hpp>

using namespace boost::asio;
using boost::system::error_code;
using ip::tcp;

/*
class Session : public std::enable_shared_from_this<Session> {

public:
    Session(tcp::socket socket): _socket(std::move(socket)) {}

    void start() {
        auto self(shared_from_this());
        _socket.async_read_some(boost::asio::buffer(_buffer, max_length),
            [this, self](error_code ec, std::size_t length) {
                this->on_read(ec, length);
            });
    }

    void on_read(error_code ec, std::size_t length) {
        if (ec) { return; }

        auto self(shared_from_this());
        boost::asio::async_write(_socket, boost::asio::buffer(_buffer, length),
            [this, self](error_code ec, std::size_t ) {
                this->on_write(ec);
        });
    }

    void on_write(error_code ec)
    {
        return;
    }

private:
    tcp::socket _socket;
    enum {
        max_length = 1024
    };
    char _buffer[max_length];
};

class Server {

public:
    Server(boost::asio::io_context & io_context, short port): _acceptor(io_context, tcp::endpoint(tcp::v4(), port)) {
        _acceptor.async_accept(
            [this](error_code ec, tcp::socket socket) {
                this->on_accept(ec, socket);
            });
    }

    void on_accept(error_code ec, tcp::socket socket) {
        if (!ec) {
            std::make_shared<Session>(std::move(socket))->start();
        }
        _acceptor.async_accept(
            [this](error_code ec, tcp::socket socket) {
                this->on_accept(ec, socket);
            });
    }

private:
    tcp::acceptor _acceptor;
};
*/

void on_write_done(std::shared_ptr<tcp::socket> psocket, 
                   std::shared_ptr<std::string> pmsg, std::size_t length, 
                   error_code ec)
{
    if(ec) { return; }   
    std::cout << "msg sent" << std::endl;
}

void on_read_done(std::shared_ptr<tcp::socket> psocket, 
                  std::shared_ptr<std::string> pmsg, std::size_t length, 
                  error_code ec)
{
    if(ec) {
        std::cout << "read error" << ec << std::endl;
        return; 
    }
    std::cout << "recv pmsg: " << *pmsg << std::endl;

    boost::asio::async_write(*psocket, 
        boost::asio::buffer((void*)(pmsg->c_str()), length),
        [pmsg, psocket](error_code ec, std::size_t length) {
            on_write_done(psocket, pmsg, length, ec);
        });
}

void on_accept_done(tcp::socket& socket, tcp::acceptor& acceptor, error_code ec)
{
    if(ec) return;
    std::cout << "new connection" << std::endl;

    std::shared_ptr<tcp::socket> psocket(new tcp::socket(std::move(socket)));
    std::shared_ptr<std::string> pmsg(new std::string(256, '\0'));
    psocket->async_read_some(
        boost::asio::buffer((void*)(pmsg->c_str()), pmsg->size()),
        [pmsg, psocket](error_code ec, std::size_t length) {
            on_read_done(psocket, pmsg, length, ec);
        });
    
    acceptor.async_accept(
        [&](error_code ec, tcp::socket socket) {
            on_accept_done(socket, acceptor, ec);
        });
}

int main(int argc, char * argv[]) {
    try {
        boost::asio::io_context io_context;
        tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 8888));
        acceptor.async_accept(
            [&acceptor](error_code ec, tcp::socket socket) {              
                on_accept_done(socket, acceptor, ec);
            });

        io_context.run();
    } catch (std::exception & e) {
        std::cerr << "exception: " << e.what() << "\n";
    }

    return 0;
}
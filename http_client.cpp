#include <iostream>
#include <string>
#include <vector>

#include "boost/asio.hpp"
#include "boost/asio/ssl.hpp"

using boost::asio::ip::tcp;
namespace ssl = boost::asio::ssl;

constexpr int MAX_BUFF_SIZE = 1024;
using ssl_socket = ssl::stream<tcp::socket>;
//typedef ssl::stream<tcp::socket> ssl_socket;

class Client {
public:
	Client(boost::asio::io_context& io_context, const std::string& host, const std::string& path)
		: io_context_(io_context),
		host_(host), path_(path),
		ssl_context_(ssl::context::sslv23),
		ssl_socket_(io_context_, ssl_context_),
		buffer_(MAX_BUFF_SIZE) {

		// Use the default paths for finding CA certificates.
		ssl_context_.set_default_verify_paths();

		boost::system::error_code ec;

		// Get a list of endpoints corresponding to the server name.
		tcp::resolver resolver(io_context_);
		auto endpoints = resolver.resolve(host_, "https", ec);

		if (ec) {
			std::cerr << "Resolve failed: " << ec.message() << std::endl;
			return;
		}

		// ConnectHandler: void (boost::system::error_code, tcp::endpoint)
		boost::asio::async_connect(ssl_socket_.lowest_layer(), endpoints,
			std::bind(&Client::ConnectHandler, this,
				std::placeholders::_1,
				std::placeholders::_2));
	}

private:
	void ConnectHandler(boost::system::error_code ec, tcp::endpoint) {
		if (ec) {
			std::cout << "Connect failed: " << ec.message() << std::endl;
		}
		else {
			//ssl_socket_.set_verify_mode(ssl::verify_peer);
			ssl_socket_.set_verify_mode(ssl::verify_none);

			ssl_socket_.set_verify_callback(ssl::rfc2818_verification(host_));

			// HandshakeHandler: void (boost::system::error_code)
			ssl_socket_.async_handshake(ssl::stream_base::client,
				std::bind(&Client::HandshakeHandler,
					this,
					std::placeholders::_1));
		}
	}

	void HandshakeHandler(boost::system::error_code ec) {
		if (ec) {
			std::cerr << "Handshake failed: " << ec.message() << std::endl;
		}
		else {
			AsyncWrite();
		}
	}

	void AsyncWrite() {
		std::ostream request_stream(&request_);
		request_stream << "GET " << path_ << " HTTP/1.1\r\n";
		request_stream << "Host: " << host_ << "\r\n\r\n";

		// WriteHandler: void (boost::system::error_code, std::size_t)
		boost::asio::async_write(ssl_socket_, request_,
			std::bind(&Client::WriteHandler, this,
				std::placeholders::_1,
				std::placeholders::_2));
	}
	void WriteHandler(boost::system::error_code ec, std::size_t length) {
		if (ec) {
			std::cerr << "Write failed: " << ec.message() << std::endl;
		}
		else {
			AsyncReadSome();
		}
	}

	void AsyncReadSome() {
		ssl_socket_.async_read_some(boost::asio::buffer(buffer_),
			std::bind(&Client::ReadHandler, this,
				std::placeholders::_1,
				std::placeholders::_2));
	}

	void ReadHandler(boost::system::error_code ec, std::size_t length) {
		if (ec) {
			std::cout << ec.message() << std::endl;
		}
		else {
			std::cout.write(buffer_.data(), length);

			// TODO: Call AsyncReadSome() to read until the end.
		}
	}

	boost::asio::io_context& io_context_;

	std::string host_;
	std::string path_;

	ssl::context ssl_context_;
	ssl::stream<tcp::socket> ssl_socket_;

	boost::asio::streambuf request_;
	std::vector<char> buffer_;
};

void Help(const char* argv0) {
	std::cout << "Usage: " << argv0 << " <host> <path>" << std::endl;
	std::cout << "  E.g.," << std::endl;
	std::cout << "    " << argv0 << " www.boost.org /LICENSE_1_0.txt" << std::endl;
	std::cout << "    " << argv0 << " www.google.com /" << std::endl;
}

int main(int argc, char* argv[]) {
	if (argc != 3) {
		Help(argv[0]);
		return 1;
	}

	std::string host = argv[1];
	std::string path = argv[2];

	boost::asio::io_context io_context;

	Client client(io_context, host, path);

	io_context.run();

	return 0;
}

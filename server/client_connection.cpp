#include "client_connection.h"
#include "server.h"


ClientConnection::ClientConnection(boost::asio::io_service& service, Server::ptr server) : _sock(service), _started(false), _server(server) {
}

ClientConnection::ptr ClientConnection::new_(boost::asio::io_service& service, Server::ptr server) {
	ptr new_(new ClientConnection(service, server));
	return new_;
}

void ClientConnection::start() {
	_started = true;
	do_read();
}

void ClientConnection::stop() {
	BOOST_LOG_TRIVIAL(info) << "ClientConnection stop called";
	if (!_started) {
		return;
	}
	BOOST_LOG_TRIVIAL(info) << "Stopping client connection...";
	// TODO: send stop command to the client
	_started = false;
	_sock.close();
}

ClientConnection::~ClientConnection() {
	BOOST_LOG_TRIVIAL(info) << "ClientConnection desctruction...";
	stop();
}

void ClientConnection::do_read() {
	if (!_started) {
		return;
	}
	async_read(_sock, buffer(_read_buffer),
		boost::bind(&ClientConnection::read_complete, shared_from_this(), _1, _2),
		boost::bind(&ClientConnection::on_read, shared_from_this(), _1, _2));
}

size_t ClientConnection::read_complete(const boost::system::error_code &err, size_t bytes) {
	if (!_started) {
		return 0;
	}
	if (err) {
		BOOST_LOG_TRIVIAL(error) << "read_complete error: " << err;
		return 0;
	}
	bool found = std::find(_read_buffer, _read_buffer + bytes, '\n') < _read_buffer + bytes;
	return found ? 0 : 1;
}


void ClientConnection::on_read(const boost::system::error_code &err, size_t bytes) {
	if (!_started) {
		BOOST_LOG_TRIVIAL(info) << "Reading interrupted, connection is stopped";
		return;
	}
	if (err) {
		BOOST_LOG_TRIVIAL(error) << "on_read error: " << err;
		stop();
		return;
	}
	std::string msg(_read_buffer, bytes);
	BOOST_LOG_TRIVIAL(info) << "Received a msg: " << msg;
	int num;
	try {
		num = std::stoi(msg);
	}
	catch (std::exception& ex) {
		BOOST_LOG_TRIVIAL(error) << "Could not parse number (" << msg << "), reason: " << ex.what();
		stop();
		return;
	}
	double res = _server.lock()->add_num_calc_res(num);
	std::string resStr = std::to_string(res);
	do_write("Server response: " + resStr + "\n");
}

void ClientConnection::do_write(const std::string & msg) {
	if (!_started) {
		return;
	}
	std::copy(msg.begin(), msg.end(), _write_buffer);
	_sock.async_write_some(buffer(_write_buffer, msg.size()),
		boost::bind(&ClientConnection::on_write, shared_from_this(), _1, _2));
}

void ClientConnection::on_write(const boost::system::error_code & err, size_t bytes) {
	if (err) {
		BOOST_LOG_TRIVIAL(error) << "on_write error: " << err;
		stop();
		return;
	}
	if (!_started) {
		return;
	}
	do_read();
}

ip::tcp::socket& ClientConnection::sock() {
	return _sock;
}


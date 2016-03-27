#include "client_connection.h"
#include "server.h"
#include "server_exception.h"
#include <boost/algorithm/string.hpp>

int ClientConnection::_next_id = 1;

ClientConnection::ClientConnection(boost::asio::io_service& service, Server::ptr server) 
	: _id(ClientConnection::_next_id++), _sock(service), _started(false), _stopped(true), _busy(false), _server(server) {
}

ClientConnection::ptr ClientConnection::new_(boost::asio::io_service& service, Server::ptr server) {
	ptr c(new ClientConnection(service, server));
	return c;
}

const int ClientConnection::id() const {
	return _id;
}

void ClientConnection::start() {
	_started = true;
	_stopped = false;
	do_read();
}

void ClientConnection::stop() {
	if (!_started) {
		return;
	}
	BOOST_LOG_TRIVIAL(info) << "Stopping client connection id=" << _id << "...";
	do_write("stop\n");
	_started = false;
	if (!_busy) {
		stop_sock_close();
	}
}

void ClientConnection::disconnect() {
	throw server_exception("client disconnected", shared_from_this());
}

void ClientConnection::stop_sock_close() {
	_stopped = true;
	_sock.close();
	BOOST_LOG_TRIVIAL(info) << "Client connection id=" << _id << " stopped";
}

bool ClientConnection::is_stopped() {
	return _stopped; // TODO: try to use !_started && !_busy
}

ClientConnection::~ClientConnection() {
	BOOST_LOG_TRIVIAL(info) << "ClientConnection id=" << _id << " destruction...";
	stop();
}

void ClientConnection::do_read() {
	if (!_started && !_busy) {
		stop_sock_close();
		return;
	}
	async_read(_sock, buffer(_read_buffer),
		boost::bind(&ClientConnection::read_complete, shared_from_this(), _1, _2),
		boost::bind(&ClientConnection::on_read, shared_from_this(), _1, _2));
}

size_t ClientConnection::read_complete(const boost::system::error_code &err, size_t bytes) {
	if (err) {
		BOOST_LOG_TRIVIAL(error) << "read_complete error: " << err << " client id=" << _id;
		_busy = false;
		return 0;
	}
	_busy = bytes > 0;
	bool found = std::find(_read_buffer, _read_buffer + bytes, '\n') < _read_buffer + bytes;
	return found ? 0 : 1;
}


void ClientConnection::on_read(const boost::system::error_code &err, size_t bytes) {
	if (err) {
		BOOST_LOG_TRIVIAL(error) << "on_read error: " << err << " client id=" << _id;
		_busy = false;
		throw server_exception("on_read error", shared_from_this());
	}
	std::string msg(_read_buffer, bytes);
	BOOST_LOG_TRIVIAL(info) << "Received a msg: " << msg << " client id=" << _id;
	handle_msg(msg);
}


void ClientConnection::handle_msg(const std::string &msg) {
	std::vector<std::string> strs;
	boost::split(strs, msg, boost::is_any_of(":\n"));
	std::string cmd = strs[0];
	if (cmd == "num") {
		std::string numStr = strs[1];
		int num = std::stoi(numStr);
		double res = process_num(num);
		std::string resStr = std::to_string(res);
		do_write("ok:" + resStr + "\n");
	}
	else if (cmd == "disconnect") {
		disconnect();
	}
	else {
		ostringstream oss;
		oss << "Unexpected msg from client: " << msg;
		throw server_exception(oss.str(), shared_from_this());
	}
}

double ClientConnection::process_num(int num) {
	_server.lock()->add_num(num);
	double res = _server.lock()->calc_res();
	return res;
}


void ClientConnection::do_write(const std::string & msg) {
	BOOST_LOG_TRIVIAL(info) << "Sending to client id=" << _id << " msg: " << msg;
	_busy = true;
	std::copy(msg.begin(), msg.end(), _write_buffer);
	_sock.async_write_some(buffer(_write_buffer, msg.size()),
		boost::bind(&ClientConnection::on_write, shared_from_this(), _1, _2));
}

void ClientConnection::on_write(const boost::system::error_code & err, size_t bytes) {
	_busy = false; // TODO: multiple calls to do_write would fail
	if (err) {
		BOOST_LOG_TRIVIAL(error) << "on_write error: " << err << " client id=" << _id;
		throw server_exception("on_write error", shared_from_this());
	}
	do_read();
}

ip::tcp::socket& ClientConnection::sock() {
	return _sock;
}


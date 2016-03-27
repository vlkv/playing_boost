#include "client_connection.h"
#include "server.h"
#include "server_exception.h"
#include <boost/algorithm/string.hpp>

int ClientConnection::_next_id = 1;

ClientConnection::ClientConnection(boost::asio::io_service& service, Server::ptr server) 
	: _id(ClientConnection::_next_id++), _sock(service), _started(false), _busy(false), _need_stop(false), _server(server) {
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
	do_read_write();
}

void ClientConnection::stop() {
	if (!_started) {
		return;
	}
	BOOST_LOG_TRIVIAL(info) << "Stopping client connection id=" << _id << "...";
	_started = false;
	if (!_busy) {
		do_write_stop();
	} else
	{
		_need_stop = true;
	}
}

void ClientConnection::disconnect() { // TODO: remove copy-paste
	if (!_started) {
		return;
	}
	BOOST_LOG_TRIVIAL(info) << "Stopping client connection (disconnecting) id=" << _id << "...";
	_started = false;
	if (!_busy) {
		stop_sock_close();
	}
}

void ClientConnection::stop_sock_close() {
	_sock.close();
	BOOST_LOG_TRIVIAL(info) << "Client connection id=" << _id << " stopped";
}

bool ClientConnection::is_stopped() {
	return !_started && !_busy;
}

ClientConnection::~ClientConnection() {
	BOOST_LOG_TRIVIAL(info) << "ClientConnection id=" << _id << " destruction...";
	stop();
}

void ClientConnection::do_read_write() {
	_busy = true;
	if (_need_stop) {
		_need_stop = false;
		do_write_stop();
		return;
	}
	do_read();
}

void ClientConnection::do_read() {
	if (is_stopped()) {
		return;
	}
	async_read(_sock, buffer(_read_buffer),
		boost::bind(&ClientConnection::read_complete, shared_from_this(), _1, _2),
		boost::bind(&ClientConnection::on_read, shared_from_this(), _1, _2));
}

size_t ClientConnection::read_complete(const boost::system::error_code &err, size_t bytes) {
	if (is_stopped()) {
		return 0;
	}
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
	if (is_stopped()) {
		return;
	}
	std::string msg(_read_buffer, bytes);
	BOOST_LOG_TRIVIAL(info) << "Received from client id=" << _id << " msg: " << msg;
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
		do_write_disconnected();
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
	std::copy(msg.begin(), msg.end(), _write_buffer);
	_sock.async_write_some(buffer(_write_buffer, msg.size()),
		boost::bind(&ClientConnection::on_write, shared_from_this(), _1, _2));
}

void ClientConnection::on_write(const boost::system::error_code & err, size_t bytes) {
	if (err) {
		BOOST_LOG_TRIVIAL(error) << "on_write error: " << err << " client id=" << _id;
		throw server_exception("on_write error", shared_from_this());
	}
	_busy = false;
	do_read_write();
}

void ClientConnection::do_write_disconnected() { // TODO: remove copy-paste
	std::string msg("disconnected\n");
	BOOST_LOG_TRIVIAL(info) << "Sending to client id=" << _id << " msg: " << msg;
	std::copy(msg.begin(), msg.end(), _write_buffer);
	_sock.async_write_some(buffer(_write_buffer, msg.size()),
		boost::bind(&ClientConnection::on_write_disconnected, shared_from_this(), _1, _2));
}

void ClientConnection::on_write_disconnected(const boost::system::error_code & err, size_t bytes) {
	if (err) {
		BOOST_LOG_TRIVIAL(error) << "on_write error: " << err << " client id=" << _id;
		throw server_exception("on_write error", shared_from_this());
	}
	_busy = false;
	disconnect();
	throw disconnected_exception(shared_from_this());
}

void ClientConnection::do_write_stop() { // TODO: remove copy-paste
	std::string msg("stop\n");
	BOOST_LOG_TRIVIAL(info) << "Sending to client id=" << _id << " msg: " << msg;
	std::copy(msg.begin(), msg.end(), _write_buffer);
	_sock.async_write_some(buffer(_write_buffer, msg.size()),
		boost::bind(&ClientConnection::on_write_disconnected, shared_from_this(), _1, _2));
}

ip::tcp::socket& ClientConnection::sock() {
	return _sock;
}


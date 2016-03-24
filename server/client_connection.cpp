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
	if (!_started) return;
	_started = false;
	_sock.close();
}

void ClientConnection::on_read(const boost::system::error_code & err, size_t bytes) {
	if (err) stop();
	if (!_started) return;
	std::string msg(_read_buffer, bytes);
	// TODO: convert msg to int, pass it to the server and take from server the average of squares, then, send the average to the client back...
	std::cout << "Received a msg: " << msg << std::endl;
	do_write("Server response.\n");
}

void ClientConnection::on_write(const boost::system::error_code & err, size_t bytes) {
	cout << "Written " << bytes << " bytes" << endl;
	do_read();
}

void ClientConnection::do_read() {
	async_read(_sock, buffer(_read_buffer),
		boost::bind(&ClientConnection::read_complete, shared_from_this(), _1, _2),
		boost::bind(&ClientConnection::on_read, shared_from_this(), _1, _2));
}

void ClientConnection::do_write(const std::string & msg) {
	if (!_started) return;
	std::copy(msg.begin(), msg.end(), _write_buffer);
	_sock.async_write_some(buffer(_write_buffer, msg.size()),
		boost::bind(&ClientConnection::on_write, shared_from_this(), _1, _2));
}

size_t ClientConnection::read_complete(const boost::system::error_code & err, size_t bytes) {
	if (err) return 0;
	bool found = std::find(_read_buffer, _read_buffer + bytes, '\n') < _read_buffer + bytes;
	return found ? 0 : 1;
}



ip::tcp::socket& ClientConnection::sock() { return _sock; }


#include "server.h"
#include "client_connection.h"


Server::Server(int port) : _acceptor(_service, ip::tcp::endpoint(ip::tcp::v4(), port)) {
}

void Server::start() {
	_service.run();
	accept_client();
}

void Server::accept_client() {
	ClientConnection::ptr client = ClientConnection::new_(_service, this->shared_from_this());
	_clients.push_back(client);
	_acceptor.async_accept(client->sock(), boost::bind(&Server::on_accept, this, client, _1));
}

void Server::on_accept(ClientConnection::ptr client, const boost::system::error_code & err) {
	client->start();
	accept_client();
}

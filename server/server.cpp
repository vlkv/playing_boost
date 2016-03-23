#include "server.h"
#include "client_connection.h"


Server::Server(int port) : _acceptor(_service, ip::tcp::endpoint(ip::tcp::v4(), port)) {
}

void Server::start() {
	accept_client();
	_service.run();
}

void Server::accept_client() {
	std::cout << "Waiting for client..." << std::endl;
	ClientConnection::ptr client = ClientConnection::new_(_service, shared_from_this());
	_clients.push_back(client);
	_acceptor.async_accept(client->sock(), boost::bind(&Server::on_accept, shared_from_this(), client, _1));
}

void Server::on_accept(ClientConnection::ptr client, const boost::system::error_code & err) {
	std::cout << "Client accepted!" << std::endl;
	client->start();
	accept_client();
}

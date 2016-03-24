#include "server.h"
#include "client_connection.h"
#include <fstream>

Server::Server(int port, int dump_interval_sec, std::string dump_filename) :
	_acceptor(_service, ip::tcp::endpoint(ip::tcp::v4(), port)), 
	_dump_interval_sec(dump_interval_sec),
	_dump_filename(dump_filename),
	_tree_dumper(boost::thread(boost::bind(&Server::dump_tree, this))) {
}

void Server::start() {
	accept_client();
	_service.run();
}

void Server::dump_tree() {
	while (true) {
		boost::posix_time::seconds dump_interval_sec(_dump_interval_sec);
		boost::this_thread::sleep(dump_interval_sec);

		_mutex.lock();
		std::fstream ofile(_dump_filename.c_str(), std::ios::binary | std::ios::out);
		boost::archive::binary_oarchive oa(ofile);

		BOOST_LOG_TRIVIAL(info) << "Dumping the tree...";
		oa & _bin_tree.size();
		for (BinTree::const_iterator it = _bin_tree.cbegin(); it != _bin_tree.cend(); it++) {
			oa & it->first;
			oa & it->second;
		}
		
		_mutex.unlock();
		ofile.close();
	}
}

void Server::accept_client() {
	BOOST_LOG_TRIVIAL(info) << "Waiting for client...";
	ClientConnection::ptr client = ClientConnection::new_(_service, shared_from_this());
	_clients.push_back(client);
	_acceptor.async_accept(client->sock(), boost::bind(&Server::on_accept, shared_from_this(), client, _1));
}

void Server::on_accept(ClientConnection::ptr client, const boost::system::error_code & err) {
	BOOST_LOG_TRIVIAL(info) << "Client accepted!";
	client->start();
	accept_client();
}

double Server::add_num_calc_res(int num) {
	TreeItem ti(num);

	_mutex.lock();
	_bin_tree.insert(BinTree::value_type(num, ti));
	BinTree::const_iterator i = _bin_tree.cbegin();
	double squares_sum = 0;
	while (i != _bin_tree.cend()) {
		squares_sum += i->second.square_value();
		i++;
	}
	double res = squares_sum / _bin_tree.size();
	_mutex.unlock();

	// Simulate hard work on server... // TODO: you may switch it off
	boost::posix_time::milliseconds coffe_break(100);
	boost::this_thread::sleep(coffe_break);

	return res;
}

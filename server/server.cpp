#include "server.h"
#include "client_connection.h"
#include "client_exception.h"
#include <fstream>

Server::Server(int port, int dump_interval_sec, std::string dump_filename) :
	_acceptor(_service, ip::tcp::endpoint(ip::tcp::v4(), port)), 
	_started(false),
	_dump_interval_sec(dump_interval_sec),
	_dump_filename(dump_filename),
	_tree_dumper(boost::thread(boost::bind(&Server::dump_tree, this))) {
}

void Server::start() {
	_started = true;
	while (_started) {
		accept_client();
		try {
			_service.run();
		}
		catch (const client_exception &e) {
			BOOST_LOG_TRIVIAL(error) << "Client exception: " << e.what();
			// TODO: stop only the failed client
		}
		catch (const std::exception &e) {
			BOOST_LOG_TRIVIAL(error) << "Unexpected std::exception: " << e.what();
			stop();
			break;
		}
		catch (...) {
			BOOST_LOG_TRIVIAL(error) << "Unknown exception caugth";
			stop();
			break;
		}
	}
}

Server::~Server() {
	BOOST_LOG_TRIVIAL(info) << "Server destruction...";
	stop();
}

void Server::stop_async() {
	_service.dispatch(boost::bind(&Server::stop, shared_from_this()));
}

void Server::stop() {
	BOOST_LOG_TRIVIAL(info) << "Server stop called";
	if (!_started) {
		return;
	}
	BOOST_LOG_TRIVIAL(info) << "Stopping server...";
	_started = false;
	_acceptor.close();
	
	_tree_dumper.interrupt();
	_tree_dumper.join(); // TODO: use timeout maybe?..
	BOOST_LOG_TRIVIAL(info) << "TreeDump thread stopped";

	while (!_clients.empty()) { // TODO: we should protect _clients with lock, because a new client may try to connect
		ClientConnection::ptr c = _clients.front();
		_clients.pop_front();
		c->stop();
		c.reset();
	}

	
	_service.stop();
	BOOST_LOG_TRIVIAL(info) << "Server stopped";
}

void Server::dump_tree() {
	while (true) {
		try {
			boost::posix_time::seconds dump_interval_sec(_dump_interval_sec);
			boost::this_thread::sleep(dump_interval_sec);
		}
		catch (boost::thread_interrupted) {
			BOOST_LOG_TRIVIAL(info) << "TreeDump thread is interrupted";
			break;
		}

		BOOST_LOG_TRIVIAL(info) << "Dumping the tree...";
		std::fstream ofile(_dump_filename.c_str(), std::ios::binary | std::ios::out);
		boost::archive::binary_oarchive oa(ofile);
		dump_tree_impl(oa);
		ofile.close();
		BOOST_LOG_TRIVIAL(info) << "Tree dump completed";
	}
}

void Server::dump_tree_impl(boost::archive::binary_oarchive &oa) {
	boost::shared_lock<boost::shared_mutex> lock(_mutex);
	oa & _bin_tree.size();
	for (BinTree::const_iterator it = _bin_tree.cbegin(); it != _bin_tree.cend(); it++) {
		oa & it->first;
		oa & it->second;
	}
}

void Server::accept_client() {
	if (!_started) {
		return;
	}
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
		squares_sum += i->second.square_num();
		i++;
	}
	double res = squares_sum / _bin_tree.size();
	_mutex.unlock();

	// We may simulate hard work on server here... 
	//boost::posix_time::milliseconds coffe_break(100);
	//boost::this_thread::sleep(coffe_break);

	return res;
}


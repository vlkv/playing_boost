#include "server.h"
#include "client_connection.h"
#include "server_exception.h"
#include <fstream>

Server::Server(int port, int dump_interval_sec, std::string dump_filename) :
	_acceptor(_service, ip::tcp::endpoint(ip::tcp::v4(), port)), 
	_started(false),
	_stopped(true),
	_dump_interval_sec(dump_interval_sec),
	_dump_filename(dump_filename),
	_tree_dumper(boost::thread(boost::bind(&Server::dump_tree, this))) {
}

void Server::start() {
	BOOST_LOG_TRIVIAL(info) << "Server start";
	_started = true;
	_stopped = false;
	try {
		accept_client();
		service_run_loop();
	}
	catch (const std::exception &e) {
		BOOST_LOG_TRIVIAL(fatal) << "Unexpected std::exception: " << e.what();
		stop_finish();
	}
	catch (...) {
		BOOST_LOG_TRIVIAL(fatal) << "Unexpected unknown exception";
		stop_finish();
	}
}

void Server::service_run_loop() {
	while (!_stopped) {
		try {
			_service.run();
		}
		catch (const server_exception &e) {
			BOOST_LOG_TRIVIAL(error) << "Client id=" << e.client()->id() << " failed, reason: " << e.what();
			e.client()->stop();
			_clients.remove(e.client());
		}
		catch (...) {
			throw;
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
	if (!_started) {
		return;
	}
	BOOST_LOG_TRIVIAL(info) << "Stopping server...";
	_started = false;
	_acceptor.close();
	BOOST_LOG_TRIVIAL(info) << "tcp acceptor closed";
	
	std::for_each(_clients.cbegin(), _clients.cend(), [](const ClientConnection::ptr &c) { c->stop(); });
	stop_wait_for_clients_to_stop();
}

void Server::stop_wait_for_clients_to_stop() {
	BOOST_LOG_TRIVIAL(info) << "Waiting for clients to stop...";
	bool all_clients_stopped = std::all_of(_clients.cbegin(), _clients.cend(), [](const ClientConnection::ptr &c) { return c->is_stopped(); });
	// TODO: how to stop gracefully, if here we'd have an exception?
	if (!all_clients_stopped) {
		boost::asio::deadline_timer timer(_service, boost::posix_time::milliseconds(100));
		timer.async_wait(boost::bind(&Server::stop_wait_for_clients_to_stop, shared_from_this()));
		return;
	}
	BOOST_LOG_TRIVIAL(info) << "All client connections stopped";
	stop_finish();
}

void Server::stop_finish() {
	_clients.clear();
	_service.stop();
	BOOST_LOG_TRIVIAL(info) << "io_service stopped";

	_tree_dumper.interrupt();
	_tree_dumper.join(); // TODO: use timeout maybe?..
	BOOST_LOG_TRIVIAL(info) << "Tree dump thread stopped";

	_stopped = true;
	BOOST_LOG_TRIVIAL(info) << "Server stopped";
}

void Server::dump_tree() {
	while (true) {
		try {
			boost::posix_time::seconds dump_interval_sec(_dump_interval_sec);
			boost::this_thread::sleep(dump_interval_sec);
			
			BOOST_LOG_TRIVIAL(info) << "Tree dump started...";
			std::fstream ofile(_dump_filename.c_str(), std::ios::binary | std::ios::out);
			boost::archive::binary_oarchive oa(ofile);
			dump_tree_impl(oa);
			ofile.close();
			BOOST_LOG_TRIVIAL(info) << "Tree dump completed";
		}
		catch (const boost::thread_interrupted &) {
			BOOST_LOG_TRIVIAL(info) << "Tree dump thread is interrupted";
			break;
		}
		catch (const std::exception &ex) {
			BOOST_LOG_TRIVIAL(error) << "Tree dump failed, reason: " << ex.what();
		}
		catch (...) {
			BOOST_LOG_TRIVIAL(error) << "Tree dump failed, reason: unknown";
		}
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
	if (!_acceptor.is_open()) {
		return;
	}
	BOOST_LOG_TRIVIAL(info) << "Waiting for client...";
	ClientConnection::ptr client = ClientConnection::new_(_service, shared_from_this());
	_clients.push_back(client);
	_acceptor.async_accept(client->sock(), boost::bind(&Server::on_accept, shared_from_this(), client, _1));
}

void Server::on_accept(ClientConnection::ptr client, const boost::system::error_code & err) {
	if (err.value() == boost::asio::error::operation_aborted) {
		throw server_exception("on_accept operation_aborted", client); // TODO: this should not be logged as 'error'
	}
	if (err) {
		ostringstream oss;
		oss << "on_accept error: " << err;
		throw server_exception(oss.str(), client);
	}
	BOOST_LOG_TRIVIAL(info) << "Client accepted!";
	client->start();
	accept_client();
}

void Server::add_num(int num) {
	boost::make_lock_guard(_mutex);
	TreeItem ti(num);
	_bin_tree.insert(BinTree::value_type(num, ti));
}

double Server::calc_res() {
	boost::shared_lock<boost::shared_mutex> lock(_mutex);
	BinTree::const_iterator i = _bin_tree.cbegin();
	double squares_sum = 0;
	while (i != _bin_tree.cend()) {
		squares_sum += i->second.square_num();
		++i;
	}
	double res = squares_sum / _bin_tree.size();
	return res;
}


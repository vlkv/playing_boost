#pragma once
#include "client_connection.h"
#include "tree_item.h"
#include <list>
#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/make_shared.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/lock_guard.hpp>
#include <boost/thread/locks.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/log/trivial.hpp>

using namespace boost::asio;

class Server : public boost::enable_shared_from_this<Server> {
	typedef std::list<ClientConnection::ptr> ClientsList;

	boost::asio::io_service _service;
	bool _started;
	bool _stopped;
	boost::asio::ip::tcp::acceptor _acceptor;
	ClientsList _clients;
	BinTree _bin_tree;
	boost::thread _tree_dumper;
	int _dump_interval_sec;
	boost::shared_mutex _mutex;
	std::string _dump_filename;


public:
	typedef boost::shared_ptr<Server> ptr;
	
	Server(int port, int dump_interval_sec, std::string dump_filename);
	void start();
	void stop_async();
	void add_num(int num);
	double calc_res();
	void dump_tree();
	virtual ~Server();
	
private:
	void dump_tree_impl(boost::archive::binary_oarchive &oa);
	void service_run_loop();
	void accept_client();
	void on_accept(ClientConnection::ptr client, const boost::system::error_code &err);
	void stop();
	void stop_wait_for_clients_to_stop();
	void stop_finish();
};
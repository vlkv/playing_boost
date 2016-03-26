#pragma once
#include "client_connection.h"
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/make_shared.hpp>
#include <list>
#include "tree_item.h"
#include <boost/thread/thread.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/log/trivial.hpp>

using namespace boost::asio;
using namespace std;

class Server : public boost::enable_shared_from_this<Server> {
	typedef std::list<ClientConnection::ptr> ClientsList;

	io_service _service;
	bool _started;
	ip::tcp::acceptor _acceptor;
	ClientsList _clients;
	BinTree _bin_tree;
	boost::thread _tree_dumper;
	int _dump_interval_sec;
	boost::shared_mutex _mutex;
	std::string _dump_filename;


public:
	typedef boost::shared_ptr<Server> ptr;
	
	Server(int port, int dump_interval_sec, std::string dump_filename);
	virtual ~Server();
	void start();
	void stop_async();
	double add_num_calc_res(int num);
	void dump_tree();
	
	

private:
	void dump_tree_impl(boost::archive::binary_oarchive &oa);
	void Server::accept_client();
	void on_accept(ClientConnection::ptr client, const boost::system::error_code &err);
	void stop();
};
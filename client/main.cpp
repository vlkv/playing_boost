#include <boost/make_shared.hpp>
#include "client.h"
#include <boost/program_options.hpp>
#include <iostream>
#include <string>

namespace po = boost::program_options;
using namespace std;

int main(int argc, char* argv[]) {

	int port;
	string host;

	po::options_description desc("Client options");
	desc.add_options()
		("help", "print help")
		("port", po::value<int>(&port)->default_value(8001), "server port number")
		("host", po::value<std::string>(&host)->default_value("127.0.0.1"), "server IP address")
		;

	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);

	if (vm.count("help")) {
		cout << desc << endl;
		return 1;
	}

	boost::shared_ptr<Client> client = boost::make_shared<Client>(host, port);
	client->start();
}














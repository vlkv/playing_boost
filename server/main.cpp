#include "server.h"
#include <boost/make_shared.hpp>
#include <iostream>
#include <boost/program_options.hpp>
#include <string>

namespace po = boost::program_options;
using namespace std;

int main(int argc, char* argv[]) {

	int port;
	std::string dump_filename;
	int dump_interval_sec;

	po::options_description desc("Server options");
	desc.add_options()
		("help", "print help")
		("port", po::value<int>(&port)->default_value(8001), "port number that server listens to")
		("dump_filename", po::value<std::string>(&dump_filename)->default_value("bin_tree.dump"), "filename where to write binary tree dumps")
		("dump_interval_sec", po::value<int>(&dump_interval_sec)->default_value(2), "Interval (seconds) between binary tree dump")
	;

	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);

	if (vm.count("help")) {
		cout << desc << endl;
		return 1;
	}

	boost::shared_ptr<Server> s = boost::make_shared<Server>(port, dump_interval_sec, dump_filename);
	s->start();
	cout << "Exit" << endl;
}

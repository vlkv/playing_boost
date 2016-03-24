#include "client.h"
#include <iostream>
#include <string>
#include <boost/make_shared.hpp>
#include <boost/program_options.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/console.hpp>

namespace po = boost::program_options;
using namespace std;

int main(int argc, char* argv[]) {
	int port;
	string host;
	string log_filename;

	po::options_description desc("Client options");
	desc.add_options()
		("help", "Print help")
		("port", po::value<int>(&port)->default_value(8001), "Server port number")
		("host", po::value<std::string>(&host)->default_value("127.0.0.1"), "Server IP address")
		("log_file", po::value<std::string>(&log_filename)->default_value("client.log"), "Log filename")
		;

	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);

	if (vm.count("help")) {
		cout << desc << endl;
		return 1;
	}

	boost::log::add_file_log(log_filename);
	boost::log::add_console_log();
	boost::log::add_common_attributes();
	
	BOOST_LOG_TRIVIAL(info) << ">>>>>> Client started >>>>>>";
	boost::shared_ptr<Client> client = boost::make_shared<Client>(host, port);
	client->start();
	BOOST_LOG_TRIVIAL(info) << "<<<<<< Client is done <<<<<<";
}














#include "server.h"
#include <iostream>
#include <string>
#include <boost/program_options.hpp>
#include <boost/make_shared.hpp>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/console.hpp>

namespace po = boost::program_options;

int main(int argc, char* argv[]) {
	int port;
	std::string dump_filename;
	int dump_interval_sec;
	std::string log_filename;

	po::options_description desc("Server options");
	desc.add_options()
		("help", "Print help")
		("port", po::value<int>(&port)->default_value(8001), "Port number that server listens to")
		("dump_file", po::value<std::string>(&dump_filename)->default_value("bin_tree.dump"), "Filename where to write binary tree dumps")
		("dump_interval_sec", po::value<int>(&dump_interval_sec)->default_value(2), "Interval (seconds) between binary tree dump")
		("log_file", po::value<std::string>(&log_filename)->default_value("server.log"), "Log filename")
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
	boost::log::add_common_attributes(); // TODO: it has no effect, why?..

	BOOST_LOG_TRIVIAL(info) << ">>>>>> Server started >>>>>>";
	boost::shared_ptr<Server> s = boost::make_shared<Server>(port, dump_interval_sec, dump_filename);
	s->start();
	BOOST_LOG_TRIVIAL(info) << "<<<<<< Server is done <<<<<<";
}

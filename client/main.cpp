#include "client.h"
#include <iostream>
#include <string>
#include <boost/make_shared.hpp>
#include <boost/program_options.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/thread.hpp>
#include <windows.h>

namespace po = boost::program_options;

Client *c_ptr;
BOOL WINAPI CtrlHandler(DWORD fdwCtrlType);

int main(int argc, char* argv[]) {
	int port;
	std::string host;
	std::string log_filename;

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
		std::cout << desc << std::endl;
		return 1;
	}

	boost::log::add_file_log(log_filename);
	boost::log::add_console_log();
	
	BOOST_LOG_TRIVIAL(info) << ">>>>>> Client app started >>>>>>";
	boost::shared_ptr<Client> c = boost::make_shared<Client>(host, port);
	c_ptr = c.get();
	BOOL ret = SetConsoleCtrlHandler(CtrlHandler, TRUE);
	c->start();
	if (!c.unique()) {
		BOOST_LOG_TRIVIAL(error) << "Client failed to stop correctly! Use count=" << c.use_count();
	}

	BOOST_LOG_TRIVIAL(info) << "<<<<<< Client app is done <<<<<<";
}

BOOL WINAPI CtrlHandler(DWORD ctrlType) {
	switch (ctrlType) {
	case CTRL_C_EVENT:
	case CTRL_CLOSE_EVENT:
	case CTRL_BREAK_EVENT:
	case CTRL_LOGOFF_EVENT:
	case CTRL_SHUTDOWN_EVENT:
		BOOST_LOG_TRIVIAL(info) << "ConsoleCtrl event detected, " << ctrlType;
		c_ptr->stop_async();
		return TRUE;
	default:
		return FALSE;
	}
}













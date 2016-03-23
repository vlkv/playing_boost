
#include <boost/make_shared.hpp>
#include "client.h"

int main(int argc, char* argv[]) {
	// TODO: get host port from args
	boost::shared_ptr<Client> client = boost::make_shared<Client>("127.0.0.1", 8001);
	client->start();
}














#include "server.h"
#include <boost/make_shared.hpp>

int main(int argc, char* argv[]) {
	// TODO: get port from args
	boost::shared_ptr<Server> s = boost::make_shared<Server>(8001);
	s->start();
}

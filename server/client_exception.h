#pragma once
#include <string>
#include "client_connection.h"

class client_exception : public std::exception
{
	ClientConnection::ptr _client;
public:
	explicit client_exception(const std::string& _Message, ClientConnection::ptr c) 
		: std::exception(_Message.c_str()),
		_client(c) {
	}

	explicit client_exception(const char *_Message, ClientConnection::ptr c) 
		: std::exception(_Message),
		_client(c) {
	}
};
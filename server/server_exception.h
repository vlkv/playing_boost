#pragma once
#include <string>
#include "client_connection.h"

class server_exception : public std::exception
{
	ClientConnection::ptr _client; // TODO: maybe it's better use client id here?..
public:
	explicit server_exception(const std::string& _Message, ClientConnection::ptr c)
		: std::exception(_Message.c_str()),
		_client(c) {
	}

	explicit server_exception(const char *_Message, ClientConnection::ptr c)
		: std::exception(_Message),
		_client(c) {
	}

	ClientConnection::ptr client() const { return _client; }
};
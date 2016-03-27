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

class disconnected_exception : public server_exception
{
public:
	explicit disconnected_exception(ClientConnection::ptr c)
		: server_exception("Client disconnected", c) {
	}
};


class accept_aborted_exception : public server_exception
{
public:
	explicit accept_aborted_exception(ClientConnection::ptr c)
		: server_exception("Accept of client aborted", c) {
	}
};

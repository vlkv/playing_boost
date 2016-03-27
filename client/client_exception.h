#pragma once
#include <string>

class client_exception : public std::exception
{
public:
	explicit client_exception(const std::string& _Message)
		: std::exception(_Message.c_str()) {
	}

	explicit client_exception(const char *_Message)
		: std::exception(_Message) {
	}
};
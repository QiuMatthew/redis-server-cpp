#include <algorithm>
#include <iostream>
#include <string>

#include "redis_command.h"

RedisCommand::RedisCommand(std::string& raw_command) {
	// Parse the raw command
	int argc = std::stoi(raw_command.substr(1, raw_command.find("\r\n") - 1));
	raw_command = raw_command.substr(raw_command.find("\r\n") + 2);
	// get the command
	int length = std::stoi(raw_command.substr(1, raw_command.find("\r\n") - 1));
	raw_command = raw_command.substr(raw_command.find("\r\n") + 2);
	command_type = raw_command.substr(0, raw_command.find("\r\n"));
	for (char& c : command_type) {
		c = toupper(c);
	}
	raw_command = raw_command.substr(raw_command.find("\r\n") + 2);
	std::cout << "Command: " << command_type << std::endl;
	// get the arguments
	for (int i = 0; i < argc - 1; i++) {
		int length =
			std::stoi(raw_command.substr(1, raw_command.find("\r\n") - 1));
		raw_command = raw_command.substr(raw_command.find("\r\n") + 2);
		command_args.push_back(raw_command.substr(0, length));
		raw_command = raw_command.substr(raw_command.find("\r\n") + 2);
	}
}

std::string RedisCommand::get_command_type() const { return command_type; }

std::vector<std::string> RedisCommand::get_command_args() const {
	return command_args;
}

std::string RedisCommand::get_command_arg(int index) const {
	return command_args[index];
}

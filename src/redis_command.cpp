#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

#include "redis_command.h"

RedisCommand::RedisCommand(std::string& raw_command) {
	/* // Parse the raw command
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
	} */
	// Parse the raw command
	// First line is the number of arguments
	int start = 0;
	int end = raw_command.find("\r\n");
	if (raw_command[start] != '*') {
		throw std::runtime_error("Invalid command format");
	}
	int argc = std::stoi(raw_command.substr(start + 1, end - 1));
	std::vector<std::string> argv(argc);
	for (int i = 0; i < argc; i++) {
		// Next line is the length of the argument
		start = end + 2;
		end = raw_command.find("\r\n", start);
		if (raw_command[start] != '$') {
			throw std::runtime_error("Invalid command format");
		}
		int length = std::stoi(raw_command.substr(start + 1, end - 1));
		// Next line is the argument
		start = end + 2;
		end = raw_command.find("\r\n", start);
		if (end - start != length) {
			throw std::runtime_error("Lenth of argument does not match");
		}
		argv[i] = raw_command.substr(start, length);
	}
	separate_command_type_and_args(argv);
}

std::string RedisCommand::get_command_type() const { return command_type; }

std::vector<std::string> RedisCommand::get_command_args() const {
	return command_args;
}

std::string RedisCommand::get_command_arg(int index) const {
	return command_args[index];
}

void RedisCommand::separate_command_type_and_args(
	std::vector<std::string>& args) {
	if (args[0] == "CONFIG") {
		command_type = args[0] + " " + args[1];
		command_args = std::vector<std::string>(args.begin() + 2, args.end());
	} else {
		command_type = args[0];
		command_args = std::vector<std::string>(args.begin() + 1, args.end());
	}
	for (char& c : command_type) {
		c = toupper(c);
	}
}

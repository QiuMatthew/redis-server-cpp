#pragma once

#include <string>
#include <vector>

class RedisCommand {
   public:
	RedisCommand(std::string& raw_command);
	std::string get_command_type() const;
	std::vector<std::string> get_command_args() const;
	std::string get_command_arg(int index) const;

   private:
	std::string command_type;
	std::vector<std::string> command_args;
};

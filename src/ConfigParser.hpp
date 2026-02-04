#pragma once

#include "Config.hpp"
#include <string>

class ConfigParser {
public:
	static AppConfig parse(const std::string& filename);
};

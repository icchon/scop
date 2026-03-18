#pragma once
#include "IParser.hpp"

class ObjParser : public IParser {
public:
    ParsedData parse(const std::string& filePath) override;
};

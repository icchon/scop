#pragma once
#include "IParser.hpp"

class TinyObjLoaderParser : public IParser {
public:
    ParsedData parse(const std::string& filePath) override;
};

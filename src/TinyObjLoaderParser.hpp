#pragma once
#include "IParser.hpp"

class TinyObjLoaderParser : public IParser {
public:
    virtual ~TinyObjLoaderParser() = default;
    virtual ParsedData parse(const std::string& filePath) override;
};

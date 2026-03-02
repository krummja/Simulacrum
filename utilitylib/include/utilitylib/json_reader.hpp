#ifndef JSON_READER_HPP__
#define JSON_READER_HPP__

#include <iostream>
#include <optional>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>
#include <cstdint>

class JsonValue;

using JsonObject = std::unordered_map<std::string, JsonValue>;

#endif // JSON_READER_HPP__
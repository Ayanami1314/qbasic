//
// Created by ayanami on 12/2/24.
//
#pragma once
#ifndef UTIL_H
#define UTIL_H
#include <string>
#include <optional>
#include <cassert>
#include <utility>
#include <vector>
#include <sstream>
#include <any>
#include <functional>
template <typename T> class Result {
    std::optional<T> value{};
    std::string message;

  public:
    Result() = default;
    explicit Result(const T &value) : value(value) {}
    explicit Result(std::string message) : message(std::move(message)) {}
    [[nodiscard]] bool is_ok() const { return value.has_value(); }

    const T &get_value() const {
        assert(value.has_value());
        return value.value();
    }
    void set_value(T value) { this->value = value; }
    void set_message(const std::string &msg) { this->message = msg; }
    void set_ok(bool ok) { this->value = std::nullopt; }
    [[nodiscard]] const std::string &get_message() const { return message; }
};
// functions
namespace util {
std::vector<std::string> inline split_by_space(const std::string &line) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream iss(line);
    while (iss >> token) {
        tokens.push_back(token);
    }
    return tokens;
}

template<typename T>
bool ConvAny(const std::any& operand) {
    return std::any_cast<T>(&operand) != nullptr;
}

} // namespace util

#endif // UTIL_H

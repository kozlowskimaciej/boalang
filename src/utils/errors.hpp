/*! @file errors.hpp
    @brief Interpreter related errors.
*/

#ifndef BOALANG_ERRORS_HPP
#define BOALANG_ERRORS_HPP

#include <stdexcept>

/**
 * @brief Represents an interpreter related error.
 */
class RuntimeError : public std::runtime_error {

 public:
//  RuntimeError(const Position& position, const std::string& message)
//      : runtime_error("Line " + std::to_string(position.line) +
//      " column " + std::to_string(position.column) +
//      " at '" + message) {};
  RuntimeError(const std::string& message) : std::runtime_error(message) {};
};

#endif  // BOALANG_ERRORS_HPP

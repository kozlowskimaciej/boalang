/*! @file position.hpp
    @brief Position struct.
*/

#include <variant>

#ifndef BOALANG_POSITION_HPP
#define BOALANG_POSITION_HPP

enum BuiltinType { INT, FLOAT, STR, BOOL, VOID };

using VarType = std::variant<std::string, BuiltinType>;

/**
 * @brief Represents the position in source.
 */
struct Position {
  unsigned int line;
  unsigned int column;
};

#endif  // BOALANG_POSITION_HPP

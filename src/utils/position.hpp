/*! @file position.hpp
    @brief Position struct.
*/

#include <variant>

#ifndef BOALANG_POSITION_HPP
#define BOALANG_POSITION_HPP

enum BuiltinType { IDENTIFIER, INT, FLOAT, STR, BOOL, VOID };

struct VarType {
  std::string name;
  BuiltinType type;

  VarType(BuiltinType type) : type(type) {};
  VarType(std::string name, BuiltinType type) : name(std::move(name)), type(type) {};
};

/**
 * @brief Represents the position in source.
 */
struct Position {
  unsigned int line;
  unsigned int column;
};

#endif  // BOALANG_POSITION_HPP

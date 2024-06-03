/*! @file position.hpp
    @brief Position struct.
*/

#ifndef BOALANG_POSITION_HPP
#define BOALANG_POSITION_HPP

/**
 * @brief Represents the position in source.
 */
struct Position {
  unsigned int line;
  unsigned int column;
};

#endif  // BOALANG_POSITION_HPP

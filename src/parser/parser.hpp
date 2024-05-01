/*! @file parser.hpp
    @brief Parser.
*/

#ifndef BOALANG_PARSER_HPP
#define BOALANG_PARSER_HPP

#include <memory>
#include <vector>

#include <parser/expr/expr.hpp>


class Parser {
 public:
  std::vector<std::unique_ptr<Expr>> parse();

};

#endif //BOALANG_PARSER_HPP

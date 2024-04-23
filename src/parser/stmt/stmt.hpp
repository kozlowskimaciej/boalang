/*! @file stmt.hpp
    @brief Source-related classes.
*/

#ifndef BOALANG_STMT_HPP
#define BOALANG_STMT_HPP

class Stmt {
 public:
  virtual ~Stmt() = default;

  virtual void accept(Stmt_Visitor& stmt_visitor) const = 0;
};

#endif //BOALANG_STMT_HPP

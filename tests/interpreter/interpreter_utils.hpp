#include <gtest/gtest.h>

#include "../utils.hpp"
#include "interpreter/interpreter.hpp"
#include "lexer/lexer.hpp"
#include "parser/parser.hpp"

inline static std::unique_ptr<Program> get_ast(const std::string &code) {
  StringSource source(code);
  Lexer lexer(source);
  Parser parser(lexer);
  return parser.parse();
}

inline static std::string capture_interpreted_stdout(const std::string &code) {
  testing::internal::CaptureStdout();
  auto program = get_ast(code);
  Interpreter interpreter;
  interpreter.visit(*program.get());
  return testing::internal::GetCapturedStdout();
}

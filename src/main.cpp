#include <iostream>
#include <memory>
#include <string>

#include "argparse/argparse.hpp"
#include "ast/astprinter.hpp"
#include "interpreter/interpreter.hpp"
#include "lexer/lexer.hpp"
#include "parser/parser.hpp"
#include "source/source.hpp"

void parse_args(int& argc, char* argv[], argparse::ArgumentParser& program) {
  program.add_argument("source");
  program.add_argument("-c", "--cmd")
      .help("program passed in as string")
      .default_value(false)
      .implicit_value(true);
  program.add_argument("--ast")
      .help("print AST instead of interpreting")
      .flag();

  try {
    program.parse_args(argc, argv);
  } catch (const std::exception& err) {
    std::cerr << err.what() << std::endl;
    std::cerr << program;
    std::exit(1);
  }
}

int main(int argc, char* argv[]) {
  try {
    argparse::ArgumentParser program("boalang");
    parse_args(argc, argv, program);

    std::unique_ptr<Source> src;
    if (program.is_used("--cmd")) {
      src = std::make_unique<StringSource>(program.get<std::string>("source"));
    } else {
      src = std::make_unique<FileSource>(program.get<std::string>("source"));
    }

    Lexer lexer(*src);
    LexerCommentFilter filter(lexer);
    Parser parser(filter);
    if (program.is_used("--ast")) {
      ASTPrinter().print(parser.parse().get());
    } else {
      Interpreter().visit(*parser.parse());
    }
  } catch (const std::runtime_error& error) {
    std::cerr << "[[[Error occurred: " << error.what() << "]]]\n";
    return 1;
  }
  return 0;
}

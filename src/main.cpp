#include <iostream>
#include <memory>
#include <string>

#include "argparse/argparse.hpp"
#include "lexer/lexer.hpp"
#include "parser/parser.hpp"
#include "source/source.hpp"
#include "ast/astprinter.hpp"

void parse_args(int& argc, char* argv[], argparse::ArgumentParser& program) {
  program.add_argument("source");
  program.add_argument("-c", "--cmd")
      .help("program passed in as string")
      .default_value(false)
      .implicit_value(true);

  try {
    program.parse_args(argc, argv);
  } catch (const std::exception& err) {
    std::cerr << err.what() << std::endl;
    std::cerr << program;
    std::exit(1);
  }
}

int main(int argc, char* argv[]) {
  argparse::ArgumentParser program("boalang");
  parse_args(argc, argv, program);

  std::unique_ptr<Source> src;
// if (program.is_used("--cmd")) {
//   src = std::make_unique<StringSource>(program.get<std::string>("source"));
// } else {
//   src = std::make_unique<FileSource>(program.get<std::string>("source"));
// }

//  src = std::make_unique<StringSource>("1 and 3");
  src = std::make_unique<StringSource>("1 + 2 and 3");

  Lexer lexer(*src);
  LexerCommentFilter filter(lexer);
  Parser parser(filter);
  ASTPrinter().print(parser.parse());

  return 0;
}

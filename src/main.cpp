#include <iostream>
#include <memory>
#include <string>

#include "argparse/argparse.hpp"
#include "lexer/lexer.hpp"
#include "source/source.hpp"

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
  if (program["--cmd"] == true) {
    std::cout << "string" << std::endl;
    src = std::make_unique<StringSource>(program.get<std::string>("source"));
  } else {
    std::cout << "file" << std::endl;
    src = std::make_unique<FileSource>(program.get<std::string>("source"));
  }
  auto lexer = Lexer(*src);
  while (true) {
    auto token = lexer.next_token();
    std::cout << token << ' ';
    if (token.type == TokenType::TOKEN_ETX) {
      break;
    }
  }

  return 0;
}

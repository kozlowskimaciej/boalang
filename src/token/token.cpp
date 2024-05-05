#include "token.hpp"

#include <magic_enum/magic_enum.hpp>

template <class... Ts>
struct overloaded : Ts... {
  using Ts::operator()...;
};

template <class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

std::string Token::stringify() const {
  return std::visit(
      overloaded{
          [](auto) { return std::string(); },
          [](int arg) { return std::to_string(arg); },
          [](const std::string& arg) { return arg; },
          [](bool arg) { return std::string(arg ? "true" : "false"); },
      },
      value);
}

std::string Token::stringify_type() const {
  return std::string(magic_enum::enum_name(type));
}

bool Token::has_value() const {
  return !std::holds_alternative<std::monostate>(value);
}

std::ostream& operator<<(std::ostream& os, const Token& token) {
  os << "<" << magic_enum::enum_name(token.get_type());
  if (token.has_value()) {
    os << ", " << token.stringify();
  }
  os << ">";
  return os;
}

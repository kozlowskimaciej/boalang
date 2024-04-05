#include "token.hpp"

#include <magic_enum/magic_enum.hpp>

std::string Token::stringify() const {
  return std::visit(
      [](auto&& arg) -> std::string {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, int> || std::is_same_v<T, float>) {
          return std::to_string(arg);
        } else if constexpr (std::is_same_v<T, std::string>) {
          return arg;
        } else if constexpr (std::is_same_v<T, bool>) {
          return arg ? "true" : "false";
        } else if constexpr (std::is_same_v<T, std::monostate>) {
          return "";
        }
      },
      value);
}

bool Token::has_value() const {
  return std::holds_alternative<std::monostate>(value);
}

std::ostream& operator<<(std::ostream& os, const Token& token) {
  os << "<" << magic_enum::enum_name(token.type);
  if (token.has_value()) {
    os << ", " << token.stringify();
  }
  os << ">";
  return os;
}

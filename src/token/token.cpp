#include "token.hpp"

std::wstring Token::stringify() const {
  if (!value.has_value()) {
    return L"";
  }

  return std::visit(
      [](auto&& arg) -> std::wstring {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, int> || std::is_same_v<T, float>) {
          return std::to_wstring(arg);
        } else if constexpr (std::is_same_v<T, std::wstring>) {
          return arg;
        } else if constexpr (std::is_same_v<T, bool>) {
          return arg ? L"true" : L"false";
        }
      },
      value.value());
}

std::wostream& operator<<(std::wostream& os, const Token& token) {
  std::wstring repr = L"<" + token_repr.at(token.type);
  if (token.value) {
    repr += L", " + token.stringify();
  }
  repr += L">";
  os << repr;
  return os;
}

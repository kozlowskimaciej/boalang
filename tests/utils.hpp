#include <string>

inline static bool str_contains(const std::string& str,
                                const std::string& str2) {
  return str.find(str2) != std::string::npos;
}

#ifndef BOALANG_SOURCE_HPP
#define BOALANG_SOURCE_HPP

#include <fstream>
#include <memory>
#include <sstream>
#include <string>

#include "../utils/position.hpp"

using stream_ptr = std::unique_ptr<std::wistream>;

class Source {
  Position position_ = {1, 0};
  wchar_t current_ = L'\0';

 protected:
  stream_ptr stream_;

 public:
  virtual ~Source() = default;
  [[nodiscard]] const Position& position() const { return position_; }
  wchar_t next();
  wchar_t peek();
};

class FileSource : public Source {
 public:
  explicit FileSource(const std::string& path) {
    stream_ = std::make_unique<std::wifstream>(path);
  }
  ~FileSource() override = default;
};

class StringSource : public Source {
 public:
  explicit StringSource(const std::wstring& source) {
    stream_ = std::make_unique<std::wistringstream>(source);
  }
  ~StringSource() override = default;
};

#endif  // BOALANG_SOURCE_HPP

#ifndef BOALANG_SOURCE_HPP
#define BOALANG_SOURCE_HPP

#include <fstream>
#include <memory>
#include <sstream>
#include <string>

#include "../utils/position.hpp"

using stream_ptr = std::unique_ptr<std::istream>;

class Source {
  Position position_ = {1, 0};
  char current_ = L'\0';

 protected:
  stream_ptr stream_;

 public:
  Source() = default;
  virtual ~Source() = default;

  Source(const Source&) = delete;
  Source& operator=(const Source&) = delete;

  Source(Source&& other) noexcept : stream_(std::move(other.stream_)) {}

  Source& operator=(Source&& other) noexcept {
    if (this != &other) {
      stream_ = std::move(other.stream_);
    }
    return *this;
  }

  [[nodiscard]] const Position& position() const { return position_; }
  char next();
  [[nodiscard]] char peek() const;
  [[nodiscard]] char current() const;
  [[nodiscard]] bool eof() const;
};

class FileSource : public Source {
 public:
  explicit FileSource(const std::string& path) {
    stream_ = std::make_unique<std::ifstream>(path);
  }
};

class StringSource : public Source {
 public:
  explicit StringSource(const std::string& source) {
    stream_ = std::make_unique<std::istringstream>(source);
  }
};

#endif  // BOALANG_SOURCE_HPP

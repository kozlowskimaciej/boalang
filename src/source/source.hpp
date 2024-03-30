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
  char current_ = '\0';

 protected:
  stream_ptr stream_;

 public:
  Source() = default;
  explicit Source(stream_ptr stream) : stream_(std::move(stream)) {}
  virtual ~Source() = default;

  [[nodiscard]] const Position& position() const { return position_; }
  char next();
  [[nodiscard]] char peek() const;
  [[nodiscard]] char current() const;
  [[nodiscard]] bool eof() const;

  virtual void attach(std::string source) = 0;
};

class FileSource : public Source {
 public:
  FileSource() = default;
  explicit FileSource(const std::string& path)
      : Source(std::make_unique<std::ifstream>(path)){};

  void attach(std::string source) override;
};

class StringSource : public Source {
 public:
  StringSource() = default;
  explicit StringSource(const std::string& source)
      : Source(std::make_unique<std::istringstream>(source)){};

  void attach(std::string source) override;
};

#endif  // BOALANG_SOURCE_HPP

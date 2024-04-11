/*! @file source.hpp
    @brief Source-related classes.
*/

#ifndef BOALANG_SOURCE_HPP
#define BOALANG_SOURCE_HPP

#include <fstream>
#include <memory>
#include <sstream>
#include <string>

#include "../utils/position.hpp"

using stream_ptr =
    std::unique_ptr<std::istream>; /**< Unique pointer to a stream. */

/**
 * @brief The Source class represents a source of characters, such as a file or
 * a string.
 */
class Source {
  Position position_ = {1, 0}; /**< Current position within the source. */
  char current_ = '\0';        /**< Current character being processed. */
  stream_ptr stream_;          /**< Pointer to the input stream. */

 public:
  /**
   * @brief Constructs a Source object with the given input stream.
   * @param stream The input stream.
   */
  explicit Source(stream_ptr stream) : stream_(std::move(stream)) {}
  Source(const Source&) = delete;
  Source& operator=(const Source&) = delete;
  virtual ~Source() = default;

  [[nodiscard]] const Position& get_position() const { return position_; }

  /**
   * @brief Retrieves the next character from the source.
   * @return The next character.
   */
  char next();

  /**
   * @brief Peeks at the next character in the source without advancing the
   * position.
   * @return The next character.
   */
  [[nodiscard]] char peek() const;

  /**
   * @brief Retrieves the current character being processed.
   * @return The current character.
   */
  [[nodiscard]] char current() const;

  /**
   * @brief Checks if the end of the source has been reached.
   * @return True if the end of the source has been reached, false otherwise.
   */
  [[nodiscard]] bool eof() const;
};

/**
 * @brief The FileSource class represents a source of characters from a file.
 */
class FileSource : public Source {
 public:
  /**
   * @brief Constructs a FileSource object with the given file path.
   * @param path The path to the file.
   */
  explicit FileSource(const std::string& path)
      : Source(std::make_unique<std::ifstream>(path)){};
};

/**
 * @brief The StringSource class represents a source of characters from a
 * string.
 */
class StringSource : public Source {
 public:
  /**
   * @brief Constructs a StringSource object with the given string.
   * @param source The source string.
   */
  explicit StringSource(const std::string& source)
      : Source(std::make_unique<std::istringstream>(source)){};
};

#endif  // BOALANG_SOURCE_HPP

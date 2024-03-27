#include "source.hpp"

char Source::next() {
  auto prev = current_;
  stream_->get(current_);

  if (stream_->eof()) {
    current_ = '\0';
    return current_;
  }

  if (current_ == '\r' && stream_->peek() == '\n') {
    stream_->ignore();
    current_ = '\n';
  }

  if (prev == '\n') {
    ++position_.line;
    position_.column = 1;
  } else {
    ++position_.column;
  }

  return current_;
}

char Source::peek() const {
  std::wistream::int_type c = stream_->peek();
  if (c == std::wistream::traits_type::eof()) {
    return '\0';
  }
  return static_cast<char>(c);
}

char Source::current() const { return current_; }

bool Source::eof() const { return peek() == '\0'; }

#include "source.hpp"

const char& Source::next() {
  auto prev = current_;
  stream_->get(current_);

  if (stream_->eof()) {
    current_ = L'\0';
    return current_;
  }

  if (current_ == L'\r' && stream_->peek() == L'\n') {
    stream_->ignore();
    current_ = L'\n';
  }

  if (prev == L'\n') {
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
    return L'\0';
  }
  return static_cast<char>(c);
}

const char& Source::current() const { return current_; }

bool Source::eof() const { return peek() == L'\0'; }

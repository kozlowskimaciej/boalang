#include "source.hpp"

wchar_t Source::next() {
  auto prev = current_;
  stream_->get(current_);
  if (stream_->eof()) {
    return L'\0';
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

wchar_t Source::peek() const {
  std::wistream::int_type c = stream_->peek();
  if (c == std::wistream::traits_type::eof()) {
    return L'\0';
  }
  return static_cast<wchar_t>(c);
}

wchar_t Source::current() const { return current_; }

bool Source::eof() const { return peek() == L'\0'; }

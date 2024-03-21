#include "source.hpp"

wchar_t Source::next() {
  auto prev = current_;
  stream_->get(current_);
  if (stream_->eof()) {
    return L'\0';
  }

  if (prev == L'\n') {
    ++position_.line;
    position_.column = 1;
  } else {
    ++position_.column;
  }

  if (current_ == L'\n' and stream_->peek() == L'\r') {
    stream_->ignore();
  }
  return current_;
}

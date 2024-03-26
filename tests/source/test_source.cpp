#include <gtest/gtest.h>

#include "source/source.hpp"

class FileSourceTest : public ::testing::Test {
 protected:
  const std::string tempname_ = std::tmpnam(nullptr);
  std::wofstream stream_;

  FileSourceTest() {
    stream_.open(tempname_);
    std::cout << "temp: " << tempname_ << std::endl;
  }

  ~FileSourceTest() override {
    if (stream_.is_open()) {
      stream_.close();
    }
    std::remove(tempname_.c_str());
  }
};

TEST_F(FileSourceTest, read_from_file) {
  stream_ << L"Hi";
  stream_.flush();

  FileSource source(tempname_);
  EXPECT_EQ(source.next(), L'H');
  EXPECT_EQ(source.next(), L'i');

  EXPECT_EQ(source.position().column, 2);
  EXPECT_EQ(source.position().line, 1);

  // Reaching the end of the file
  EXPECT_EQ(source.next(), L'\0');

  // Check if the position is still the same after reaching the end of the file
  EXPECT_EQ(source.position().column, 2);
  EXPECT_EQ(source.position().line, 1);
}

TEST_F(FileSourceTest, crlf_to_lf) {
  stream_ << L"A\r\nB";
  stream_.flush();

  FileSource source(tempname_);
  EXPECT_EQ(source.next(), L'A');
  EXPECT_EQ(source.next(), L'\n');
  EXPECT_EQ(source.next(), L'B');

  EXPECT_EQ(source.position().column, 1);
  EXPECT_EQ(source.position().line, 2);

  EXPECT_EQ(source.next(), L'\0');
}

TEST(StringSourceTest, read_from_string) {
  StringSource source(L"Hi");
  EXPECT_EQ(source.next(), L'H');
  EXPECT_EQ(source.next(), L'i');

  EXPECT_EQ(source.position().column, 2);
  EXPECT_EQ(source.position().line, 1);

  // Reaching the end of the file
  EXPECT_EQ(source.next(), L'\0');

  // Check if the position is still the same after reaching the end of the file
  EXPECT_EQ(source.position().column, 2);
  EXPECT_EQ(source.position().line, 1);
}

TEST(StringSourceTest, crlf_to_lf) {
  StringSource source(L"A\r\nB");
  EXPECT_EQ(source.next(), L'A');
  EXPECT_EQ(source.next(), L'\n');
  EXPECT_EQ(source.next(), L'B');

  EXPECT_EQ(source.position().column, 1);
  EXPECT_EQ(source.position().line, 2);

  EXPECT_EQ(source.next(), L'\0');
}

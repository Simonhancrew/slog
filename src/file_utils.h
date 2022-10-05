#pragma once

#include <cstdint>
#include <string>

#include "noncopyable.h"
#include "sys/types.h"  // off_t

namespace slog {

// read small file < 64KB
class ReadSmallFile : Noncopyable {
 public:
  ReadSmallFile(std::string filename);
  ~ReadSmallFile();

  // return errno
  template<typename String>
  int ReadToString(int max_size, String* content, int64_t* file_size,
                   int64_t* modify_time, int64_t* create_time);

  /// Read at maxium kBufferSize into buf_
  // return errno
  int ReadToBuffer(int* size);

  const char* Buffer() const {
    return buf_;
  }

  static const int kBufferSize = 64 * 1024;

 private:
  int fd_;
  int err_;
  char buf_[kBufferSize];
};

// read the file content, returns errno if error happens.
template<typename String>
int ReadFile(std::string file_name, int max_size, String* content,
             int64_t* file_size = nullptr, int64_t* modify_time = nullptr,
             int64_t* create_time = nullptr) {
  ReadSmallFile file(filename);
  return file.ReadToString(max_size, content, file_size, modify_time,
                           create_time);
}

// not thread safe
class AppendFile : Noncopyable {
 public:
  explicit AppendFile(std::string filename);

  ~AppendFile();

  void Append(const char* logline, size_t len);

  void Flush();

  off_t WrittenBytes() const {
    return written_bytes_;
  }

 private:
  size_t Write(const char* logline, size_t len);

  FILE* fp_;
  char buffer_[64 * 1024];
  off_t written_bytes_;
};

}  // namespace slog

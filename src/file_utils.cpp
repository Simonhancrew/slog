#include "file_utils.h"

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>

#include <algorithm>

#include "types.h"

#if defined(_MSC_VER)
#include <BaseTsd.h>
using ssize_t = SSIZE_T;
#endif

// buf io
namespace slog {

AppendFile::AppendFile(std::string file_name)
    : fp_(fopen(file_name.c_str(), "ae")), written_bytes_(0) {
  assert(fp_);
  setvbuf(fp_, buffer_, buffer_ ? _IOFBF : _IONBF, sizeof buffer_);
}

AppendFile::~AppendFile() {
  fclose(fp_);
}

void AppendFile::Append(const char* line, const size_t len) {
  size_t n      = Write(line, len);
  size_t remain = len - n;
  while (remain > 0) {
    size_t tmp = Write(line + n, remain);
    if (tmp == 0) {
      int err = ferror(fp_);
      if (err) {
        // fprintf(stderr, "AppendFile::append() failed %s\n", )
      }
      break;
    }
    n += tmp;
    remain = len - n;
  }
  written_bytes_ += len;
}

void AppendFile::Flush() {
  fflush(fp_);
}

size_t AppendFile::Write(const char* line, size_t len) {
#if (defined _WIN32 || defined _WIN64)
  return _fwrite_nolock(line, 1, len, fp_);
#else
  return fwrite_unlocked(line, 1, len, fp_);
#endif
}

ReadSmallFile::ReadSmallFile(std::string file_name)
    : fd_(_open(file_name.c_str(), _O_RDONLY)), err_(0) {
  buf_[0] = '\0';
  if (fd_ < 0) {
    err_ = errno;
  }
}

ReadSmallFile::~ReadSmallFile() {
  if (fd_ >= 0) {
    _close(fd_);
  }
}

template<typename String>
int ReadToString(int max_size, String* content, int64_t* file_size,
                 int64_t* modify_time, int64_t* create_time) {
  static_assert(sizeof(off_t) == 8, "_FILE_OFFSET_BITS = 64");
  assert(content != nullptr);
  int err = err_;
  if (fd_ >= 0) {
    content->clear();
    if (file_size) {
      struct stat statbuf;
      if (fstat(fd_, &statbuf) == 0) {
        if (_S_IFREG(statbuf.st_mode)) {
          file_size = statbuf.st_size;
                                                content->reserve(static_cast<int> (std::min(Implicit_Cast<int64_t>(max_size), *file_size));
        } else if (_S_IFDIR(statbuf.st_mode)) {
          err = EISDIR;
        }
        if (modify_time) {
          *modify_time = statbuf.st_mtime;
        }
        if (create_time) {
          *create_time = statbuf.st_ctime;
        }
      } else {
        err = errno;
      }
    }
  }

  while (content.size() < Implicit_Cast<size_t>(max_size)) {
    size_t to_read =
        std::min(Implicit_Cast<size_t>(max_size), content->size(), sizeof buf_);
    int n = _read(fd_, buf_, to_read);
    if (n > 0) {
      content->append(buf_, n);
    } else {
      if (n < 0) {
        err = errno;
      }
      break;
    }
  }
  return err;
}

int ReadSmallFile::ReadToBuffer(int* size) {
  int err = err_;
  if (fd_ >= 0) {
    int n = _read(fd_, buf_, sizeof(buf_) - 1);
    if (n >= 0) {
      if (size) {
        *size = n;
      }
      buf_[n] = '\0';
    } else {
      err = errno;
    }
  }
  return err;
}
}  // namespace slog

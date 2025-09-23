#include "trace.h"
#include <iostream>
#include <string>
#include <cstring> // Required for std::memcpy


namespace prat {
class string {
public:
  string() : buffer_(), len_(0), cap_(0) {}
  string(const char *obj) {
    std::size_t len{0};
    while (obj[len] != '\0') {
      len++;
    }
    if (len <= SSO_LEN) {
      // initialise stack string
      std::memcpy(buffer_.sbuf_, obj, len);
      len_ = len;
      cap_ = SSO_LEN;
    } else {
      // intiialise heap string
      buffer_.hbuf_ = new char[len];
      std::memcpy(buffer_.hbuf_, obj, len);
      cap_ = len_ = len;
      }
    }

    string(const string &other) {
      if (other.is_sso()) {
       std::memcpy(buffer_.sbuf_, other.buffer_.sbuf_, other.len_);
       len_ = other.len_;
       cap_ = SSO_LEN;
      } else {
	 buffer_.hbuf_ = new char[other.len_];
        std::memcpy(buffer_.hbuf_, other.buffer_.hbuf_, other.len_);
        cap_ = len_ = other.len_;
        }
      }

      string(string &&other) noexcept {
        if (other.is_sso()) {
         std::memcpy(buffer_.sbuf_, other.buffer_.sbuf_, len_);
         cap_ = SSO_LEN; // SSO capacity is fixed
          } else {
            buffer_.hbuf_ = other.buffer_.hbuf_;
            cap_ = other.cap_;
            other.buffer_.hbuf_ = nullptr;
            other.cap_ = 0;
          }


         len_ = other.len_;          
        }



private:
  static constexpr std::size_t SSO_LEN = 15;
  union str{
    char sbuf_[SSO_LEN];
    char *hbuf_;
  };
  str buffer_;
  std::size_t len_;
  std::size_t cap_;

  bool is_sso() const {
    return cap_ <= SSO_LEN;
    }

  
};
};






int main() {
  std::string s;
  std::cout <<sizeof(s);
  prat::string t;
  std::cout <<" " <<sizeof(t);
}

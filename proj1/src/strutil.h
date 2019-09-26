#ifndef STRUTIL_H
#define STRUTIL_H

#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <stdarg.h>
#include <string.h>
#include <cstdio>
#include <cctype>
#include <locale>


#define FMT_MAX_LEN 512

class str_utl {

public:

  static std::vector<std::string> split(const std::string &s, char delim) {
    std::stringstream ss(s);
    std::string item;
    std::vector<std::string> elems;
    while (std::getline(ss, item, delim)) {
      elems.push_back(item);
      //elems.push_back(std::move(item)); // if C++11 (based on comment from @mchiasson)
    }
    return elems;
  }

  // std::vector<std::string> split(const string& input, const string& regex) {
  // // passing -1 as the submatch index parameter performs splitting
  // std::regex re(regex);
  // std::sregex_token_iterator
  // first{input.begin(), input.end(), re, -1},
  // last;
  // return {first, last};
  // }

  template<typename T>
  static std::vector<T> split(const T & str, const T & delimiters) {
    std::vector<T> v;
    typename T::size_type start = 0;
    auto pos = str.find_first_of(delimiters, start);
    while (pos != T::npos) {
      // ignore empty tokens
      if (pos != start) {
        v.emplace_back(str, start, pos - start);
      }
      start = pos + 1;
      pos = str.find_first_of(delimiters, start);
    }
    // ignore trailing delimiter
    if (start < str.length()) {
      v.emplace_back(str, start, str.length() - start); // add what's left of the string
    }
    return v;
  }

  //https://stackoverflow.com/questions/216823/whats-the-best-way-to-trim-stdstring
  static inline bool ends_with(std::string const & value, std::string const & ending) {
    if (ending.size() > value.size()) return false;
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
  }

  static inline std::string to_upper(std::string const &value) {
    std::string str(value);
    std::transform(str.begin(), str.end(), str.begin(), ::toupper);
    return str;
  }

  static inline std::string to_lower(std::string const &value) {
    std::string str(value);
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    return str;
  }

  // trim from start (in place)
  static inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
      return !std::isspace(ch);
    }));
  }

  // trim from end (in place)
  static inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
      return !std::isspace(ch);
    }).base(), s.end());
  }

  // trim from end (in place)
  static inline std::string trim_upper(const std::string &s) {
    std::string r = s;
    str_utl::trim(r);
    return str_utl::to_upper(r);
  }

  // trim from end (in place)
  static inline std::string trim_lower(const std::string &s) {
    std::string r = s;
    str_utl::trim(r);
    return str_utl::to_lower(r);
  }

  // removes all spaces and convert the string to upper
  static inline std::string compact_upper(const std::string &s) {
    std::string r = s;
    r.erase(std::remove_if(r.begin(), r.end(), isspace), r.end());
    return str_utl::to_upper(r);
  }

  // removes all spaces and convert the string to upper
  static inline std::string compact_lower(const std::string &s) {
    std::string r = s;
    r.erase(std::remove_if(r.begin(), r.end(), isspace), r.end());
    return str_utl::to_lower(r);
  }

  // removes all spaces
  static inline std::string compact(const std::string &s) {
    std::string r = s;
    r.erase(std::remove_if(r.begin(), r.end(), isspace), r.end());
    return r;
  }

  // trim from both ends (in place)
  static inline void trim(std::string &s) {
    str_utl::ltrim(s);
    str_utl::rtrim(s);
  }

  // trim from start (copying)
  static inline std::string ltrim_copy(std::string s) {
    str_utl::ltrim(s);
    return s;
  }

  // trim from end (copying)
  static inline std::string rtrim_copy(std::string s) {
    str_utl::rtrim(s);
    return s;
  }

  // trim from both ends (copying)
  static inline std::string trim_copy(std::string s) {
    str_utl::trim(s);
    return s;
  }

  static std::string format(const char *fmt, ...) {
    char buff[FMT_MAX_LEN];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buff, sizeof(buff) - 1, fmt, args);
    va_end(args);
    return std::string(buff);
  }

  static inline std::string to_string(std::ostream& os) {
    std::stringstream ss;
    ss << os.rdbuf();
    std::string s = ss.str();
    return s;
  }

};

#endif
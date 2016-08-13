// Copyright (c) 2016 Philipp Classen
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
//    The above copyright notice and this permission notice shall be included in
//    all copies or substantial portions of the Software.
//
//    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
//    THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//    DEALINGS IN THE SOFTWARE.
//
#ifndef LOW_LEVEL_STRING_POOL_H
#define LOW_LEVEL_STRING_POOL_H

#include <cassert>
#include <cstring>

#ifdef LLSP_HAS_SPARSE_MAP
  #include <sparsehash/sparse_hash_map>
#else
  #include <unordered_map>
#endif

namespace LLSP
{

// Low-level data type. Assumes that strings are interned,
// so it can compare by the pointer alone.
// Should be passed by value.
//
// Cannot be created without the StringPool factory.
class PString
{
public:
  operator const char*() const { return data; }
  bool operator==(const PString& rhs) const { return data == rhs.data; }
private:
  friend class StringPool;
  explicit PString(const char* s) : data{s} { assert(data != 0); }

  const char* data;
};

// Not thread-safe!
//
// Low-level factory to create interned strings.
// Uses reference counting to remove the old data.
//
class StringPool
{
  using ref_counter = uint32_t;
public:
  StringPool();

  // precondition: all PStrings were removed
  ~StringPool();

  // Deallocates all PString references.
  //
  // Note: If possible, avoid this function as it will might leave
  // dangling pointers. Explicitely calling remove(PString) is safer.
  void destroy_all_references();

  PString add(const char* s);
  PString add(const std::string& s);
  void remove(PString s);

  // hint to release unnecessary memory if possible
  void shrink();

  std::string get_debug_info() const;

  ref_counter count(const char* s) const;
  ref_counter count(const std::string& s) const;

  // these operations are expensive as they require a full scan
  ref_counter is_dead(PString s) const;
  ref_counter count_zombie(PString s) const;

  // fast operation but assumes that the string is alive
  ref_counter count_alive(PString s) const;

private:
  struct HashByContent
  {
    inline std::size_t operator()(const char* s) const
    {
      return std::hash<std::string>{}(std::string{s});
    }
  };
  struct EqualByContent
  {
    inline bool operator()(const char* s1, const char* s2) const
    {
#ifdef LLSP_HAS_SPARSE_MAP
      if(s1 == nullptr || s2 == nullptr)
	return s1 == s2;
#endif
      return strcmp(s1, s2) == 0;
    }
  };

#ifdef LLSP_HAS_SPARSE_MAP
  google::sparse_hash_map<const char*, ref_counter,
			  HashByContent, EqualByContent> strings;
#else
  std::unordered_map<const char*, ref_counter,
		     HashByContent, EqualByContent> strings;
#endif
};

} // end namespace LLSP


#endif // end inclusion guard

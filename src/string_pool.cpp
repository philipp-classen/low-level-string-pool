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
#include "string_pool.h"

#include <sstream>
#include <iomanip>
#include <array>
#include <algorithm>

#include <iostream>

namespace LLSP
{

StringPool::StringPool()
{
#ifdef LLSP_HAS_SPARSE_MAP
  strings.set_deleted_key(nullptr);
#endif
}

StringPool::~StringPool()
{
  if(strings.empty() == false)
  {
    constexpr int NUM_TO_SHOW = 20;
    std::cerr << '\n' << get_debug_info() << "\n\n";
    std::cerr << "ERROR: " << strings.size() << " dangling pointers detected!\n"
	      << "Here are the first " << NUM_TO_SHOW << " mappings:\n";
    int counter = 0;
    for(const auto& entry : strings)
    {
      std::cerr << " - " << entry.first << " (" << entry.second << " references)\n";
      if(++counter >= NUM_TO_SHOW)
	break;
    }
    abort();
  }
}

void StringPool::destroy_all_references()
{
  for(auto& entry : strings)
    delete [] entry.first;
  strings.clear();
}

PString StringPool::add(const char* s)
{
  assert(s != nullptr);

  auto it = strings.find(s);
  if(it != strings.end())
  {
    (it->second)++;
    assert(strings[s] >= 2);
    return PString{it->first};
  }
  else
  {
    const auto len = strlen(s);
    char* pCopy = new char[len + 1];
    std::memcpy(pCopy, s, len);
    pCopy[len] = '\0';
    strings[pCopy] = 1;
    assert(strings[pCopy] == 1);
    return PString{pCopy};
  }
}

PString StringPool::add(const std::string& s)
{
  return add(s.c_str());
}

void StringPool::remove(PString s)
{
  auto it = strings.find(s);
  assert(it != strings.end());
  assert(it->first == static_cast<const char*>(s));
  assert(it->second > 0);

  if(--it->second == 0)
  {
    delete [] it->first;
    strings.erase(it);
  }
}

void StringPool::shrink()
{
#ifdef LLSP_HAS_SPARSE_MAP
  strings.resize(0);
#endif
}

std::string StringPool::get_debug_info() const
{
  std::array<uint64_t, 16> counters;
  counters.fill(0);
  uint64_t untracked = 0;
  uint64_t max_value = 0;
  for(const auto& entry : strings)
  {
    uint64_t count = entry.second;
    assert(count > 0);
    if(count < counters.size())
      counters[count]++;
    else
      untracked++;

    max_value = std::max(max_value, count);
  }

  std::ostringstream report;
  report << "Total keys: " << std::setw(10) << strings.size() << '\n';
  for(size_t i = 1; i < counters.size(); i++)
    report << "- with count " << std::setw(6) << i << ": " << counters[i] << '\n';
  report << "- with higher count: " << untracked << '\n';
  report << "The most shared string seen had " << max_value << " references.";
  return report.str();
}

StringPool::ref_counter StringPool::count(const char* s) const
{
  assert(s != nullptr);
  auto it = strings.find(s);
  return it != strings.end() ? it->second : 0;
}

StringPool::ref_counter StringPool::count(const std::string& s) const
{
  return count(s.c_str());
}

StringPool::ref_counter StringPool::is_dead(PString s) const
{
  return count_zombie(s) == 0;
}

StringPool::ref_counter StringPool::count_zombie(PString s) const
{
  for(const auto& entry : strings)
  {
    if(entry.first == s)
    {
      assert(entry.second > 0);
      return entry.second;
    }
  }
  return 0;
}

StringPool::ref_counter StringPool::count_alive(PString s) const
{
#ifndef NDEBUG
  if(strings.size() < 100)
  {
    // the table is still quite small, so we can afford the full scan
    assert(is_dead(s) == false);
  }
#endif

  auto result = count(static_cast<const char*>(s));
  assert(result > 0);
  return result;
}

} // end namespace LLSP

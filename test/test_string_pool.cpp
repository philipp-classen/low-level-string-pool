#include "test/catch.hpp"

#include "string_pool.h"
#include <iostream>

using namespace LLSP;
using namespace std::literals::string_literals;

TEST_CASE("creating and destroying an empty pool should work", "[string_pool]")
{
  StringPool pool;
}

TEST_CASE("deleting dangling references should not leak memory", "[string_pool]")
{
  StringPool pool;
  pool.add("a");
  pool.add("a");
  pool.add("b");
  pool.destroy_all_references();
}

TEST_CASE("must still be usable after destroy", "[string_pool]")
{
  StringPool pool;
  pool.add("a");
  pool.add("a");
  pool.add("b");
  pool.destroy_all_references();

  auto s1 = pool.add("a");
  auto s2 = pool.add("a");
  REQUIRE(pool.count("a") == 2);
  pool.remove(s1);
  pool.remove(s2);
}

TEST_CASE("should not share different strings", "[string_pool]")
{
  StringPool pool;
  auto s1 = pool.add("abc");
  auto s2 = pool.add("def");
  auto s3 = pool.add("ghi"s);

  REQUIRE(std::string(s1) == "abc"s);
  REQUIRE(std::string(s2) == "def"s);
  REQUIRE(std::string(s3) == "ghi"s);
  REQUIRE(pool.count_alive(s1) == 1);
  REQUIRE(pool.count_alive(s2) == 1);
  REQUIRE(pool.count_alive(s3) == 1);

  pool.remove(s1);
  pool.remove(s2);
  pool.remove(s3);

  REQUIRE(pool.count("def") == false);
  REQUIRE(pool.count("ghi") == false);
  REQUIRE(pool.count("abc") == false);
  REQUIRE(pool.is_dead(s1) == true);
  REQUIRE(pool.is_dead(s2) == true);
  REQUIRE(pool.is_dead(s3) == true);
}

TEST_CASE("should share identical strings", "[string_pool]")
{
  StringPool pool;
  auto s1 = pool.add("abc");
  auto s2 = pool.add("abc"s);
  auto s3 = pool.add("ab"s + "c");

  REQUIRE(std::string(s1) == "abc"s);
  REQUIRE(s1 == s2);
  REQUIRE(s1 == s3);
  REQUIRE(static_cast<const char*>(s1) == static_cast<const char*>(s2));
  REQUIRE(static_cast<const char*>(s1) == static_cast<const char*>(s3));

  REQUIRE(pool.count(s1) == 3);
  REQUIRE(pool.count(s2) == 3);
  REQUIRE(pool.count(s3) == 3);
  REQUIRE(pool.count("abc") == 3);
  REQUIRE(pool.count("abc"s) == 3);

  pool.remove(s1);
  pool.remove(s2);
  pool.remove(s3);

  REQUIRE(pool.is_dead(s1) == true);
  REQUIRE(pool.is_dead(s2) == true);
  REQUIRE(pool.is_dead(s3) == true);
}

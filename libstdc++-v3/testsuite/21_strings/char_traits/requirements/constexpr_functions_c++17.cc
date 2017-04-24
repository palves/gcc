// { dg-options "-std=gnu++17" }
// { dg-do compile { target c++1z } }

// Copyright (C) 2017 Free Software Foundation, Inc.
//
// This file is part of the GNU ISO C++ Library.  This library is free
// software; you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the
// Free Software Foundation; either version 3, or (at your option)
// any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along
// with this library; see the file COPYING3.  If not see
// <http://www.gnu.org/licenses/>.

#include <string>

template<typename CT>
  constexpr bool
  test_assign()
  {
    using char_type = typename CT::char_type;

    auto check = [](char_type* s1, const char_type* s2)
      {
	CT::assign(s1[0], s2[0]);
	return s1[0] == char_type{1};
      };

    // const strings.

    {
      char_type s1[2] = {};
      const char_type s2[2] = {1, 0};
      if (!check (s1, s2))
	return false;
    }

    // non-const strings.

    {
      char_type s1[2] = {};
      char_type s2[2] = {1, 0};
      if (!check (s1, s2))
	return false;
    }

    {
      char_type s1[2] = {};
      char_type s2[2] = {};
      s2[0] = char_type{1};
      if (!check (s1, s2))
	return false;
    }

    return true;
  }

template<typename CT>
  constexpr bool
  test_compare()
  {
    using char_type = typename CT::char_type;

    auto check = [](const char_type* s1, const char_type* s2)
      {
	if (CT::compare(s1, s2, 3) != 0)
	  return false;
	if (CT::compare(s2, s1, 3) != 0)
	  return false;
	if (CT::compare(s1+1, s2, 2) <= 0)
	  return false;
	return true;
      };

    // const arrays.

    {
      const char_type s1[3] = {1, 2, 3};
      const char_type s2[3] = {1, 2, 3};
      if (!check (s1, s2))
	return false;
    }

    // non-const arrays.

    {
      char_type s1[3] = {1, 2, 3};
      char_type s2[3] = {1, 2, 3};
      if (!check (s1, s2))
	return false;
    }

    {
      char_type s1[3] = {};
      char_type s2[3] = {};
      for (size_t i = 0; i < 3; i++)
	{
	  s1[i] = char_type(i+1);
	  s2[i] = char_type(i+1);
	}
      if (!check (s1, s2))
	return false;
    }

    return true;
  }

template<typename CT>
  constexpr bool
  test_length()
  {
    using char_type = typename CT::char_type;

    auto check = [](const char_type* s)
      {
	if (CT::length(s) != 3)
	  return false;
	if (CT::length(s+1) != 2)
	  return false;
	return true;
      };

    // const strings.

    {
      const char_type s[4] = {1, 2, 3, 0};
      if (!check (s))
	return false;
    }

    // non-const strings.

    {
      char_type s[4] = {1, 2, 3, 0};
      if (!check (s))
	return false;
    }

    {
      char_type s[4] = {};
      for (size_t i = 0; i < 3; i++)
	s[i] = char_type(i+1);
      if (!check (s))
	return false;
    }

    return true;
  }

template<typename CT>
  constexpr bool
  test_find()
  {
    using char_type = typename CT::char_type;

    auto check = [](const char_type* s)
      {
	if (CT::find(s, 3, char_type{2}) != s+1)
	  return false;
	if (CT::find(s, 3, char_type{4}) != nullptr)
	  return false;
	return true;
      };

    // const arrays.

    {
      const char_type s[3] = {1, 2, 3};
      if (!check(s))
	return false;
    }

    // non-const arrays.

    {
      char_type s[3] = {1, 2, 3};
      if (!check(s))
	return false;
    }

    {
      char_type s[3] = {};
      for (size_t i = 0; i < 3; i++)
	s[i] = char_type(i+1);
      if (!check(s))
	return false;
    }

    return true;
  }

#ifndef __cpp_lib_constexpr_char_traits
# error Feature-test macro for constexpr char_traits is missing
#elif __cpp_lib_constexpr_char_traits != 201611
# error Feature-test macro for constexpr char_traits has the wrong value
#endif

static_assert( test_assign<std::char_traits<char>>() );
static_assert( test_compare<std::char_traits<char>>() );
static_assert( test_length<std::char_traits<char>>() );
static_assert( test_find<std::char_traits<char>>() );
#ifdef _GLIBCXX_USE_WCHAR_T
static_assert( test_assign<std::char_traits<wchar_t>>() );
static_assert( test_compare<std::char_traits<wchar_t>>() );
static_assert( test_length<std::char_traits<wchar_t>>() );
static_assert( test_find<std::char_traits<wchar_t>>() );
#endif
static_assert( test_assign<std::char_traits<char16_t>>() );
static_assert( test_compare<std::char_traits<char16_t>>() );
static_assert( test_length<std::char_traits<char16_t>>() );
static_assert( test_find<std::char_traits<char16_t>>() );
static_assert( test_assign<std::char_traits<char32_t>>() );
static_assert( test_compare<std::char_traits<char32_t>>() );
static_assert( test_length<std::char_traits<char32_t>>() );
static_assert( test_find<std::char_traits<char32_t>>() );

struct C
{
  C() = default;
  constexpr C(auto c_) : c(c_) {}
  unsigned char c;
};
constexpr bool operator==(const C& c1, const C& c2) { return c1.c == c2.c; }
constexpr bool operator<(const C& c1, const C& c2) { return c1.c < c2.c; }
static_assert( test_assign<std::char_traits<C>>() );
static_assert( test_compare<std::char_traits<C>>() );
static_assert( test_length<std::char_traits<C>>() );
static_assert( test_find<std::char_traits<C>>() );

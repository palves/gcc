// class template regex -*- C++ -*-

// Copyright (C) 2013-2014 Free Software Foundation, Inc.
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

// Under Section 7 of GPL version 3, you are granted additional
// permissions described in the GCC Runtime Library Exception, version
// 3.1, as published by the Free Software Foundation.

// You should have received a copy of the GNU General Public License and
// a copy of the GCC Runtime Library Exception along with this program;
// see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
// <http://www.gnu.org/licenses/>.

/**
 *  @file bits/regex_compiler.tcc
 *  This is an internal header file, included by other library headers.
 *  Do not attempt to use it directly. @headername{regex}
 */

// FIXME make comments doxygen format.

// This compiler refers to "Regular Expression Matching Can Be Simple And Fast"
// (http://swtch.com/~rsc/regexp/regexp1.html"),
// but doesn't strictly follow it.
//
// When compiling, states are *chained* instead of tree- or graph-constructed.
// It's more like structured programs: there's if statement and loop statement.
//
// For alternative structure(say "a|b"), aka "if statement", two branchs should
// be constructed. However, these two shall merge to an "end_tag" at the end of
// this operator:
//
//                branch1
//              /        \
// => begin_tag            end_tag =>
//              \        /
//                branch2
//
// This is the difference between this implementation and that in Russ's
// article.
//
// That's why we introduced dummy node here ------ "end_tag" is a dummy node.
// All dummy node will be eliminated at the end of compiling process.

namespace std _GLIBCXX_VISIBILITY(default)
{
namespace __detail
{
_GLIBCXX_BEGIN_NAMESPACE_VERSION

  template<typename _TraitsT>
    _Compiler<_TraitsT>::
    _Compiler(_IterT __b, _IterT __e,
	      const _TraitsT& __traits, _FlagT __flags)
    : _M_flags((__flags
		& (regex_constants::ECMAScript
		   | regex_constants::basic
		   | regex_constants::extended
		   | regex_constants::grep
		   | regex_constants::egrep
		   | regex_constants::awk))
	       ? __flags
	       : __flags | regex_constants::ECMAScript),
    _M_traits(__traits),
    _M_ctype(std::use_facet<_CtypeT>(_M_traits.getloc())),
    _M_scanner(__b, __e, _M_flags, _M_traits.getloc()),
    _M_nfa(_M_flags)
    {
      _StateSeqT __r(_M_nfa, _M_nfa._M_start());
      __r._M_append(_M_nfa._M_insert_subexpr_begin());
      this->_M_disjunction();
      if (!_M_match_token(_ScannerT::_S_token_eof))
	__throw_regex_error(regex_constants::error_paren);
      __r._M_append(_M_pop());
      _GLIBCXX_DEBUG_ASSERT(_M_stack.empty());
      __r._M_append(_M_nfa._M_insert_subexpr_end());
      __r._M_append(_M_nfa._M_insert_accept());
      _M_nfa._M_eliminate_dummy();
    }

  template<typename _TraitsT>
    void
    _Compiler<_TraitsT>::
    _M_disjunction()
    {
      this->_M_alternative();
      while (_M_match_token(_ScannerT::_S_token_or))
	{
	  _StateSeqT __alt1 = _M_pop();
	  this->_M_alternative();
	  _StateSeqT __alt2 = _M_pop();
	  auto __end = _M_nfa._M_insert_dummy();
	  __alt1._M_append(__end);
	  __alt2._M_append(__end);
	  _M_stack.push(_StateSeqT(_M_nfa,
				   _M_nfa._M_insert_alt(__alt1._M_start,
							__alt2._M_start, false),
				   __end));
	}
    }

  template<typename _TraitsT>
    void
    _Compiler<_TraitsT>::
    _M_alternative()
    {
      if (this->_M_term())
	{
	  _StateSeqT __re = _M_pop();
	  this->_M_alternative();
	  __re._M_append(_M_pop());
	  _M_stack.push(__re);
	}
      else
	_M_stack.push(_StateSeqT(_M_nfa, _M_nfa._M_insert_dummy()));
    }

  template<typename _TraitsT>
    bool
    _Compiler<_TraitsT>::
    _M_term()
    {
      if (this->_M_assertion())
	return true;
      if (this->_M_atom())
	{
	  while (this->_M_quantifier());
	  return true;
	}
      return false;
    }

  template<typename _TraitsT>
    bool
    _Compiler<_TraitsT>::
    _M_assertion()
    {
      if (_M_match_token(_ScannerT::_S_token_line_begin))
	_M_stack.push(_StateSeqT(_M_nfa, _M_nfa._M_insert_line_begin()));
      else if (_M_match_token(_ScannerT::_S_token_line_end))
	_M_stack.push(_StateSeqT(_M_nfa, _M_nfa._M_insert_line_end()));
      else if (_M_match_token(_ScannerT::_S_token_word_bound))
	// _M_value[0] == 'n' means it's negtive, say "not word boundary".
	_M_stack.push(_StateSeqT(_M_nfa, _M_nfa.
	      _M_insert_word_bound(_M_value[0] == 'n')));
      else if (_M_match_token(_ScannerT::_S_token_subexpr_lookahead_begin))
	{
	  auto __neg = _M_value[0] == 'n';
	  this->_M_disjunction();
	  if (!_M_match_token(_ScannerT::_S_token_subexpr_end))
	    __throw_regex_error(regex_constants::error_paren);
	  auto __tmp = _M_pop();
	  __tmp._M_append(_M_nfa._M_insert_accept());
	  _M_stack.push(
	      _StateSeqT(
		_M_nfa,
		_M_nfa._M_insert_lookahead(__tmp._M_start, __neg)));
	}
      else
	return false;
      return true;
    }

  template<typename _TraitsT>
    bool
    _Compiler<_TraitsT>::
    _M_quantifier()
    {
      bool __neg = (_M_flags & regex_constants::ECMAScript);
      auto __init = [this, &__neg]()
	{
	  if (_M_stack.empty())
	    __throw_regex_error(regex_constants::error_badrepeat);
	  __neg = __neg && _M_match_token(_ScannerT::_S_token_opt);
	};
      if (_M_match_token(_ScannerT::_S_token_closure0))
	{
	  __init();
	  auto __e = _M_pop();
	  _StateSeqT __r(_M_nfa, _M_nfa._M_insert_repeat(_S_invalid_state_id,
							 __e._M_start, __neg));
	  __e._M_append(__r);
	  _M_stack.push(__r);
	}
      else if (_M_match_token(_ScannerT::_S_token_closure1))
	{
	  __init();
	  auto __e = _M_pop();
	  __e._M_append(_M_nfa._M_insert_repeat(_S_invalid_state_id,
						__e._M_start, __neg));
	  _M_stack.push(__e);
	}
      else if (_M_match_token(_ScannerT::_S_token_opt))
	{
	  __init();
	  auto __e = _M_pop();
	  auto __end = _M_nfa._M_insert_dummy();
	  _StateSeqT __r(_M_nfa, _M_nfa._M_insert_repeat(_S_invalid_state_id,
							 __e._M_start, __neg));
	  __e._M_append(__end);
	  __r._M_append(__end);
	  _M_stack.push(__r);
	}
      else if (_M_match_token(_ScannerT::_S_token_interval_begin))
	{
	  if (_M_stack.empty())
	    __throw_regex_error(regex_constants::error_badrepeat);
	  if (!_M_match_token(_ScannerT::_S_token_dup_count))
	    __throw_regex_error(regex_constants::error_badbrace);
	  _StateSeqT __r(_M_pop());
	  _StateSeqT __e(_M_nfa, _M_nfa._M_insert_dummy());
	  long __min_rep = _M_cur_int_value(10);
	  bool __infi = false;
	  long __n;

	  // {3
	  if (_M_match_token(_ScannerT::_S_token_comma))
	    if (_M_match_token(_ScannerT::_S_token_dup_count)) // {3,7}
	      __n = _M_cur_int_value(10) - __min_rep;
	    else
	      __infi = true;
	  else
	    __n = 0;
	  if (!_M_match_token(_ScannerT::_S_token_interval_end))
	    __throw_regex_error(regex_constants::error_brace);

	  __neg = __neg && _M_match_token(_ScannerT::_S_token_opt);

	  for (long __i = 0; __i < __min_rep; ++__i)
	    __e._M_append(__r._M_clone());

	  if (__infi)
	    {
	      auto __tmp = __r._M_clone();
	      _StateSeqT __s(_M_nfa,
			     _M_nfa._M_insert_repeat(_S_invalid_state_id,
						     __tmp._M_start, __neg));
	      __tmp._M_append(__s);
	      __e._M_append(__s);
	    }
	  else
	    {
	      if (__n < 0)
		__throw_regex_error(regex_constants::error_badbrace);
	      auto __end = _M_nfa._M_insert_dummy();
	      // _M_alt is the "match more" branch, and _M_next is the
	      // "match less" one. Switch _M_alt and _M_next of all created
	      // nodes. This is a hacking but IMO works well.
	      std::stack<_StateIdT> __stack;
	      for (long __i = 0; __i < __n; ++__i)
		{
		  auto __tmp = __r._M_clone();
		  auto __alt = _M_nfa._M_insert_repeat(__tmp._M_start,
						       __end, __neg);
		  __stack.push(__alt);
		  __e._M_append(_StateSeqT(_M_nfa, __alt, __tmp._M_end));
		}
	      __e._M_append(__end);
	      while (!__stack.empty())
		{
		  auto& __tmp = _M_nfa[__stack.top()];
		  __stack.pop();
		  swap(__tmp._M_next, __tmp._M_alt);
		}
	    }
	  _M_stack.push(__e);
	}
      else
	return false;
      return true;
    }

#define __INSERT_REGEX_MATCHER(__func, args...)\
	do\
	  if (!(_M_flags & regex_constants::icase))\
	    if (!(_M_flags & regex_constants::collate))\
	      __func<false, false>(args);\
	    else\
	      __func<false, true>(args);\
	  else\
	    if (!(_M_flags & regex_constants::collate))\
	      __func<true, false>(args);\
	    else\
	      __func<true, true>(args);\
	while (false)

  template<typename _TraitsT>
    bool
    _Compiler<_TraitsT>::
    _M_atom()
    {
      if (_M_match_token(_ScannerT::_S_token_anychar))
	{
	  if (!(_M_flags & regex_constants::ECMAScript))
	    __INSERT_REGEX_MATCHER(_M_insert_any_matcher_posix);
	  else
	    __INSERT_REGEX_MATCHER(_M_insert_any_matcher_ecma);
	}
      else if (_M_try_char())
	__INSERT_REGEX_MATCHER(_M_insert_char_matcher);
      else if (_M_match_token(_ScannerT::_S_token_backref))
	_M_stack.push(_StateSeqT(_M_nfa, _M_nfa.
				 _M_insert_backref(_M_cur_int_value(10))));
      else if (_M_match_token(_ScannerT::_S_token_quoted_class))
	__INSERT_REGEX_MATCHER(_M_insert_character_class_matcher);
      else if (_M_match_token(_ScannerT::_S_token_subexpr_no_group_begin))
	{
	  _StateSeqT __r(_M_nfa, _M_nfa._M_insert_dummy());
	  this->_M_disjunction();
	  if (!_M_match_token(_ScannerT::_S_token_subexpr_end))
	    __throw_regex_error(regex_constants::error_paren);
	  __r._M_append(_M_pop());
	  _M_stack.push(__r);
	}
      else if (_M_match_token(_ScannerT::_S_token_subexpr_begin))
	{
	  _StateSeqT __r(_M_nfa, _M_nfa._M_insert_subexpr_begin());
	  this->_M_disjunction();
	  if (!_M_match_token(_ScannerT::_S_token_subexpr_end))
	    __throw_regex_error(regex_constants::error_paren);
	  __r._M_append(_M_pop());
	  __r._M_append(_M_nfa._M_insert_subexpr_end());
	  _M_stack.push(__r);
	}
      else if (!_M_bracket_expression())
	return false;
      return true;
    }

  template<typename _TraitsT>
    bool
    _Compiler<_TraitsT>::
    _M_bracket_expression()
    {
      bool __neg =
	_M_match_token(_ScannerT::_S_token_bracket_neg_begin);
      if (!(__neg || _M_match_token(_ScannerT::_S_token_bracket_begin)))
	return false;
      __INSERT_REGEX_MATCHER(_M_insert_bracket_matcher, __neg);
      return true;
    }
#undef __INSERT_REGEX_MATCHER

  template<typename _TraitsT>
  template<bool __icase, bool __collate>
    void
    _Compiler<_TraitsT>::
    _M_insert_any_matcher_ecma()
    {
      _M_stack.push(_StateSeqT(_M_nfa,
	_M_nfa._M_insert_matcher
	  (_AnyMatcher<_TraitsT, true, __icase, __collate>
	    (_M_traits))));
    }

  template<typename _TraitsT>
  template<bool __icase, bool __collate>
    void
    _Compiler<_TraitsT>::
    _M_insert_any_matcher_posix()
    {
      _M_stack.push(_StateSeqT(_M_nfa,
	_M_nfa._M_insert_matcher
	  (_AnyMatcher<_TraitsT, false, __icase, __collate>
	    (_M_traits))));
    }

  template<typename _TraitsT>
  template<bool __icase, bool __collate>
    void
    _Compiler<_TraitsT>::
    _M_insert_char_matcher()
    {
      _M_stack.push(_StateSeqT(_M_nfa,
	_M_nfa._M_insert_matcher
	  (_CharMatcher<_TraitsT, __icase, __collate>
	    (_M_value[0], _M_traits))));
    }

  template<typename _TraitsT>
  template<bool __icase, bool __collate>
    void
    _Compiler<_TraitsT>::
    _M_insert_character_class_matcher()
    {
      _GLIBCXX_DEBUG_ASSERT(_M_value.size() == 1);
      _BracketMatcher<_TraitsT, __icase, __collate> __matcher
	(_M_ctype.is(_CtypeT::upper, _M_value[0]), _M_traits);
      __matcher._M_add_character_class(_M_value, false);
      __matcher._M_ready();
      _M_stack.push(_StateSeqT(_M_nfa,
	_M_nfa._M_insert_matcher(std::move(__matcher))));
    }

  template<typename _TraitsT>
  template<bool __icase, bool __collate>
    void
    _Compiler<_TraitsT>::
    _M_insert_bracket_matcher(bool __neg)
    {
      _BracketMatcher<_TraitsT, __icase, __collate> __matcher(__neg, _M_traits);
      while (!_M_match_token(_ScannerT::_S_token_bracket_end))
	_M_expression_term(__matcher);
      __matcher._M_ready();
      _M_stack.push(_StateSeqT(_M_nfa,
			       _M_nfa._M_insert_matcher(std::move(__matcher))));
    }

  template<typename _TraitsT>
  template<bool __icase, bool __collate>
    void
    _Compiler<_TraitsT>::
    _M_expression_term(_BracketMatcher<_TraitsT, __icase, __collate>& __matcher)
    {
      if (_M_match_token(_ScannerT::_S_token_collsymbol))
	__matcher._M_add_collating_element(_M_value);
      else if (_M_match_token(_ScannerT::_S_token_equiv_class_name))
	__matcher._M_add_equivalence_class(_M_value);
      else if (_M_match_token(_ScannerT::_S_token_char_class_name))
	__matcher._M_add_character_class(_M_value, false);
      else if (_M_try_char()) // [a
	{
	  auto __ch = _M_value[0];
	  if (_M_try_char())
	    {
	      if (_M_value[0] == '-') // [a-
		{
		  if (_M_try_char()) // [a-z]
		    {
		      __matcher._M_make_range(__ch, _M_value[0]);
		      return;
		    }
		  // If the dash is the last character in the bracket
		  // expression, it is not special.
		  if (_M_scanner._M_get_token()
		      != _ScannerT::_S_token_bracket_end)
		    __throw_regex_error(regex_constants::error_range);
		}
	      __matcher._M_add_char(_M_value[0]);
	    }
	  __matcher._M_add_char(__ch);
	}
      else if (_M_match_token(_ScannerT::_S_token_quoted_class))
	__matcher._M_add_character_class(_M_value,
					 _M_ctype.is(_CtypeT::upper,
						     _M_value[0]));
      else
	__throw_regex_error(regex_constants::error_brack);
    }

  template<typename _TraitsT>
    bool
    _Compiler<_TraitsT>::
    _M_try_char()
    {
      bool __is_char = false;
      if (_M_match_token(_ScannerT::_S_token_oct_num))
	{
	  __is_char = true;
	  _M_value.assign(1, _M_cur_int_value(8));
	}
      else if (_M_match_token(_ScannerT::_S_token_hex_num))
	{
	  __is_char = true;
	  _M_value.assign(1, _M_cur_int_value(16));
	}
      else if (_M_match_token(_ScannerT::_S_token_ord_char))
	__is_char = true;
      return __is_char;
    }

  template<typename _TraitsT>
    bool
    _Compiler<_TraitsT>::
    _M_match_token(_TokenT token)
    {
      if (token == _M_scanner._M_get_token())
	{
	  _M_value = _M_scanner._M_get_value();
	  _M_scanner._M_advance();
	  return true;
	}
      return false;
    }

  template<typename _TraitsT>
    int
    _Compiler<_TraitsT>::
    _M_cur_int_value(int __radix)
    {
      long __v = 0;
      for (typename _StringT::size_type __i = 0;
	   __i < _M_value.length(); ++__i)
	__v =__v * __radix + _M_traits.value(_M_value[__i], __radix);
      return __v;
    }

  template<typename _TraitsT, bool __icase, bool __collate>
    bool
    _BracketMatcher<_TraitsT, __icase, __collate>::
    _M_apply(_CharT __ch, false_type) const
    {
      bool __ret = false;
      if (std::find(_M_char_set.begin(), _M_char_set.end(),
		    _M_translator._M_translate(__ch))
	  != _M_char_set.end())
	__ret = true;
      else
	{
	  auto __s = _M_translator._M_transform(__ch);
	  for (auto& __it : _M_range_set)
	    if (__it.first <= __s && __s <= __it.second)
	      {
		__ret = true;
		break;
	      }
	  if (_M_traits.isctype(__ch, _M_class_set))
	    __ret = true;
	  else if (std::find(_M_equiv_set.begin(), _M_equiv_set.end(),
			     _M_traits.transform_primary(&__ch, &__ch+1))
		   != _M_equiv_set.end())
	    __ret = true;
	  else
	    {
	      for (auto& __it : _M_neg_class_set)
		if (!_M_traits.isctype(__ch, __it))
		  {
		    __ret = true;
		    break;
		  }
	    }
	}
      if (_M_is_non_matching)
	return !__ret;
      else
	return __ret;
    }

_GLIBCXX_END_NAMESPACE_VERSION
} // namespace __detail
} // namespace

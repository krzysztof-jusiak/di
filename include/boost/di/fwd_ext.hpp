//
// Copyright (c) 2012-2016 Krzysztof Jusiak (krzysztof at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOST_DI_FWD_EXT_HPP
#define BOOST_DI_FWD_EXT_HPP

// libc++
#if __has_include(<__config>)  // __pph__
#include <__config>            // __pph__
#endif                         // __pph__

#if defined(_LIBCPP_VERSION)  // __pph__
_LIBCPP_BEGIN_NAMESPACE_STD
#else   // __pph__
namespace std {
#endif  // __pph__
template <class>
class shared_ptr;
template <class>
class weak_ptr;
template <class, class>
class unique_ptr;
template <class>
struct char_traits;
template <class...>
class tuple;
template <class, class>
class vector;
template <class, class, class>
class set;
template <class>
class move_iterator;
#if defined(_LIBCPP_VERSION)  // __pph__
_LIBCPP_END_NAMESPACE_STD
#else   // __pph__
}  // std
#endif  // __pph__

namespace std {
template<class> class reference_wrapper;
template <class>
class initializer_list;
}  // std
namespace boost {
template<class> class reference_wrapper;
template <class>
class shared_ptr;
}  // boost

#endif

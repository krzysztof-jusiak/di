// Copyright (c) 2012-2016 Krzysztof Jusiak (krzysztof at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOST_DI_TYPE_TRAITS_scope_traits_ext_HPP
#define BOOST_DI_TYPE_TRAITS_scope_traits_ext_HPP

#include "boost/di/fwd.hpp"
#include "boost/di/fwd_ext.hpp"
#include "boost/di/scopes/singleton.hpp"
#include "boost/di/scopes/detail/underlying.hpp"
#include "boost/di/scopes/unique.hpp"

namespace type_traits {

template <class T>
struct scope_traits_ext {
  using type = scopes::unique;
};

template <class T>
struct scope_traits_ext<std::shared_ptr<T>> {
  using type = scopes::singleton_shared;
};

template <class T>
struct scope_traits_ext<T&> {
  using type = scopes::detail::EXTERNAL;
};

template <class T>
using scope_traits_ext_t = typename scope_traits_ext<T>::type;

}  // type_traits

#endif

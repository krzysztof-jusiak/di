// Copyright (c) 2012-2016 Krzysztof Jusiak (krzysztof at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOST_DI_TYPE_TRAITS_EXTERNAL_TRAITS_HPP
#define BOOST_DI_TYPE_TRAITS_EXTERNAL_TRAITS_HPP

#include "boost/di/fwd.hpp"
#include "boost/di/fwd_ext.hpp"

namespace type_traits {

template<class TScope, class>
struct external_traits {
  using type = TScope;
};

template<class T>
struct external_traits<scopes::deduce, T> {
  using type = scopes::unique;
};

template<class T>
struct external_traits<scopes::deduce, std::shared_ptr<T>> {
  using type = scopes::singleton_shared;
};

template<class T>
struct external_traits<scopes::deduce, T&> {
  using type = scopes::EXTERNAL;
};

template<class T>
struct external_traits<scopes::singleton, T> {
  using type = scopes::singleton_non_shared;
};

template<class T, class TDeleter>
struct external_traits<scopes::singleton, std::unique_ptr<T, TDeleter>> {
  using type = scopes::singleton_shared;
};

template<class T>
struct external_traits<scopes::singleton, std::shared_ptr<T>> {
  using type = scopes::singleton_shared;
};

template<class T>
struct external_traits<scopes::singleton, std::shared_ptr<T>&> {
  using type = scopes::singleton_shared;
};

template <class TScope, class T>
using external_traits_t = typename external_traits<TScope, T>::type;

}  // type_traits

#endif

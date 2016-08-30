// Copyright (c) 2012-2016 Krzysztof Jusiak (krzysztof at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOST_DI_TYPE_TRAITS_WRAPPER_TRAITS_HPP
#define BOOST_DI_TYPE_TRAITS_WRAPPER_TRAITS_HPP

#include "boost/di/fwd.hpp"
#include "boost/di/fwd_ext.hpp"

namespace type_traits {

template <class T>
struct wrapper_traits {
  using type = wrappers::unique<class unique, T>;
};

template <class T>
struct wrapper_traits<std::shared_ptr<T>> {
  using type = wrappers::shared<class shared, T>;
};

template <class T>
struct wrapper_traits<std::shared_ptr<T>&> {
  using type = wrappers::shared<class shared, T, std::shared_ptr<T>&>;
};

template <class T>
using wrapper_traits_t = typename wrapper_traits<T>::type;

}  // type_traits

#endif

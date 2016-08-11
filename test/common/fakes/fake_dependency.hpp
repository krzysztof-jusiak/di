//
// Copyright (c) 2012-2016 Krzysztof Jusiak (krzysztof at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOST_DI_FAKE_DEPENDENCY_HPP
#define BOOST_DI_FAKE_DEPENDENCY_HPP

#include "boost/di/core/dependency.hpp"
#include "boost/di/fwd.hpp"
#include "common/fakes/fake_scope.hpp"

template <class T, class TScope = fake_scope<>, class TIsReferable = std::false_type, class TName = no_name>
struct fake_dependency : core::dependency_base, TScope::template scope<T, T> {
  using creator = typename TScope::template scope<T, T>;
  using expected = T;
  using given = T;
  using scope = TScope;
  using name = TName;
  using priority = aux::none_type;

  template <class, class>
  using is_referable = TIsReferable;
};

#endif

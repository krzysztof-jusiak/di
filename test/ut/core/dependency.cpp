//
// Copyright (c) 2012-2016 Krzysztof Jusiak (krzysztof at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "boost/di/core/dependency.hpp"
#include <type_traits>
#include "boost/di/concepts/boundable.hpp"
#include "common/fakes/fake_injector.hpp"
#include "common/fakes/fake_scope.hpp"

namespace core {

test is_dependency_types = [] {
  expect(!std::is_base_of<dependency_base, void>::value);
  expect(!std::is_base_of<dependency_base, int>::value);
  expect(std::is_base_of<dependency_base, dependency<scopes::deduce, int>>::value);
  expect(std::is_base_of<dependency_base, dependency<scopes::deduce, double, double>>::value);
};

struct name {};

test types = [] {
  using dep = dependency<fake_scope<>, int, double, name>;
  expect(std::is_same<fake_scope<>, typename dep::scope>::value);
  expect(std::is_same<int, typename dep::expected>::value);
  expect(std::is_same<double, typename dep::given>::value);
  expect(std::is_same<name, typename dep::name>::value);
};

test def_ctor = [] {
  dependency<scopes::deduce, int> dep;
  (void)dep;
};

test ctor = [] {
  fake_scope<>::ctor_calls() = 0;
  dependency<fake_scope<>, int> dep{0};
  expect(1 == fake_scope<>::ctor_calls());
};

test named = [] {
  using dep1 = dependency<scopes::deduce, int>;
  expect(std::is_same<no_name, typename dep1::name>::value);

  using dep2 = decltype(dep1{}.named(name{}));
  expect(std::is_same<name, typename dep2::name>::value);
};

test in = [] {
  using dep1 = dependency<scopes::deduce, int>;
  expect(std::is_same<scopes::deduce, typename dep1::scope>::value);

  using dep2 = decltype(dep1{}.in(fake_scope<>{}));
  expect(std::is_same<fake_scope<>, typename dep2::scope>::value);
};

test to = [] {
  using dep1 = dependency<scopes::deduce, int>;
  expect(std::is_same<scopes::deduce, typename dep1::scope>::value);

  using dep2 = decltype(dep1{}.to(42));
  expect(std::is_same<scopes::instance, typename dep2::scope>::value);
  expect(std::is_same<int, typename dep2::expected>::value);
  expect(std::is_same<int, typename dep2::given>::value);
};

}  // core

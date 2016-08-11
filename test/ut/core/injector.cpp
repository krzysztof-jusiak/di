//
// Copyright (c) 2012-2016 Krzysztof Jusiak (krzysztof at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "boost/di/injector.hpp"
#include "boost/di/core/injector.hpp"
#include "common/fakes/fake_config.hpp"
#include "common/fakes/fake_dependency.hpp"
#include "common/fakes/fake_injector.hpp"

namespace core {

struct no_ctor {
  template <class...>
  struct scope {};
};

struct ctor {
  template <class...>
  struct scope {
    explicit scope(int) {}
  };
};

struct def1 {
  using expected = int;
  using given = int;
  using scope = no_ctor;
};
struct def2 {
  using expected = int;
  using given = int;
  using scope = no_ctor;
};
struct def_ctor1 {
  using expected = int;
  using given = int;
  using scope = ctor;
};
struct def_ctor2 {
  using expected = int;
  using given = int;
  using scope = ctor;
};

test copyable_types = [] {
  static_expect(std::is_same<aux::type_list<>, copyable_t<aux::type_list<>>>::value);
  static_expect(std::is_same<aux::type_list<>, copyable_t<aux::type_list<def1>>>::value);
  static_expect(std::is_same<aux::type_list<>, copyable_t<aux::type_list<def1, def2>>>::value);
  static_expect(std::is_same<aux::type_list<def_ctor1>, copyable_t<aux::type_list<def1, def_ctor1, def2>>>::value);
  static_expect(
      std::is_same<aux::type_list<def_ctor1, def_ctor2>, copyable_t<aux::type_list<def1, def_ctor1, def2, def_ctor2>>>::value);
};

template <class T, class TIsReferable>
using dep = fake_dependency<T, fake_scope<>, TIsReferable>;

struct c {};

test referable_types = [] {
  static_expect(std::is_same<int, referable_t<int, fake_injector<>, dep<int, std::false_type>>>::value);
  static_expect(std::is_same<int, referable_t<int, fake_injector<>, dep<int, std::true_type>>>::value);
  static_expect(std::is_same<c, referable_t<const c&, fake_injector<>, dep<c, std::false_type>>>::value);
  static_expect(std::is_same<const c&, referable_t<const c&, fake_injector<>, dep<c, std::true_type>>>::value);
  static_expect(std::is_same<c, referable_t<c&, fake_injector<>, dep<c, std::false_type>>>::value);
  static_expect(std::is_same<c&, referable_t<c&, fake_injector<>, dep<c, std::true_type>>>::value);
  static_expect(std::is_same<c, referable_t<c&, fake_injector<>, dep<c, std::false_type>>>::value);
  static_expect(std::is_same<c&&, referable_t<c&&, fake_injector<>, dep<c, std::true_type>>>::value);
  static_expect(std::is_same<int, referable_t<int&, fake_injector<>, dep<int, std::false_type>>>::value);
  static_expect(std::is_same<int&, referable_t<int&, fake_injector<>, dep<int, std::true_type>>>::value);
  static_expect(std::is_same<int, referable_t<const int&, fake_injector<>, dep<int, std::false_type>>>::value);
  static_expect(std::is_same<const int&, referable_t<const int&, fake_injector<>, dep<int, std::true_type>>>::value);
  static_expect(std::is_same<int&&, referable_t<int&&, fake_injector<>, dep<int, std::true_type>>>::value);
};

test def_ctor = [] {
  injector<config> injector_{core::init{}};
  expect(std::is_same<aux::type_list<>, decltype(injector_)::deps>{});
};

test ctor = [] {
  using dep1 = fake_dependency<int>;
  using dep2 = fake_dependency<double>;
  injector<config, dep1, dep2> injector_{core::init{}};
};

test ctor_injector = [] {
  injector<config> injector1{core::init{}};
  injector<config> injector2{static_cast<injector<config>&&>(injector1)};
  (void)injector2;
};

test create = [] {
  injector<config> injector_{core::init{}};
  expect(0 == injector_.create<int>());
};

}  // core

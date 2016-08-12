//
// Copyright (c) 2012-2016 Krzysztof Jusiak (krzysztof at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOST_DI_CORE_DEPENDENCY_HPP
#define BOOST_DI_CORE_DEPENDENCY_HPP

#include "boost/di/aux_/type_traits.hpp"
#include "boost/di/aux_/utility.hpp"
#include "boost/di/concepts/boundable.hpp"
#include "boost/di/concepts/scopable.hpp"
#include "boost/di/core/array.hpp"
#include "boost/di/fwd.hpp"
#include "boost/di/scopes/deduce.hpp"
#include "boost/di/scopes/external.hpp"
#include "boost/di/scopes/detail/underlying.hpp"

namespace core {

template <class, class>
struct dependency_concept {};

template <class T, class TDependency>
struct dependency_impl : aux::pair<T, TDependency> {};

template <class T>
struct make_dependency_concept {
  using type = dependency_concept<T, no_name>;
};

template <class TName, class T>
struct make_dependency_concept<named<TName, T>> {
  using type = dependency_concept<T, TName>;
};

template <class... Ts, class TName, class TDependency>
struct dependency_impl<dependency_concept<concepts::any_of<Ts...>, TName>, TDependency>
    : aux::pair<dependency_concept<Ts, TName>, TDependency>... {};

template <class... Ts, class TDependency>
struct dependency_impl<dependency_concept<aux::type_list<Ts...>, no_name>, TDependency>
    : aux::pair<typename make_dependency_concept<Ts>::type, TDependency>... {};

struct override {};

template <class TScope, class TExpected, class TGiven, class TName, class TPriority>
class dependency
    : dependency_base,
      TScope::template scope<TExpected, TGiven>,
      public dependency_impl<dependency_concept<TExpected, TName>, dependency<TScope, TExpected, TGiven, TName, TPriority>> {
  template <class, class, class, class, class>
  friend class dependency;
  using scope_t = typename TScope::template scope<TExpected, TGiven>;

  template <class T>
  using externable = aux::integral_constant<bool, aux::always<T>::value && aux::is_same<TExpected, TGiven>::value>;

  template <class T>
  struct ref_traits {
    using type = T;
  };

  template <int N>
  struct ref_traits<const char (&)[N]> {
    using type = TExpected;
  };

  template <int N>
  struct ref_traits<char [N]> {
    using type = TExpected;
  };

  template <class R, class... Ts>
  struct ref_traits<R (&)(Ts...)> {
    using type = TExpected;
  };

  template <class R, class... Ts>
  struct ref_traits<R (Ts...)> {
    using type = TExpected;
  };

  template <class T>
  struct ref_traits<std::shared_ptr<T>&> {
    using type = std::shared_ptr<T>;
  };

  template <class T, class>
  struct deduce_traits {
    using type = T;
  };

  template <class T>
  struct deduce_traits<deduced, T> {
    using type = aux::decay_t<T>;
  };

  template <class T, class U>
  using deduce_traits_t = typename deduce_traits<T, U>::type;

 public:
  using scope = TScope;
  using expected = TExpected;
  using given = TGiven;
  using name = TName;
  using priority = TPriority;

  dependency() noexcept {}

  template <class T>
  explicit dependency(T&& object) noexcept : scope_t(static_cast<T&&>(object)) {}

  template <class T, __BOOST_DI_REQUIRES(aux::is_same<TName, no_name>::value && !aux::is_same<T, no_name>::value) = 0>
  auto named() noexcept {
    return dependency<TScope, TExpected, TGiven, T, TPriority>{static_cast<dependency&&>(*this)};
  }

  template <class T, __BOOST_DI_REQUIRES(aux::is_same<TName, no_name>::value && !aux::is_same<T, no_name>::value) = 0>
  auto named(const T&) noexcept {
    return dependency<TScope, TExpected, TGiven, T, TPriority>{static_cast<dependency&&>(*this)};
  }

  template <class T, __BOOST_DI_REQUIRES_MSG(concepts::scopable<T>) = 0,
            __BOOST_DI_REQUIRES(aux::always<T>::value&& aux::is_same<TScope, scopes::deduce>::value) = 0>
  auto in(const T&)noexcept {
    return dependency<T, TExpected, TGiven, TName, TPriority>{};
  }

  template <class T, __BOOST_DI_REQUIRES(!aux::is_array<TExpected, T>::value) = 0,
            __BOOST_DI_REQUIRES_MSG(concepts::boundable<TExpected, T>) = 0>
  auto to() noexcept {
    return dependency<TScope, TExpected, T, TName, TPriority>{};
  }

  template <class... Ts, __BOOST_DI_REQUIRES(aux::is_array<TExpected, Ts...>::value) = 0>
  auto to() noexcept {
    using type = aux::remove_pointer_t<aux::remove_extent_t<TExpected>>;
    return dependency<TScope, array<type>, array<type, Ts...>, TName, TPriority>{};
  }

  template <class T, __BOOST_DI_REQUIRES_MSG(concepts::boundable<TExpected, T>) = 0,
            __BOOST_DI_REQUIRES(aux::always<T>::value&& aux::is_same<TScope, scopes::deduce>::value) = 0>
  auto to(std::initializer_list<T>&& object) noexcept {
    using type = aux::remove_pointer_t<aux::remove_extent_t<TExpected>>;
    using dependency = dependency<scopes::external, array<type>, std::initializer_list<T>, TName, TPriority>;
    return dependency{object};
  }

  template <class T, __BOOST_DI_REQUIRES(externable<T>::value && aux::is_same<TScope, scopes::deduce>::value) = 0,
            __BOOST_DI_REQUIRES_MSG(concepts::boundable<deduce_traits_t<TExpected, T>, aux::decay_t<T>, aux::valid<>>) = 0>
  auto to(reference_wrapper<T> object) noexcept {
    using dependency =
        dependency<scopes::external, deduce_traits_t<TExpected, T>, typename ref_traits<T>::type, TName, TPriority>;
    return dependency{object};
  }

  template <class T, __BOOST_DI_REQUIRES(externable<T>::value) = 0,
            __BOOST_DI_REQUIRES_MSG(concepts::boundable<deduce_traits_t<TExpected, T>, aux::decay_t<T>, aux::valid<>>) = 0>
  auto to(const T& object) noexcept {
    using dependency =
        dependency<scopes::detail::underlying<typename ref_traits<T>::type, TScope>, deduce_traits_t<TExpected, T>, typename ref_traits<T>::type, TName, TPriority>;
    return dependency{object};
  }

  template <class...>
  dependency& to(...) const noexcept;

  auto operator[](const override&) noexcept {
    return dependency<TScope, TExpected, TGiven, TName, override>{static_cast<dependency&&>(*this)};
  }

#if defined(__cpp_variable_templates)  // __pph__
  dependency& operator()() noexcept { return *this; }
#endif  // __pph__

 protected:
  using scope_t::is_referable;
  using scope_t::create;
  using scope_t::try_create;
  template <class, class>
  static void try_create(...);
};

}  // core

#endif

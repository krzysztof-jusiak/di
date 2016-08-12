//
// Copyright (c) 2012-2016 Krzysztof Jusiak (krzysztof at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOST_DI_INJECTOR_HPP
#define BOOST_DI_INJECTOR_HPP

#include "boost/di/concepts/boundable.hpp"
#include "boost/di/concepts/configurable.hpp"
#include "boost/di/config.hpp"
#include "boost/di/core/injector.hpp"
#include "boost/di/fwd.hpp"
#include "boost/di/type_traits/named_traits.hpp"
#include "boost/di/scopes/detail/expose.hpp"

namespace detail {

template <class>
void create(const aux::true_type&) {}

// clang-format off
template <class>
__BOOST_DI_CONCEPTS_CREATABLE_ERROR_MSG void
create
(const aux::false_type&) {}
// clang-format on

template <class, class, class...>
struct injector;

template <class, class>
struct is_creatable_impl;

template <class TInjector, class TName, class T>
struct is_creatable_impl<TInjector, named<TName, T>> {
  static constexpr auto value = core::injector__<TInjector>::template is_creatable<T, TName>::value;
};

template <class TConfig, class T, class... TGivens>
struct injector<TConfig, int, core::dependency<scopes::detail::expose, T, aux::type_list<TGivens...>>>
    : core::injector<TConfig, core::pool<>, core::dependency<scopes::detail::expose, T, aux::type_list<TGivens...>>> {
  template <class... Ts>
  injector(core::injector<Ts...>&& injector) noexcept  // non explicit
      : core::injector<TConfig, core::pool<>, core::dependency<scopes::detail::expose, T, aux::type_list<TGivens...>>>(
            static_cast<core::injector<Ts...>&&>(injector)) {
    using injector_t = core::injector<Ts...>;
    int _[]{0,
            // clang-format off
            (detail::
create<T> (
			  aux::integral_constant<bool, is_creatable_impl<injector_t, TGivens>::value>{}),
             0)...};
    // clang-format on
    (void)_;
  }
};

}  // detail

template <class T, class... Ts>
using injector = detail::injector<
    BOOST_DI_CFG, __BOOST_DI_REQUIRES_MSG(concepts::boundable<aux::type<T, Ts...>>),
    core::dependency<scopes::detail::expose, aux::unique_t<type_traits::named_decay_t<T>, type_traits::named_decay_t<Ts>...>,
                     aux::type_list<type_traits::add_named_t<T>, type_traits::add_named_t<Ts>...>>>;

// clang-format off
#define __BOOST_DI_EXPOSE_IMPL__(...) decltype(BOOST_DI_NAMESPACE::detail::__VA_ARGS__), // __pph__
#define __BOOST_DI_EXPOSE_IMPL(...) BOOST_DI_NAMESPACE::named<__BOOST_DI_EXPOSE_IMPL__ __VA_ARGS__> // __pph__
#define BOOST_DI_EXPOSE(...) __BOOST_DI_IF(__BOOST_DI_IBP(__VA_ARGS__), __BOOST_DI_EXPOSE_IMPL, __BOOST_DI_EXPAND)(__VA_ARGS__) // __pph__
// clang-format on

#endif

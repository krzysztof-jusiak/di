//
// Copyright (c) 2012-2016 Krzysztof Jusiak (krzysztof at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOST_DI_CONCEPTS_BOUNDABLE_HPP
#define BOOST_DI_CONCEPTS_BOUNDABLE_HPP

#include "boost/di/aux_/type_traits.hpp"
#include "boost/di/aux_/utility.hpp"
#include "boost/di/core/bindings.hpp"
#include "boost/di/fwd.hpp"

namespace concepts {

template <class T, class...>
struct type_ {
  template <class TName>
  struct named {
    struct is_bound_more_than_once : aux::false_type {};
  };
  struct is_bound_more_than_once : aux::false_type {};
  struct is_neither_a_dependency_nor_an_injector : aux::false_type {};
  struct has_disallowed_qualifiers : aux::false_type {};
  struct is_abstract :
#if (BOOST_DI_CFG_DIAGNOSTICS_LEVEL >= 2)  // __pph__
                                           // clang-format off
      decltype(
		  T{}
	  ),
                                           // clang-format on
#endif                                     // __pph__
      aux::false_type {
  };
  template <class>
  struct is_not_related_to : aux::false_type {};
};

template <class...>
struct any_of : aux::false_type {};

template <class... TDeps>
struct is_supported : aux::is_same<aux::bool_list<aux::always<TDeps>::value...>,
                                   aux::bool_list<(aux::is_constructible<TDeps, TDeps&&>::value &&
                                                   (aux::is_a<core::injector_base, TDeps>::value ||
                                                    aux::is_a<core::dependency_base, TDeps>::value))...>> {};

template <class...>
struct get_not_supported;

template <class T>
struct get_not_supported<T> {
  using type = T;
};

template <class T, class... TDeps>
struct get_not_supported<T, TDeps...>
    : aux::conditional<aux::is_a<core::injector_base, T>::value || aux::is_a<core::dependency_base, T>::value,
                       typename get_not_supported<TDeps...>::type, T> {};

template <class>
struct is_unique;

template <class T, class = int>
struct unique_dependency : aux::type<T> {};

template <class T>
struct unique_dependency<T, __BOOST_DI_REQUIRES(aux::is_a<core::dependency_base, T>::value)>
    : aux::pair<aux::pair<typename T::expected, typename T::name>, typename T::priority> {};

template <class... TDeps>
struct is_unique<aux::type_list<TDeps...>> : aux::is_unique<typename unique_dependency<TDeps>::type...> {};

template <class>
struct get_is_unique_error_impl : aux::true_type {};

template <class T, class TName, class TPriority>
struct get_is_unique_error_impl<aux::not_unique<aux::pair<aux::pair<T, TName>, TPriority>>> {
  using type = typename type_<T>::template named<TName>::is_bound_more_than_once;
};

template <class T, class TPriority>
struct get_is_unique_error_impl<aux::not_unique<aux::pair<aux::pair<T, no_name>, TPriority>>> {
  using type = typename type_<T>::is_bound_more_than_once;
};

template <class T>
struct get_is_unique_error_impl<aux::not_unique<T>> {
  using type = typename type_<T>::is_bound_more_than_once;
};

template <class>
struct get_is_unique_error;

template <class... TDeps>
struct get_is_unique_error<aux::type_list<TDeps...>>
    : get_is_unique_error_impl<typename aux::is_unique<typename unique_dependency<TDeps>::type...>::type> {};

template <class... TDeps>
using boundable_bindings =
    aux::conditional_t<is_supported<TDeps...>::value, typename get_is_unique_error<core::bindings_t<TDeps...>>::type,
                       typename type_<typename get_not_supported<TDeps...>::type>::is_neither_a_dependency_nor_an_injector>;

template <class... Ts>
using get_any_of_error = aux::conditional_t<
    aux::is_same<aux::bool_list<aux::always<Ts>::value...>, aux::bool_list<aux::is_same<aux::true_type, Ts>::value...>>::value,
    aux::true_type, any_of<Ts...>>;

template <bool, class...>
struct is_related {
  static constexpr auto value = true;  // allows incomplete types
};

template <class I, class T>
struct is_related<true, I, T> {
  static constexpr auto value =
      aux::is_callable<T>::value ||
      (aux::is_base_of<I, T>::value || (aux::is_convertible<T, I>::value && !aux::is_narrowed<I, T>::value));
};

template <bool, class>
struct is_abstract {
  static constexpr auto value = false;  // allows incomplete types
};

template <class T>
struct is_abstract<true, T> {
  static constexpr auto value = aux::is_abstract<T>::value;
};

auto boundable_impl(any_of<> &&) -> aux::true_type;

template <class T, class... Ts>  // expected
auto boundable_impl(any_of<T, Ts...> &&)
    -> aux::conditional_t<aux::is_same<T, aux::decay_t<T>>::value, decltype(boundable_impl(aux::declval<any_of<Ts...>>())),
                          typename type_<T>::has_disallowed_qualifiers>;

template <class I, class T>
using boundable_impl__ = aux::conditional_t<
    is_related<aux::is_complete<I>::value && aux::is_complete<T>::value, I, T>::value,
    aux::conditional_t<is_abstract<aux::is_complete<T>::value, T>::value, typename type_<T>::is_abstract, aux::true_type>,
    typename type_<T>::template is_not_related_to<I>>;

template <class I, class T>                                                                   // expected -> given
auto boundable_impl(I&&, T &&) -> aux::conditional_t<aux::is_same<T, aux::decay_t<T>>::value  // I is already verified
                                                     ,
                                                     boundable_impl__<I, T>, typename type_<T>::has_disallowed_qualifiers>;

template <class I, class T>  // expected -> given
auto boundable_impl(I&&, T&&, aux::valid<> &&)
    -> aux::conditional_t<is_related<aux::is_complete<I>::value && aux::is_complete<T>::value, I, T>::value, aux::true_type,
                          typename type_<T>::template is_not_related_to<I>>;

template <class I, class T>  // array[]
auto boundable_impl(I* [], T &&) -> aux::conditional_t<aux::is_same<I, aux::decay_t<I>>::value, boundable_impl__<I, T>,
                                                       typename type_<I>::has_disallowed_qualifiers>;

template <class I, class T>  // array[]
auto boundable_impl(I[], T &&) -> aux::conditional_t<aux::is_same<I, aux::decay_t<I>>::value, boundable_impl__<I, T>,
                                                     typename type_<I>::has_disallowed_qualifiers>;

template <class... TDeps>  // bindings
auto boundable_impl(aux::type_list<TDeps...> &&) -> boundable_bindings<TDeps...>;

template <class T, class... Ts>  // any_of
auto boundable_impl(concepts::any_of<Ts...>&&, T &&)
    -> get_any_of_error<decltype(boundable_impl(aux::declval<Ts>(), aux::declval<T>()))...>;

template <class... TDeps>  // make_injector
auto boundable_impl(aux::type<TDeps...> &&) -> typename get_is_unique_error_impl<typename aux::is_unique<TDeps...>::type>::type;

aux::true_type boundable_impl(...);

template <class... Ts>
struct boundable__ {
  using type = decltype(boundable_impl(aux::declval<Ts>()...));
};

template <class... Ts>
using boundable = typename boundable__<Ts...>::type;

}  // concepts

#endif

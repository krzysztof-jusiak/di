//
// Copyright (c) 2012-2016 Krzysztof Jusiak (krzysztof at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOST_DI_AUX_TYPE_TRAITS_HPP
#define BOOST_DI_AUX_TYPE_TRAITS_HPP

#include "boost/di/aux_/compiler.hpp"
#include "boost/di/aux_/utility.hpp"
#include "boost/di/fwd.hpp"

#define __BOOST_DI_HAS_TYPE(name, call_name)            \
  template <class, class = int>                         \
  struct name : BOOST_DI_NAMESPACE::aux::false_type {}; \
                                                        \
  template <class T>                                    \
  struct name<T, BOOST_DI_NAMESPACE::aux::valid_t<typename T::call_name>> : BOOST_DI_NAMESPACE::aux::true_type {}

#define __BOOST_DI_HAS_METHOD(name, call_name)                                                            \
  template <class T, class... TArgs>                                                                      \
  decltype(BOOST_DI_NAMESPACE::aux::declval<T>().call_name(BOOST_DI_NAMESPACE::aux::declval<TArgs>()...), \
           BOOST_DI_NAMESPACE::aux::true_type()) name##_impl(int);                                        \
                                                                                                          \
  template <class, class...>                                                                              \
  BOOST_DI_NAMESPACE::aux::false_type name##_impl(...);                                                   \
                                                                                                          \
  template <class T, class... TArgs>                                                                      \
  struct name : decltype(name##_impl<T, TArgs...>(0)) {}

#define __BOOST_DI_REQUIRES(...) typename BOOST_DI_NAMESPACE::aux::enable_if<__VA_ARGS__, int>::type     // __pph__
#define __BOOST_DI_REQUIRES_MSG(...) typename BOOST_DI_NAMESPACE::aux::concept_check<__VA_ARGS__>::type  // __pph__

namespace aux {

template <class T>
T&& declval();

template <class T, T V>
struct integral_constant {
  using type = integral_constant;
  static constexpr T value = V;
};

using true_type = integral_constant<bool, true>;
using false_type = integral_constant<bool, false>;

template <bool B, class T, class F>
struct conditional {
  using type = T;
};
template <class T, class F>
struct conditional<false, T, F> {
  using type = F;
};
template <bool B, class T, class F>
using conditional_t = typename conditional<B, T, F>::type;

template <bool B, class T = void>
struct enable_if {};
template <class T>
struct enable_if<true, T> {
  using type = T;
};
template <bool B, class T = void>
using enable_if_t = typename enable_if<B, T>::type;

template <class T>
struct concept_check {
  static_assert(T::value, "constraint not satisfied");
};
template <>
struct concept_check<true_type> {
  using type = int;
};

template <class T>
struct remove_reference {
  using type = T;
};
template <class T>
struct remove_reference<T&> {
  using type = T;
};
template <class T>
struct remove_reference<T&&> {
  using type = T;
};
template <class T>
using remove_reference_t = typename remove_reference<T>::type;

template <class T>
struct remove_pointer {
  using type = T;
};
template <class T>
struct remove_pointer<T*> {
  using type = T;
};
template <class T>
using remove_pointer_t = typename remove_pointer<T>::type;

template <class T>
struct remove_smart_ptr {
  using type = T;
};
template <class T, class TDeleter>
struct remove_smart_ptr<std::unique_ptr<T, TDeleter>> {
  using type = T;
};
template <class T>
struct remove_smart_ptr<std::shared_ptr<T>> {
  using type = T;
};
template <class T>
struct remove_smart_ptr<std::weak_ptr<T>> {
  using type = T;
};
template <class T>
struct remove_smart_ptr<boost::shared_ptr<T>> {
  using type = T;
};
template <class T>
using remove_smart_ptr_t = typename remove_smart_ptr<T>::type;

template <class T>
struct remove_qualifiers {
  using type = T;
};
template <class T>
struct remove_qualifiers<const T> {
  using type = T;
};
template <class T>
struct remove_qualifiers<T&> {
  using type = T;
};
template <class T>
struct remove_qualifiers<const T&> {
  using type = T;
};
template <class T>
struct remove_qualifiers<T*> {
  using type = T;
};
template <class T>
struct remove_qualifiers<const T*> {
  using type = T;
};
template <class T>
struct remove_qualifiers<T* const&> {
  using type = T;
};
template <class T>
struct remove_qualifiers<T* const> {
  using type = T;
};
template <class T>
struct remove_qualifiers<const T* const> {
  using type = T;
};
template <class T>
struct remove_qualifiers<T&&> {
  using type = T;
};
template <class T>
using remove_qualifiers_t = typename remove_qualifiers<T>::type;

template <class T>
struct remove_extent {
  using type = T;
};
template <class T>
struct remove_extent<T[]> {
  using type = T;
};
template <class T>
using remove_extent_t = typename remove_extent<T>::type;

template <class T>
struct deref_type {
  using type = T;
};

template <class T, class TDeleter>
struct deref_type<std::unique_ptr<T, TDeleter>> {
  using type = remove_qualifiers_t<typename deref_type<T>::type>;
};

template <class T>
struct deref_type<std::shared_ptr<T>> {
  using type = remove_qualifiers_t<typename deref_type<T>::type>;
};

template <class T>
struct deref_type<boost::shared_ptr<T>> {
  using type = remove_qualifiers_t<typename deref_type<T>::type>;
};

template <class T>
struct deref_type<std::weak_ptr<T>> {
  using type = remove_qualifiers_t<typename deref_type<T>::type>;
};

template <class T, class TAllocator>
struct deref_type<std::vector<T, TAllocator>> {
  using type = core::array<remove_qualifiers_t<typename deref_type<T>::type>>;
};

template <class TKey, class TCompare, class TAllocator>
struct deref_type<std::set<TKey, TCompare, TAllocator>> {
  using type = core::array<remove_qualifiers_t<typename deref_type<TKey>::type>>;
};

template <class T>
using decay_t = typename deref_type<remove_qualifiers_t<T>>::type;

template <class, class>
struct is_same : false_type {};
template <class T>
struct is_same<T, T> : true_type {};

template <class T, class U>
struct is_base_of : integral_constant<bool, __is_base_of(T, U)> {};

template <class T>
struct is_class : integral_constant<bool, __is_class(T)> {};

template <class T>
struct is_abstract : integral_constant<bool, __is_abstract(T)> {};

template <class T>
struct is_polymorphic : integral_constant<bool, __is_polymorphic(T)> {};

template <class...>
using is_valid_expr = true_type;

#if __has_extension(is_constructible) && !((__clang_major__ == 3) && (__clang_minor__ == 5))  // __pph__
template <class T, class... TArgs>
using is_constructible = integral_constant<bool, __is_constructible(T, TArgs...)>;
#else   // __pph__
template <class T, class... TArgs>
decltype(void(T(declval<TArgs>()...)), true_type{}) test_is_constructible(int);
template <class, class...>
false_type test_is_constructible(...);
template <class T, class... TArgs>
struct is_constructible : decltype(test_is_constructible<T, TArgs...>(0)) {};
#endif  // __pph__
template <class T, class... TArgs>
using is_constructible_t = typename is_constructible<T, TArgs...>::type;

template <class T, class... TArgs>
decltype(void(T{declval<TArgs>()...}), true_type{}) test_is_braces_constructible(int);
template <class, class...>
false_type test_is_braces_constructible(...);
template <class T, class... TArgs>
using is_braces_constructible = decltype(test_is_braces_constructible<T, TArgs...>(0));
template <class T, class... TArgs>
using is_braces_constructible_t = typename is_braces_constructible<T, TArgs...>::type;

#if defined(__MSVC__)  // __pph__
template <class T>
struct is_copy_constructible : integral_constant<bool, __is_constructible(T, const T&)> {};
template <class T>
struct is_default_constructible : integral_constant<bool, __is_constructible(T)> {};
#else   // __pph__
template <class T>
using is_copy_constructible = is_constructible<T, const T&>;
template <class T>
using is_default_constructible = is_constructible<T>;
#endif  // __pph__

#if defined(__CLANG__) || defined(__MSVC__)  // __pph__
template <class T, class U>
struct is_convertible : integral_constant<bool, __is_convertible_to(T, U)> {};
#else   // __pph__
struct test_is_convertible__ {
  template <class T>
  static void test(T);
};

template <class T, class U, class = decltype(test_is_convertible__::test<U>(declval<T>()))>
true_type test_is_convertible(int);

template <class, class>
false_type test_is_convertible(...);

template <class T, class U>
using is_convertible = decltype(test_is_convertible<T, U>(0));
#endif  // __pph__

template <class TSrc, class TDst, class U = remove_qualifiers_t<TDst>>
using is_narrowed = integral_constant<bool, !is_class<TSrc>::value && !is_class<U>::value && !is_same<TSrc, U>::value>;

template <class, class...>
struct is_array : false_type {};
template <class T, class... Ts>
struct is_array<T[], Ts...> : true_type {};

template <class T, class = decltype(sizeof(T))>
true_type is_complete_impl(int);

template <class T>
false_type is_complete_impl(...);

template <class T>
struct is_complete : decltype(is_complete_impl<T>(0)) {};

template <class T, class U, class = decltype(sizeof(U))>
is_base_of<T, U> is_a_impl(int);

template <class T, class U>
false_type is_a_impl(...);

template <class T, class U>
struct is_a : decltype(is_a_impl<T, U>(0)) {};

template <class, class...>
struct is_unique_impl;

template <class...>
struct not_unique : false_type {
  using type = not_unique;
};

template <>
struct not_unique<> : true_type {
  using type = not_unique;
};

template <class T>
struct is_unique_impl<T> : not_unique<> {};

template <class T1, class T2, class... Ts>
struct is_unique_impl<T1, T2, Ts...>
    : conditional_t<is_base_of<type<T2>, T1>::value, not_unique<T2>, is_unique_impl<inherit<T1, type<T2>>, Ts...>> {};

template <class... Ts>
using is_unique = is_unique_impl<none_type, Ts...>;

template <class...>
struct unique;

template <class... Rs, class T, class... Ts>
struct unique<type<Rs...>, T, Ts...> : conditional_t<is_base_of<type<T>, inherit<type<Rs>...>>::value,
                                                     unique<type<Rs...>, Ts...>, unique<type<Rs..., T>, Ts...>> {};

template <class... Rs>
struct unique<type<Rs...>> : type_list<Rs...> {};

template <class... Ts>
using unique_t = typename unique<type<>, Ts...>::type;

__BOOST_DI_HAS_METHOD(is_callable_with, operator());

struct callable_base_impl {
  void operator()(...) {}
};

template <class T>
struct callable_base : callable_base_impl, aux::conditional_t<aux::is_class<T>::value, T, aux::none_type> {};

template <typename T>
aux::false_type is_callable_impl(T*, aux::non_type<void (callable_base_impl::*)(...), &T::operator()>* = 0);
aux::true_type is_callable_impl(...);

template <class T>
struct is_callable : decltype(is_callable_impl((callable_base<T>*)0)) {};

template <class>
struct function_traits;

aux::false_type has_shared_ptr_impl(...);

template <class T>
auto has_shared_ptr_impl(T &&) -> aux::is_valid_expr<decltype(std::shared_ptr<T>{})>;

template<class T>
using has_shared_ptr = decltype(has_shared_ptr_impl(declval<T>()));

template <class R, class... TArgs>
struct function_traits<R (*)(TArgs...)> {
  using result_type = R;
  using args = type_list<TArgs...>;
};

template <class R, class... TArgs>
struct function_traits<R(TArgs...)> {
  using result_type = R;
  using args = type_list<TArgs...>;
};

template <class R, class T, class... TArgs>
struct function_traits<R (T::*)(TArgs...)> {
  using result_type = R;
  using args = type_list<TArgs...>;
};

template <class R, class T, class... TArgs>
struct function_traits<R (T::*)(TArgs...) const> {
  using result_type = R;
  using args = type_list<TArgs...>;
};

template <class T>
using function_traits_t = typename function_traits<T>::args;

}  // aux

#endif

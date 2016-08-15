//
// Copyright (c) 2012-2016 Krzysztof Jusiak (krzysztof at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOST_DI_WRAPPERS_SHARED_HPP
#define BOOST_DI_WRAPPERS_SHARED_HPP

#include "boost/di/aux_/type_traits.hpp"
#include "boost/di/aux_/utility.hpp"
#include "boost/di/fwd.hpp"

namespace wrappers {

template <class TScope, class T, class TObject = std::shared_ptr<T>>
struct shared {
  using scope = TScope;

  template <class>
  struct is_referable_impl : aux::true_type {};

  template <class I>
  struct is_referable_impl<std::shared_ptr<I>> : aux::is_same<I, T> {};

  template <class I>
  struct is_referable_impl<boost::shared_ptr<I>> : aux::false_type {};

  template <class T_>
  using is_referable = is_referable_impl<aux::remove_qualifiers_t<T_>>;

  template <class I, __BOOST_DI_REQUIRES(aux::is_convertible<T*, I*>::value) = 0>
  inline operator std::shared_ptr<I>() const noexcept {
    return object;
  }

  inline operator std::shared_ptr<T>&() noexcept { return object; }

  template <class I, __BOOST_DI_REQUIRES(aux::is_convertible<T*, I*>::value) = 0>
  inline operator boost::shared_ptr<I>() const noexcept {
    struct sp_holder {
      std::shared_ptr<T> object;
      void operator()(...) noexcept { object.reset(); }
    };
    return {object.get(), sp_holder{object}};
  }

  template <class I, __BOOST_DI_REQUIRES(aux::is_convertible<T*, I*>::value) = 0>
  inline operator std::weak_ptr<I>() const noexcept {
    return object;
  }

  inline operator T&() noexcept { return *object; }
  inline operator const T&() const noexcept { return *object; }

  TObject object;
};

template <class TScope, class T>
struct shared<TScope, T&> {
  using scope = TScope;

  template <class>
  struct is_referable : aux::true_type {};

  explicit shared(T& object) : object(&object) {}
  explicit shared(std::reference_wrapper<T> object) : object(&object.get()) {}

  template <class I>
  explicit shared(I);  // compile clean

  template <class I, __BOOST_DI_REQUIRES(aux::is_convertible<T, I>::value) = 0>
  inline operator I() const noexcept {
    return *object;
  }

  inline operator T&() const noexcept { return *object; }

  T* object = nullptr;
};

}  // wrappers

#endif

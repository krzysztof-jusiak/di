//
// Copyright (c) 2012-2016 Krzysztof Jusiak (krzysztof at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOST_DI_CORE_ANY_TYPE_HPP
#define BOOST_DI_CORE_ANY_TYPE_HPP

#include "boost/di/aux_/type_traits.hpp"
#include "boost/di/core/binder.hpp"
#include "boost/di/fwd.hpp"

namespace core {

template <class T, class TInjector, class TError = aux::false_type>
struct is_referable__ {
  static constexpr auto value = dependency__<binder::resolve_t<TInjector, T>>::template is_referable<T, TInjector>::value;
};

template <class T, class TInjector>
struct is_referable__<T, TInjector, aux::true_type> {
  static constexpr auto value = true;
};

template <class T, class TInjector, class TError>
struct is_creatable__ {
  static constexpr auto value = injector__<TInjector>::template is_creatable<T>::value;
};

template <class T, class TInjector>
struct is_creatable__<T, TInjector, aux::false_type> {
  static constexpr auto value = true;
};

template <class, class>
struct is_copy_ctor__ : aux::false_type {};

template <class T>
struct is_copy_ctor__<T, T> : aux::true_type {};

#if defined(__GCC__) || defined(__MSVC__)  // __pph__
template <class T>
struct is_copy_ctor__<T, const T> : aux::true_type {};
#endif  // __pph__

template <class TParent, class TInjector, class TError = aux::false_type>
struct any_type {
  template <class T, class = __BOOST_DI_REQUIRES(is_creatable__<T, TInjector, TError>::value)>
  operator T() {
    return static_cast<const core::injector__<TInjector>&>(injector_).create_impl(aux::type<T>{});
  }

  const TInjector& injector_;
};

template <class TParent, class TInjector, class TError = aux::false_type, class TRefError = aux::false_type>
struct any_type_ref {
  template <class T, class = __BOOST_DI_REQUIRES(is_creatable__<T, TInjector, TError>::value)>
  operator T() {
    return static_cast<const core::injector__<TInjector>&>(injector_).create_impl(aux::type<T>{});
  }

#if defined(__GCC__)  // __pph__
  template <class T, class = __BOOST_DI_REQUIRES(is_referable__<T&&, TInjector, TRefError>::value),
            class = __BOOST_DI_REQUIRES(is_creatable__<T&&, TInjector, TError>::value)>
  operator T &&() const {
    return static_cast<const core::injector__<TInjector>&>(injector_).create_impl(aux::type<T&&>{});
  }
#endif  // __pph__

  template <class T, class = __BOOST_DI_REQUIRES(is_referable__<T&, TInjector, TRefError>::value),
            class = __BOOST_DI_REQUIRES(is_creatable__<T&, TInjector, TError>::value)>
  operator T&() const {
    return static_cast<const core::injector__<TInjector>&>(injector_).create_impl(aux::type<T&>{});
  }

  template <class T, class = __BOOST_DI_REQUIRES(is_referable__<const T&, TInjector, TRefError>::value),
            class = __BOOST_DI_REQUIRES(is_creatable__<const T&, TInjector, TError>::value)>
  operator const T&() const {
    return static_cast<const core::injector__<TInjector>&>(injector_).create_impl(aux::type<const T&>{});
  }

  const TInjector& injector_;
};

template <class TParent, class TInjector, class TError = aux::false_type>
struct any_type_1st {
  template <class T, class = __BOOST_DI_REQUIRES(!is_copy_ctor__<TParent, T>::value),
            class = __BOOST_DI_REQUIRES(is_creatable__<T, TInjector, TError>::value)>
  operator T() {
    return static_cast<const core::injector__<TInjector>&>(injector_).create_impl(aux::type<T>{});
  }

  const TInjector& injector_;
};

template <class TParent, class TInjector, class TError = aux::false_type, class TRefError = aux::false_type>
struct any_type_1st_ref {
  template <class T, class = __BOOST_DI_REQUIRES(!is_copy_ctor__<TParent, T>::value),
            class = __BOOST_DI_REQUIRES(is_creatable__<T, TInjector, TError>::value)>
  operator T() {
    return static_cast<const core::injector__<TInjector>&>(injector_).create_impl(aux::type<T>{});
  }

#if defined(__GCC__)  // __pph__
  template <class T, class = __BOOST_DI_REQUIRES(!is_copy_ctor__<TParent, T>::value),
            class = __BOOST_DI_REQUIRES(is_referable__<T&&, TInjector, TRefError>::value),
            class = __BOOST_DI_REQUIRES(is_creatable__<T&&, TInjector, TError>::value)>
  operator T &&() const {
    return static_cast<const core::injector__<TInjector>&>(injector_).create_impl(aux::type<T&&>{});
  }
#endif  // __pph__

  template <class T, class = __BOOST_DI_REQUIRES(!is_copy_ctor__<TParent, T>::value),
            class = __BOOST_DI_REQUIRES(is_referable__<T&, TInjector, TRefError>::value),
            class = __BOOST_DI_REQUIRES(is_creatable__<T&, TInjector, TError>::value)>
  operator T&() const {
    return static_cast<const core::injector__<TInjector>&>(injector_).create_impl(aux::type<T&>{});
  }

  template <class T, class = __BOOST_DI_REQUIRES(!is_copy_ctor__<TParent, T>::value),
            class = __BOOST_DI_REQUIRES(is_referable__<const T&, TInjector, TRefError>::value),
            class = __BOOST_DI_REQUIRES(is_creatable__<const T&, TInjector, TError>::value)>
  operator const T&() const {
    return static_cast<const core::injector__<TInjector>&>(injector_).create_impl(aux::type<const T&>{});
  }

  const TInjector& injector_;
};

namespace successful {

template <class TParent, class TInjector>
struct any_type {
  template <class T>
  operator T() {
    return static_cast<const core::injector__<TInjector>&>(injector_).create_successful_impl(aux::type<T>{});
  }

  const TInjector& injector_;
};

template <class TParent, class TInjector>
struct any_type_ref {
  template <class T>
  operator T() {
    return static_cast<const core::injector__<TInjector>&>(injector_).create_successful_impl(aux::type<T>{});
  }

#if defined(__GCC__)  // __pph__
  template <class T, class = __BOOST_DI_REQUIRES(is_referable__<T&&, TInjector>::value)>
  operator T &&() const {
    return static_cast<const core::injector__<TInjector>&>(injector_).create_successful_impl(aux::type<T&&>{});
  }
#endif  // __pph__

  template <class T, class = __BOOST_DI_REQUIRES(is_referable__<T&, TInjector>::value)>
  operator T&() const {
    return static_cast<const core::injector__<TInjector>&>(injector_).create_successful_impl(aux::type<T&>{});
  }

  template <class T, class = __BOOST_DI_REQUIRES(is_referable__<const T&, TInjector>::value)>
  operator const T&() const {
    return static_cast<const core::injector__<TInjector>&>(injector_).create_successful_impl(aux::type<const T&>{});
  }

  const TInjector& injector_;
};

template <class TParent, class TInjector>
struct any_type_1st {
  template <class T, class = __BOOST_DI_REQUIRES(!is_copy_ctor__<TParent, T>::value)>
  operator T() {
    return static_cast<const core::injector__<TInjector>&>(injector_).create_successful_impl(aux::type<T>{});
  }

  const TInjector& injector_;
};

template <class TParent, class TInjector>
struct any_type_1st_ref {
  template <class T, class = __BOOST_DI_REQUIRES(!is_copy_ctor__<TParent, T>::value)>
  operator T() {
    return static_cast<const core::injector__<TInjector>&>(injector_).create_successful_impl(aux::type<T>{});
  }

#if defined(__GCC__)  // __pph__
  template <class T, class = __BOOST_DI_REQUIRES(!is_copy_ctor__<TParent, T>::value),
            class = __BOOST_DI_REQUIRES(is_referable__<T&&, TInjector>::value)>
  operator T &&() const {
    return static_cast<const core::injector__<TInjector>&>(injector_).create_successful_impl(aux::type<T&&>{});
  }
#endif  // __pph__

  template <class T, class = __BOOST_DI_REQUIRES(!is_copy_ctor__<TParent, T>::value),
            class = __BOOST_DI_REQUIRES(is_referable__<T&, TInjector>::value)>
  operator T&() const {
    return static_cast<const core::injector__<TInjector>&>(injector_).create_successful_impl(aux::type<T&>{});
  }

  template <class T, class = __BOOST_DI_REQUIRES(!is_copy_ctor__<TParent, T>::value),
            class = __BOOST_DI_REQUIRES(is_referable__<const T&, TInjector>::value)>
  operator const T&() const {
    return static_cast<const core::injector__<TInjector>&>(injector_).create_successful_impl(aux::type<const T&>{});
  }

  const TInjector& injector_;
};

}  // successful

template <class>
struct any_type_fwd {
  template <class T>
  operator T();

 private:
  template <class T>
  operator const T&() const;
};

template <class>
struct any_type_ref_fwd {
  template <class T>
  operator T();

  template <class T>
  operator T&() const;

#if defined(__GCC__)  // __pph__
  template <class T>
  operator T &&() const;
#endif  // __pph__

  template <class T>
  operator const T&() const;
};

template <class TParent>
struct any_type_1st_fwd {
  template <class T, class = __BOOST_DI_REQUIRES(!is_copy_ctor__<TParent, T>::value)>
  operator T();

 private:
  template <class T, class = __BOOST_DI_REQUIRES(!is_copy_ctor__<TParent, T>::value)>
  operator const T&() const;
};

template <class TParent>
struct any_type_1st_ref_fwd {
  template <class T, class = __BOOST_DI_REQUIRES(!is_copy_ctor__<TParent, T>::value)>
  operator T();

  template <class T, class = __BOOST_DI_REQUIRES(!is_copy_ctor__<TParent, T>::value)>
  operator T&() const;

#if defined(__GCC__)  // __pph__
  template <class T, class = __BOOST_DI_REQUIRES(!is_copy_ctor__<TParent, T>::value)>
  operator T &&() const;
#endif  // __pph__

  template <class T, class = __BOOST_DI_REQUIRES(!is_copy_ctor__<TParent, T>::value)>
  operator const T&() const;
};

}  // core

#endif

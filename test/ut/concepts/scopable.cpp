//
// Copyright (c) 2012-2016 Krzysztof Jusiak (krzysztof at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "boost/di/concepts/scopable.hpp"
#include <type_traits>
#include "boost/di/aux_/compiler.hpp"
#include "boost/di/scopes/deduce.hpp"
#include "boost/di/scopes/instance.hpp"
#include "boost/di/scopes/singleton.hpp"
#include "boost/di/scopes/unique.hpp"

namespace concepts {

template <class T>
using scopable_error = typename scope<T>::template requires_<typename scope<_, _>::is_referable,
                                                             typename scope<_, _>::try_create, typename scope<_, _>::create>;

test none = [] {
  struct none_scopable {};
  static_expect(!scopable<none_scopable>::value);
};

struct scope_type {
  template <class, class>
  struct scope {
    template <class, class>
    using is_referable = std::false_type;

    template <class T, class TProvider>
    static T try_create(const TProvider&);

    template <class T, class TProvider>
    T create(const TProvider&);
  };
};

test scopable_type = [] { static_expect(scopable<scope_type>::value); };

class scope_private_access {
  template <class, class>
  struct scope {
    template <class T, class TProvider>
    T create(const TProvider&);

    template <class T, class TProvider>
    T try_create(const TProvider&);
  };
};

#if !defined(__MSVC__)
test private_access = [] {
  static_expect(!scopable<scope_private_access>::value);
  static_expect(std::is_same<scopable_error<scope_private_access>, scopable<scope_private_access>>::value);
};
#endif

class scope_missing_create {
 public:
  template <class, class>
  struct scope {
    template <class, class>
    using is_referable = std::false_type;

    template <class T, class TProvider>
    static T try_create(const TProvider&);
  };
};

test missing_create = [] {
  static_expect(!scopable<scope_missing_create>::value);
  static_expect(std::is_same<scopable_error<scope_missing_create>, scopable<scope_missing_create>>::value);
};

class scope_missing_try_create {
 public:
  template <class, class>
  struct scope {
    template <class, class>
    using is_referable = std::false_type;

    template <class T, class TProvider>
    T create(const TProvider&);
  };
};

test missing_try_create = [] {
  static_expect(!scopable<scope_missing_try_create>::value);
  static_expect(std::is_same<scopable_error<scope_missing_try_create>, scopable<scope_missing_try_create>>::value);
};

class scope_missing_is_referable {
 public:
  template <class, class>
  struct scope {
    template <class T, class, class TProvider>
    static T try_create(const TProvider&);

    template <class T, class, class TProvider>
    T create(const TProvider&);
  };
};

test missing_is_referable = [] {
  static_expect(!scopable<scope_missing_is_referable>::value);
  static_expect(std::is_same<scopable_error<scope_missing_is_referable>, scopable<scope_missing_is_referable>>::value);
};

test scopable_scopes = [] {
  static_expect(scopable<scopes::deduce>::value);
  static_expect(scopable<scopes::instance>::value);
  static_expect(scopable<scopes::singleton>::value);
  static_expect(scopable<scopes::unique>::value);
};

}  // concepts

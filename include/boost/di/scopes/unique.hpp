//
// Copyright (c) 2012-2016 Krzysztof Jusiak (krzysztof at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOST_DI_SCOPES_UNIQUE_HPP
#define BOOST_DI_SCOPES_UNIQUE_HPP

#include "boost/di/type_traits/memory_traits.hpp"
#include "boost/di/wrappers/unique.hpp"

namespace scopes {

class unique {
 public:
  template <class, class, class = void>
  class scope {
   public:
    template <class, class>
    using is_referable = aux::false_type;

    template <class T, class, class TProvider>
    static decltype(wrappers::unique<unique, decltype(aux::declval<TProvider>().get(type_traits::memory_traits_t<T>{}))>{
        aux::declval<TProvider>().get(type_traits::memory_traits_t<T>{})})
    try_create(const TProvider&);

    template <class T, class, class TProvider>
    auto create(const TProvider& provider) const {
      using memory = type_traits::memory_traits_t<T>;
      using wrapper = wrappers::unique<unique, decltype(provider.get(memory{}))>;
      return wrapper{provider.get(memory{})};
    }
  };
};

}  // scopes

#endif

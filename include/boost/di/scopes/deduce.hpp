// Copyright (c) 2012-2016 Krzysztof Jusiak (krzysztof at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOST_DI_SCOPES_DEDUCE_HPP
#define BOOST_DI_SCOPES_DEDUCE_HPP

#include "boost/di/type_traits/scope_traits.hpp"

namespace scopes {

class deduce {
 public:
  template <class TExpected, class TGiven, class = void>
  class scope {
   public:
    template <class T, class TInjector>
    using is_referable = typename type_traits::scope_traits_t<T>::template scope<TExpected, TGiven>::template is_referable<T, TInjector>;

    template <class T, class TName, class TProvider>
    static decltype(typename type_traits::scope_traits_t<T>::template scope<TExpected, TGiven>{}.template try_create<T, TName>(
        aux::declval<TProvider>()))
    try_create(const TProvider&);

    template <class T, class TName, class TProvider>
    auto create(const TProvider& provider) {
      using scope_traits = type_traits::scope_traits_t<T>;
      using scope = typename scope_traits::template scope<TExpected, TGiven>;
      return scope{}.template create<T, TName>(provider);
    }
  };
};

}  // scopes

#endif

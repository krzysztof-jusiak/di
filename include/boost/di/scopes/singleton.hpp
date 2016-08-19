//
// Copyright (c) 2012-2016 Krzysztof Jusiak (krzysztof at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOST_DI_SCOPES_SINGLETON_HPP
#define BOOST_DI_SCOPES_SINGLETON_HPP

#include "boost/di/aux_/type_traits.hpp"
#include "boost/di/type_traits/memory_traits.hpp"  // type_traits::stack
#include "boost/di/wrappers/shared.hpp"

namespace scopes {

struct singleton_non_shared {
  template <class, class T>
  class scope {
   public:
    template <class T_, class>
    using is_referable = typename wrappers::shared<singleton, T&>::template is_referable<T_>;

    template <class, class, class TProvider>
    static decltype(wrappers::shared<singleton, T&>{aux::declval<TProvider>().get(type_traits::stack{})}) try_create(
        const TProvider&);

    template <class, class, class TProvider>
    auto create(const TProvider& provider) {
      return create_impl<typename TProvider::injector_t>(provider.get(type_traits::stack{}));
    }

   private:
    template <class, class U>
    wrappers::shared<singleton, T&> create_impl(U&& value) {
      static auto object(static_cast<U&&>(value));
      return wrappers::shared<singleton, T&>(object);
    }
  };
};

struct singleton_shared {
  template <class, class T>
  class scope {
   public:
    template <class T_, class>
    using is_referable = typename wrappers::shared<singleton, T>::template is_referable<T_>;

    template <class, class, class TProvider, class T_ = aux::decay_t<decltype(aux::declval<TProvider>().get())>>
    static decltype(wrappers::shared<singleton, T_>{std::shared_ptr<T_>{std::shared_ptr<T_>{aux::declval<TProvider>().get()}}})
    try_create(const TProvider&);

    template <class, class, class TProvider>
    auto create(const TProvider& provider) {
      return create_impl<typename TProvider::injector_t>(provider.get());
    }

   private:
    template <class, class U>
    auto create_impl(U&& value) {
      using type = aux::decay_t<U>;
      static std::shared_ptr<type> object(static_cast<U&&>(value));
      return wrappers::shared<singleton, type, std::shared_ptr<type>&>{object};
    }
  };
};

class singleton {
 public:
  template <class _, class T, class = aux::has_shared_ptr<T>>
  struct scope : singleton_non_shared::scope<_, T> {};

  template <class _, class T>
  struct scope<_, T, aux::true_type> : singleton_shared::scope<_, T> {};
};

}  // scopes

#endif

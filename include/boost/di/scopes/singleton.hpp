//
// Copyright (c) 2012-2017 Kris Jusiak (kris at jusiak dot net)
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

aux::false_type has_shared_ptr__(...);

#if !defined(BOOST_DI_DISABLE_SHARED_PTR_DEDUCTION)  // __pph__
template <class T>
auto has_shared_ptr__(T &&) -> aux::is_valid_expr<decltype(std::shared_ptr<T>{})>;
#endif  // __pph__

class singleton {
 public:
  template <class, class T, class = decltype(has_shared_ptr__(aux::declval<T>()))>
  class scope {
   public:
    template <class T_>
    using is_referable = typename wrappers::shared<singleton, T&>::template is_referable<T_>;

    template <class, class, class TProvider>
    static decltype(wrappers::shared<singleton, T&>{aux::declval<TProvider>().get(type_traits::stack{})}) try_create(
        const TProvider&);

    template <class, class, class TProvider>
    auto create(const TProvider& provider) {
      return create_impl(provider);
    }

   private:
    template <class TProvider>
    wrappers::shared<singleton, T&> create_impl(const TProvider& provider) {
      static auto object(provider.get(type_traits::stack{}));
      return wrappers::shared<singleton, T&>(object);
    }
  };

  template <class _, class T>
  class scope<_, T, aux::true_type> {
   public:
    template <class T_>
    using is_referable = typename wrappers::shared<singleton, T>::template is_referable<T_>;

    template <class, class, class TProvider, class T_ = aux::decay_t<decltype(aux::declval<TProvider>().get())>>
    static decltype(wrappers::shared<singleton, T_>{std::shared_ptr<T_>{std::shared_ptr<T_>{aux::declval<TProvider>().get()}}})
    try_create(const TProvider&);

    template <class, class, class TProvider>
    auto create(const TProvider& provider) {
      return create_impl<aux::decay_t<decltype(provider.get())>>(provider);
    }

   private:
    template <class T_, class TProvider>
    auto create_impl(const TProvider& provider) {
      auto object = std::static_pointer_cast<T_>(provider.injector_->ptrs[__PRETTY_FUNCTION__]);
      if (!object) {
        provider.injector_->ptrs[__PRETTY_FUNCTION__] = std::shared_ptr<T_>{provider.get()};
      }
      return wrappers::shared<singleton, T_, std::shared_ptr<T_>&>{object};
    }
  };
};

}  // scopes

#endif

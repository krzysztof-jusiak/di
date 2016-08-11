// Copyright (c) 2012-2016 Krzysztof Jusiak (krzysztof at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOST_DI_SCOPES_INSTANCE_HPP
#define BOOST_DI_SCOPES_INSTANCE_HPP

#include "boost/di/aux_/type_traits.hpp"
#include "boost/di/concepts/creatable.hpp"
#include "boost/di/fwd.hpp"
#include "boost/di/wrappers/shared.hpp"
#include "boost/di/wrappers/unique.hpp"

namespace scopes {

class instance;

namespace detail {

template <class T, class TExpected, class TGiven>
struct arg {
  using type = T;
  using expected = TExpected;
  using given = TGiven;
};

template <class T>
struct wrapper_traits {
  using type = wrappers::unique<instance, T>;
};

template <class T>
struct wrapper_traits<std::shared_ptr<T>> {
  using type = wrappers::shared<instance, T>;
};

template <class T>
using wrapper_traits_t = typename wrapper_traits<T>::type;

__BOOST_DI_HAS_TYPE(has_result_type, result_type);

template <class TGiven, class TProvider, class... Ts>
struct is_expr : aux::integral_constant<bool, aux::is_callable_with<TGiven, typename TProvider::injector_t, Ts...>::value &&
                                                  !has_result_type<TGiven>::value> {};

}  // detail

template <class T>
struct wrapper {
  inline operator T() noexcept { return static_cast<T&&>(object); }
  T object;
};

class instance {
 public:
  template <class, class TGiven, class = int>
  struct scope {
    template <class, class>
    using is_referable = aux::false_type;

    explicit scope(const TGiven& object) : object_{object} {}

    template <class, class, class TProvider>
    static wrappers::unique<instance, TGiven> try_create(const TProvider&);

    template <class, class, class TProvider>
    auto create(const TProvider&) const noexcept {
      return wrappers::unique<instance, TGiven>{object_};
    }

    TGiven object_;
  };

  template <class TExpected, class TGiven>
  struct scope<TExpected, std::shared_ptr<TGiven>> {
    template <class T, class>
    using is_referable = typename wrappers::shared<instance, TGiven>::template is_referable<aux::remove_qualifiers_t<T>>;

    explicit scope(const std::shared_ptr<TGiven>& object) : object_{object} {}

    template <class, class, class TProvider>
    static wrappers::shared<instance, TGiven> try_create(const TProvider&);

    template <class, class, class TProvider>
    auto create(const TProvider&) const noexcept {
      return wrappers::shared<instance, TGiven>{object_};
    }

    std::shared_ptr<TGiven> object_;
  };

  template <class TExpected, class TGiven>
  struct scope<TExpected, std::initializer_list<TGiven>> {
    template <class, class>
    using is_referable = aux::false_type;

    scope(const std::initializer_list<TGiven>& object) : object_(object) {}

    template <class, class, class TProvider>
    static std::initializer_list<TGiven> try_create(const TProvider&);

    template <class, class, class TProvider>
    auto create(const TProvider&) const noexcept {
      return wrappers::unique<instance, std::initializer_list<TGiven>>{object_};
    }

    std::initializer_list<TGiven> object_;
  };

  template <class TExpected, class TGiven>
  struct scope<TExpected, TGiven&, __BOOST_DI_REQUIRES(!aux::is_callable<TGiven>::value)> {
    template <class, class>
    using is_referable = aux::true_type;

    explicit scope(TGiven& object) : object_{object} {}

    template <class, class, class TProvider>
    static wrappers::shared<instance, TGiven&> try_create(const TProvider&);

    template <class, class, class TProvider>
    auto create(const TProvider&) const noexcept {
      return object_;
    }

    wrappers::shared<instance, TGiven&> object_;
  };

  template <class TExpected, class TGiven>
  struct scope<TExpected, TGiven, __BOOST_DI_REQUIRES(aux::is_callable<TGiven>::value)> {
    template <class, class>
    using is_referable =
        aux::integral_constant<bool, !aux::is_callable<TExpected>::value || !detail::has_result_type<TExpected>::value>;

    explicit scope(const TGiven& object) : object_(object) {}

#if defined(__MSVC__)  // __pph__
    template <class T, class, class TProvider>
    static T try_create(const TProvider&) noexcept;
#else   // __pph__

    template <class T, class, class TProvider,
              __BOOST_DI_REQUIRES(!detail::is_expr<TGiven, TProvider, const detail::arg<T, TExpected, TGiven>&>::value &&
                                  !detail::is_expr<TGiven, TProvider>::value && aux::is_callable<TGiven>::value &&
                                  aux::is_callable<TExpected>::value) = 0>
    static wrappers::unique<instance, TExpected> try_create(const TProvider&) noexcept;

    template <class T, class, class TProvider,
              __BOOST_DI_REQUIRES(!detail::is_expr<TGiven, TProvider>::value && aux::is_callable_with<TGiven>::value &&
                                  !aux::is_callable<TExpected>::value) = 0>
    static auto try_create(const TProvider&) noexcept
        -> detail::wrapper_traits_t<decltype(aux::declval<typename aux::identity<TGiven, T>::type>()())>;

    template <class, class, class TProvider, __BOOST_DI_REQUIRES(detail::is_expr<TGiven, TProvider>::value) = 0>
    static detail::wrapper_traits_t<decltype(aux::declval<TGiven>()(aux::declval<typename TProvider::injector_t>()))>
    try_create(const TProvider&) noexcept;

    template <class T, class, class TProvider,
              __BOOST_DI_REQUIRES(detail::is_expr<TGiven, TProvider, const detail::arg<T, TExpected, TGiven>&>::value) = 0>
    static detail::wrapper_traits_t<decltype(aux::declval<TGiven>()(aux::declval<typename TProvider::injector_t>(),
                                                                    aux::declval<detail::arg<T, TExpected, TGiven>>()))>
    try_create(const TProvider&) noexcept;
#endif  // __pph__

    template <class, class, class TProvider,
              __BOOST_DI_REQUIRES(!detail::is_expr<TGiven, TProvider>::value && aux::is_callable<TGiven>::value &&
                                  aux::is_callable<TExpected>::value) = 0>
    auto create(const TProvider&) const noexcept {
      return wrappers::unique<instance, TExpected>{object_};
    }

    template <class T, class, class TProvider,
              __BOOST_DI_REQUIRES(!detail::is_expr<TGiven, TProvider>::value && aux::is_callable_with<TGiven>::value &&
                                  !aux::is_callable<TExpected>::value) = 0>
    auto create(const TProvider&) const noexcept {
      using wrapper = detail::wrapper_traits_t<decltype(aux::declval<TGiven>()())>;
      return wrapper{object_()};
    }

    template <class, class, class TProvider, __BOOST_DI_REQUIRES(detail::is_expr<TGiven, TProvider>::value) = 0>
    auto create(const TProvider& provider) noexcept {
      using wrapper = detail::wrapper_traits_t<decltype((object_)(*provider.injector_))>;
      return wrapper{(object_)(*provider.injector_)};
    }

    template <class T, class, class TProvider,
              __BOOST_DI_REQUIRES(detail::is_expr<TGiven, TProvider, const detail::arg<T, TExpected, TGiven>&>::value) = 0>
    auto create(const TProvider& provider) noexcept {
      using wrapper = detail::wrapper_traits_t<decltype((object_)(*provider.injector_, detail::arg<T, TExpected, TGiven>{}))>;
      return wrapper{(object_)(*provider.injector_, detail::arg<T, TExpected, TGiven>{})};
    }

    TGiven object_;
  };

  template <class _, class... Ts>
  class scope<_, aux::type_list<Ts...>> {
    template <class>
    struct injector__;

    template <class TName, class T>
    struct injector__<named<TName, T>> {
      T (*f)(const injector__*) = nullptr;
      explicit injector__(const decltype(f)& ptr) : f(ptr) {}
    };

    struct injector : injector__<Ts>... {
      void (*dtor)(injector*) = nullptr;
      ~injector() noexcept { static_cast<injector*>(this)->dtor(this); }

      template <class TName, class T>
      T create(const named<TName, T>&, const aux::true_type&) const noexcept {
        return static_cast<const injector__<named<TName, T>>*>(this)->f(static_cast<const injector__<named<TName, T>>*>(this));
      }

      template <class T>
      T create(const named<no_name, T>&, const aux::false_type&) const noexcept {
        return typename concepts::type<T>::is_not_exposed{};
      }

      template <class TName, class T>
      T create(const named<TName, T>&, const aux::false_type&) const noexcept {
        return typename concepts::type<T>::template named<TName>::is_not_exposed{};
      }
    };

    template <class TInjector>
    class injector_impl : injector__<Ts>... {
      void (*dtor)(injector_impl*) = nullptr;
      static void dtor_impl(injector_impl* object) { object->~injector_impl(); }

      template <class, class>
      struct create;

      template <class TName, class T>
      struct create<named<TName, T>, aux::true_type> {
        static T impl(const injector__<named<TName, T>>* object) noexcept {
          using type = aux::type<aux::conditional_t<aux::is_same<TName, no_name>::value, T, named<TName, T>>>;
          return static_cast<const core::injector__<TInjector>&>(static_cast<const injector_impl*>(object)->injector_)
              .create_successful_impl(type{});
        }
      };

      template <class TName, class T>
      struct create<named<TName, T>, aux::false_type> {
        static T impl(const injector__<named<TName, T>>* object) noexcept {
          using type = aux::type<aux::conditional_t<aux::is_same<TName, no_name>::value, T, named<TName, T>>>;
          return static_cast<const core::injector__<TInjector>&>(static_cast<const injector_impl*>(object)->injector_)
              .create_impl(type{});
        }
      };

      template <class T>
      struct is_creatable : aux::integral_constant<bool, core::injector__<TInjector>::template is_creatable<T>::value> {};

      template <class TName, class T>
      struct is_creatable<named<TName, T>>
          : aux::integral_constant<bool, core::injector__<TInjector>::template is_creatable<T, TName>::value> {};

     public:
      explicit injector_impl(TInjector&& injector) noexcept
          : injector__<Ts>(&injector_impl::template create<Ts, typename is_creatable<Ts>::type>::impl)...,
            dtor(&injector_impl::dtor_impl),
            injector_(static_cast<TInjector&&>(injector)) {}

     private:
      TInjector injector_;
    };

   public:
    template <class, class>
    using is_referable = aux::true_type;

    template <class TInjector, __BOOST_DI_REQUIRES(aux::is_a<core::injector_base, TInjector>::value) = 0>
    explicit scope(TInjector&& i) noexcept : injector_((injector*)new injector_impl<TInjector>{static_cast<TInjector&&>(i)}) {}

    scope(scope&& other) noexcept : injector_(other.injector_) { other.injector_ = nullptr; }
    ~scope() noexcept { delete injector_; }

    template <class T, class TName, class TProvider>
    static aux::conditional_t<aux::is_base_of<injector__<named<TName, T>>, injector>::value, T, void> try_create(
        const TProvider&);

    template <class T, class TName, class TProvider>
    auto create(const TProvider&) {
      return wrapper<T>{injector_->create(named<TName, T>{}, aux::is_base_of<injector__<named<TName, T>>, injector>{})};
    }

   private:
    injector* injector_;
  };
};

}  // scopes

#endif

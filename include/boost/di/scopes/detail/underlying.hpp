
template <class U, class TScope>
struct scope_adapter {
  template <class TExpected, class TGiven>
  class scope {
    template <class T>
    using is_smart = aux::integral_constant<bool, !aux::is_same<aux::remove_smart_ptr_t<T>, T>::value>;

    template <class T, class, class = int>
    struct underlying_type : aux::true_type {
      using type = T;
    };

    template <class T, class TInjector>
    struct underlying_type<T, TInjector, __BOOST_DI_REQUIRES(aux::is_callable<T>::value)> {
      using type = decltype(aux::declval<T>()(aux::declval<TInjector>()));
    };

    template <class TInjector, class T = typename underlying_type<U, TInjector>::type>
    using is_shared = aux::integral_constant<bool, is_smart<T>::value || is_smart<U>::value>;

    template <class Scope, class>
    struct get_scope {
      using type = typename Scope::template scope<TExpected, TGiven>;
    };

    template <class T>
    struct get_scope<scopes::singleton, T> {
      using type = typename scopes::singleton::template scope<TExpected, TGiven, T>;
    };

   public:
    template <class T, class TInjector>
    using is_referable = typename get_scope<TScope, is_shared<TInjector>>::type::template is_referable<T, TInjector>;

    explicit scope(const U& object) : object_(object) {}

    template <class TInjector>
    struct provider {
      template <class TMemory = type_traits::heap,
                __BOOST_DI_REQUIRES(aux::always<TMemory>::value&& aux::is_callable<U>::value) = 0>
      auto get(const TMemory& = {}) const {
        return object_(injector_);
      }

      template <class TMemory = type_traits::heap,
                __BOOST_DI_REQUIRES(aux::always<TMemory>::value && !aux::is_callable<U>::value) = 0>
      auto get(const TMemory& = {}) const {
        return object_;
      }

      const TInjector& injector_;
      const U& object_;
    };

    template <class T, class TName, class TProvider>
    static decltype(typename get_scope<TScope, is_shared<typename TProvider::injector_t>>::type{}.template try_create<T, TName>(
        aux::declval<provider<typename TProvider::injector_t>>()))
    try_create(const TProvider&);

    template <class T, class TName, class TProvider>
    auto create(const TProvider& pr) {
      using scope = typename get_scope<TScope, is_shared<typename TProvider::injector_t>>::type;
      return scope{}.template create<T, TName>(provider<typename TProvider::injector_t>{*pr.injector_, object_});
    }

    U object_;
  };
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

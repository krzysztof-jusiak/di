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

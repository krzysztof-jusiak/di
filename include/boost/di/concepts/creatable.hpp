//
// Copyright (c) 2012-2016 Krzysztof Jusiak (krzysztof at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOST_DI_CONCEPTS_CREATABLE_HPP
#define BOOST_DI_CONCEPTS_CREATABLE_HPP

#include "boost/di/aux_/compiler.hpp"
#include "boost/di/aux_/type_traits.hpp"
#include "boost/di/aux_/utility.hpp"
#include "boost/di/type_traits/ctor_traits.hpp"

#define __BOOST_DI_CONCEPTS_CREATABLE_ERROR_MSG __BOOST_DI_DEPRECATED("creatable constraint not satisfied")

namespace concepts {

template <class T>
struct abstract_type {
  struct is_not_bound {
    operator T*() const {
      using constraint_not_satisfied = is_not_bound;
      return constraint_not_satisfied{}.error();
    }

    // clang-format off
    static inline T*
	error(_ = "type is not bound, did you forget to add: 'di::bind<interface>.to<implementation>()'?");
    // clang-format on
  };

  template <class TName>
  struct named {
    struct is_not_bound {
      operator T*() const {
        using constraint_not_satisfied = is_not_bound;
        return constraint_not_satisfied{}.error();
      }

      // clang-format off
      static inline T*
	  error(_ = "type is not bound, did you forget to add: 'di::bind<interface>.named(name).to<implementation>()'?");
      // clang-format on
    };
  };
};

template <class TScope, class T>
struct scoped {
  template <class To>
  struct is_not_convertible_to {
    operator To() const {
      using constraint_not_satisfied = is_not_convertible_to;
      return constraint_not_satisfied{}.error();
    }

    // clang-format off
    static inline To
	error(_ = "scoped object is not convertible to the requested type, did you mistake the scope: 'di::bind<T>.in(scope)'?");
    // clang-format on
  };
};

/*template <class T>*/
//struct scoped<scopes::external, T> {
  //template <class To>
  //struct is_not_convertible_to {
    //operator To() const {
      //using constraint_not_satisfied = is_not_convertible_to;
      //return constraint_not_satisfied{}.error();
    //}

    //// clang-format off
    //static inline To
	//error(_ = "external is not convertible to the requested type, verify binding: 'di::bind<T>.to(value)'?");
    //// clang-format on
  //};
/*};*/

template <class T>
struct type {
  struct has_ambiguous_number_of_constructor_parameters {
    template <int Given>
    struct given {
      template <int Expected>
      struct expected {
        operator T*() const {
          using constraint_not_satisfied = expected;
          return constraint_not_satisfied{}.error();
        }

        // clang-format off
        static inline T*
		error(_ = "verify BOOST_DI_INJECT_TRAITS or di::ctor_traits");
        // clang-format on
      };
    };
  };

  struct has_to_many_constructor_parameters {
    template <int TMax>
    struct max {
      operator T*() const {
        using constraint_not_satisfied = max;
        return constraint_not_satisfied{}.error();
      }

      // clang-format off
      static inline T*
	  error(_ = "increase BOOST_DI_CFG_CTOR_LIMIT_SIZE value or reduce number of constructor parameters");
      // clang-format on
    };
  };

  struct is_not_exposed {
    operator T() const {
      using constraint_not_satisfied = is_not_exposed;
      return constraint_not_satisfied{}.error();
    }

    // clang-format off
      static inline T
	  error(_ = "type is not exposed, did you forget to add: 'di::injector<T>'?");
    // clang-format on
  };

  template <class TName>
  struct named {
    struct is_not_exposed {
      operator T() const {
        using constraint_not_satisfied = is_not_exposed;
        return constraint_not_satisfied{}.error();
      }

      // clang-format off
      static inline T
	  error(_ = "type is not exposed, did you forget to add: 'di::injector<BOOST_DI_EXPOSE((named = name)T)>'?");
      // clang-format on
    };
  };
};

template <class>
struct ctor_size;

template <class TInit, class... TCtor>
struct ctor_size<aux::pair<TInit, aux::type_list<TCtor...>>> : aux::integral_constant<int, sizeof...(TCtor)> {};

template <class... TCtor>
struct ctor_size<aux::type_list<TCtor...>> : aux::integral_constant<int, sizeof...(TCtor)> {};

template <class T>
using ctor_size_t = ctor_size<typename type_traits::ctor<T, type_traits::ctor_impl_t<aux::is_constructible, T>>::type>;

template <class TInitialization, class TName, class _, class TCtor, class T = aux::decay_t<_>>
struct creatable_error_impl
    : aux::conditional_t<
          aux::is_polymorphic<T>::value,
          aux::conditional_t<aux::is_same<TName, no_name>::value, typename abstract_type<T>::is_not_bound,
                             typename abstract_type<T>::template named<TName>::is_not_bound>,
          aux::conditional_t<ctor_size_t<T>::value == ctor_size<TCtor>::value,
                             typename type<T>::has_to_many_constructor_parameters::template max<BOOST_DI_CFG_CTOR_LIMIT_SIZE>,
                             typename type<T>::has_ambiguous_number_of_constructor_parameters::template given<
                                 ctor_size<TCtor>::value>::template expected<ctor_size_t<T>::value>>> {};

template <class TInit, class T, class... TArgs>
struct creatable {
  static constexpr auto value = aux::is_constructible<T, TArgs...>::value;
};

template <class T, class... TArgs>
struct creatable<type_traits::uniform, T, TArgs...> {
  static constexpr auto value = aux::is_braces_constructible<T, TArgs...>::value;
};

template <class TInitialization, class TName, class T, class... TArgs>
T creatable_error() {
  return creatable_error_impl<TInitialization, TName, T, aux::type_list<TArgs...>>{};
}

}  // concepts

#endif

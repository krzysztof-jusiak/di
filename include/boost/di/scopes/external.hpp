// Copyright (c) 2012-2016 Krzysztof Jusiak (krzysztof at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOST_DI_SCOPES_EXTERNAL_HPP
#define BOOST_DI_SCOPES_EXTERNAL_HPP

#include "boost/di/aux_/type_traits.hpp"
#include "boost/di/fwd.hpp"
#include "boost/di/wrappers/shared.hpp"
#include "boost/di/wrappers/unique.hpp"

template <class T>
class reference_wrapper {
public:
  // types
  typedef T type;

  // construct/copy/destroy
  reference_wrapper(T& ref) noexcept : _ptr(&ref) {}
  reference_wrapper(T&&) = delete;
  reference_wrapper(const reference_wrapper&) noexcept = default;

  // assignment
  reference_wrapper& operator=(const reference_wrapper& x) noexcept = default;

  // access
  operator T&() const noexcept { return *_ptr; }

private:
  T* _ptr;
};

template< class T >
auto ref(T& t) { return reference_wrapper<T>{t}; }

namespace scopes {

class external {
 public:
  template <class TExpected, class TGiven>
  struct scope {
    template <class, class>
    using is_referable = aux::true_type;

    explicit scope(TGiven& object) : object_{object} {}

    template <class, class, class TProvider>
    static wrappers::shared<external, TGiven&> try_create(const TProvider&);

    template <class, class, class TProvider>
    auto create(const TProvider&) const noexcept {
      return object_;
    }

    wrappers::shared<external, TGiven&> object_;
  };

  template <class TExpected, class TGiven>
  struct scope<TExpected, std::shared_ptr<TGiven>> {
    template <class T, class>
    using is_referable = typename wrappers::shared<external, TGiven>::template is_referable<aux::remove_qualifiers_t<T>>;

    explicit scope(const std::shared_ptr<TGiven>& object) : object_{object} {}

    template <class, class, class TProvider>
    static wrappers::shared<external, TGiven> try_create(const TProvider&);

    template <class, class, class TProvider>
    auto create(const TProvider&) const noexcept {
      return wrappers::shared<external, TGiven>{object_};
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
      return wrappers::unique<external, std::initializer_list<TGiven>>{object_};
    }

    std::initializer_list<TGiven> object_;
  };

};

}  // scopes

#endif

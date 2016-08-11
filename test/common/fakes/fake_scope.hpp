//
// Copyright (c) 2012-2016 Krzysztof Jusiak (krzysztof at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOST_DI_FAKE_SCOPE_HPP
#define BOOST_DI_FAKE_SCOPE_HPP

#include <type_traits>

struct fake_scope_entry {};
struct fake_scope_exit {};

template <bool Priority = false>
struct fake_scope {
  template <class TExpected, class>
  struct scope {
    template <class, class>
    using is_referable = std::false_type;

    explicit scope(const TExpected& = {}) { ++ctor_calls(); }

    template <class T, class, class TProvider>
    static T try_create(const TProvider&);

    template <class T, class, class TProvider>
    auto create(const TProvider&) const noexcept {
      ++calls();
      return T{};
    }
  };

  static auto& ctor_calls() {
    static auto ctor_calls = 0;
    return ctor_calls;
  }

  static auto& calls() {
    static auto calls = 0;
    return calls;
  }
};

#endif

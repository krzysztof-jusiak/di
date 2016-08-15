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
};

}  // scopes

#endif

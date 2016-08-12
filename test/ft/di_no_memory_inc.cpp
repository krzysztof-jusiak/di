//
// Copyright (c) 2012-2016 Krzysztof Jusiak (krzysztof at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "boost/di.hpp"  // no #include <memory>

namespace di = boost::di;

struct i1 {
  virtual ~i1() noexcept = default;
  virtual void dummy1() = 0;
};
struct impl1 : i1 {
  void dummy1() override {}
};

test bind_and_create_value = [] {
  static auto i = 42;

  struct c {
    explicit c(int& i_) { expect(&i_ == &i); }
  };

  auto injector = di::make_injector(di::bind<int>().to(di::ref(i)));

  injector.create<c>();
};

test bind_and_create_value_via_inject = [] {
  static auto i = 42;

  struct c {
    BOOST_DI_INJECT(explicit c, int& i_) { expect(&i_ == &i); }
  };

  auto injector = di::make_injector(di::bind<int>().to(di::ref(i)));

  injector.create<c>();
};

test bind_and_create_interface = [] {
  struct c {
    c(i1& i1_, i1& i2_, int i) {
      expect(dynamic_cast<impl1*>(&i1_) == dynamic_cast<impl1*>(&i2_));
      expect(42 == i);
    }
  };

  auto injector = di::make_injector(di::bind<i1>().to<impl1>(), di::bind<int>().to(42));

  injector.create<c>();
};

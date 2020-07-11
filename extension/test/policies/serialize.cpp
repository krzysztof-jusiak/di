//
// Copyright (c) 2012-2019 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#define BOOST_DI_DISABLE_SHARED_PTR_DEDUCTION
#include "boost/di/extension/policies/serialize.hpp"

#include <cassert>
#include <iostream>

namespace di = boost::di;

struct data {
  unsigned int ui;
  long l;
  float f;
};

struct even_more_data {
  double d;
  bool b;
  long long ll;
};

struct more_data {
  int i;
  long double ld;
  even_more_data d;
  short s;
};

struct example {
  example(data& d, more_data& md) : d(d), md(md) {}

  void update() {
    d.ui = 13;
    d.l = 23l;
    d.f = .33f;
    md.i = 44;
    md.ld = 42.0;
    md.d.d = 55.0;
    md.d.b = true;
    md.d.ll = 66ll;
    md.s = 77;
  }

  void reset() {
    d.ui = {};
    d.l = {};
    d.f = {};
    md.i = {};
    md.ld = {};
    md.d.d = {};
    md.d.b = {};
    md.d.ll = {};
    md.s = {};
  }

  data& d;
  more_data& md;
};

#if defined(__MSVC__)
int main() {}
#else
int main() {
  // clang-format off
  auto injector = di::make_injector<di::extension::serializable_policy>(
      di::bind<>().to(42)
  );
  // clang-format on

  /*<<create `example`>>*/
  auto object = injector.create<example>();
  {
    assert(0 == object.d.ui);
    assert(0 == object.d.l);
    assert(.0f == object.d.f);
    assert(42 == object.md.i);  // bound
    assert(0.0 == object.md.ld);
    assert(0.0 == object.md.d.d);
    assert(false == object.md.d.b);
    assert(0 == object.md.d.ll);
    assert(0 == object.md.s);
  }

  /*<<update internal data>>*/
  object.update();
  {
    assert(13 == object.d.ui);
    assert(23 == object.d.l);
    assert(.33f == object.d.f);
    assert(44 == object.md.i);
    assert(42.0 == object.md.ld);
    assert(55.0 == object.md.d.d);
    assert(true == object.md.d.b);
    assert(66 == object.md.d.ll);
    assert(77 == object.md.s);
  }

  /*<<serialize internal data to the stream>>*/
  std::stringstream str{};
  di::extension::serialize(injector, str);

  /*<<reset internal data>>*/
  object.reset();

  /*<<deserialize from the stream>>*/
  di::extension::deserialize(injector, str);
  {
    assert(13 == object.d.ui);
    assert(23 == object.d.l);
    assert(.33f == object.d.f);
    assert(44 == object.md.i);
    assert(42.0 == object.md.ld);
    assert(55.0 == object.md.d.d);
    assert(true == object.md.d.b);
    assert(66 == object.md.d.ll);
    assert(77 == object.md.s);
  }
}
#endif

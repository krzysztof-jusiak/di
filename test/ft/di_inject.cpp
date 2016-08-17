//
// Copyright (c) 2012-2016 Krzysztof Jusiak (krzysztof at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include <functional>
#include <initializer_list>
#include <memory>
#include <string>
#include <tuple>
#if __has_include(<boost / function.hpp>)
#include <boost/function.hpp>
#endif
#if __has_include(<boost / shared_ptr.hpp>)
#include <boost/shared_ptr.hpp>
#endif
#include "boost/di.hpp"
#include "common/common.hpp"

namespace di = boost::di;

auto name = [] {};
auto other_name = [] {};

template <class>
struct get_type;

template <class T>
struct get_type<di::aux::type<T>> {
  using type = T;
};

struct i1 {
  virtual ~i1() noexcept = default;
  virtual void dummy1() = 0;
};
struct impl1 : i1 {
  void dummy1() override {}
};

test inject_ctor_limit_size = [] {
  struct c {
    BOOST_DI_INJECT(c, int /*p1*/, int /*p2*/, int /*p3*/, int /*p4*/, int /*p5*/, int /*p6*/, int /*p7*/, int /*p8*/,
                    int /*p9*/, int /*p10*/) {}
  };

  auto injector = di::make_injector();
  injector.create<c>();
};

test ctor_limit_size = [] {
  struct c {
    c(int /*p1*/, int /*p2*/, int /*p3*/, int /*p4*/, int /*p5*/, int /*p6*/, int /*p7*/, int /*p8*/, int /*p9*/, int /*p10*/) {
    }
  };

  auto injector = di::make_injector();
  injector.create<c>();
};

test inject_traits_no_limits = [] {
  struct c {
    using boost_di_inject__ = di::inject<int, int, int, int, int, int, int, int, int, int, int>;
    c(int, int, int, int, int, int, int, int, int, int, int) {}
  };

  auto injector = di::make_injector();
  injector.create<c>();
};

struct c_no_limits {
  c_no_limits(int, int, int, int, int, int, int, int, int, int, int) {}
};

namespace boost {
namespace di {

template <>
struct ctor_traits<c_no_limits> {
  using boost_di_inject__ = di::inject<int, int, int, int, int, int, int, int, int, int, int>;
};
}
}  // boost::di

test inject_traits_no_limits_via_ctor_traits = [] {
  auto injector = di::make_injector();
  injector.create<c_no_limits>();
};

test inject = [] {
  constexpr auto i = 42;

  struct c {
    c(std::initializer_list<int>) {}
    c(int, double, float) {}
    BOOST_DI_INJECT(c, int i, double d) : i(i), d(d) {}
    int i = 0;
    double d = 0.0;
  };

  auto injector = di::make_injector(di::bind<int>().to(i));
  auto object = injector.create<c>();
  expect(i == object.i);
  expect(0.0 == object.d);
};

test automatic_inject = [] {
  constexpr auto i = 42;

  struct c {
    c(std::initializer_list<int>) {}
    c(int i, double d) : i(i), d(d) {}
    int i = 0;
    double d = 0.0;
  };

  auto injector = di::make_injector(di::bind<int>().to(i));
  auto object = injector.create<c>();

  expect(i == object.i);
  expect(0.0 == object.d);
};

test automatic_inject_with_initializer_list = [] {
  constexpr auto i = 42;

  struct c {
    c(int i, std::initializer_list<int> il) : i(i), il(il) {}
    int i = 0;
    std::initializer_list<int> il;
  };

  auto injector = di::make_injector(di::bind<int>().to(i));
  auto object = injector.create<c>();

  expect(i == object.i);
  expect(0 == object.il.size());
};

test ctor_refs = [] {
  struct c {
    BOOST_DI_INJECT(c, const std::shared_ptr<i1>& sp, int& i, const double& d, const std::string& str,
                    (named = name) const std::string& nstr, std::function<int()> f, long&& l, short s)
        : sp(sp), i(i), d(d), str(str), nstr(nstr), f(f), l(std::move(l)), s(s) {
      expect("named str" == nstr);
    }

    std::shared_ptr<i1> sp;
    int& i;
    const double& d;
    std::string str;
    std::string nstr;
    std::function<int()> f;
    long l = 0;
    short s = 0;
  };

  struct c_inject {
    c_inject(const std::shared_ptr<i1>& sp, int& i, const double& d, const std::string& str
#if defined(__MSVC__)
             ,
             function<int()> f
#else
             ,
             std::function<int()> f
#endif
             ,
             long&& l, short s)
        : sp(sp), i(i), d(d), str(str), f(f), l(std::move(l)), s(s) {}

    std::shared_ptr<i1> sp;
    int& i;
    const double& d;
    std::string str;
    std::function<int()> f;
    long l = 0;
    short s = 0;
  };

  struct c_aggregate {
    std::shared_ptr<i1> sp;
    int& i;
    const double& d;
    std::string str = {};
    std::string nstr = {};
    std::function<int()> f = {};
    long l = 0;
    short s = 0;
  };

  auto test = [](auto type, const auto& bind_i1) {
    auto i = 0;
    constexpr auto d = 0.0;

    auto injector =
        di::make_injector(di::bind<int>().to(i), di::bind<double>().to(d), di::bind<std::string>().to("str"),
                          di::bind<std::string>().named(name).to("named str"), bind_i1, di::bind<short>().to(short{42}),
                          di::bind<long>().to(123l), di::bind<std::function<int()>>().to([] { return 87; }),
                          di::bind<function<int()>>().to([] { return 87; }));

    auto object = injector.template create<typename get_type<decltype(type)>::type>();
    expect(dynamic_cast<impl1*>(object.sp.get()));
    expect(&i == &object.i);
    expect(&d == &object.d);
    expect("str" == object.str);
    expect(42 == object.s);
    expect(87 == object.f());
    expect(123 == object.l);
  };

  test(di::aux::type<c>{}, di::bind<i1>().to<impl1>());
 test(di::aux::type<c_inject>{}, di::bind<i1>().to<impl1>());
#if !defined(__MSVC__)
  test(di::aux::type<c_aggregate>{}, di::bind<i1>().to<impl1>());
#endif

  test(di::aux::type<c>{}, di::bind<i1>().to(std::make_shared<impl1>()));
  test(di::aux::type<c_inject>{}, di::bind<i1>().to(std::make_shared<impl1>()));
#if !defined(__MSVC__)
  test(di::aux::type<c_aggregate>{}, di::bind<i1>().to(std::make_shared<impl1>()));
#endif
};

test refs_vs_copy = [] {
  struct cc {
    BOOST_DI_INJECT(cc, (named = name) const int& i, (named = name) const std::string& s, (named = other_name) int& i_ref,
                    int& ii)
        : str(s), i(i), i_ref(i_ref), ii(ii) {}

    std::string str;
    int i = 0;
    int& i_ref;
    int& ii;
  };

  struct cc_inject {
    BOOST_DI_INJECT(cc_inject, (named = name) const int& i, (named = name) const std::string& s,
                    (named = other_name) int& i_ref, int& ii)
        : str(s), i(i), i_ref(i_ref), ii(ii) {}

    std::string str;
    int i = 0;
    int& i_ref;
    int& ii;
  };

  std::string ref = "named str";
  auto i = 42;

  {
    auto injector = di::make_injector(di::bind<std::string>().named(name).to(ref), di::bind<int>().named(name).to(i),
                                      di::bind<int>().named(other_name).to(i), di::bind<int>().to(i));
    auto object = injector.create<cc>();
    expect(ref == object.str);
    expect(i == object.i);
    expect(i == object.i_ref);
    expect(&i == &object.i_ref);
    expect(i == object.ii);
    expect(&i == &object.ii);
  }

  {
    auto injector = di::make_injector(di::bind<std::string>().named(name).to(ref), di::bind<int>().named(name).to(i),
                                      di::bind<int>().named(other_name).to(i), di::bind<int>().to(i));
    auto object = injector.create<cc_inject>();
    expect(ref == object.str);
    expect(i == object.i);
    expect(i == object.i_ref);
    expect(&i == &object.i_ref);
    expect(i == object.ii);
    expect(&i == &object.ii);
  }

  {
    auto injector = di::make_injector(di::bind<std::string>().named(name).to(ref), di::bind<int>().named(name).to(i),
                                      di::bind<int>().named(other_name).to(i), di::bind<int>().to(i));
    auto object = injector.create<cc>();
    expect(ref == object.str);
    expect(i == object.i);
    expect(i == object.i_ref);
    expect(&i == &object.i_ref);
    expect(i == object.ii);
    expect(&i == &object.ii);
  }

  {
    auto injector = di::make_injector(di::bind<std::string>().named(name).to(ref), di::bind<int>().named(name).to(i),
                                      di::bind<int>().named(other_name).to(i), di::bind<int>().to(i));
    auto object = injector.create<cc_inject>();
    expect(ref == object.str);
    expect(i == object.i);
    expect(i == object.i_ref);
    expect(&i == &object.i_ref);
    expect(i == object.ii);
    expect(&i == &object.ii);
  }
};

#if __has_include(<boost / function.hpp>)
test create_with_boost_function = [] {
  struct c {
    BOOST_DI_INJECT(c, const boost::function<int()>& f1, boost::function<double(double)> f2) {
      expect(42 == f1());
      expect(87.0 == f2(87.0));
    }
  };

  auto injector = di::make_injector(di::bind<boost::function<int()>>.to([] { return 42; }),
                                    di::bind<boost::function<double(double)>>.to([](double d) { return d; }));

  injector.create<c>();
};
#endif

test create_with_default_values = [] {
  constexpr auto i = 42;

  struct default_values {
    BOOST_DI_INJECT_TRAITS(int);
    explicit default_values(int i, float f = 42.0, double d = 87.0) : i(i), f(f), d(d) {}
    int i = 0;
    float f = 0.0;
    double d = 0.0;
  };

  auto injector = make_injector(di::bind<int>().to(i));
  auto object = injector.create<default_values>();

  expect(i == object.i);
  expect(42.0 == object.f);
  expect(87.0 == object.d);
};

test smart_pointers = [] {
  struct c {
    c(std::unique_ptr<i1> up1, const std::shared_ptr<i1>& sp1, std::shared_ptr<i1> sp2) {
      expect(dynamic_cast<impl1*>(up1.get()));
      expect(dynamic_cast<impl1*>(sp1.get()));
      expect(dynamic_cast<impl1*>(sp2.get()));
      expect(sp1 == sp2);
    }
  };

  auto injector = di::make_injector(di::bind<i1>().to<impl1>());

  injector.create<c>();
};

test one_arg_class = [] {
  struct c {
    explicit c(int i) : i(i) {}
    int i = 0;
  };

  struct c_implicit {
    c_implicit(int i) : i(i) {}
    int i = 0;
  };

  struct c_named {
    BOOST_DI_INJECT(c_named, (named = name) int i) : i(i) {}
    int i = 0;
  };

  auto injector = di::make_injector(di::bind<int>().in(di::unique));

  {
    auto object = injector.create<c>();
    expect(0 == object.i);
  }

  {
    auto object = injector.create<c_implicit>();
    expect(0 == object.i);
  }

  {
    auto object = injector.create<c_named>();
    expect(0 == object.i);
  }
};

test string_creation = [] {
  struct string {
    std::string str;
  };

  expect("" == di::make_injector().create<string>().str);
};

template <class T>
struct template_inject {
  BOOST_DI_INJECT(explicit template_inject, T value) : value_(value) {}

  T value_;
};

test inject_inside_template = [] {
  constexpr auto i = 42;
  auto injector = di::make_injector(di::bind<int>().to(i));
  auto object = injector.create<template_inject<int>>();

  expect(i == object.value_);
};

template <class T>
struct template_inject_no_limits {
  using boost_di_inject__ = di::inject<T>;
  explicit template_inject_no_limits(T value) : value_(value) {}
  T value_;
};

test inject_inside_template_no_limits = [] {
  constexpr auto i = 42;
  auto injector = di::make_injector(di::bind<int>().to(i));
  auto object = injector.create<template_inject_no_limits<int>>();
  expect(i == object.value_);
};

test create_conversion = [] {
  constexpr auto i = 42;
  auto injector = di::make_injector(di::bind<i1>().to<impl1>(), di::bind<int>().to(i));
  expect(i == injector.create<int>());
  expect(dynamic_cast<impl1*>((injector.create<std::unique_ptr<i1>>()).get()));
};

test request_value_and_ptr_in_unique = [] {
  struct c {
    int i = 0;
    owner<int*> ptr = nullptr;
  };

  auto injector = di::make_injector(di::bind<int>().in(di::unique));
  auto object = injector.create<c>();
  delete object.ptr;
};

template <class... Ts>
struct custom_type_list {
  using type = custom_type_list;
};

test inject_using_custom_type_list = [] {
  struct c {
    c(int i, double d) {
      expect(42 == i);
      expect(87.0 == d);
    }

    using boost_di_inject__ = custom_type_list<int, double>;
  };

  auto injector = di::make_injector(di::bind<>().to(42), di::bind<double>().to(87.0));
  injector.create<c>();
};

struct custom_type_list_variadic {
  template <class... Ts>
  explicit custom_type_list_variadic(Ts&&... ts) {
    std::tuple<Ts...> args{ts...};
    expect(42 == std::get<0>(args));
    expect(87.0 == std::get<1>(args));
  }

  using boost_di_inject__ = custom_type_list<int, double>;
};

test inject_using_custom_type_list_variadic = [] {
  auto injector = di::make_injector(di::bind<>().to(42), di::bind<double>().to(87.0));
  injector.create<custom_type_list_variadic>();
};

#if __has_include(<boost / shared_ptr.hpp>)
test conversion_to_boost_shared_ptr_inject = [] {
  struct c {
    BOOST_DI_INJECT(explicit c, const boost::shared_ptr<int>& sp) : sp(sp) {}
    boost::shared_ptr<int> sp;
  };

  auto injector = di::make_injector(di::bind<int>().in(di::singleton));
  auto object = injector.create<c>();

  expect(object.sp.get());
};

test conversion_to_boost_shared_ctor_referable = [] {
  constexpr auto i = 42;
  struct c {
    c(const boost::shared_ptr<int>& sp, int& i_) : sp(sp) { expect(i_ == i); }
    boost::shared_ptr<int> sp;
  };

  auto injector = di::make_injector(di::bind<int>().to(std::make_shared<int>(i)));
  auto object = injector.create<c>();

  expect(object.sp.get());
  expect(i == *object.sp);
};

test conversion_to_boost_shared_ptr_uniform = [] {
  struct c {
    boost::shared_ptr<int> sp;
  };

  auto injector = di::make_injector(di::bind<int>().in(di::singleton));
  auto object = injector.create<c>();

  expect(object.sp.get());
};
#endif

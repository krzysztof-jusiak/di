//
// Copyright (c) 2012-2019 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// clang-format off
#include <initializer_list>  // has to be before, due to the bug in clang < 3.7
// clang-format on
#include "boost/di.hpp"
#include <functional>
#include <iterator>
#include <memory>
#include <set>
#include <string>
#include <tuple>
#include <vector>

struct Concept {};
struct ConceptImpl {};

BOOST_DI_NAMESPACE_BEGIN

namespace concepts {
template <class T>
struct is_related<true, Concept, T> {
  static constexpr auto value = true;
};
}  // namespace concepts

BOOST_DI_NAMESPACE_END

namespace di = boost::di;

struct i1 {
  virtual ~i1() noexcept = default;
  virtual void dummy1() = 0;
};
struct i2 {
  virtual ~i2() noexcept = default;
  virtual void dummy2() = 0;
};
struct i3 {
  virtual ~i3() noexcept = default;
  virtual void dummy3() = 0;
};
struct impl1 : i1 {
  void dummy1() override {}
};
struct impl1_final final : i1 {
  void dummy1() override {}
};
struct impl1_int : i1 {
  explicit impl1_int(int i) : i(i) {}
  void dummy1() override {}
  int i = 0;
};
struct impl2 : i2 {
  void dummy2() override {}
};
struct impl1_2 : i1, i2 {
  void dummy1() override {}
  void dummy2() override {}
};
struct impl4 : impl1_2 {};

struct impl1_with_i2 : i1 {
  explicit impl1_with_i2(std::shared_ptr<i2> i2) : i2_(i2) {}
  void dummy1() override {}

  std::shared_ptr<i2> i2_;
};

auto a = [] {};
auto b = [] {};
struct name_ {
} name;

test forward_decl = [] {
  di::make_injector(di::bind<class InterfaceFwd>().to<class ImplementationFwd>());
  di::make_injector(di::bind<class InterfaceFwd>().to<impl1>());
  di::make_injector(di::bind<i1>().to<class Impl>());
};

test bind_to = [] {
  auto injector = di::make_injector(di::bind<int>().to(42), di::bind<>().to(87l));
  expect(42 == injector.create<int>());
  expect(87l == injector.create<long>());
};

test named_to = [] {
  static constexpr auto i = 42;
  static constexpr auto d = 87.0;

  struct c {
    BOOST_DI_INJECT(c, (named = a) int i, (named = b) double d) : i_(i), d_(d) {}

    int i_ = 0;
    double d_ = 0.0;
  };

  auto injector = di::make_injector(di::bind<int>().named(a).to(i), di::bind<double>().to(d).named(b));
  auto object = injector.create<c>();

  expect(i == object.i_);
  expect(d == object.d_);
};

test deduced_named_to = [] {
  static constexpr auto i = 42;
  static constexpr auto d = 87.0;

  struct c {
    BOOST_DI_INJECT(c, (named = a) int i, (named = b) double d) : i_(i), d_(d) {}

    int i_ = 0;
    double d_ = 0.0;
  };

  auto injector = di::make_injector(di::bind<>().named(a).to(i), di::bind<>().to(d).named(b));
  auto object = injector.create<c>();

  expect(i == object.i_);
  expect(d == object.d_);
};

test named_polymorphic = [] {
  struct c {
    BOOST_DI_INJECT(explicit c, (named = name) std::shared_ptr<i1> sp) : sp(sp) {}
    std::shared_ptr<i1> sp;
  };

  auto injector = di::make_injector(di::bind<i1>().named(name).to<impl1>());
  auto object = injector.create<c>();

  expect(dynamic_cast<impl1 *>(object.sp.get()));
};

struct c {
  BOOST_DI_INJECT(explicit c, (named = name) int i);
  int i = 0;
};

c::c(int i) : i(i) {}

test named_with_ctor_def_decl = [] {
  static constexpr auto i = 42;

  auto injector = di::make_injector(di::bind<int>().to(i).named(name));
  auto object = injector.create<c>();

  expect(i == object.i);
};

test named_parameters_with_shared_scope = [] {
  struct c {
    BOOST_DI_INJECT(c, (named = a) const std::shared_ptr<i1> &n1, (named = b) std::shared_ptr<i1> n2) : n1(n1), n2(n2) {}

    std::shared_ptr<i1> n1;
    std::shared_ptr<i1> n2;
  };

  auto injector = di::make_injector(di::bind<i1>().named(a).in(di::unique).to<impl1>(),
                                    di::bind<i1>().named(b).to(std::make_shared<impl1>()));

  auto object = injector.create<c>();

  expect(object.n1 != object.n2);
};

test any_of = [] {
  auto injector = di::make_injector(di::bind<impl1_2>());

  auto object = injector.create<std::unique_ptr<impl1_2>>();
  expect(dynamic_cast<i1 *>(object.get()));
  expect(dynamic_cast<i2 *>(object.get()));
  expect(!dynamic_cast<i3 *>(object.get()));
};

test any_of_with_scope = [] {
  auto test = [](auto scope, auto same) {
    auto injector = di::make_injector(
#if defined(__MSVC__)
        di::bind<i2, i1>().in(scope).to<impl1_2>()
#else
        di::bind<i2, i1>().in(scope).template to<impl1_2>()
#endif
            );

    auto object_1 = injector.template create<std::shared_ptr<i1>>();
    auto object_2 = injector.template create<std::shared_ptr<i2>>();
    auto result = dynamic_cast<impl1_2 *>(object_1.get()) == dynamic_cast<impl1_2 *>(object_2.get());
    expect(result == same);
  };

  test(di::unique, false);
  test(di::singleton, true);
};

test any_of_with_scope_split = [] {
  auto test = [](auto scope, auto same) {
    auto injector = di::make_injector(
#if defined(__MSVC__)
        di::bind<i1>().in(scope).to<impl1_2>(), di::bind<i2>().in(scope).to<impl1_2>()
#else
        di::bind<i1>().in(scope).template to<impl1_2>(), di::bind<i2>().in(scope).template to<impl1_2>()
#endif
            );

    auto object_1 = injector.template create<std::shared_ptr<i1>>();
    auto object_2 = injector.template create<std::shared_ptr<i2>>();
    auto result = dynamic_cast<impl1_2 *>(object_1.get()) == dynamic_cast<impl1_2 *>(object_2.get());
    expect(result == same);
  };

  test(di::unique, false);
  test(di::singleton, true);
};

test any_of_unique = [] {
  auto injector = di::make_injector(di::bind<i1, i2>().in(di::unique).to<impl1_2>());

  auto object_1 = injector.create<std::shared_ptr<i1>>();
  auto object_2 = injector.create<std::shared_ptr<i2>>();
  expect(dynamic_cast<impl1_2 *>(object_1.get()) != dynamic_cast<impl1_2 *>(object_2.get()));
};

test bind_int_to_static_value = [] {
  auto injector = di::make_injector(di::bind<int>().to<std::integral_constant<int, 42>>());
  auto object = injector.create<int>();
  expect(42 == object);
};

test override_priority = [] {
  auto injector =
      di::make_injector(di::bind<int>().to(12)[di::override], di::bind<int>().to<std::integral_constant<int, 42>>());

  auto object = injector.create<int>();

  expect(12 == object);
};

test override_priority_order = [] {
  auto injector = di::make_injector(di::bind<int>().to<std::integral_constant<int, 41>>(),
                                    di::bind<int>().to([] { return 42; })[di::override]);

  expect(42 == injector.create<int>());
};

test override_priority_interface = [] {
  auto injector = di::make_injector(di::bind<i1>().to<impl1>(), di::bind<i1>().to<impl1_int>()[di::override]);
  auto object = injector.create<std::unique_ptr<i1>>();
  expect(dynamic_cast<impl1_int *>(object.get()));
};

test override_priority_interface_module = [] {
  auto module = [] { return di::make_injector(di::bind<i1>().to<impl1_int>()); };
  auto injector = di::make_injector(module(), di::bind<i1>().to<impl1>()[di::override]);
  auto object = injector.create<std::unique_ptr<i1>>();
  expect(dynamic_cast<impl1 *>(object.get()));
};

test bind_non_interface_in_singleton_scope = [] {
  struct c {};
  auto injector = di::make_injector(di::bind<c>().in(di::singleton));
  expect(injector.create<std::shared_ptr<c>>() == injector.create<std::shared_ptr<c>>());
};

test bind_interface_implementation_in_singleton_scope = [] {
  struct c {
    c(std::shared_ptr<i1> sp1, std::shared_ptr<impl1> sp2) { expect(sp1 == sp2); }
  };
  auto injector = di::make_injector(di::bind<i1, impl1>().to<impl1>().in(di::singleton));
  injector.create<c>();
};

test bind_interface_implementation_ref_in_singleton_scope = [] {
  struct c {
    c(const std::shared_ptr<i1> &sp1, const std::shared_ptr<impl1> &sp2) { expect(sp1 == sp2); }
  };
  auto injector = di::make_injector(di::bind<i1, impl1>().to<impl1>().in(di::singleton));
  injector.create<c>();
};

test bind_const_ref_type_in_singleton_scope = [] {
  struct c_inject {
    BOOST_DI_INJECT(c_inject, const int &i1, const int &i2) { expect(&i1 == &i2); }
  };

  struct c {
    c(const int &i1, const int &i2) { expect(&i1 == &i2); }
  };

  auto injector = di::make_injector(di::bind<int>().in(di::singleton));

  injector.create<c_inject>();
  di::make_injector().create<c_inject>();

  injector.create<c>();
  di::make_injector().create<c>();
};

test bind_shared_ptr_ref = [] {
  struct c {
    c(std::shared_ptr<int> &sp1, std::shared_ptr<int> &sp2) { expect(sp1 == sp2); }
  };

  di::make_injector().create<c>();
};

test bind_shared_ptr_interface = [] {
  struct c {
    c(std::shared_ptr<i1> sp1, i1 &sp2) { expect(sp1.get() == &sp2); }
  };

  auto injector = di::make_injector(di::bind<i1>().to<impl1>());
  injector.create<c>();
};

test scopes_instance_interface_ptr = [] {
  std::unique_ptr<i1> object = std::make_unique<impl1>();
  auto injector = di::make_injector(di::bind<i1>().to(*object));
  injector.create<const i1 &>();
  injector.create<i1 &>();
};

test scopes_instance_interface_shared = [] {
  std::shared_ptr<i1> object = std::make_shared<impl1>();
  auto injector = di::make_injector(di::bind<i1>().to(object));
  injector.create<std::shared_ptr<i1>>();
  injector.create<const i1 &>();
  injector.create<i1 &>();
};

test scopes_instance_shared = [] {
  auto i = std::make_shared<int>(42);
  auto injector = di::make_injector(di::bind<int>().to(i));

  {
    auto object = injector.create<std::shared_ptr<int>>();
    expect(i.get() == object.get());
    expect(42 == *i);
  }

  {
    ++*i;
    auto object = injector.create<std::shared_ptr<int>>();
    expect(43 == *i);
  }
};

test scopes_instance_lambda = [] {
  auto i = std::make_shared<int>(42);
  auto injector = di::make_injector(di::bind<int>().to([&i] { return i; }));

  {
    auto object = injector.create<std::shared_ptr<int>>();
    expect(i.get() == object.get());
    expect(42 == *i);
  }

  {
    ++*i;
    auto object = injector.create<std::shared_ptr<int>>();
    expect(43 == *i);
  }
};

test scopes_instance_lambda_injector = [] {
  auto i = std::make_shared<int>(42);
  auto injector = di::make_injector(di::bind<int>().to([&i](const auto &) { return i; }));

  {
    auto object = injector.create<std::shared_ptr<int>>();
    expect(i.get() == object.get());
    expect(42 == *i);
  }

  {
    ++*i;
    auto object = injector.create<std::shared_ptr<int>>();
    expect(43 == *i);
  }
};

test scopes_instance_lambda_injector_mix = [] {
  auto injector = di::make_injector(
      di::bind<short>().to([] { return 42; }), di::bind<long>().to(87l),
      di::bind<int>().to([](const auto &injector) -> int { return static_cast<int>(injector.template create<long>()); }));

  expect(42 == injector.create<short>());
  expect(87 == injector.create<int>());
  expect(87l == injector.create<long>());
};

test scopes_instance_deduced = [] {
  auto injector = di::make_injector(di::bind<>().to(short{42}), di::bind<>().to(87l), di::bind<>().to(123));
  expect(42 == injector.create<short>());
  expect(123 == injector.create<int>());
  expect(87l == injector.create<long>());
};

test instances_ref_cref = [] {
  auto i = 42;
  const auto d = 87.0;

  struct refs {
    BOOST_DI_INJECT(refs, int &i, const double &d) : i_(i), d_(d) {}

    int &i_;
    const double &d_;
  };

  auto injector = make_injector(di::bind<int>().to(i), di::bind<double>().to(d));
  auto object = injector.create<refs>();

  expect(i == object.i_);
  expect(d == object.d_);
};

test bind_chars_to_string = [] {
  auto injector = di::make_injector(di::bind<std::string>().to("str"));

  expect("str" == injector.create<std::string>());
};

test dynamic_binding_ref = [] {
  auto test = [](bool runtime_value) -> i1 & {
    auto injector = di::make_injector(di::bind<i1>().to([&](const auto &injector) -> i1 & {
      if (runtime_value) {
        return injector.template create<impl1 &>();
      } else {
        return injector.template create<impl1_int &>();
      }
    }),
                                      di::bind<>().to(42));

    return injector.create<i1 &>();
  };

  {
    auto &&object = test(true);
    expect(dynamic_cast<impl1 *>(&object));
  }

  {
    auto &&object = test(false);
    expect(dynamic_cast<impl1_int *>(&object));
    expect(42 == dynamic_cast<impl1_int &>(object).i);
  }
};

test dynamic_binding_expose_ref = [] {
  auto test = [](bool runtime_value) -> i1 & {
    di::injector<i1 &> injector = di::make_injector(di::bind<i1>().to([&](const auto &injector) -> i1 & {
      if (runtime_value) {
        return injector.template create<impl1 &>();
      } else {
        return injector.template create<impl1_int &>();
      }
    }),
                                                    di::bind<>().to(42));

    return injector.create<i1 &>();
  };

  {
    auto &&object = test(true);
    expect(dynamic_cast<impl1 *>(&object));
  }

  {
    auto &&object = test(false);
    expect(dynamic_cast<impl1_int *>(&object));
    expect(42 == dynamic_cast<impl1_int &>(object).i);
  }
};

test dynamic_binding_using_polymorphic_lambdas_with_dependend_interfaces = [] {
  auto test = [&](bool debug_property) {
    auto injector = make_injector(di::bind<i1>().to([&](const auto &injector) -> std::shared_ptr<i1> {
      if (debug_property) {
        return std::make_shared<impl1>();
      }

      return injector.template create<std::shared_ptr<impl1_with_i2>>();
    }),
                                  di::bind<i2>().to<impl2>());

    return injector.create<std::shared_ptr<i1>>();
  };

  {
    auto object = test(false);
    expect(dynamic_cast<impl1_with_i2 *>(object.get()));
    expect(dynamic_cast<impl2 *>(dynamic_cast<impl1_with_i2 *>(object.get())->i2_.get()));
  }

  {
    auto object = test(true);
    expect(dynamic_cast<impl1 *>(object.get()));
  }
};

double return_double(double d) { return d; }

#if !defined(__MSVC__)
test bind_to_function_ptr = [] {
  static constexpr auto i = 42;
  static constexpr auto d = 87.0;

  struct functions {
    BOOST_DI_INJECT(functions, const std::function<int()> &fi, std::function<double()> fd) : fi(fi), fd(fd) {}

    std::function<int()> fi;
    std::function<double()> fd;
  };

  auto injector = di::make_injector(di::bind<std::function<int()>>().to([&] { return i; }),
                                    di::bind<std::function<double()>>().to(std::bind(&return_double, d)));

  auto object = injector.create<functions>();

  expect(i == object.fi());
  expect(d == object.fd());
};
#endif

test bind_to_function = [] {
  using f_t = std::function<double(double)>;
  const auto injector = di::make_injector(di::bind<f_t>().to(return_double));
  const auto object = injector.create<f_t>();

  expect(42.0 == object(42.0));
};

test runtime_factory_impl = [] {
  static constexpr auto i = 42;

  auto test = [&](bool debug_property) {
    auto injector = make_injector(di::bind<int>().to(i), di::bind<i1>().to([&](const auto &injector) -> std::shared_ptr<i1> {
      if (debug_property) {
        return std::make_shared<impl1>();
      }

      return injector.template create<std::shared_ptr<impl1_int>>();
    }));

    return injector.create<std::shared_ptr<i1>>();
  };

  {
    auto object = test(false);
    expect(dynamic_cast<impl1_int *>(object.get()));
    expect(i == dynamic_cast<impl1_int *>(object.get())->i);
  }

  {
    auto object = test(true);
    expect(dynamic_cast<impl1 *>(object.get()));
  }
};

struct call_operator {
  bool &b;

  template <class TInjector>
  std::shared_ptr<i1> operator()(const TInjector &injector) const {
    if (b) {
      return injector.template create<std::shared_ptr<impl1>>();
    }

    return injector.template create<std::shared_ptr<impl1_int>>();
  }
};

test runtime_factory_call_operator_impl = [] {
  static constexpr auto i = 42;

  auto test = [&](bool debug_property) {
    auto injector = make_injector(di::bind<int>().to(i), di::bind<i1>().to(call_operator{debug_property}));
    return injector.create<std::shared_ptr<i1>>();
  };

  {
    auto object = test(false);
    expect(dynamic_cast<impl1_int *>(object.get()));
    expect(i == dynamic_cast<impl1_int *>(object.get())->i);
  }

  {
    auto object = test(true);
    expect(dynamic_cast<impl1 *>(object.get()));
  }
};

test scopes_injector_lambda_injector = [] {
  constexpr double d = 42.0;
  auto injector = di::make_injector(di::bind<double>().to(d), di::bind<int>().to([](const auto &injector) -> int {
    return static_cast<int>(injector.template create<double>());
  }));

  expect(static_cast<int>(d) == injector.create<int>());
};

test bind_function_to_callable = [] {
  struct functions {
    BOOST_DI_INJECT(explicit functions, const std::function<int(int)> &f) { expect(f(42) == 42); }
  };

  auto injector =
      di::make_injector(di::bind<std::function<int(int)>>().to([](const auto &) { return [](int i) { return i; }; }));

  injector.create<functions>();
};

test bind_non_owning_ptr = [] {
  static constexpr auto i = 42;

  using Pointer = int;

  struct c {
    c(Pointer &ptr) { expect(i == ptr); }
  };
  auto module = [](Pointer *ptr) { return di::bind<Pointer>().to(*ptr); };
  di::aux::owner<Pointer *> ptr{new Pointer{i}};
  auto injector = di::make_injector(module(ptr));

  injector.create<c>();

  delete ptr;
};

#if !defined(__MSVC__)
test bind_to_function_via_interface = [] {
  struct i {
    virtual ~i() = default;
    virtual void operator()() = 0;
  };

  struct impl : i {
    void operator()() override {}
  };

  struct c {
    c(i &object) { expect(dynamic_cast<impl *>(&object)); }
  };

  const auto injector =
      di::make_injector(di::bind<i>().to([&](const auto &injector) -> i & { return injector.template create<impl &>(); }));

  injector.create<c>();
};
#endif

template <class TIf, class TImpl>
struct expr {
  template <class TInjector>
  TIf &operator()(const TInjector &injector) {
    return injector.template create<TImpl &>();
  }
};
test bind_to_function_via_interface_inject = [] {
  struct i {
    virtual ~i() = default;
    virtual void operator()() = 0;
  };

  struct impl : i {
    void operator()() override {}
  };

  struct c {
    BOOST_DI_INJECT(c, i &object) { expect(dynamic_cast<impl *>(&object)); }
  };

  const auto injector = di::make_injector(di::bind<i>().to(expr<i, impl>{}));

  injector.create<c>();
};

test multi_bindings_empty = [] {
  struct c {
    c(std::vector<int> v, const std::set<std::unique_ptr<i1>> &s) {
      expect(v.empty());
      expect(s.empty());
    }
  };

  auto injector = di::make_injector();
  injector.create<c>();
};

test multi_bindings_containers = [] {
  auto test = [](auto object) {
    expect(object.size() == 2);
    auto it = object.begin();
    expect(*(std::next(it, 0)) == 11);
    expect(*(std::next(it, 1)) == 87);
  };

  auto injector =
      di::make_injector(di::bind<int[]>().to<int, di::named<class Int42>>(), di::bind<int>().to(11),
                        di::bind<>().to(42).named<class Int42>(), di::bind<int>().to(87).named<class Int42>()[di::override]);

  test(injector.create<std::vector<int>>());
  test(injector.create<std::set<int>>());
};

test multi_bindings_inject_named = [] {
  struct c {
    BOOST_DI_INJECT(c, (named = a) const std::vector<std::shared_ptr<i1>> &v1,
                    (named = b) std::vector<std::unique_ptr<i1>> v2) {
      expect(v1.size() == 2);
      expect(dynamic_cast<impl1 *>(v1[0].get()));
      expect(dynamic_cast<impl1_2 *>(v1[1].get()));

      expect(v2.size() == 1);
      expect(dynamic_cast<impl1 *>(v2[0].get()));
    }
  };

  auto injector = di::make_injector(di::bind<i1 *[]>().to<impl1, di::named<decltype(a)>>(),
                                    di::bind<i1 *[]>().named(a).to<i1, di::named<class Impl2>>(),
                                    di::bind<i1 *[]>().named(b).to<impl1>(), di::bind<i1>().to<impl1>(),
                                    di::bind<i1>().to<impl1_2>().named<class Impl2>(), di::bind<i1>().to<impl1_2>().named(a));

  injector.create<c>();
};

test multi_bindings_ctor_with_exposed_module = [] {
  struct c {
    explicit c(std::vector<std::unique_ptr<i1>> v) {
      expect(v.size() == 6);
      expect(dynamic_cast<impl1 *>(v[0].get()));
      expect(dynamic_cast<impl1_2 *>(v[1].get()));
      expect(dynamic_cast<impl1_2 *>(v[2].get()));
      expect(dynamic_cast<impl1_int *>(v[3].get()));
      expect(dynamic_cast<impl1 *>(v[4].get()));
      expect(dynamic_cast<impl1_2 *>(v[5].get()));
    }
  };

  struct ExposedI1 {};
  struct ExposedI1_ {};

  auto module = []() -> di::injector<BOOST_DI_EXPOSE((named = ExposedI1{}) std::unique_ptr<i1>)> {
    return di::make_injector(di::bind<i1>().to<impl1>().named(ExposedI1{}));
  };
  auto module2 = []() -> di::injector<std::unique_ptr<i1>> { return di::make_injector(di::bind<i1>().to<impl1_int>()); };
  auto module3 = [] { return di::make_injector(di::bind<i1>().to<impl1_2>().named(ExposedI1_{})); };
  auto module4 = [&]() -> di::injector<BOOST_DI_EXPOSE((named = ExposedI1_{}) std::unique_ptr<i1>)> {
    return di::make_injector(module3());
  };
  auto injector =
      di::make_injector(di::bind<i1 *[]>().to<impl1, impl1_2, impl1_2, i1, di::named<ExposedI1>, di::named<ExposedI1_>>(),
                        module(), module2(), module4());

  injector.create<c>();
};

test multi_bindings_share_object_between_list_and_parameter = [] {
  struct c {
    c(std::vector<std::shared_ptr<i1>> v, std::shared_ptr<i1> i) {
      expect(v.size() == 2);
      expect(dynamic_cast<impl1 *>(v[0].get()));
      expect(dynamic_cast<impl1_2 *>(v[1].get()));
      expect(i == v[0]);
    }
  };

  auto injector = di::make_injector(di::bind<i1 *[]>().to<i1, di::named<class Impl1_2>>(),
                                    di::bind<i1>().to<impl1>().in(di::singleton) /*deduced as singleton as well*/
                                    ,
                                    di::bind<i1>().to<impl1_2>().named<class Impl1_2>());

  injector.create<c>();
};

test multi_bindings_smart_ptrs = [] {
  struct c {
    c(std::unique_ptr<std::vector<std::shared_ptr<i1>>> v1, std::shared_ptr<std::vector<std::unique_ptr<i1>>> v2) {
      expect(v1->size() == 1);
      expect(dynamic_cast<impl1 *>((*v1)[0].get()));
      expect(v2->size() == 1);
      expect(dynamic_cast<impl1 *>((*v2)[0].get()));
    }
  };

  auto injector = di::make_injector(di::bind<i1 *[]>().to<impl1>());

  injector.create<c>();
};

test multi_bindings_with_scope = [] {
  struct c {
    c(std::shared_ptr<std::vector<std::shared_ptr<i1>>> v1, std::shared_ptr<std::vector<std::shared_ptr<i1>>> v2,
      std::vector<std::shared_ptr<i1>> &v3) {
      expect(v1 == v2);
      expect(*v2 == v3);
    }
  };

  auto injector = di::make_injector(di::bind<i1 *[]>().to<impl1>());

  injector.create<c>();
};

test multi_bindings_with_initializer_list = [] {
  auto test = [](auto object) {
    expect(object.size() == 4);
    auto it = object.begin();
    expect(*(std::next(it, 0)) == 1);
    expect(*(std::next(it, 1)) == 2);
    expect(*(std::next(it, 2)) == 3);
    expect(*(std::next(it, 3)) == 4);
  };

  auto il = {1, 2, 3, 4};
  auto injector = di::make_injector(di::bind<int[]>().to(il));

  test(injector.create<std::vector<int>>());
  test(injector.create<std::set<int>>());
};

test multi_bindings_with_initializer_list_with_ptr_type = [] {
  auto test = [](auto object) {
    expect(object.size() == 4);
    auto it = object.begin();
    expect(*(std::next(it, 0)) == 1);
    expect(*(std::next(it, 1)) == 2);
    expect(*(std::next(it, 2)) == 3);
    expect(*(std::next(it, 3)) == 4);
  };

  auto il = {1, 2, 3, 4};
  auto injector = di::make_injector(di::bind<int *[]>().to(il));

  test(injector.create<std::vector<int>>());
  test(injector.create<std::set<int>>());
};

test bind_tuple = [] {
  struct c {
    c(std::tuple<>, const std::tuple<int, double> &t) {
      expect(42 == std::get<0>(t));
      expect(87.0 == std::get<1>(t));
    }
  };

  auto injector = di::make_injector(di::bind<>().to(42), di::bind<>().to(87.0));
  injector.create<c>();
};

test bind_tuple_ref = [] {
  struct c {
    c(std::tuple<int, double> &t1, std::tuple<int, double> &t2) {
      expect(42 == std::get<0>(t1));
      expect(87.0 == std::get<1>(t1));
      expect(t1 == t2);
    }
  };

  auto injector = di::make_injector(di::bind<>().to(42), di::bind<>().to(87.0));
  injector.create<c>();
};

template <class T>
class lazy {
  template <class TInjector>
  static auto create(const void *injector) {
    return ((TInjector *)injector)->template create<T>();
  }

 public:
  using boost_di_inject__ = di::inject<di::self<T>>;

  template <class TInjector>
  explicit lazy(const TInjector &i) noexcept : injector_((void *)&i), f(create<TInjector>) {}

  T get() const { return (*f)(injector_); }

 private:
  const void *injector_ = nullptr;
  T (*f)(const void *) = nullptr;
};

test bind_self_lazy = [] {
  struct c {
    explicit c(lazy<int> i) { expect(42 == i.get()); }
  };

  struct c_inject {
    BOOST_DI_INJECT(explicit c_inject, lazy<int> i) { expect(42 == i.get()); }
  };

  auto injector = di::make_injector(di::bind<int>().to(42));

  injector.create<c>();
  injector.create<c_inject>();
};

test bind_self_lazy_const_ref = [] {
  struct c {
    c(const lazy<int> &i) {  // non explicit
      expect(42 == i.get());
    }
  };

  struct c_inject {
    BOOST_DI_INJECT(c_inject, const lazy<int> &i) {  // non explicit
      expect(42 == i.get());
    }
  };

  auto injector = di::make_injector(di::bind<int>().to(42));

  injector.create<c>();
  injector.create<c_inject>();
};

test bind_self_lazy_ref = [] {
  struct c {
    explicit c(lazy<int> &i) { expect(42 == i.get()); }
  };

  struct c_inject {
    BOOST_DI_INJECT(explicit c_inject, lazy<int> &i) { expect(42 == i.get()); }
  };

  auto injector = di::make_injector(di::bind<int>().to(42));

  injector.create<c>();
  injector.create<c_inject>();
};

test bind_self_lazy_interface_mix = [] {
  struct c {
    c(lazy<int> i1_, const lazy<std::unique_ptr<i1>> &i2_) : i1_(i1_), i2_(i2_) {}

    void initialize() {
      expect(42 == i1_.get());
      auto object = i2_.get();
      expect(dynamic_cast<impl1 *>(object.get()));
    }

    lazy<int> i1_;
    lazy<std::unique_ptr<i1>> i2_;
  };

  struct c_inject {
    BOOST_DI_INJECT(c_inject, lazy<int> i1_, const lazy<std::unique_ptr<i1>> &i2_) : i1_(i1_), i2_(i2_) {}

    void initialize() {
      expect(42 == i1_.get());
      auto object = i2_.get();
      expect(dynamic_cast<impl1 *>(object.get()));
    }

    lazy<int> i1_;
    lazy<std::unique_ptr<i1>> i2_;
  };

  struct c_uniform {
    lazy<int> i1_;
    lazy<std::unique_ptr<i1>> i2_;

    void initialize() {
      expect(42 == i1_.get());
      auto object = i2_.get();
      expect(dynamic_cast<impl1 *>(object.get()));
    }
  };

  auto injector = di::make_injector(di::bind<int>().to(42), di::bind<i1>().to<impl1>());

  {
    auto object = injector.create<c>();
    object.initialize();
  }

  {
    auto object = injector.create<c_inject>();
    object.initialize();
  }

  {
    auto object = injector.create<c_uniform>();
    object.initialize();
  }
};

#if !defined(__MSVC__)
test bind_movable_only_type = [] {
  struct c {
    c(c &&) = default;
  };
  di::make_injector().create<c>();
};
#endif

test bind_movable_only_type_with_default_ctor = [] {
  struct c {
    c() = default;
    c(c &&) = default;
  };
  di::make_injector().create<c>();
};

test bind_movable_only_type_with_defined_default_ctor = [] {
  struct c {
    c() {}
    c(c &&) = default;
  };
  di::make_injector().create<c>();
};

#if !defined(__MSVC__)
test bind_movable_only_type_deleted_copy_ctor = [] {
  struct c {
    c(c &&) = default;
    c(const c &) = delete;
  };
  di::make_injector().create<c>();
};
#endif

struct empty {};
struct c_explicit {
  explicit c_explicit(empty &) { called = true; }
  c_explicit(const c_explicit &) = delete;
  bool called{};
};

test bind_deleted_copy_ctor_and_explicit_ctor = [] { expect(di::make_injector().create<c_explicit &>().called); };

struct c_non_explicit {
  c_non_explicit(empty &) {  // non explicit
    called = true;
  }
  c_non_explicit(const c_non_explicit &) = delete;
  bool called{};
};

test bind_deleted_copy_ctor_and_non_explicit_ctor = [] { expect(di::make_injector().create<c_non_explicit &>().called); };

test bind_unique_ptr_with_one_explicit_ctor_arg = [] {
  struct c {
    explicit c(i1 *ptr) : i1_(ptr) {}
    std::unique_ptr<i1> i1_;
  };
  const auto injector = di::make_injector(di::bind<i1>().to<impl1>());
  auto object = injector.create<c>();
  expect(dynamic_cast<impl1 *>(object.i1_.get()));
};

test bind_unique_ptr_with_one_non_explicit_ctor_arg = [] {
  struct c {
    c(i1 *ptr)  // non explicit
        : i1_(ptr) {}
    std::unique_ptr<i1> i1_;
  };
  const auto injector = di::make_injector(di::bind<i1>().to<impl1>());
  auto object = injector.create<c>();
  expect(dynamic_cast<impl1 *>(object.i1_.get()));
};

test bind_unique_ptr_dtor_defined = [] {
  struct c {
    c() {}
    ~c() {}
    std::unique_ptr<i1> i1_;
  };
  const auto injector = di::make_injector(di::bind<i1>().to<impl1>());
  auto &&object = injector.create<c &>();
  expect(nullptr == object.i1_.get());
};

test bind_unique_ptr_dtor_defined_with_move_ctor = [] {
  struct c {
    c() {}
    c(c &&) = default;
    ~c() = default;
    std::unique_ptr<i1> i1_;
  };
  const auto injector = di::make_injector(di::bind<i1>().to<impl1>());
  auto &&object = injector.create<c>();
  expect(nullptr == object.i1_.get());
};

class A;
class B;
class Concept1;

template <class T = A>
struct app1 {
  T t;
  explicit app1(T t) : t(t) {}
};

test bind_template_to_type = [] {
  struct classA {
#if defined(__MSVC__)
    explicit classA(int i) : i(i) {}
#endif
    int i = 0;
  };
  const auto injector = di::make_injector(di::bind<A>().to<classA>(), di::bind<>().to(42));
  auto object = injector.create<app1>();
  expect(42 == object.t.i);
};

template <class T = A, class U = Concept1>
struct app2 {
  T &t;
  const U &u;
  app2(T &t, const U &u) : t(t), u(u) {}
};

test bind_template_to_types = [] {
  struct classA {
#if defined(__MSVC__)
    explicit classA(int i) : i(i) {}
#endif
    int i = 0;
  };
  struct ConceptImpl {};
  const auto injector =
      di::make_injector(di::bind<A>().to<classA>(), di::bind<Concept1>().to<ConceptImpl>(), di::bind<>().to(42));
  auto object = injector.create<app2>();
  expect(42 == object.t.i);
  static_expect(std::is_same<classA, std::decay_t<decltype(object.t)>>::value);
  static_expect(std::is_same<ConceptImpl, std::decay_t<decltype(object.u)>>::value);
};

struct classB {};

template <class T = B>
struct classA {
  using type = T;
  explicit classA(int i) : i(i) {}
  int i = 0;
};

template <class T = A>
struct app3 {
  T t;
};

test bind_template_to_type_templates_type = [] {
  const auto injector = di::make_injector(di::bind<A>().to<classA>(), di::bind<B>().to<classB>(), di::bind<>().to(42));

  {
    auto object = injector.create<app3>();
    static_expect(std::is_same<classA<classB>, std::decay_t<decltype(object.t)>>::value);
    static_expect(std::is_same<classB, std::decay_t<typename decltype(object.t)::type>>::value);
    expect(42 == object.t.i);
  }

  {
    auto object = di::create<app3>(injector);
    static_expect(std::is_same<classA<classB>, std::decay_t<decltype(object.t)>>::value);
    static_expect(std::is_same<classB, std::decay_t<typename decltype(object.t)::type>>::value);
    expect(42 == object.t.i);
  }
};

template <typename T = A>
struct appc {
  explicit appc(T &t) : t(t) { static_expect(std::is_same<T, std::vector<std::string>>::value); }
  T &t;
};

test bind_template_to_container_type = [] {
  using T = std::vector<std::string>;
  T t{"str1", "str2"};
  const auto injector = di::make_injector(di::bind<A>().to<T>(), di::bind<>().to(t));
  auto object = injector.create<appc>();

  expect(2 == object.t.size());
  expect("str1" == object.t[0]);
  expect("str2" == object.t[1]);
};

#if !defined(__MSVC__)
template <class T = class External>
struct app4 {
  using type = T;
  T &t;
};

test bind_template_to_instance = [] {
  auto i = 42;
  const auto injector = di::make_injector(di::bind<class External>().to<int>(i));
  auto object = injector.create<app4>();
  static_expect(std::is_same<int, typename decltype(object)::type>::value);
  expect(42 == object.t);
  i = 87;
  expect(87 == object.t);
};

template <class T = Concept>
struct app5 {
  using type = T;
};

test bind_template_to_concept_type = [] {
  const auto injector = di::make_injector(di::bind<Concept>().to<ConceptImpl>());
  auto object = injector.create<app5>();
  static_expect(std::is_same<ConceptImpl, typename decltype(object)::type>::value);
};
#endif

test bind_final_class = [] {
  struct c final {
    explicit c(std::shared_ptr<i1> sp) : sp(sp) {}
    std::shared_ptr<i1> sp;
  };

  auto injector = di::make_injector(di::bind<i1>().to<impl1_final>());
  auto object = injector.create<c>();

  expect(dynamic_cast<impl1_final *>(object.sp.get()));
};

test bind_final_class_instance = [] {
  struct c final {
    explicit c(std::shared_ptr<i1> sp) : sp(sp) {}
    std::shared_ptr<i1> sp;
  };

  auto injector = di::make_injector(di::bind<i1>().to(std::make_shared<impl1_final>()));
  auto object = injector.create<c>();

  expect(dynamic_cast<impl1_final *>(object.sp.get()));
};

test bind_final_class_callable = [] {
  struct c final {
    int operator()() const { return {}; };
  };

  auto injector = di::make_injector(di::bind<std::function<int()>>().to<c>(), di::bind<int>().to([] { return 42; }));
  expect(0 == injector.create<std::function<int()>>()());
  expect(42 == injector.create<int>());
};

test bind_final_class_instance_unique = [] {
  auto injector = di::make_injector(di::bind<impl1_final>().to([]() { return std::make_unique<impl1_final>(); }));
  auto object = injector.create<std::unique_ptr<impl1_final>>();
  expect(dynamic_cast<impl1_final *>(object.get()));
};

test bind_multiple_empty_injectors = [] {
  auto i1 = di::make_injector();
  const auto module1 = [] { return [] {}; };
  const auto module2 = [] { return di::make_injector([] {}); };

  const auto injector =
      di::make_injector([] {}, [] {}, di::make_injector([] {}), std::move(i1), di::make_injector([] {}), module1(), module2());

  expect(0 == injector.create<int>());
};

test bind_to_ctor_long_notation = [] {
  struct c {
    c(int a, double b) : a{a}, b{b} {}

    int a{};
    double b{};
  };

  struct app {
    c &c_;
  };

  const auto injector = di::make_injector(di::bind<c>().to<c>(42, 87.0));

  auto object = injector.create<app>();

  expect(42 == object.c_.a);
  expect(87.0 == object.c_.b);
};

test bind_to_ctor_ambigious_ctor = [] {
  struct c {
    c(int a, double b) : a{a}, b{b} {}
    c(double b, int a) : a{a}, b{b} {}

    int a{};
    double b{};
  };

  struct app {
    c &c_;
  };

  const auto injector = di::make_injector(di::bind<c>()(42, 87.0));

  auto object = injector.create<app>();

  expect(42 == object.c_.a);
  expect(87.0 == object.c_.b);
};

test bind_to_ctor_ambigious_ctor_1_arg = [] {
  struct c {
    c(int a) : a(a) {}
    c(double a) : a(int(a)) {}
    int a{};
  };

  struct app {
    c &c_;
  };

  const auto injector = di::make_injector(di::bind<c>()(42));

  auto object = injector.create<app>();

  expect(42 == object.c_.a);
};

test bind_to_ctor_ambigious_ctor_1_arg_explicit = [] {
  struct c {
    explicit c(int a) : a(a) {}
    explicit c(double a) : a(int(a)) {}
    int a{};
  };

  struct app {
    c &c_;
  };

  const auto injector = di::make_injector(di::bind<c>()(42));
  auto object = injector.create<app>();

  expect(42 == object.c_.a);
};

test bind_to_ctor_placeholders = [] {
  struct c {
    c(int a, double b) : a{a}, b{b} {}
    c(double b, int a) : a{a}, b{b} {}

    int a{};
    double b{};
  };

  struct app {
    c &c_;
  };

  const auto injector = di::make_injector(di::bind<c>()(42, di::placeholders::_), di::bind<>().to(87.0));

  auto object = injector.create<app>();

  expect(42 == object.c_.a);
  expect(87.0 == object.c_.b);
};

test bind_to_ctor_multiple_placeholders = [] {
  struct c {
    explicit c(...) {}
    c(int a, std::shared_ptr<i1> sp, double b, std::unique_ptr<i2> up) : a{a}, b{b}, sp{sp}, up{std::move(up)} {}

    int a{};
    double b{};
    std::shared_ptr<i1> sp{};
    std::unique_ptr<i2> up{};
  };

  struct app {
    c &c_;
  };

  const auto injector = di::make_injector(di::bind<i1>().to<impl1>(), di::bind<i2>().to<impl2>(),
                                          di::bind<c>()(42, di::placeholders::_, 87.0, di::placeholders::_));

  auto object = injector.create<app>();

  expect(42 == object.c_.a);
  expect(87.0 == object.c_.b);
  expect(dynamic_cast<impl1 *>(object.c_.sp.get()));
  expect(dynamic_cast<impl2 *>(object.c_.up.get()));
};

test bind_to_ctor_short_notation = [] {
  struct c {
    c(int a, int b) : a{a}, b{b} {}

    int a{};
    int b{};
  };

  struct app {
    c &c_;
  };

  const auto injector = di::make_injector(di::bind<c>()(42, 87));

  auto object = injector.create<app>();

  expect(42 == object.c_.a);
  expect(87 == object.c_.b);
};

#if defined(__cpp_variable_templates)
test bind_to_ctor_short_notation_variable = [] {
  struct c {
    c(int a, int b) : a{a}, b{b} {}

    int a{};
    int b{};
  };

  struct app {
    c &c_;
  };

  const auto injector = di::make_injector(di::bind<c>(42, 87));

  auto object = injector.create<app>();

  expect(42 == object.c_.a);
  expect(87 == object.c_.b);
};

test bind_mix = [] {
  static constexpr auto i = 42;

  struct c {
    c(int i_, std::unique_ptr<i1> i1_, std::unique_ptr<i2> i2_) : i_(i_), i1_(std::move(i1_)), i2_(std::move(i2_)) {}

    int i_ = 0;
    std::unique_ptr<i1> i1_;
    std::unique_ptr<i2> i2_;
  };

  auto injector = di::make_injector(di::bind<i1>().to<impl1>()  // cross compiler call
                                    ,
                                    di::bind<i2>.to<impl2>()  // requires variable templates
                                    ,
                                    di::bind<int>().to(i));

  auto object = injector.create<c>();
  expect(i == object.i_);
  expect(dynamic_cast<impl1 *>(object.i1_.get()));
  expect(dynamic_cast<impl2 *>(object.i2_.get()));
};
#endif

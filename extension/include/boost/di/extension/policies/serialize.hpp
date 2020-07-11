//
// Copyright (c) 2012-2019 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include <algorithm>
#include <functional>
#include <sstream>
#include <string>
#include <vector>

#include "boost/di.hpp"

BOOST_DI_NAMESPACE_BEGIN
namespace extension {

template <class T>
auto get_type() -> std::string {
#if defined(__clang__)
  return {&__PRETTY_FUNCTION__[63], sizeof(__PRETTY_FUNCTION__) - 65};
#elif defined(__GNUC__)
  return {&__PRETTY_FUNCTION__[77], sizeof(__PRETTY_FUNCTION__) - 136};
#endif
}

template <class T>
auto id(T out) -> std::string {
  std::replace(out.begin(), out.end(), ' ', '_');
  return out;
}

struct archive {
  std::string path;
  std::function<char*()> ptr;
  std::string type;
  int offset;
};
struct context : std::vector<archive> {
  context() {}
  int offset = 0;
};
struct serializable : std::vector<archive> {
  serializable() {}
};

struct policy_guard {
  policy_guard() { check() = true; }
  ~policy_guard() { check() = false; }
  static bool& check() {
    static auto guard = false;
    return guard;
  }
};

template <class TGiven, class TInjector>
std::function<char*()> get_ptr(const TInjector&, std::false_type) {
  return {};
}

template <class TGiven, class TInjector>
std::function<char*()> get_ptr(const TInjector& injector, std::true_type) {
  return [&injector] {
    policy_guard _{};
    return reinterpret_cast<char*>(&injector.template create<TGiven&>());
  };
}

class serializable_policy : public config {
 public:
  template <class TInjector>
  static auto policies(TInjector* injector) noexcept {
    return make_policies([&injector](auto type) {
      if (policy_guard::check()) {
        return;
      }
      policy_guard _{};
      using T = decltype(type);
      using given = aux::decay_t<typename decltype(type)::type>;
      auto&& v = injector->template create<context&>();
      auto ptr = get_ptr<given>(*injector, std::is_reference<typename T::type>{});
      if (std::is_standard_layout<typename T::type>::value && T::arity::value) {
        ptr = v.back().ptr;
      }

      std::string element{};
      if (!v.empty()) {
        element = v.back().path + "->";
        if (v.back().ptr) {
          auto& serialize = injector->template create<serializable&>();
          if (!T::arity::value) {
            const auto align = alignof(given);
            v.offset = ((v.offset + align - 1) / align) * align;
            serialize.push_back({v.back().path, v.back().ptr, id(get_type<given>()), v.offset});
            v.offset += sizeof(given);
          }
        }
        v.pop_back();
        if (v.empty()) {
          v.offset = {};
        }
      }

      auto ctor_size = T::arity::value;
      while (ctor_size--) {
        v.push_back({element + id(get_type<given>()), ptr, {}, 0});
      }
    });
  }
};

template <class... Ts>
struct serializable_call {
  template <class TInjector, class TExpr>
  static void apply(const TInjector& injector, TExpr expr) {
    for (const auto& o : injector.template create<serializable&>()) {
      apply_impl(o, expr, aux::type_list<Ts...>{});
    }
  }

  template <class TExpr, class T, class... Us>
  static void apply_impl(const archive& o, TExpr expr, aux::type_list<T, Us...>) {
    if (id(get_type<T>()) == o.type) {
      expr(o, T{});
    } else {
      apply_impl(o, expr, aux::type_list<Us...>{});
    }
  }

  template <class TExpr>
  static void apply_impl(const archive&, TExpr, aux::type_list<>) {}
};

// clang-format off
using serializable_call_t = serializable_call<
  unsigned char
, unsigned short
, unsigned int
, unsigned long
, unsigned long long
, signed char
, short
, int
, long
, long long
, char
, float
, double
, long double
, bool
>;
// clang-format on

auto serialize = [](const auto& injector, auto& str) {
  serializable_call_t::apply(injector, [&](const auto& o, auto t) {
    str << o.path << " " << o.type << " " << o.offset << " "
        << std::to_string(*reinterpret_cast<decltype(t)*>(o.ptr() + o.offset)) << std::endl;
  });
};

auto deserialize = [](const auto& injector, auto& str) {
  serializable_call_t::apply(injector, [&](const auto& o, auto t) {
    std::string line, path, type;
    decltype(t) value = {};
    auto offset = 0;

    for (std::string line; std::getline(str, line);) {
      std::istringstream iss{line};
      iss >> path >> type >> offset >> value;
      if (type == o.type) {
        *reinterpret_cast<decltype(t)*>(o.ptr() + offset) = value;
        break;
      }
    }
  });
};

}  // namespace extension
BOOST_DI_NAMESPACE_END

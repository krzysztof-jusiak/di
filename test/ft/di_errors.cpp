//
// Copyright (c) 2012-2016 Krzysztof Jusiak (krzysztof at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <initializer_list>
#include <iostream>
#include <memory>
#include <regex>
#include <sstream>
#include <string>
#include <vector>
#include "boost/di/aux_/compiler.hpp"
#include "common/utils.hpp"

namespace {

template <class... TArgs>
auto errors(const TArgs&... args) {
  return std::vector<std::string>{(".*" + std::string{args} + ".*")...};
}

auto compail_fail(int id, const std::string& defines, const std::vector<std::string>& errors, const std::string& code) {
  std::stringstream command;
  {
    file<> source_code{std::string{"error"} + std::to_string(id) + ".cpp"};
    source_code << "#include <boost/di.hpp>" << std::endl;
    source_code << "namespace di = boost::di;" << std::endl;
    source_code << code;

    std::stringstream errors;

#if defined(__CLANG__)
    errors << "-Werror";
#elif defined(__GCC__)
    errors << "-Werror";
#elif defined(__MSVC__)
    errors << "/EHsc /W3 /WX";
#endif

    auto include_rgx = std::regex{"<include>"};

#if defined(__MSVC__)
    auto include = std::regex_replace(defines, include_rgx, "/FI");
#else
    auto include = std::regex_replace(defines, include_rgx, "-include");
#endif

    command << cxx() << " " << cxxflags(true) << " " << include << " " << errors.str() << " " << source_code
#if defined(__MSVC__)
            << " >"
#else
            << " 2>"
#endif
            << " error" << id << ".out";
  }

  std::clog << command.str() << std::endl;
  expect(std::system(command.str().c_str()));

  file<std::ifstream> output{std::string{"error"} + std::to_string(id) + ".out"};
  std::vector<bool> matches(errors.size(), false);
  std::vector<std::string> out;
  auto lines = 0;
  for (std::string line; std::getline(output, line);) {
    out.push_back(line);
    auto i = 0;
    for (const auto& rgx : errors) {
      if (std::regex_match(line, std::regex{rgx})) {
        matches[i] = true;
      }
      ++i;
    }
    ++lines;
  }

  if (!errors.empty()) {
#if defined(__MSVC__)
    constexpr auto MAX_ERROR_LINES_COUNT = 128;
#else
    constexpr auto MAX_ERROR_LINES_COUNT = 64;
#endif

    if (lines >= MAX_ERROR_LINES_COUNT) {
      std::clog << command.str() << " | lines < MAX_ERROR_LINES_COUNT" << std::endl;
      expect(false);
    }
  }

  if (std::find(matches.begin(), matches.end(), false) != matches.end()) {
    for (const auto& line : out) {
      std::clog << line << std::endl;
    }
    std::clog << command.str() << std::endl;
    expect(false);
  }
}
}

#define expect_compile_fail(defines, error, ...) compail_fail(__LINE__, defines, error, #__VA_ARGS__)

// ---------------------------------------------------------------------------

test bind_external_with_given_scope = [] {
  expect_compile_fail("", errors(), di::make_injector(di::bind<int>().in(di::unique).to(42)););
};

#if defined(__cpp_variable_templates)
test bind_external_with_given_scope_v = [] {
  expect_compile_fail("", errors(), di::make_injector(di::bind<int>.in(di::unique).to(42)););
};
#endif

test bind_external_with_given_type = [] {
  expect_compile_fail("", errors(), struct i{}; struct impl
                      : i{};
                      int main() { di::make_injector(di::bind<i>().to(impl{}).to<impl>()); });
};

#if defined(__cpp_variable_templates)
test bind_external_with_given_type_v = [] {
  expect_compile_fail("", errors(), struct i{}; struct impl
                      : i{};
                      int main() { di::make_injector(di::bind<i>.to(impl{}).to<impl>()); });
};
#endif

test bind_named_to_named = [] {
  expect_compile_fail("", errors(), struct a{}; struct b{};
                      int main() { di::make_injector(di::bind<int>().named(a{}).named(b{})); });
};

#if defined(__cpp_variable_templates)
test bind_named_to_named_v = [] {
  expect_compile_fail("", errors(), struct a{}; struct b{};
                      int main() { di::make_injector(di::bind<int>.named(a{}).named(b{})); });
};
#endif

test bind_external_with_given_value = [] {
  expect_compile_fail("", errors(), int main() { di::make_injector(di::bind<int>().to<std::integral_constant<int>>(0)); });
};

#if defined(__cpp_variable_templates)
test bind_external_with_given_value_v = [] {
  expect_compile_fail("", errors(), int main() { di::make_injector(di::bind<int>.to<std::integral_constant<int>>(0)); });
};
#endif

test bind_in_not_scopable_type = [] {
  auto errors_ =
      errors("constraint not satisfied",
#if defined(__MSVC__)
             "scope<.*>::requires_<.*scope<.*>::is_referable,.*scope<.*>::try_create,.*scope<.*>::create>", "=.*dummy"
#else
             "scope<.*dummy>::requires_<.*scope<.*>::is_referable,.*scope<.*>::try_create,.*scope<.*>::create>"
#endif
             );

  expect_compile_fail("", errors_, struct dummy{}; int main() { di::make_injector(di::bind<int>().in(dummy{})); });
};

#if defined(__cpp_variable_templates)
test bind_in_not_scopable_type_v = [] {
  auto errors_ =
      errors("constraint not satisfied",
#if defined(__MSVC__)
             "scope<.*>::requires_<.*scope<.*>::is_referable,.*scope<.*>::try_create,.*scope<.*>::create>", "=.*dummy"
#else
             "scope<.*dummy>::requires_<.*scope<.*>::is_referable,.*scope<.*>::try_create,.*scope<.*>::create>"
#endif
             );

  expect_compile_fail("", errors_, struct dummy{}; int main() { di::make_injector(di::bind<int>.in(dummy{})); });
};
#endif

test bind_has_disallowed_qualifiers_expected = [] {
  auto errors_ = errors("constraint not satisfied", "type_<.*int.*\\*>::has_disallowed_qualifiers");
  expect_compile_fail("", errors_, int main() { di::make_injector(di::bind<int*>()); });
};

#if defined(__cpp_variable_templates)
test bind_has_disallowed_qualifiers_expected_v = [] {
  auto errors_ = errors("constraint not satisfied", "type_<.*int.*\\*>::has_disallowed_qualifiers");
  expect_compile_fail("", errors_, int main() { di::make_injector(di::bind<int*>); });
};
#endif

test bind_has_disallowed_qualifiers_given = [] {
  auto errors_ = errors("constraint not satisfied", "type_<const.*int.*&>::has_disallowed_qualifiers");
  expect_compile_fail("", errors_, int main() { di::make_injector(di::bind<int>().to<const int&>()); });
};

#if defined(__cpp_variable_templates)
test bind_has_disallowed_qualifiers_given_v = [] {
  auto errors_ = errors("constraint not satisfied", "type_<const.*int.*&>::has_disallowed_qualifiers");
  expect_compile_fail("", errors_, int main() { di::make_injector(di::bind<int>.to<const int&>()); });
};
#endif

test bind_has_disallowed_qualifiers_expected_complex = [] {
  auto errors_ = errors("constraint not satisfied", "type_<.*shared_ptr.*int.*>::has_disallowed_qualifiers");
  expect_compile_fail("<include> memory", errors_, int main() { di::make_injector(di::bind<std::shared_ptr<int>>()); });
};

#if defined(__cpp_variable_templates)
test bind_has_disallowed_qualifiers_expected_complex_v = [] {
  auto errors_ = errors("constraint not satisfied", "type_<.*shared_ptr.*int.*>::has_disallowed_qualifiers");
  expect_compile_fail("<include> memory", errors_, int main() { di::make_injector(di::bind<std::shared_ptr<int>>); });
};
#endif

test bind_has_disallowed_qualifiers_given_complex = [] {
  auto errors_ = errors("constraint not satisfied", "type_<.*shared_ptr.*int.*>::has_disallowed_qualifiers");
  expect_compile_fail("<include> memory", errors_, int main() { di::make_injector(di::bind<int, std::shared_ptr<int>>()); });
};

#if defined(__cpp_variable_templates)
test bind_has_disallowed_qualifiers_given_complex_v = [] {
  auto errors_ = errors("constraint not satisfied", "type_<.*shared_ptr.*int.*>::has_disallowed_qualifiers");
  expect_compile_fail("<include> memory", errors_, int main() { di::make_injector(di::bind<int, std::shared_ptr<int>>); });
};
#endif

test bind_narrowed_type = [] {
  auto errors_ = errors("constraint not satisfied",
#if defined(__MSVC__)
                        "type_<.*>::is_not_related_to<int>", "=.*double"
#else
                        "type_<.*double>::is_not_related_to<int>"
#endif
                        );

  expect_compile_fail("", errors_, int main() { di::make_injector(di::bind<int>().to<double>()); });
};

#if defined(__cpp_variable_templates)
test bind_narrowed_type_v = [] {
  auto errors_ = errors("constraint not satisfied",
#if defined(__MSVC__)
                        "type_<.*>::is_not_related_to<int>", "=.*double"
#else
                        "type_<.*double>::is_not_related_to<int>"
#endif
                        );

  expect_compile_fail("", errors_, int main() { di::make_injector(di::bind<int>.to<double>()); });
};
#endif

test bind_not_compatible_types = [] {
  auto errors_ = errors("constraint not satisfied",
#if defined(__MSVC__)
                        "type_<.*>::is_not_related_to<int>", "=.*impl"
#else
                        "type_<.*impl>::is_not_related_to<int>"
#endif
                        );

  expect_compile_fail("", errors_, struct i{}; struct impl
                      : i{};
                      int main() { di::make_injector(di::bind<int>().to<impl>()); });
};

#if defined(__cpp_variable_templates)
test bind_not_compatible_types_v = [] {
  auto errors_ = errors("constraint not satisfied",
#if defined(__MSVC__)
                        "type_<.*>::is_not_related_to<int>", "=.*impl"
#else
                        "type_<.*impl>::is_not_related_to<int>"
#endif
                        );

  expect_compile_fail("", errors_, struct i{}; struct impl : i{}; int main() { di::make_injector(di::bind<int>.to<impl>()); });
};
#endif

test bind_not_compatible_external = [] {
  auto errors_ = errors("constraint not satisfied",
#if defined(__MSVC__)
                        "type_<.*>::is_not_related_to<int>", "=.*impl"
#else
                        "type_<.*impl>::is_not_related_to<int>"
#endif
                        );

  expect_compile_fail("", errors_, struct i{}; struct impl
                      : i{};
                      int main() { di::make_injector(di::bind<int>().to(impl{})); });
};

#if defined(__cpp_variable_templates)
test bind_not_compatible_external_v = [] {
  auto errors_ = errors("constraint not satisfied",
#if defined(__MSVC__)
                        "type_<.*>::is_not_related_to<int>", "=.*impl"
#else
                        "type_<.*impl>::is_not_related_to<int>"
#endif
                        );

  expect_compile_fail("", errors_, struct i{}; struct impl : i{}; int main() { di::make_injector(di::bind<int>.to(impl{})); });
};
#endif

test bind_not_compatible_narrowed_types = [] {
  auto errors_ = errors("constraint not satisfied",
#if defined(__MSVC__)
                        "type_<.*>::is_not_related_to<int>", "=.*long"
#elif defined(__CLANG__)
                        "type_<.*long>::is_not_related_to<int>"
#else
                        "type_<.*long.*int>::is_not_related_to<int>"
#endif
                        );

  expect_compile_fail("", errors_, int main() { di::make_injector(di::bind<int>().to(42l)); });
};

#if defined(__cpp_variable_templates)
test bind_not_compatible_narrowed_types_v = [] {
  auto errors_ = errors("constraint not satisfied",
#if defined(__MSVC__)
                        "type_<.*>::is_not_related_to<int>", "=.*long"
#elif defined(__CLANG__)
                        "type_<.*long>::is_not_related_to<int>"
#else
                        "type_<.*long.*int>::is_not_related_to<int>"
#endif
                        );

  expect_compile_fail("", errors_, int main() { di::make_injector(di::bind<int>.to(42l)); });
};
#endif

test bind_not_compatible_initializer_list = [] {
  auto errors_ = errors("constraint not satisfied",
#if defined(__MSVC__)
                        "type_<.*>::is_not_related_to<int>", "=.*const.*char.*\\*"
#else
                        "type_<const.*char.*\\*>::is_not_related_to<int>"
#endif
                        );

  expect_compile_fail("", errors_, int main() { di::make_injector(di::bind<int[]>().to({"a", "b"})); });
};

#if defined(__cpp_variable_templates)
test bind_not_compatible_initializer_list_v = [] {
  auto errors_ = errors("constraint not satisfied",
#if defined(__MSVC__)
                        "type_<.*>::is_not_related_to<int>", "=.*const.*char.*\\*"
#else
                        "type_<const.*char.*\\*>::is_not_related_to<int>"
#endif
                        );

  expect_compile_fail("", errors_, int main() { di::make_injector(di::bind<int[]>.to({"a", "b"})); });
};
#endif

test bind_any_of_not_related = [] {
  auto errors_ = errors("constraint not satisfied",
#if defined(__MSVC__)
                        "type_<.*>::is_not_related_to<.*a>.*type_<.*>::is_not_related_to<.*b>", "=.*c"
#else
                        "type_<.*c>::is_not_related_to<.*a>.*type_<.*c>::is_not_related_to<.*b>"
#endif
                        );

  expect_compile_fail("", errors_, struct a{}; struct b
                      : a{};
                      struct c{}; int main() { di::make_injector(di::bind<a, b>().to<c>()); });
};

#if defined(__cpp_variable_templates)
test bind_any_of_not_related_v = [] {
  auto errors_ = errors("constraint not satisfied",
#if defined(__MSVC__)
                        "type_<.*>::is_not_related_to<.*a>.*type_<.*>::is_not_related_to<.*b>", "=.*c"
#else
                        "type_<.*c>::is_not_related_to<.*a>.*type_<.*c>::is_not_related_to<.*b>"
#endif
                        );

  expect_compile_fail("", errors_, struct a{}; struct b
                      : a{};
                      struct c{}; int main() { di::make_injector(di::bind<a, b>.to<c>()); });
};
#endif

test bind_deduced_not_related = [] {
  auto errors_ = errors("constraint not satisfied",
#if defined(__MSVC__)
                        "type_<.*>::is_not_related_to<.*deduced>", "=.*a"
#else
                        "type_<.*a>::is_not_related_to<.*deduced>"
#endif
                        );

  expect_compile_fail("", errors_, struct a{}; int main() { di::make_injector(di::bind<>().to<a>()); });
};

#if defined(__cpp_variable_templates)
test bind_deduced_not_related_v = [] {
  auto errors_ = errors("constraint not satisfied",
#if defined(__MSVC__)
                        "type_<.*>::is_not_related_to<.*deduced>", "=.*a"
#else
                        "type_<.*a>::is_not_related_to<.*deduced>"
#endif
                        );

  expect_compile_fail("", errors_, struct a{}; int main() { di::make_injector(di::bind<>.to<a>()); });
};
#endif

test bind_is_abstract_type = [] {
  auto errors_ = errors("constraint not satisfied",
#if defined(__MSVC__)
                        "type_<.*>::is_abstract", "=.*impl"
#else
                        "type_<.*impl>::is_abstract"
#endif
                        );

  expect_compile_fail("", errors_,
                      struct i {
                        virtual ~i() noexcept = default;
                        virtual void dummy() = 0;
                      };
                      struct impl
                      : i{};
                      struct c{c(i*){}}; int main() { di::make_injector(di::bind<i>().to<impl>()); });
};

#if defined(__cpp_variable_templates)
test bind_is_abstract_type_v = [] {
  auto errors_ = errors("constraint not satisfied",
#if defined(__MSVC__)
                        "type_<.*>::is_abstract", "=.*impl"
#else
                        "type_<.*impl>::is_abstract"
#endif
                        );

  expect_compile_fail("", errors_,
                      struct i {
                        virtual ~i() noexcept = default;
                        virtual void dummy() = 0;
                      };
                      struct impl
                      : i{};
                      struct c{c(i*){}}; int main() { di::make_injector(di::bind<i>.to<impl>()); });
};
#endif

test bind_is_abstract_type_with_missing_error = [] {
  auto errors_ = errors("constraint not satisfied",
#if defined(__MSVC__)
                        "type_<.*>::is_abstract", "=.*impl", "void i::dummy.*is abstract"
#else
                        "type_<.*impl>::is_abstract", "pure.*impl", "virtual void dummy().*=.*0"
#endif
                        );

  expect_compile_fail("-DBOOST_DI_CFG_DIAGNOSTICS_LEVEL=2", errors_,
                      struct i {
                        virtual ~i() noexcept = default;
                        virtual void dummy() = 0;
                      };
                      struct impl
                      : i{};
                      struct c{c(i*){}}; int main() { di::make_injector(di::bind<i>().to<impl>()); });
};

#if defined(__cpp_variable_templates)
test bind_is_abstract_type_with_missing_error_v = [] {
  auto errors_ = errors("constraint not satisfied",
#if defined(__MSVC__)
                        "type_<.*>::is_abstract", "=.*impl", "void i::dummy.*is abstract"
#else
                        "type_<.*impl>::is_abstract", "pure.*impl", "virtual void dummy().*=.*0"
#endif
                        );

  expect_compile_fail("-DBOOST_DI_CFG_DIAGNOSTICS_LEVEL=2", errors_,
                      struct i {
                        virtual ~i() noexcept = default;
                        virtual void dummy() = 0;
                      };
                      struct impl
                      : i{};
                      struct c{c(i*){}}; int main() { di::make_injector(di::bind<i>.to<impl>()); });
};
#endif

test bind_is_abstract_type_named = [] {
  auto errors_ = errors("constraint not satisfied",
#if defined(__MSVC__)
                        "type_<.*>::is_abstract", "=.*impl"
#else
                        "type_<.*impl>::is_abstract"
#endif
                        );
  expect_compile_fail("", errors_,
                      struct i {
                        virtual ~i() noexcept = default;
                        virtual void dummy() = 0;
                      };
                      struct impl
                      : i{};
                      struct dummy{}; struct c{BOOST_DI_INJECT(c, (named = dummy{})i*){}};
                      int main() { di::make_injector(di::bind<i>().named(dummy{}).to<impl>()); });
};

#if defined(__cpp_variable_templates)
test bind_is_abstract_type_named_v = [] {
  auto errors_ = errors("constraint not satisfied",
#if defined(__MSVC__)
                        "type_<.*>::is_abstract", "=.*impl"
#else
                        "type_<.*impl>::is_abstract"
#endif
                        );
  expect_compile_fail("", errors_,
                      struct i {
                        virtual ~i() noexcept = default;
                        virtual void dummy() = 0;
                      };
                      struct impl
                      : i{};
                      struct dummy{}; struct c{BOOST_DI_INJECT(c, (named = dummy{})i*){}};
                      int main() { di::make_injector(di::bind<i>.named(dummy{}).to<impl>()); });
};
#endif

test bind_deduced_external_repeated = [] {
  auto errors_ = errors("constraint not satisfied",
#if defined(__MSVC__)
                        "type_<.*>::is_bound_more_than_once", "=.*int"
#else
                        "type_<.*int>::is_bound_more_than_once"
#endif
                        );
  expect_compile_fail("", errors_, int main() { di::make_injector(di::bind<>().to(42), di::bind<>().to(42)); });
};

#if defined(__cpp_variable_templates)
test bind_deduced_external_repeated_v = [] {
  auto errors_ = errors("constraint not satisfied",
#if defined(__MSVC__)
                        "type_<.*>::is_bound_more_than_once", "=.*int"
#else
                        "type_<.*int>::is_bound_more_than_once"
#endif
                        );
  expect_compile_fail("", errors_, int main() { di::make_injector(di::bind<>.to(42), di::bind<>.to(42)); });
};
#endif

test bind_deduced_external_repeated_mix = [] {
  auto errors_ = errors("constraint not satisfied", "type_<.*int>::is_bound_more_than_once");
  expect_compile_fail("", errors_, int main() { di::make_injector(di::bind<int>().to(42), di::bind<>().to(42)); });
};

#if defined(__cpp_variable_templates)
test bind_deduced_external_repeated_mix_v = [] {
  auto errors_ = errors("constraint not satisfied", "type_<.*int>::is_bound_more_than_once");
  expect_compile_fail("", errors_, int main() { di::make_injector(di::bind<int>().to(42), di::bind<>.to(42)); });
};
#endif

test bind_repeated = [] {
  auto errors_ = errors("constraint not satisfied", "type_<.*i>::is_bound_more_than_once");
  expect_compile_fail("", errors_, struct i{}; struct impl1
                      : i{};
                      struct impl2
                      : i{};
                      int main() { di::make_injector(di::bind<i>().to<impl1>(), di::bind<i>().to<impl1>()); });
};

#if defined(__cpp_variable_templates)
test bind_repeated_v = [] {
  auto errors_ = errors("constraint not satisfied", "type_<.*i>::is_bound_more_than_once");
  expect_compile_fail("", errors_, struct i{}; struct impl1
                      : i{};
                      struct impl2
                      : i{};
                      int main() { di::make_injector(di::bind<i>.to<impl1>(), di::bind<i>.to<impl1>()); });
};
#endif

#if defined(__cpp_variable_templates)
test bind_repeated_mix_v = [] {
  auto errors_ = errors("constraint not satisfied", "type_<.*i>::is_bound_more_than_once");
  expect_compile_fail("", errors_, struct i{}; struct impl1
                      : i{};
                      struct impl2
                      : i{};
                      int main() { di::make_injector(di::bind<i>.to<impl1>(), di::bind<i>().to<impl1>()); });
};
#endif

test bind_to_different_types = [] {
  auto errors_ = errors("constraint not satisfied", "type_<.*i>::is_bound_more_than_once");
  expect_compile_fail("", errors_, struct i{}; struct impl1
                      : i{};
                      struct impl2
                      : i{};
                      int main() { di::make_injector(di::bind<i>().to<impl1>(), di::bind<i>().to<impl2>()); });
};

#if defined(__cpp_variable_templates)
test bind_to_different_types_v = [] {
  auto errors_ = errors("constraint not satisfied", "type_<.*i>::is_bound_more_than_once");
  expect_compile_fail("", errors_, struct i{}; struct impl1
                      : i{};
                      struct impl2
                      : i{};
                      int main() { di::make_injector(di::bind<i>.to<impl1>(), di::bind<i>.to<impl2>()); });
};
#endif

test bind_external_repeated = [] {
  auto errors_ = errors("constraint not satisfied", "type_<.*int>::is_bound_more_than_once");
  expect_compile_fail("", errors_, int main() { di::make_injector(di::bind<int>().to(42), di::bind<int>().to(87)); });
};

#if defined(__cpp_variable_templates)
test bind_external_repeated_v = [] {
  auto errors_ = errors("constraint not satisfied", "type_<.*int>::is_bound_more_than_once");
  expect_compile_fail("", errors_, int main() { di::make_injector(di::bind<int>.to(42), di::bind<int>.to(87)); });
};
#endif

#if defined(__cpp_variable_templates)
test bind_external_repeated_mix_v = [] {
  auto errors_ = errors("constraint not satisfied", "type_<.*int>::is_bound_more_than_once");
  expect_compile_fail("", errors_, int main() { di::make_injector(di::bind<int>().to(42), di::bind<int>.to(87)); });
};
#endif

test bind_multiple_times = [] {
  auto errors_ = errors("constraint not satisfied", "type_<.*i>::is_bound_more_than_once");

  expect_compile_fail("", errors_, struct i{}; struct impl1
                      : i{};
                      struct impl2
                      : i{};
                      int main() { di::make_injector(di::bind<i>().to<impl1>(), di::bind<i>().to<impl2>()); });
};

#if defined(__cpp_variable_templates)
test bind_multiple_times_v = [] {
  auto errors_ = errors("constraint not satisfied", "type_<.*i>::is_bound_more_than_once");

  expect_compile_fail("", errors_, struct i{}; struct impl1
                      : i{};
                      struct impl2
                      : i{};
                      int main() { di::make_injector(di::bind<i>.to<impl1>(), di::bind<i>.to<impl2>()); });
};
#endif

test bind_in_not_scopable = [] {
  expect_compile_fail("", errors(), struct not_scopable{}; int main() { di::bind<int>().in(not_scopable{}); });
};

#if defined(__cpp_variable_templates)
test bind_in_not_scopable_v = [] {
  expect_compile_fail("", errors(), struct not_scopable{}; int main() { di::bind<int>.in(not_scopable{}); });
};
#endif

// ---------------------------------------------------------------------------

test make_injector_wrong_arg = [] {
  auto errors_ = errors("constraint not satisfied",
#if defined(__MSVC__)
                        "type_<.*>::is_neither_a_dependency_nor_an_injector", "=.*dummy"
#else
                        "type_<.*dummy>::is_neither_a_dependency_nor_an_injector"
#endif
                        );

  expect_compile_fail("", errors_, struct dummy{}; int main() { di::make_injector(dummy{}); });
};

test make_injector_with_from_not_movable = [] {
  expect_compile_fail(
      "", errors(), struct c{}; int main() {
        auto module = di::make_injector();
        di::make_injector(module);
      });
};

test make_injector_with_from_not_movable_exposed = [] {
  expect_compile_fail(
      "", errors(), struct c{}; int main() {
        auto module = di::make_injector();
        di::injector<> injector = di::make_injector(module);
      });
};

test exposed_multiple_times = [] {
  auto errors_ = errors("constraint not satisfied",
#if defined(__MSVC__)
                        "type_<.*>::is_bound_more_than_once", "=.*c"
#else
                        "type_<.*c>::is_bound_more_than_once"
#endif
                        );

  expect_compile_fail("", errors_, struct c{}; int main() { di::injector<c, c> injector = di::make_injector(); });
};

// ---------------------------------------------------------------------------

test not_configurable_config = [] {
  expect_compile_fail("", errors(), struct dummy_config{}; int main() { di::make_injector<dummy_config>(); });
};

test make_policies_with_non_const_policy = [] {
  auto errors_ = errors("constraint not satisfied",
#if defined(__MSVC__)
                        "policy<.*>::requires_<.*call_operator_with_one_argument>", "=.*non_const_policy"
#else
                        "policy<.*non_const_policy>::requires_<.*call_operator_with_one_argument>"
#endif
                        );

    expect_compile_fail("", errors_,
	struct non_const_policy {
		template<class T>
		void operator()(const T&) {}
	};
    struct test_config : di::config {
    static auto policies(...) { return di::make_policies(non_const_policy{});
}
}
;);
}
;

test make_policies_with_non_movable_policy = [] {
    expect_compile_fail("", errors(),
	struct non_movable_policy {
    template <class T>
    void operator()(const T&) {}
    non_movable_policy(non_movable_policy &&) = delete;
	};
    struct test_config : di::config {
    static auto policies(...) { return di::make_policies(non_movable_policy{});
}
}
;);
}
;

test config_wrong_policy = [] {
  auto errors_ = errors("constraint not satisfied",
#if defined(__MSVC__)
                        "policy<.*>::requires_<.*call_operator_with_one_argument>", "=.*int"
#else
                        "policy<.*int>::requires_<.*call_operator_with_one_argument>"
#endif
                        );

    expect_compile_fail("", errors_,
        struct test_config : di::config {
            static auto policies(...) { return 42;
}
}
;
int main() { di::make_injector<test_config>(); }
    );
    }
    ;

    test config_policy_not_callable = [] {
      auto errors_ = errors("constraint not satisfied",
#if defined(__MSVC__)
                            "policy<.*>::requires_<.*call_operator_with_one_argument>", "=.*dummy"
#else
                            "policy<.*dummy>::requires_<.*call_operator_with_one_argument>"
#endif
                            );

      expect_compile_fail("", errors_,
        struct test_config : di::config {
            struct dummy { };
            static auto policies(...) { return di::make_policies(dummy{}); }
        };
        int main() {
            di::make_injector<test_config>();
        }
    );
    };

    test config_not_providable = [] {
      auto errors_ = errors("constraint not satisfied",
#if defined(__MSVC__)
                            "provider<.*>::requires_<.*get,.*is_creatable>", "=.*dummy"
#else
                            "provider<.*dummy>::requires_<.*get,.*is_creatable>"
#endif
                            );

      expect_compile_fail("", errors_,
        struct test_config : di::config {
            struct dummy { };
            static auto provider(...) { return dummy{}; }
        };
        int main() {
            di::make_injector<test_config>();
        }
    );
    };

    test config_wrong_provider = [] {
      auto errors_ = errors("constraint not satisfied",
#if defined(__MSVC__)
                            "config<.*>::requires_<.*provider<.*providable_type.*(...)>.*policies<.*callable_type.*(...)>",
                            "=.*test_config"
#else
                            "config<.*test_config>::requires_<.*provider<.*providable_type.*(...)>.*policies<.*callable_type.*("
                            "..."
                            ")>"
#endif
                            );

    expect_compile_fail("", errors_,
        struct test_config : di::config {
            static auto provider() { return di::providers::heap{};
    }
    }
    ;
    int main() { di::make_injector<test_config>(); }
    );
    }
    ;

    // ---------------------------------------------------------------------------

    test mismatch_between_ctor_declaration_and_definition = [] {
      expect_compile_fail("", errors(), struct c { BOOST_DI_INJECT(c, int, double); }; c::c(int) {} int main(){});
    };

    test mismatch_between_ctor_declaration_and_definition_named = [] {
      expect_compile_fail("", errors(), struct c { BOOST_DI_INJECT(c, int, (named = int{}) double); };
                          c::c(double, int) {} int main(){});
    };

    // ---------------------------------------------------------------------------

    test create_polymorphic_type_without_binding = [] {
      auto errors_ = errors(
#if (__clang_major__ == 3) && (__clang_minor__ > 4) || defined(__GCC___) || defined(__MSVC__)
          "creatable constraint not satisfied",
#endif
          "abstract_type<.*>::is_not_bound"
#if !defined(__MSVC__)
          ,
          "create<c>()", "type is not bound, did you forget to add: 'di::bind<interface>.to<implementation>()'?"
#endif
          );

      expect_compile_fail("", errors_,
                          struct i {
                            virtual ~i() noexcept = default;
                            virtual void dummy() = 0;
                          };
                          struct c{c(i*){}}; int main() { di::make_injector().create<c>(); });
    };

    test create_polymorphic_type_without_binding_ref = [] {
      auto errors_ = errors(
#if (__clang_major__ == 3) && (__clang_minor__ > 4) || defined(__GCC___) || defined(__MSVC__)
          "creatable constraint not satisfied",
#endif
          "abstract_type<.*>::is_not_bound"
#if !defined(__MSVC__)
          ,
          "create<c>()", "type is not bound, did you forget to add: 'di::bind<interface>.to<implementation>()'?"
#endif
          );

      expect_compile_fail("", errors_,
                          struct i {
                            virtual ~i() noexcept = default;
                            virtual void dummy() = 0;
                          };
                          struct c{c(i&){}}; int main() { di::make_injector().create<c>(); });
    };

    test create_polymorphic_type_without_binding_using_multi_bindings = [] {
      auto errors_ = errors(
#if (__clang_major__ == 3) && (__clang_minor__ > 4) || defined(__GCC___) || defined(__MSVC__)
          "creatable constraint not satisfied",
#endif
          "abstract_type<.*>::is_not_bound"
#if !defined(__MSVC__)
          ,
          "create<c>()", "type is not bound, did you forget to add: 'di::bind<interface>.to<implementation>()'?"
#endif
          );

      expect_compile_fail("<include> memory <include> vector", errors_,
                          struct i {
                            virtual ~i() noexcept = default;
                            virtual void dummy() = 0;
                          };
                          struct c{c(std::vector<std::unique_ptr<i>>){}}; int main() {
                            auto injector = di::make_injector(di::bind<i* []>().to<i, i>());
                            injector.create<c>();
                          });
    };

    test create_polymorphic_type_without_binding_using_multi_bindings_named = [] {
      auto errors_ = errors(
#if (__clang_major__ == 3) && (__clang_minor__ > 4) || defined(__GCC___) || defined(__MSVC__)
          "creatable constraint not satisfied",
#endif
          "abstract_type<.*>::named<.*>::is_not_bound"
#if !defined(__MSVC__)
          ,
          "create<c>()", "type is not bound, did you forget to add: 'di::bind<interface>.named\\(name\\).to<implementation>()'?"
#endif
          );

      expect_compile_fail("<include> memory <include> set", errors_,
                          struct i {
                            virtual ~i() noexcept = default;
                            virtual void dummy() = 0;
                          };
                          struct impl
                          : i{void dummy() override{}};
                          struct c{c(std::set<std::unique_ptr<i>>){}}; int main() {
                            auto injector = di::make_injector(di::bind<i* []>().to<i, di::named<class I>>(),
                                                              di::bind<i>().to<impl>(), di::bind<i>().named<class I>());
                            injector.create<c>();
                          });
    };

    test create_polymorphic_type_without_binding_named = [] {
      auto errors_ = errors(
#if (__clang_major__ == 3) && (__clang_minor__ > 4) || defined(__GCC___) || defined(__MSVC__)
          "creatable constraint not satisfied",
#endif
          "abstract_type<.*>::named<.*>::is_not_bound"
#if !defined(__MSVC__)
          ,
          "type is not bound, did you forget to add: 'di::bind<interface>.named\\(name\\).to<implementation>()'?"
#endif
          );

      expect_compile_fail("", errors_,
                          struct i {
                            virtual ~i() noexcept = default;
                            virtual void dummy() = 0;
                          };
                          struct dummy{}; struct c{BOOST_DI_INJECT(c, (named = dummy{})i*){}};
                          int main() { di::make_injector().create<c>(); });
    };

    test exposed_not_creatable = [] {
      auto errors_ = errors(
#if (__clang_major__ == 3) && (__clang_minor__ > 4) || defined(__GCC___) || defined(__MSVC__)
          "creatable constraint not satisfied",
#endif
          "abstract_type<.*>::is_not_bound"
#if !defined(__MSVC__)
          ,
          "create<T>", "type is not bound, did you forget to add: 'di::bind<interface>.to<implementation>()'?"
#endif
          );

      expect_compile_fail("<include> memory", errors_,
                          struct i {
                            virtual ~i() noexcept = default;
                            virtual void dummy() = 0;
                          };
                          struct c{c(int, std::unique_ptr<i>){}};
                          int main() { di::injector<std::unique_ptr<i>> injector = di::make_injector(); });
    };

    test exposed_polymorphic_type_without_binding = [] {
      auto errors_ = errors(
#if (__clang_major__ == 3) && (__clang_minor__ > 4) || defined(__GCC___) || defined(__MSVC__)
          "creatable constraint not satisfied",
#endif
          "abstract_type<.*>::is_not_bound"
#if !defined(__MSVC__)
          ,
          "create<T>", "type is not bound, did you forget to add: 'di::bind<interface>.to<implementation>()'?"
#endif
          );

      expect_compile_fail("<include> memory", errors_,
                          struct i {
                            virtual ~i() noexcept = default;
                            virtual void dummy() = 0;
                          };
                          struct c { std::shared_ptr<i> i_; }; int main() {
                            di::injector<c> injector = di::make_injector();  // di::bind<i>().to<impl>()
                          });
    };

    test exposed_type_injector_conversions_fail = [] {
      auto errors_ = errors(
#if (__clang_major__ == 3) && (__clang_minor__ > 4) || defined(__GCC___) || defined(__MSVC__)
          "creatable constraint not satisfied",
#endif
          "abstract_type<.*>::is_not_bound"
#if !defined(__MSVC__)
          ,
          "create<T>", "type is not bound, did you forget to add: 'di::bind<interface>.to<implementation>()'?"
#endif
          );

      expect_compile_fail("<include> memory", errors_,
                          struct i1 {
                            virtual ~i1() noexcept = default;
                            virtual void dummy() = 0;
                          };
                          struct impl1
                          : i1{void dummy() override{}};
                          struct i2 {
                            virtual ~i2() noexcept = default;
                            virtual void dummy() = 0;
                          };
                          int main() {
                            auto module = []() -> di::injector<std::unique_ptr<i1>> {
                              return di::make_injector(di::bind<i1>().to<impl1>());
                            };
                            di::injector<std::unique_ptr<i2>> injector = di::make_injector(module());
                          });
    };

    test injector_singleton_by_copy = [] {
      auto errors_ = errors(
#if (__clang_major__ == 3) && (__clang_minor__ > 4) || defined(__GCC___) || defined(__MSVC__)
          "creatable constraint not satisfied",
#endif
          "scoped<.*>::is_not_convertible_to<.*>", "singleton", "int", "int.*\\*"
#if !defined(__MSVC__)
          ,
          "scoped object is not convertible to the requested type, did you mistake the scope: "
          "'di::bind<T>.in\\(scope\\)'?"
#endif
          );

      expect_compile_fail(
          "", errors_, struct c{c(int*){}}; int main() {
            auto injector = di::make_injector(di::bind<int>().in(di::singleton));
            injector.create<c>();
          });
    };

    test bind_external_not_convertible = [] {
      auto errors_ = errors(
#if (__clang_major__ == 3) && (__clang_minor__ > 4) || defined(__GCC___) || defined(__MSVC__)
          "creatable constraint not satisfied",
#endif
          "scoped<.*>::is_not_convertible_to<.*>", "external", "int", "int.*\\*"
#if !defined(__MSVC__)
          ,
          "external is not convertible to the requested type, verify binding: 'di::bind<T>.to\\(value\\)'?"
#endif
          );

      expect_compile_fail(
          "", errors_, struct c{c(int*){}}; int main() {
            auto injector = di::make_injector(di::bind<int>().to(42));
            injector.create<c>();
          });
    };

    test bind_external_not_referable = [] {
      auto errors_ = errors(
#if (__clang_major__ == 3) && (__clang_minor__ > 4) || defined(__GCC___) || defined(__MSVC__)
          "creatable constraint not satisfied",
#endif
          "scoped<.*>::is_not_convertible_to<.*>", "external", "int", "int.*&"
#if !defined(__MSVC__)
          ,
          "external is not convertible to the requested type, verify binding: 'di::bind<T>.to\\(value\\)'?"
#endif
          );

      expect_compile_fail(
          "", errors_, struct c{c(int&){}}; int main() {
            auto injector = di::make_injector(di::bind<int>().to(42)  // lvalue can't be converted to a reference
                                              );
            injector.create<c>();
          });
    };

    test bind_external_not_referable_named = [] {
      auto errors_ = errors(
#if (__clang_major__ == 3) && (__clang_minor__ > 4) || defined(__GCC___) || defined(__MSVC__)
          "creatable constraint not satisfied",
#endif
          "scoped<.*>::is_not_convertible_to<.*>", "external", "int", "int.*&"
#if !defined(__MSVC__)
          ,
          "external is not convertible to the requested type, verify binding: 'di::bind<T>.to\\(value\\)'?"
#endif
          );

      expect_compile_fail("", errors_, struct dummy{}; struct c{BOOST_DI_INJECT(c, (named = dummy{}) int&){}}; int main() {
        auto injector = di::make_injector(di::bind<int>().named(dummy{}).to(42)  // lvalue can't be converted to a reference
                                          );
        injector.create<c>();
      });
    };

    test policy_constructible = [] {
      auto errors_ = errors(
#if (__clang_major__ == 3) && (__clang_minor__ > 4) || defined(__GCC___) || defined(__MSVC__)
          "creatable constraint not satisfied",
#endif
#if defined(__GCC__) || defined(__CLANG__)
          "type disabled by constructible policy, added by BOOST_DI_CFG or make_injector<CONFIG>",
#endif
#if defined(__GCC__)
          "type<.*>::not_allowed_by.*int", "type<.*>::not_allowed_by.*double", "type<.*>::not_allowed_by.*float"
#else
          "type<int>::not_allowed_by", "type<double>::not_allowed_by", "type<float>::not_allowed_by"
#endif
          );

    expect_compile_fail("", errors_,
        class config : public di::config {
        public:
            static auto policies(...) noexcept {
                using namespace di::policies;
                return di::make_policies(constructible(is_bound<di::_>{}));
    }
    }
    ;

    struct c {
      c(int, double, float) {}
    };
    int main() {
      auto injector = di::make_injector<config>();
      injector.create<c>();
    }
    );
    }
    ;

    test create_error_with_call_stack = [] {
      auto errors_ = errors(
#if (__clang_major__ == 3) && (__clang_minor__ > 4) || defined(__GCC___) || defined(__MSVC__)
          "creatable constraint not satisfied",
#endif
          "abstract_type<.*>::is_not_bound",
#if defined(__GCC__)
          "creating<.*>.*c", "creating<.*>.*c1", "creating<.*>.*c2"
#elif defined(__MSVC__)
          "creating<.*>", "=.*c", "creating<.*>", "=.*c1", "creating<.*>", "=.*c2"
#else
          "creating<c>", "creating<c1>", "creating<c2>"
#endif
#if !defined(__MSVC__)
          ,
          "create<c>()", "type is not bound, did you forget to add: 'di::bind<interface>.to<implementation>()'?"
#endif
          );

      expect_compile_fail("-DBOOST_DI_CFG_DIAGNOSTICS_LEVEL=2", errors_,
                          struct i {
                            virtual ~i() noexcept = default;
                            virtual void dummy() = 0;
                          };
                          struct c2{c2(i*){}}; struct c1{c1(int, double, const c2&){}}; struct c{c(int, c1){}};
                          int main() { di::make_injector().create<c>(); });
    };

    // ---------------------------------------------------------------------------

    test ctor_inject_limit_out_of_range = [] {
      expect_compile_fail(
          "-DBOOST_DI_CFG_CTOR_LIMIT_SIZE=3",
          errors("Number of constructor arguments is out of range - see BOOST_DI_CFG_CTOR_LIMIT_SIZE"),
          struct c{BOOST_DI_INJECT(c, int, int, int, int){}};
          int main() {
            auto injector = di::make_injector();
            injector.create<c>();
          });
    };

    test ctor_limit_out_of_range = [] {
      auto errors_ = errors(
#if defined(__GCC__)
          "type<.*>::has_to_many_constructor_parameters::max<.*>.*= 3.*=.*c"
#elif defined(__MSVC__)
          "type<.*>::has_to_many_constructor_parameters::max<3>", "T=.*c"
#else
          "type<.*c>::has_to_many_constructor_parameters::max<3>"
#endif
          );

      expect_compile_fail(
          "-DBOOST_DI_CFG_CTOR_LIMIT_SIZE=3", errors_, struct c{c(int, int, int, int){}}; int main() {
            auto injector = di::make_injector();
            injector.create<c>();
          });
    };

    test injector_ctor_ambiguous = [] {
      auto errors_ = errors(
#if defined(__GCC__)
          "type<.*>::has_to_many_constructor_parameters::max<.*>.*= 10.*=.*ctor"
#elif defined(__MSVC__)
          "type<.*>::has_to_many_constructor_parameters::max<10>", "T=.*ctor"
#else
          "type<.*ctor>::has_to_many_constructor_parameters::max<10>"
#endif
          );

      expect_compile_fail("", errors_, struct ctor{ctor(int, double){} ctor(double, int){}};
                          int main() { di::make_injector().create<ctor>(); });
    };

    test ctor_number_of_args_is_not_equal = [] {
      auto errors_ = errors(
#if defined(__GCC__)
          "type<.*>::has_ambiguous_number_of_constructor_parameters::given<.*>::expected<.*>.*= 4.*= 2.*=.*c"
#elif defined(__MSVC__)
          "type<.*>::has_ambiguous_number_of_constructor_parameters::given<2>::expected<4>", "T=.*c"
#else
          "type<.*c>::has_ambiguous_number_of_constructor_parameters::given<2>::expected<4>"
#endif
          );

      expect_compile_fail(
          "", errors_,
          struct c {
            BOOST_DI_INJECT_TRAITS(int, int);  // 2
            c(int, int, int, int) {}           // 4
          };
          int main() {
            auto injector = di::make_injector();
            injector.create<c>();
          });
    };

    test named_paramater_spelling = [] {
      expect_compile_fail("", errors(), auto name = [] {}; struct c{BOOST_DI_INJECT(c, (NAMED = name) int){}}; int main(){});
    };

    test circular_dependencies_simple = [] {
      expect_compile_fail("", errors(), struct cd2; struct cd1 { cd1(cd2*){}; }; struct cd2 { cd2(cd1*){}; }; int main() {
        auto injector = di::make_injector();
        injector.create<cd1>();
      });
    };

    test circular_dependencies_complex = [] {
      expect_compile_fail("", errors(), struct cd2; struct cd5; struct cd1 { cd1(cd2*){}; }; struct cd2 { cd2(cd1*){}; };
                          struct cd3 { cd3(cd5*){}; }; struct cd4 { cd4(cd3*){}; }; struct cd5 { cd5(cd4*){}; }; int main() {
                            auto injector = di::make_injector();
                            injector.create<cd5>();
                          });
    };

    // ---------------------------------------------------------------------------

#if (__GNUC__ < 6)
    test access_injector_is_creatable = [] {
      expect_compile_fail("", errors(), int main() {
        auto injector = di::make_injector();
        using type = decltype(injector)::is_creatable<int>;
      });
    };
#endif

    test access_injector_try_create = [] {
      expect_compile_fail("", errors(), int main() {
        auto injector = di::make_injector();
        injector.try_create<int>();
      });
    };

    test access_injector_create_impl = [] {
      expect_compile_fail("", errors(), int main() {
        auto injector = di::make_injector();
        injector.create_impl(di::aux::type<int>{});
      });
    };

    test access_injector_create_succesful_impl = [] {
      expect_compile_fail("", errors(), int main() {
        auto injector = di::make_injector();
        injector.create_successful_impl(di::aux::type<int>{});
      });
    };

    test access_injector_exposed_is_creatable = [] {
      expect_compile_fail("", errors(), int main() {
        di::injector<> injector = di::make_injector();
        using type = decltype(injector)::is_creatable<int>;
      });
    };

    test access_injector_exposed_try_create = [] {
      expect_compile_fail("", errors(), int main() {
        di::injector<> injector = di::make_injector();
        injector.try_create<int>();
      });
    };

    test access_injector_exposed_create_impl = [] {
      expect_compile_fail("", errors(), int main() {
        di::injector<> injector = di::make_injector();
        injector.create_impl(di::aux::type<int>{});
      });
    };

    test access_injector_exposed_create_succesful_impl = [] {
      expect_compile_fail("", errors(), int main() {
        di::injector<> injector = di::make_injector();
        injector.create_successful_impl(di::aux::type<int>{});
      });
    };

    test access_dependency_try_create = [] {
      expect_compile_fail("", errors(), int main() { di::bind<int>().try_create<int>(); });
    };

    test access_dependency_create = [] {
      expect_compile_fail("", errors(), struct provider{}; int main() { di::bind<int>().create<int>(provider{}); });
    };

#if !defined(__MSVC__) && (__GNUC__ < 6)
    test access_dependency_is_referable = [] {
      expect_compile_fail("<include> type_traits", errors(),
                          int main() { using type = std::remove_reference_t<decltype(di::bind<int>())>::is_referable<int>; });
    };
#endif

    /// TODO
    //{
    // const auto injector = di::make_injector(
    // di::bind<int>.to(42).in(di::singleton)
    //, di::bind<double>.to(82.0)
    //);

    // std::cout << injector.create<int&>() << std::endl;

    //}

//
// Copyright (c) 2012-2016 Krzysztof Jusiak (krzysztof at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
//<-
#include <cassert>
#include <memory>
//->
#include <boost/di.hpp>

namespace di = boost::di;

template <class, class TScope>
class session_scope {
  class session_guard {
   public:
    explicit session_guard(bool& guard) : guard_(guard) { guard = true; }
    ~session_guard() { guard_ = false; }

   private:
    bool& guard_;
  };

 public:
  template <class TExpected, class TGiven>
  class scope {
    using scope_type = typename TScope::template scope<TExpected, TGiven>;

   public:
    template <class T, class TInjector>
    using is_referable = typename scope_type::template is_referable<T, TInjector>;

    template <class T, class TName, class TProvider>
    static auto try_create(const TProvider& provider) -> decltype(scope_type{}.template try_create<T, TName>(provider));

    template <class T, class TName, class TProvider>
    auto create(const TProvider& provider) {
      static std::shared_ptr<TGiven> null{nullptr};
      return get_session() ? scope_.template create<T, TName>(provider) : null;
    }

   private:
    scope_type scope_;
  };

  auto operator()() const { return session_guard{get_session()}; }

 private:
  static auto& get_session() {
    static auto is_in_session = false;
    return is_in_session;
  }
};

template <class TName, class TScope = di::scopes::singleton>
auto session(const TName&, const TScope& = {}) {
  return session_scope<TName, TScope>{};
}

//<-
struct interface1 {
  virtual ~interface1() noexcept = default;
  virtual void dummy() = 0;
};
struct implementation1 : interface1 {
  void dummy() override {}
};
//->

auto my_session = [] {};

int main() {
  {
    // clang-format off
    auto injector = di::make_injector(
      di::bind<interface1>().to<implementation1>().in(session(my_session))
    );
    // clang-format on
    assert(!injector.create<std::shared_ptr<interface1>>());

    {
      auto ms = session(my_session)();
      assert(injector.create<std::shared_ptr<interface1>>());
    }  // end of my_session

    assert(!injector.create<std::shared_ptr<interface1>>());
  }

  {
    // clang-format off
    di::injector<std::shared_ptr<interface1>> injector =
      di::make_injector(
        di::bind<interface1>().to<implementation1>().in(session(my_session))
      );
    // clang-format on
    assert(!injector.create<std::shared_ptr<interface1>>());

    {
      auto ms = session(my_session)();
      assert(injector.create<std::shared_ptr<interface1>>());
    }  // end of my_session

    assert(!injector.create<std::shared_ptr<interface1>>());
  }
}

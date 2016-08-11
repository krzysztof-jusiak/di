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

class shared_scope {
 public:
  template <class, class T>
  class scope {
   public:
    template <class T_, class>
    using is_referable = typename di::wrappers::shared<shared_scope, T>::template is_referable<T_>;

    template <class, class, class TProvider>
    static auto try_create(const TProvider& provider)
        -> decltype(di::wrappers::shared<shared_scope, T>{std::shared_ptr<T>{provider.get()}});

    template <class, class, class TProvider>
    auto create(const TProvider& provider) {
      if (!object_) {
        object_ = std::shared_ptr<T>{provider.get()};
      }
      return di::wrappers::shared<shared_scope, T>{object_};
    }

   private:
    std::shared_ptr<T> object_;
  };
};

static constexpr shared_scope shared{};

//<-
struct interface1 {
  virtual ~interface1() noexcept = default;
  virtual void dummy() = 0;
};
struct implementation1 : interface1 {
  void dummy() override {}
};
struct implementation2 : interface1 {
  void dummy() override {}
};
//->

auto name = [] {};

class example {
 public:
  BOOST_DI_INJECT(example, (named = name)std::shared_ptr<interface1> sp1, (named = name)std::shared_ptr<interface1> sp2,
                  std::shared_ptr<interface1> sp3) {
    assert(sp1 == sp2);
    assert(dynamic_cast<implementation2*>(sp1.get()));
    assert(dynamic_cast<implementation2*>(sp2.get()));
    assert(dynamic_cast<implementation1*>(sp3.get()));
  }
};

int main() {
  {
    // clang-format off
    auto injector = di::make_injector(
      di::bind<interface1>().to<implementation1>().in(shared)  // di::unique will fail (sp1 == sp2)
    , di::bind<interface1>().to<implementation2>().named(name).in(shared)
    );
    // clang-format on
    injector.create<example>();
  }

  {
    // clang-format off
    di::injector<std::shared_ptr<interface1>> injector =
      di::make_injector(
        di::bind<interface1>().to<implementation1>().in(shared)
      );
    // clang-format on

    injector.create<std::shared_ptr<interface1>>();
    // injector.create<std::unique_ptr<interface1>>(); // error, std::unique_ptr<interface1> is not exposed
  }

  {
    // clang-format off
    di::injector<BOOST_DI_EXPOSE((named = name)std::shared_ptr<interface1>), std::shared_ptr<interface1>> injector =
        di::make_injector(
          di::bind<interface1>().to<implementation1>().in(shared).named(name)
        , di::bind<interface1>().to<implementation2>().in(shared).named(name)[di::override]
        , di::bind<interface1>().to<implementation1>().in(shared)
        );
    // clang-format on

    injector.create<example>();
  }
}

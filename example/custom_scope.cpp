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

/*<define scope class>*/
class custom_scope {
 public:
  /*<define `entry`, `exit` actions>*/
  class entry {};
  class exit {};

  template <class T, class>
  class scope {
    /*<define wrapper for shared_ptr conversion>*/
    class custom_wrapper {
     public:
      custom_wrapper(const std::shared_ptr<T>& object)  // non explicit
          : object_(object) {}

      /*<<conversion operator to shared_ptr>>*/
      inline operator std::shared_ptr<T>() const noexcept { return object_; }

      std::shared_ptr<T> object_;
    };

   public:
    template <class, class>
    using is_referable = std::false_type;

    template <class, class, class TProvider>
    static custom_wrapper try_create(const TProvider&);

    /*<<create shared_ptr when in scope out of provider pointer>>*/
    template <class, class, class TProvider>
    custom_wrapper create(const TProvider& provider) const noexcept {
      return std::shared_ptr<T>{provider.get()};
    }
  };
};

struct example {
  explicit example(const std::shared_ptr<int>& sp) : sp_(sp) {}
  std::shared_ptr<int> sp_;
};

int main() {
  /*<<create injector with `int` in `custom scope`>>*/
  // clang-format off
  auto injector = di::make_injector(
    di::bind<int>().in(custom_scope{})
  );
  // clang-format on

  assert(injector.create<example>().sp_);
}

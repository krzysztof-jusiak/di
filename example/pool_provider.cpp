//
// Copyright (c) 2012-2019 Kris Jusiak (kris at jusiak dot net)
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

//<-
BOOST_DI_NAMESPACE_BEGIN namespace wrappers {
  template <class TScope, class T, class TDeleter>
  struct unique<TScope, std::unique_ptr<T, TDeleter>> {
    using scope = TScope;

    template <class I>
    inline operator I() const noexcept {
      return *object;
    }

    template <class I>
    inline operator I*() noexcept {
      return object.release();
    }

    template <class I>
    inline operator const I*() noexcept {
      return object.release();
    }

    template <class I>
    inline operator std::shared_ptr<I>() noexcept {
      return {object.release(), object.get_deleter()};
    }

    template <class I, class D>
    inline operator std::unique_ptr<I, D>() noexcept {
      return static_cast<std::unique_ptr<T, TDeleter>&&>(object);
    }

    std::unique_ptr<T, TDeleter> object;
  };
}
BOOST_DI_NAMESPACE_END

struct interface {
  virtual ~interface() noexcept = default;
};
struct implementation : interface {};

/*<example `pool allocator`>*/
struct pool_allocator {
  template <class T>
  static di::aux::owner<T*> allocate();
  static void deallocate(di::aux::owner<void*>);
};

template <class T>
di::aux::owner<T*> pool_allocator::allocate() {
  return (T*)::operator new(sizeof(T));
}

void pool_allocator::deallocate(di::aux::owner<void*> ptr) { return ::operator delete(ptr); }

struct pool_deleter {
  void operator()(di::aux::owner<void*> ptr) const noexcept { pool_allocator::deallocate(ptr); }
};
//->

/*<define `pool provider`>*/
struct pool_provider {
  template <class...>
  struct is_creatable {
    static constexpr auto value = true;
  };

  template <class T, class TInitialization, class TMemory, class... TArgs>
  auto get(const TInitialization&  // direct/uniform
           ,
           const TMemory&  // stack/heap
           ,
           TArgs&&... args) const {
    auto memory = pool_allocator::allocate<T>();
    return std::unique_ptr<T, pool_deleter>{new (memory) T(std::forward<TArgs>(args)...)};
  }
};

/*<override `di` provider configuration>*/
class config : public di::config {
 public:
  static auto provider(...) noexcept { return pool_provider{}; }
};

/*<define `example` class as usual>*/
struct example {
  explicit example(int i, std::unique_ptr<interface, pool_deleter> up, std::shared_ptr<interface> sp) {
    assert(i == 42);
    assert(dynamic_cast<implementation*>(up.get()));
    assert(dynamic_cast<implementation*>(sp.get()));
  }
};

#if defined(_MSC_VER)
int main() {}
#else
int main() {
  /*<<make injector with simple configuration>>*/
  // clang-format off
  auto injector = di::make_injector<config>(
    di::bind<int>().to(42)
  , di::bind<interface>().to<implementation>()
  );
  // clang-format on

  /*<<create `example` using configuration with `pool_provider`>>*/
  injector.create<example>();
}
#endif

#include <iostream>
#include <memory>
#include <unordered_map>
#include <string>
#include <boost/di.hpp>
#include <array>

namespace di = boost::di;

#if 0
std::array<char, 1024> buff{};
char* ptr = buff.begin();

class buffer {
 public:
  template <class TInitialization, class T, class... TArgs>
  struct is_creatable {
    static constexpr auto value = true;
  };

  template <class T, class... TArgs>
  auto get(const di::type_traits::direct&, const di::type_traits::heap&, TArgs&&... args) {
    return new T(static_cast<TArgs&&>(args)...);
  }

  template <class T, class... TArgs>
  auto get(const di::type_traits::uniform&, const di::type_traits::heap&, TArgs&&... args) {
    return new T{static_cast<TArgs&&>(args)...};
  }

  template <class T, class... TArgs>
  auto get(const di::type_traits::direct&, const di::type_traits::stack&, TArgs&&... args) const noexcept {
    return T(static_cast<TArgs&&>(args)...);
  }

  template <class T, class... TArgs>
  auto get(const di::type_traits::uniform&, const di::type_traits::stack&, TArgs&&... args) const noexcept {
    return T{static_cast<TArgs&&>(args)...};
  }


  template <class T, class... TArgs>
  auto get(const di::type_traits::direct&, const di::type_traits::buffer&, TArgs&&... args) {
    new (ptr) T(static_cast<TArgs&&>(args)...);
    char* p = ptr;
    ptr += sizeof(T);
    std::cout << (int*)ptr << (int*)p << std::endl;
    return reinterpret_cast<T*>(ptr);
  }

  template <class T, class... TArgs>
  auto get(const di::type_traits::uniform&, const di::type_traits::buffer&, TArgs&&... args) {
    new (ptr) T{static_cast<TArgs&&>(args)...};
    char* p = ptr;
    ptr += sizeof(T);
    return reinterpret_cast<T*>(p);
  }
};

class config : public di::config {
 public:
  static auto provider(...) noexcept { return buffer{}; }
};
#endif

struct interface {
  virtual ~interface() = default;
  virtual void set(int) = 0;
  virtual int get() const = 0;
};

struct impl : interface {
  void set(int i) override { this->i = i; }
  int get() const override { return this->i; }
  int i{};
};

struct example {
  example(interface& i) : i(i) {
  }

  interface& i;
};

int main() {
  {
  const auto injector = di::make_injector(
    di::bind<interface>.to<impl>()
  );

  injector.create<example>().i.set(42);

  std::cout << injector.create<example>().i.get() << std::endl;
  }

  {
  const auto injector = di::make_injector(
    di::bind<interface>.to<impl>()
  );

  std::cout << injector.create<example>().i.get() << std::endl;
  }
}

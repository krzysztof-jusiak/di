//
// Copyright (c) 2012-2019 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include <cassert>
#include <sstream>

#include "example/polymorphism/common/config.hpp"

//<-
#if defined(__cpp_concepts) && !defined(_MSC_VER)
//->

template <class T>
concept Drawable = not boost::di::aux::is_complete<T>::value or requires(T t, std::ostream& out) {
  t.draw(out);
};

/*<<Static polymorphism - concepts>>*/
template <Drawable TDrawable = class Drawable>  // requires Drawable<TDrawable>
class App {
 public:
  explicit App(const TDrawable drawable) : drawable{drawable} {}
  void draw(std::ostream& out) const { drawable.draw(out); }

 private:
  const TDrawable drawable;
};

struct Square {
  void draw(std::ostream& out) const { out << "Square"; }
};

struct Circle {
  void draw(std::ostream& out) const { out << "Circle"; }
};

int main() {
  std::stringstream str{};
  auto example = config().create<App>();
  example.draw(str);
  assert("Square" == str.str());
}

//<-
#else
int main() {}
#endif
//->

#include <iostream>
#include <boost/di.hpp>

namespace di = boost::di;

int main() {
  const auto injector = di::make_injector(
      di::bind<int>.in(di::singleton).to(42)
    , di::bind<double>.to(82.0)
      );

  std::cout << injector.create<int&>() << std::endl;
}

#include <memory>
#include <vector>
#include <iostream>
#include <cstdlib>

#include <boost/di.hpp>

namespace di = boost::di;

using namespace std;

struct ITypeInterface
{
    virtual ~ITypeInterface() { };

    virtual void say_hallo() = 0;
};

struct TypeImplementation : ITypeInterface
{
    virtual void say_hallo() override
    {
        cout << "Hallo from " << this << "!" << endl;
    }
};


int main()
{
  {
    int i = 42;
  const auto injector = di::make_injector(
      di::bind<int>.to(i)
    , di::bind<double>.to(82.0)
      );

  std::cout << injector.create<int&>() << std::endl;
  //std::cout << injector.create<int&>() << std::endl;

  }

  {
  const auto injector = di::make_injector(
      di::bind<int>.to(42)
    , di::bind<double>.to(82.0)
      );

  std::cout << injector.create<int>() << std::endl;
  //std::cout << injector.create<int&>() << std::endl;

  }

  {
  const auto injector = di::make_injector(
      di::bind<int>.to(std::make_shared<int>(42))
    , di::bind<double>.to(82.0)
      );

  std::cout << *injector.create<std::shared_ptr<int>>() << std::endl;
  //std::cout << injector.create<int&>() << std::endl;

  }

  {
  const auto injector = di::make_injector(
      di::bind<int>.in(di::singleton).to(std::make_shared<int>(42))
    , di::bind<double>.to(82.0)
      );

  std::cout << *injector.create<std::shared_ptr<int>>() << std::endl;
  //std::cout << injector.create<int&>() << std::endl;

  }

  {
  const auto injector = di::make_injector(
      di::bind<int>.in(di::singleton).to(42)
    , di::bind<double>.to(82.0)
      );

  std::cout << injector.create<int&>() << std::endl;

  }

  //{
  //const auto injector = di::make_injector(
      //di::bind<int>.to(42).in(di::singleton)
    //, di::bind<double>.to(82.0)
      //);

  //std::cout << injector.create<int&>() << std::endl;

  //}

  {
  const auto injector = di::make_injector(
      di::bind<int[]>.in(di::singleton).to({1, 2, 3})
    , di::bind<double>.to(82.0)
      );

  //std::cout << injector.create<std::shared_ptr<std::vector<int>>>()->size() << std::endl;
  std::cout << injector.create<std::vector<int>>().size() << std::endl;

  }

  {
    auto injector
            = di::make_injector(
                    boost::di::bind<ITypeInterface>().in(di::singleton).to([&](const auto& injector) -> shared_ptr<ITypeInterface>
                    {
                        return shared_ptr<ITypeInterface>(new TypeImplementation());
                    }));

    auto instance1 = injector.create<shared_ptr<ITypeInterface>>();
    instance1->say_hallo();

    auto instance2 = injector.create<shared_ptr<ITypeInterface>>();
    instance2->say_hallo();
  }

  {
    auto injector
            = di::make_injector(
                    boost::di::bind<ITypeInterface>().to([&](const auto& injector) -> shared_ptr<ITypeInterface>
                    {
                        return shared_ptr<ITypeInterface>(new TypeImplementation());
                    }));

    auto instance1 = injector.create<shared_ptr<ITypeInterface>>();
    instance1->say_hallo();

    auto instance2 = injector.create<shared_ptr<ITypeInterface>>();
    instance2->say_hallo();
  }
  {
    auto injector
            = di::make_injector(
                    boost::di::bind<ITypeInterface>().in(di::unique).to([&](const auto& injector) -> shared_ptr<ITypeInterface>
                    {
                        return shared_ptr<ITypeInterface>(new TypeImplementation());
                    //return std::make_shared<TypeImplementation>();
                    }));

    auto instance1 = injector.create<shared_ptr<ITypeInterface>>();
    instance1->say_hallo();

    auto instance2 = injector.create<shared_ptr<ITypeInterface>>();
    instance2->say_hallo();
  }
  std::cout << "here" << std::endl;
 {
    auto injector
            = di::make_injector(
                    boost::di::bind<ITypeInterface>().in(di::singleton).to([&](const auto& injector) -> std::unique_ptr<ITypeInterface>
                    {
                    return std::make_unique<TypeImplementation>();
                    }));

    auto&& instance1 = injector.create<ITypeInterface&>();
    instance1.say_hallo();

    auto&& instance2 = injector.create<ITypeInterface&>();
    instance2.say_hallo();
  }
}

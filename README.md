
BabyDI
===================

A stupid-simple, header-only, zero-dependency DI framework for C++17. What you see is what you get, nothing fancy, nothing crazy robust.

Example:
```c++17
#include <iostream>

#include "BabyDI.hpp"

// Define a simple interface..
struct IAnimal {
  virtual void Speak() const = 0;
};

// Define a simple implementation of the interface..
struct Dog : public IAnimal {
  void Speak() const override {
    std::cout << "Woof!" << std::endl;
  }
};

// Define a basic class which depends on IAnimal
struct BasicDependent {
  // Inject with alias, or alternatively use INJECT(IAnimal) for no alias
  // (Expands to a static inline variable..)
  INJECT(IAnimal, Animal);

  void CallImplementation() {
    Animal->Speak(); // Call the implementation
  }
};

int main(int argc, char* argv[]) {
  // Provide IAnimal
  PROVIDE(IAnimal, new Dog())

  // Will fire the callback if not everything has been provided
  // Alternatively, calling this with no argument will use this same default callback
  BabyDI::AssertAllProvided([](auto& interfaceNames) {
    std::cerr << "BabyDI: Injections not provided:" << std::endl;

    for (const auto& interfaceName : interfaceNames) {
      std::cerr << "  " << interfaceName << std::endl;
    }

    std::terminate();
  });

  BasicDependent dependent;
  dependent.CallImplementation();

  return 0;
}
```

[Building Example](example/README.md)
[Running Tests](tests/README.md)

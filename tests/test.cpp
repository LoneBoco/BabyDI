#include "acutest.h"
#include "BabyDI.hpp"

struct IAnimal {
  virtual const char* Speak() const = 0;
};

struct Dog : public IAnimal {
  const char* Speak() const override {
    return "Woof!";
  }
};

struct Cat : public IAnimal {
  const char* Speak() const override {
    return "Meow!";
  }
};

struct TestDependent {
  INJECT(IAnimal, Animal)

  const char* CallImplementation() const {
    return Animal->Speak();
  }
};

void test_provided() {
  PROVIDE(IAnimal, new Dog());
  bool providedAll = true;

  BabyDI::AssertAllProvided([&](auto& missing) {
    providedAll = false;
  });

  TEST_CHECK(providedAll);
}

void test_unprovided() {
  const char* missingName;
  uint32_t numMissing = 0;

  BabyDI::AssertAllProvided([&](auto& missing) {
    missingName = missing[0];

    numMissing = missing.size();
  });

  TEST_CHECK(numMissing == 1);
  TEST_CHECK(strcmp(missingName, "IAnimal") == 0);
}

void test_injection() {
  PROVIDE(IAnimal, new Dog());

  TestDependent dependent;
  dependent.CallImplementation();
}

void test_dog_inject() {
  PROVIDE(IAnimal, new Dog());

  TestDependent dependent;
  TEST_CHECK(strcmp(dependent.CallImplementation(), "Woof!") == 0);
}

void test_cat_inject() {
  PROVIDE(IAnimal, new Cat());

  TestDependent dependent;
  TEST_CHECK(strcmp(dependent.CallImplementation(), "Meow!") == 0);
}

void test_manual_get() {
  PROVIDE(IAnimal, new Cat());

  auto animal = BabyDI::Get<IAnimal>();
  TEST_CHECK(strcmp(animal->Speak(), "Meow!") == 0);
}

void test_failed_get() {
  auto animal = BabyDI::Get<IAnimal>();
  TEST_CHECK(animal == nullptr);
}

TEST_LIST = {
  { "provided",   test_provided   },
  { "unprovided", test_unprovided },
  { "injection",  test_injection  },
  { "dog_inject", test_dog_inject },
  { "cat_inject", test_cat_inject },
  { "manual_get", test_manual_get },
  { "failed_get", test_failed_get },
  { 0 }
};

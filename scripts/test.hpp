#pragma once

namespace Test {

  class SomeClass {
  public:
    SomeClass() = default;
    ~SomeClass() = default;

    int getFoo() const { return foo_; }

    void someMethod();

  private:
    int foo_{0};
  };

}

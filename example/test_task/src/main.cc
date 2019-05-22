#include <framework.h>

namespace engine {

namespace example {

class testFramework : public engine::framework {
private:
protected:
  virtual bool initialize() { return true; }

  virtual bool loop() { return true; }

  virtual void finalize() {}
};

static testFramework app;

} // namespace example
} // namespace engine

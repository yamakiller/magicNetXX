#include <framework.h>
#include <commandLineOption.h>
#include <string>

namespace wolf
{

namespace example
{

class testFramework : public wolf::framework
{
private:
protected:
  virtual bool initialize(const commandLineOption *opt)
  {
    std::string p = ((commandLineOption *)opt)->getOption("p");
    fprintf(stderr, "aaaa %s\n", p.c_str());
    return true;
  }

  virtual bool loop() { return true; }

  virtual void finalize() {}
};

static testFramework app;

} // namespace example
} // namespace wolf

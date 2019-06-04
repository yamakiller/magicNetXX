#include "api.h"
#include <Python.h>

using namespace wolf;
using namespace wolf::module;
using namespace wolf::api;
using namespace wolf::util;

class lnpy : public mobject {
public:
  lnpy();
  virtual ~lnpy();

protected:
  int32_t onLaunch(void *parm) {
    //运行指定的python 脚本格式如 脚本名 传入参数
    // python 脚本采用固定格式
    return 0;
  }
};

WOLF_C_API void *lnpy_create() {
  lnpy *ptr = new lnpy();
  assert(ptr);
  return (void *)ptr;
}

WOLF_C_API void lnpy_signal(void *inst, int signal) {}

WOLF_C_API void lnpy_release(void *inst) {
  lnpy *ptr = (lnpy *)inst;
  delete ptr;
}
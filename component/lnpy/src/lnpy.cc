#include <Python.h>
#include <api.h>
#include <string>
#include <vector>

using namespace wolf;
using namespace wolf::module;
using namespace wolf::api;
using namespace wolf::util;

class lnpy : public mobject {
public:
  lnpy() {}

  virtual ~lnpy() {}

protected:
  int32_t onLaunch(void *parm) {
    if (!param) {
      LOCAL_LOG_ERROR("Please enter a script name");
      return -1;
    }

    std::string strParam = (const char *)parm;
    if (strParam.empty() || strParam.compare("") == 0) {
      LOCAL_LOG_ERROR("Please enter a script name");
      return -1;
    }

    std::vector<std::string> parmArray = stringUtil::split(strParam, " ");
    if (parmArray.size() == 0) {
      parmArray.push_back(strParam);
    }

    m_pModule = PyImport_ImportModule(parmArray[0].c_str());
    if (!m_pModule) {
      LOCAL_LOG_ERROR("Python get {} module failed", parmArray[0].c_str());
      return -1;
    }

    PyObject *pStartFunc = PyObject_GetAttrString(m_pModule, "onStart");
    if (!pStartFunc || !PyCallable_Check(pStartFunc)) {
      LOCAL_LOG_ERROR("Python get {} module undefined onStart function: "
                      "[format onStart(string)]",
                      parmArray[0].c_str());
      return -1;
    }

    PyObject *pStartArgs = PyTuple_New(1);
    PyTuple_SetItem(pStartArgs, 0, Py_BuildValue("s", parmArray[1].c_str()));

    PyObject *pRet = PyObject_CallObject(pStartFunc, pStartArgs);
    if (!pRet) {
      LOCAL_LOG_ERROR("Python Run onStart Fail");
      return -1;
    }
  }

private:
  PyObject *m_pModule;
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
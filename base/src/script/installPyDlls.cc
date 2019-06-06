#include "installPyDlls.h"
#include "base.h"

namespace wolf {
namespace script {
bool installPyDlls(void) {
  Py_Initialize();
  if (!Py_IsInitialized()) {
    SYSLOG_ERROR(0, "Python Init fail.");
    exit(0);
  }

  PyEval_InitThreads();
  int nInit = PyEval_ThreadsInitialized();
  if (!nInit) {
    SYSLOG_ERROR(0, "Python does not have thread support enabled");
    exit(0);
  }

  PyEval_SaveThread();

  std::string scriptDir = "sys.path.append('";
  scriptDir += INST(coroutineOptions, getString, "python_path", "./pyscript");
  scriptDir += "')";
  PyRun_SimpleString("import sys");
  PyRun_SimpleString(scriptDir.c_str());

  return true;
}

bool uninstallPyDlls(void) {
  PyGILState_Ensure();
  Py_Finalize();
  return true;
}

} // namespace script
} // namespace wolf
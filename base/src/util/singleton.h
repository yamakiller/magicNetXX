#ifndef WOLF_UTIL_SINGLETON_H
#define WOLF_UTIL_SINGLETON_H

#include "platform.h"
#include <stdlib.h>

NS_CC_U_BEGIN

template <class T> class singleton {
public:
  virtual ~singleton() {
    if (m_inst) {
      delete m_inst;
      m_inst = NULL;
    }
  }

  static T *instance() {
    if (m_inst)
      return m_inst;
    m_inst = new T();
    return m_inst;
  }

protected:
  singleton() {}

private:
  static T *m_inst;
};

template <class T> T *singleton<T>::m_inst = NULL;

NS_CC_U_END

#endif
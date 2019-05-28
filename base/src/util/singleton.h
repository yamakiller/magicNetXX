#ifndef CIS_ENGINE_UTIL_SINGLETON_H
#define CIS_ENGINE_UTIL_SINGLETON_H

#include <stdlib.h>

namespace wolf
{
namespace util
{
template <class T>
class singleton
{
public:
  virtual ~singleton()
  {
    if (m_inst)
    {
      delete m_inst;
      m_inst = NULL;
    }
  }

  static T *instance()
  {
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

template <class T>
T *singleton<T>::m_inst = NULL;

} // namespace util
} // namespace wolf

#endif
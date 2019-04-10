#ifndef CIS_ENGINE_UEXCEPTION_H
#define CIS_ENGINE_UEXCEPTION_H

#include <string>

using namespace std;

namespace cis
{
class uexception
{
public:
    uexception(){};
    uexception(const char *err);

    inline const char *getMessage() { return m_strError.c_str(); }

private:
    string m_strError;
};
} // namespace cis

#endif

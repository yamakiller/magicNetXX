#ifndef CIS_ENGINE_MSTRING_H
#define CIS_ENGINE_MSTRING_H

#include "base.h"

namespace engine {
class mstring {
public:
  inline static char *strdup(const char *str) {
    size_t strsz = strlen(str);
    char *dstr = (char *)en_malloc(strsz + 1);
    memcpy(dstr, str, strsz);
    dstr[strsz] = '\0';
    return dstr;
  }

  inline static void strdel(char *str) { en_free(str); }
};
} // namespace engine

#endif
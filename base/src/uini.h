#ifndef CIS_ENGINE_UINI_H
#define CIS_ENGINE_UINI_H

#include <map>
#include <string>
#include <vector>

#define UINI_BUF_SIZE 1024

using namespace std;
namespace cis
{

class uini
{
public:
  uini();
  ~uini() { local_release(); }

public:
  typedef map<string, string>::iterator iterator;

  iterator begin()
  {
    return m_mapSections.begin();
  }

  iterator end()
  {
    return m_mapSections.end();
  }

public:
  int load(const string &filename);

  string get_string_value(const string &key, const string default_v);
  int get_int_value(const string &key, const int default_v);
  float get_float_value(const string &key, const float default_v);
  double get_double_value(const string &key, double default_v);
  bool get_boolean_value(const string &key, bool default_v);

private:
  int local_read_line(string &str, FILE *fp);
  string local_get_value(const string &key);
  bool local_iscomment(const string &str);
  bool local_parse(const string &content, string &key, string &value, char c = '=');
  void local_release();

  static void __trimleft__(string &str, char c = ' ');
  static void __trimright__(string &str, char c = ' ');
  static void __trim__(string &str);

private:
  map<string, string> m_mapSections;
  vector<string> m_vecFlags;
  string m_strName;
};

} // namespace cis

#endif
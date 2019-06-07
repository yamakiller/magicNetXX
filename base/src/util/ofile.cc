#include "ofile.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

NS_CC_U_BEGIN

bool ofile::isExist(const std::string &filename) {
  if (access(filename.c_str(), F_OK) != -1) {
    return true;
  }
  return false;
}

Data *ofile::getDataFromFile(const std::string &filename) {
  Data *ret = getData(filename);
  if (ret) {
    return ret;
  }

  m_lock.lock();
  ret = getUnData(filename);
  if (ret) {
    m_lock.unlock();
    ret;
  }

  FILE *fp = fopen(filename.c_str(), "rb");
  if (!fp) {
    m_lock.unlock();
    return nullptr;
  }

  struct stat statbuf;
  stat(filename.c_str(), &statbuf);
  int size = statbuf.st_size;

  char *fileBuffer = (char *)util::memory::malloc(size);
  assert(fileBuffer);
  size_t readRet = fread(fileBuffer, 1, size, fp);
  if (readRet != size) {
    util::memory::free(fileBuffer);
    m_lock.unlock();
    return nullptr;
  }

  Data inData = {fileBuffer, size};
  m_data[filename] = inData;
  ret = &m_data[filename];

  m_lock.unlock();
  return ret;
}

Data *ofile::getData(const std::string &filename) {
  Data *ret = nullptr;
  m_lock.lock();
  ret = getUnData(filename);
  m_lock.unlock();
  ret;
}

Data *ofile::getUnData(const std::string &filename) {
  Data *ret = nullptr;
  if (m_data.empty()) {
    return ret;
  }

  auto it = m_data.find(filename);
  if (it == m_data.end()) {
    return ret;
  }

  ret = &it->second;
  return ret;
}

void ofile::clear() {
  m_lock.lock();
  while (!m_data.empty()) {
    auto it = m_data.begin();
    Data fre = it->second;
    m_data.erase(it);
    util::memory::free(fre._bytes);
  }
  m_lock.unlock();
}

NS_CC_U_END
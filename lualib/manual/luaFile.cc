#include "luaFile.h"
#include <sys/stat.h>

NS_CC_BEGIN

fileData *luaFile::getDataFromFile(const std::string &filename) {
  fileData *ret = getData(filename);
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

  fileData inData = {fileBuffer, size};
  m_data[filename] = inData;
  ret = &m_data[filename];

  m_lock.unlock();
  return ret;
}

fileData *luaFile::getData(const std::string &filename) {
  fileData *ret = nullptr;
  m_lock.lock();
  ret = getUnData(filename);
  m_lock.unlock();
  ret;
}

fileData *luaFile::getUnData(const std::string &filename) {
  fileData *ret = nullptr;
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

void luaFile::clear() {
  m_lock.lock();
  if (!m_data.empty()) {
  }
  m_lock.unlock();
}

NS_CC_END
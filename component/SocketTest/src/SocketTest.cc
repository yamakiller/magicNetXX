#include "api.h"
#include <boost/any.hpp>
#include <memory>
#include <unordered_map>

using namespace wolf;
using namespace wolf::module;
using namespace wolf::api;
using namespace wolf::util;

#define TEST_MAX_BUFFER 2048

class clientTest : public mobject {
public:
  int32_t _sock;
  char _addr[64];
  char _buffer[TEST_MAX_BUFFER];
  int32_t _size;
  int32_t _auto;

public:
  clientTest() = default;
  ~clientTest() = default;
};

class SocketTest : public actorComponent {
public:
  SocketTest();
  virtual ~SocketTest();

protected:
  void onLaunch() {
    const char *addr = "127.0.0.1";
    const int port = 29810;
    socketApi::doRequire(this);
    int32_t sock = socketApi::doListen(getActorPtr(), addr, port, 1024);
    if (sock == -1) {
      LOCAL_LOG_ERROR("监听失败:[{}:{}]", addr, port);
      return;
    }

    socketApi::doStart(getActorPtr(), sock,
                       std::bind(&SocketTest::onAccept, this,
                                 std::placeholders::_1, std::placeholders::_2));

    LOCAL_LOG_INFO("监听成功:[{}:{}]", addr, port);
  }

  void onAccept(int32_t clientId, const char *addr) {
    LOCAL_LOG_INFO("accept socket:{}[{}]", clientId, addr);
    std::shared_ptr<clientTest> p(new clientTest());
    p->_sock = clientId;
    p->_size = 0;
    p->_auto = 0;
    strcpy(p->_addr, addr);
    p->_addr[strlen(addr)] = '\0';

    boost::any ret = socketApi::doStart(getActorPtr(), clientId);
    if (ret.type() == typeid(int32_t) || ret.type() == typeid(int)) {
      if (boost::any_cast<int>(ret) == -1) {
        LOCAL_LOG_ERROR("client create fail:{}[{}]", clientId, addr);
        return;
      }
    } else {
      LOCAL_LOG_ERROR("client create fail:{},error:{}", clientId,
                      boost::any_cast<std::string>(ret).c_str());
      return;
    }

    socketApi::doSetLimit(clientId, TEST_MAX_BUFFER * 2);
    socketApi::doSetNodelay(clientId);

    char *sendinf = (char *)memory::malloc(8);
    strcpy(sendinf, "accept");
    sendinf[6] = '\0';
    if (socketApi::doSend(clientId, sendinf, 7) != 0) {
      LOCAL_LOG_ERROR("client send accept info fail:{}[{}]", clientId, addr);
      return;
    }

    addClient(clientId, p);

    //最佳选择交给另外，ACTOR来处理，需要提供套接字同步锁用协程代替
    LOCAL_LOG_INFO("等待读取数据...");
    for (;;) {
      ret = socketApi::doRead(clientId, p->_buffer + p->_size,
                              TEST_MAX_BUFFER - p->_size);
      int tmp = boost::any_cast<int>(ret);
      if (tmp == -1) {
        removeClient(clientId);
        LOCAL_LOG_INFO("连接被断开:{}[{}]", clientId, addr);
        break;
      }

      p->_size += tmp;
      if (p->_size >= TEST_MAX_BUFFER) {
        p->_size = 0;
        LOCAL_LOG_INFO("数据缓冲区已满，清除");
      }

      LOCAL_LOG_INFO("读取数据:{}", tmp);
    }
  }

private:
  std::shared_ptr<clientTest> getClient(int32_t id) {
    std::unique_lock<util::spinlock> lock(m_lock);
    if (m_clients.empty()) {
      return nullptr;
    }

    auto it = m_clients.find(id);
    if (it == m_clients.end()) {
      return nullptr;
    }

    return m_clients[id];
  }
  void addClient(int32_t id, std::shared_ptr<clientTest> ptr) {
    std::unique_lock<util::spinlock> lock(m_lock);
    m_clients[id] = ptr;
  }

  void removeClient(int32_t id) {
    std::unique_lock<util::spinlock> lock(m_lock);
    if (m_clients.empty()) {
      return;
    }

    auto it = m_clients.find(id);
    if (it == m_clients.end()) {
      return;
    }
    m_clients.erase(it);
  }

private:
  std::unordered_map<int32_t, std::shared_ptr<clientTest>> m_clients;
  util::spinlock m_lock;
};
#ifndef CIS_ENGINE_DEQUE2_H
#define CIS_ENGINE_DEQUE2_H

#include <assert.h>
#include <condition_variable>
#include <mutex>

#include "shared_ptr.h"
#include "spinlock.h"

namespace engine
{

struct invNode
{
    invNode *_prev;
    invNode *_next;
};

template <typename T>
class invList
{
    static_assert((std::is_base_of<invNode, T>::value), "T must be baseof dequeNode2");

public:
    struct iterator
    {
        T *_ptr;
        T *_prev;
        T *_next;

        iterator() : _ptr(nullptr), _prev(nullptr), _next(nullptr) {}
        iterator(T *p) { reset(p); }
        void reset(T *p)
        {
            _ptr = p;
            _next = _ptr ? (T *)_ptr->_next : nullptr;
            _prev = _ptr ? (T *)_ptr->_prev : nullptr;
        }

        friend bool operator==(iterator const &lhs, iterator const &rhs)
        {
            return lhs._ptr == rhs._ptr;
        }
        friend bool operator!=(iterator const &lhs, iterator const &rhs)
        {
            return !(lhs._ptr == rhs._ptr);
        }

        iterator &operator++()
        {
            reset(_next);
            return *this;
        }
        iterator operator++(int)
        {
            iterator ret = *this;
            ++(*this);
            return ret;
        }
        iterator &operator--()
        {
            reset(_prev);
            return *this;
        }
        iterator operator--(int)
        {
            iterator ret = *this;
            --(*this);
            return ret;
        }
        T &operator*() { return *(T *)_ptr; }
        T *operator->() { return (T *)_ptr; }
    };

public:
    invList() : m_head(nullptr), m_tail(nullptr), m_count(0) {}

    invList(T *h, T *t, size_t count)
        : m_head((invNode *)h), m_tail((invNode *)t), m_count(count) {}

    invList(invList const &) = delete;
    invList &operator=(invList const &) = delete;

    invList(invList<T> &&other)
    {
        m_head = other.m_head;
        m_tail = other.m_tail;
        m_count = other.m_count;
        other.zero();
    }

    ~invList()
    {
        assert(m_count == 0);
    }

    invList &operator=(invList<T> &&other)
    {
        clear();
        m_head = other.m_head;
        m_tail = other.m_tail;
        m_count = other.m_count;
        other.zero();
        return *this;
    }

    iterator begin() { return iterator{m_head}; }
    iterator end() { return iterator(); }
    bool empty() const { return m_head == nullptr; }

    iterator erase(iterator it)
    {
        T *ptr = (it++)._ptr;
        erase(ptr);
        return it;
    }
    void erase(void *param)
    {
        invNode *ptr = dynamic_cast<invNode *>(param);
        assert(ptr);
        if (ptr->_prev)
        {
            ptr->_prev->_next = ptr->_next;
        }
        else
        {
            m_head = m_head->_next;
        }

        if (ptr->_next)
        {
            ptr->_next->_prev = ptr->_prev;
        }
        else
        {
            m_tail = m_tail->_prev;
        }

        ptr->_prev = ptr->_next = nullptr;
        --m_count;

        decrementRef(dynamic_cast<struct SharedRefObject *>(ptr));
    }

    void append(invList<T> &&other)
    {
        if (other.empty())
            return;

        if (empty())
        {
            *this = std::move(other);
            return;
        }

        m_tail->_next = other.m_head;
        if (other.m_head)
        {
            other.m_head->_prev = m_tail;
        }

        if (other.m_tail)
        {
            m_tail = other.m_tail;
        }
        m_count += other.m_count;
        other.zero();
    }

    invList<T> cut(size_t n)
    {
        if (empty())
        {
            return invList<T>();
        }

        if (n >= size())
        {
            invList<T> o(std::move(*this));
            return o;
        }

        if (n == 0)
        {
            return invList<T>();
        }

        invList<T> o;
        auto pos = m_head;
        for (size_t i = 1; i < n; ++i)
            pos = pos->_next;
        o.m_head = m_head;
        o.m_tail = pos;
        o.m_count = n;

        m_count -= n;
        m_head = pos->_next;

        assert(pos->_next == m_head);
        assert(m_head->_prev == this);

        pos->_next = nullptr;
        m_head->_prev = nullptr;

        return o;
    }

    size_t size() const
    {
        return m_count;
    }
    void clear()
    {
        auto it = begin();
        while (it != end())
            it = erase(it);
        zero();
    }

    void zero()
    {
        m_head = m_tail = nullptr;
        m_count = 0;
    }

    invNode *head() { return m_head; }
    invNode *tail() { return m_tail; }

public:
    invNode *m_head;
    invNode *m_tail;

    size_t m_count;
};

template <typename T, bool threadSafe>
class invDeque final
{
public:
    typedef typename std::conditional<threadSafe, spinlock, unspinlock>::type
        lock_handle;
    typedef typename std::conditional<threadSafe, std::lock_guard<spinlock>,
                                      unslk_lock_guard>::type locked_guard_hd;

public:
    invDeque()
    {
        m_head = m_tail = nullptr;
        m_count = 0;
    }

    ~invDeque()
    {
        locked_guard_hd lcked(m_lock);
        while (m_tail)
        {
            invNode *prev = m_tail->_prev;
            decrementRef(m_tail);
            m_tail = prev;
        }
        m_head = nullptr;
    }

    void pushUnlock(T *element)
    {
        invNode *newNode = static_cast<invNode *>(element);

        fprintf(stderr, "gggg:%p\n", newNode);
        if (m_head == nullptr)
        {
            m_head = m_tail = newNode;
            newNode->_prev = newNode->_next = NULL;
        }
        else
        {
            m_tail->_next = newNode;
            newNode->_prev = m_tail;
            newNode->_next = nullptr;
            m_tail = newNode;
        }

        incrementRef(element);
        ++m_count;
    }

    void pushUnlock(const invList<T> &s)
    {
        if (s.empty())
        {
            return;
        }

        if (empty())
        {
            m_head = s.head();
            m_tail = s.tail();
            m_count = s.size();
            s.zero();
            return;
        }

        m_tail->_next = s.head();
        s.head()->_prev = m_tail;
        m_tail = s.tail();
        m_count += s.size();
        s.zero();
    }

    void push(T *value)
    {
        locked_guard_hd lcked(m_lock);
        fprintf(stderr, "push %p\n", value);
        pushUnlock(value);
    }

    void push(const invList<T> &s)
    {
        locked_guard_hd lcked(m_lock);
        pushUnlock(s);
    }

    bool front(T **outValue)
    {
        locked_guard_hd lcked(m_lock);
        if (m_head == nullptr)
        {
            *outValue = nullptr;
            return false;
        }

        *outValue = (T *)m_head;
        return true;
    }

    T *popUnlock()
    {
        if (m_head == NULL)
        {
            return nullptr;
        }

        invNode *tmp = nullptr;
        if (m_head == m_tail)
        {
            tmp = m_head;
            m_head = m_tail = nullptr;
        }
        else
        {
            tmp = m_head;
            m_head = m_head->_next;
            m_head->_prev = nullptr;
            tmp->_prev = tmp->_next = nullptr;
        }

        --m_count;
        return (T *)tmp;
    }

    T *pop()
    {
        locked_guard_hd lcked(m_lock);
        return popUnlock();
    }

    T *popBackUnlock()
    {
        if (m_tail == nullptr)
        {
            return nullptr;
        }

        invNode *tmp = m_tail;
        m_tail = m_tail->_prev;
        m_tail->_next = nullptr;
        tmp->_next = tmp->_prev = nullptr;

        --m_count;
        return (T *)tmp;
    }

    invList<T> popBackAllUnlock()
    {
        if (empty())
        {
            return invList<T>();
        }

        invList<T> o(m_head, m_tail, size);
        m_head = m_tail = nullptr;
        m_count = 0;
        return o;
    }

    T *popBack()
    {
        locked_guard_hd lcked(m_lock);
        return popBackUnlock();
    }

    invList<T> popBackAll()
    {
        locked_guard_hd lcked(m_lock);
        return popBackAllUnlock();
    }

    size_t size()
    {
        return m_count;
    }

    bool empty()
    {
        locked_guard_hd lcked(m_lock);
        if (m_head == nullptr)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    lock_handle &lockRef()
    {
        return m_lock;
    }

    void assertLink()
    {
        //---调试用---
        if (m_head == nullptr)
        {
            return;
        }

        assert(!m_head->_prev);
        assert(m_tail);
        assert(!m_tail->_next);

        int n = 0;
        invNode *pos = m_tail;
        for (; pos != m_head; pos = pos->_prev, ++n)
        {
            assert(!!pos->_prev);
        }
        assert(pos == m_head);
    }

private:
    invNode *m_head;
    invNode *m_tail;
    lock_handle m_lock;

    volatile size_t m_count;
};
} // namespace engine

#endif

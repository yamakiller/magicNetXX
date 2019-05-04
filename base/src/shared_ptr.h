#ifndef CIS_ENGINE_REFOBJECT_H
#define CIS_ENGINE_REFOBJECT_H

#include "base.h"
#include <string>

namespace engine
{

struct SharedRefObject;

struct Releaser
{
    typedef void (*func_t)(SharedRefObject *ptr, void *arg);
    func_t _func;
    void *_arg;

    Releaser() : _func(nullptr), _arg(nullptr) {}
    Releaser(func_t func, void *arg) : _func(func), _arg(arg) {}

    inline void operator(SharedRefObject *ptr);
};

inline void Releaser::operator()(SharedRefObject *ptr)
{
    if (_func)
        _func(ptr, _arg);
    else
        delete ptr;
}

struct SharedRefObject
{
    atomic_t<long> _reference;
    Releaser _releaser;

    SharedRefObject() : _reference{1}
    {
    }

    ~SharedRefObject() {}

    void incrementRef()
    {
        ++*reference_;
    }

    virtual bool decrementRef()
    {
        if (--*_reference == 0)
        {
            _releaser(this);
            return true;
        }
        return false;
    }

    long useCount()
    {
        return *_reference;
    }

    void setReleaser(Releaser rd)
    {
        _releaser = rd;
    }
};

void incrementRef(void *p)
{
    struct SharedRefObject *object = dynamic_cast<struct SharedRefObject *>(p);
    if (p == nullptr)
    {
        return;
    }

    object->incrementRef();
}

bool decrementRef(void *p)
{
    struct SharedRefObject *object = dynamic_cast<struct SharedRefObject *>(p);
    if (p == nullptr)
    {
        return;
    }
    return object->decrementRef();
}

// ID
/*template <typename T>
struct IdCounter
{
    IdCounter() { id_ = ++counter(); }
    IdCounter(IdCounter const &) { id_ = ++counter(); }
    IdCounter(IdCounter &&) { id_ = ++counter(); }

    long getId() const
    {
        return id_;
    }

private:
    static atomic_t<long> &counter()
    {
        static atomic_t<long> c;
        return c;
    }

    long id_;
};*/

///////////////////////////////////////

// 创建协程的源码文件位置
/*struct SourceLocation
{
    const char *file_ = nullptr;
    int lineno_ = 0;

    void Init(const char *file, int lineno)
    {
        file_ = file, lineno_ = lineno;
    }

    friend bool operator<(SourceLocation const &lhs, SourceLocation const &rhs)
    {
        if (lhs.lineno_ != rhs.lineno_)
            return lhs.lineno_ < rhs.lineno_;

        if (lhs.file_ == rhs.file_)
            return false;

        if (lhs.file_ == nullptr)
            return true;

        if (rhs.file_ == nullptr)
            return false;

        return strcmp(lhs.file_, rhs.file_) == -1;
    }

    std::string ToString() const
    {
        std::string s("{file:");
        if (file_)
            s += file_;
        s += ", line:";
        s += std::to_string(lineno_) + "}";
        return s;
    }
};*/
} // namespace engine

#endif
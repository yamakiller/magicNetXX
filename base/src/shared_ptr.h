#ifndef CIS_ENGINE_REFOBJECT_H
#define CIS_ENGINE_REFOBJECT_H

#include "base.h"
#include <string>

namespace engine
{

struct SharedRefObject;

struct Releaser
{
    typedef void (*func_t)(struct SharedRefObject *ptr, void *arg);
    func_t _func;
    void *_arg;

    Releaser() : _func(nullptr), _arg(nullptr) {}
    Releaser(func_t func, void *arg) : _func(func), _arg(arg) {}

    inline void operator()(struct SharedRefObject *ptr);
};

struct SharedRefObject
{
    atomic_t<long> _reference;
    Releaser _releaser;

    SharedRefObject() : _reference{1}
    {
    }

    virtual ~SharedRefObject() {}

    void incrementRef()
    {
        fprintf(stderr, "inc 1-1:%d\n", (long)_reference);
        ++_reference;
    }

    bool decrementRef()
    {
        fprintf(stderr, "dec 1-1:%d,%p\n", (long)_reference, this);
        if (--_reference == 0)
        {
            fprintf(stderr, "dec 1-2\n");
            _releaser(this);
            fprintf(stderr, "dec 1-3\n");
            return true;
        }
        fprintf(stderr, "dec 1-4:%d\n", (long)_reference);
        return false;
    }

    long useCount()
    {
        return _reference;
    }

    void setReleaser(Releaser rd)
    {
        _releaser = rd;
    }

    SharedRefObject(SharedRefObject const &) = delete;
    SharedRefObject &operator=(SharedRefObject const &) = delete;
};

inline void
Releaser::operator()(struct SharedRefObject *ptr)
{
    if (_func)
        _func(ptr, _arg);
    else
        delete ptr;
}

template <typename T>
typename std::enable_if<std::is_base_of<SharedRefObject, T>::value>::type
incrementRef(T *ptr)
{
    ptr->incrementRef();
}
template <typename T>
typename std::enable_if<!std::is_base_of<SharedRefObject, T>::value>::type
incrementRef(T *ptr)
{
}
template <typename T>
typename std::enable_if<std::is_base_of<SharedRefObject, T>::value>::type
decrementRef(T *ptr)
{
    ptr->decrementRef();
}
template <typename T>
typename std::enable_if<!std::is_base_of<SharedRefObject, T>::value>::type
decrementRef(T *ptr)
{
    fprintf(stderr, "null\n");
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
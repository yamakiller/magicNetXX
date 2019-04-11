#ifndef CIS_ENGINE_UNONCOPYABLE_H
#define CIS_ENGINE_UNONCOPYABLE_H

namespace cis
{
struct unoncopyable
{
public:
    unoncopyable() = default;

private:
    unoncopyable &operator=(const unoncopyable &) = delete;
    unoncopyable(const unoncopyable &) = delete;
};
} // namespace cis
#endif

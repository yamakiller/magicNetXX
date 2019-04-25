#ifndef CIS_ENGINE_FCONTEXT_H
#define CIS_ENGINE_FCONTEXT_H

#include <boost/context/detail/fcontext.hpp>
#include <functional>

#define STACK_MALLOC(sz) engine::memory::malloc(sz)
#define STACK_FREE(p) engine::memory::free(p)

namespace engine
{

using boost::context::detail::fcontext_t;
using boost::context::detail::transfer_t;
using boost::context::detail::jump_fcontext;
using boost::context::detail::make_fcontext;

/**
 * @biref 跳转到目标上下文
 * @param ofc 当前的上下文会保存到ofc中
 * @param nfc 跳转到的目标上下文
 * @param vp 跳转到的目标上下文的附加参数。如果是第一次跳转，
 *          作为函数参数传入，如果是调回到jump_fcontext，这个是返回值
 * @param preserve_fpu 是否复制FPU（浮点数寄存器）数据
 * @return 如果调回时的透传参数
 */
/*intptr_t BOOST_CONTEXT_CALLDECL jump_fcontext(fcontext_t *ofc, fcontext_t nfc,
 * intptr_t vp, bool preserve_fpu = false);*/

/**
 * @biref 初始化执行环境上下文
 * @param sp 栈空间地址
 * @param size 栈空间的大小
 * @param fn 入口函数
 * @return 返回初始化完成后的执行环境上下文
 */
/*BOOST_CONTEXT_DECL fcontext_t BOOST_CONTEXT_CALLDECL
make_fcontext(void *sp, std::size_t size, void (*fn)(intptr_t));*/

// https://owent.net/2016/1270.html
// mprotect

typedef void (*fn_t)(transfer_t);

struct StackTraits
{
    static int &getProtectStackPageSize();

    static bool protectStack(void *stack, size_t size, int pageSize);

    static void unprotectStack(void *stack, int pageSize);
};

} // namespace engine

#endif
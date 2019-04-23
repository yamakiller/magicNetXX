#ifndef CIS_ENGINE_FCONTEXT_H
#define CIS_ENGINE_FCONTEXT_H

#include <boost/context/fcontext.hpp>
#include <functional>

namespace engine {

using boost::context::fcontext_t;
using boost::context::jump_fcontext;
using boost::context::make_fcontext;

typedef std::function<void(intptr_t)> fcontext_call;

} // namespace engine

#endif
#include "actor.h"

namespace engine {

actor::actor(acb const &cb, size_t stacksize)
                : ctx(&actor::defaultExecute, (void*)this, stacksize),
                  fn(cb) {
}

actor::~actor(){

}

void actor::Execute() {
    auto call_fn = [this]() {
         this->fn();
         this->fn_ = acb();
    };

    call_fn();

    state = actorState::done;
}

void actor::defaultExecute(transfer_t trans)
{
    actor *at = static_cast<actor*>(trans.data);
    at->run();
}

const char *getActorStateName(actorState state){
    switch(state)
    {
        case actorState::runnable:
            return "RUNNABLE";
        case actorState::block:
            return "BLOCK";
        case actorState::done:
            return "DONE";
        default:
            return "UNKONW";
    }
}
}
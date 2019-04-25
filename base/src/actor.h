/*TaskState state_ = TaskState::runnable;
    uint64_t id_;
    Processer* proc_ = nullptr;
    Context ctx_;
    TaskF fn_;
    std::exception_ptr eptr_;           // 保存exception的指针
    TaskAnys anys_;

    uint64_t yieldCount_ = 0;

    atomic_t<uint64_t> suspendId_ {0};*/

#include "base.h"
#include "context.h"
#include "shared_ptr.h"


namespace engine {
    class vpid;
    enum class actorState {
         runnable,
         block,
         done,
    };

    typedef std::function<void()> acotr_callback;
    typedef acotr_callback acb;

    class actor : public SharedRefObject {
    public:
        uint64_t id;
        context  ctx;
        acb  fn;
        actorState state=actorState::runnable;
        vpid *proc = nullptr;
        std::exception_ptr eptr;

        uint64_t yieldCount;
        atomic_t<uint64_t> suspendId {0};

        actor(acb const &cb, size_t stacksize);
        ~actor();

        inline void SwapIn(){
            ctx.SwapIn();
        }

        inline void SwapOut(){
            ctx.SwapOut();
        }
    private:
        void Execute();
        static void defaultExecute(transfer_t trans);

        actor(actor const&) = delete;
        actor(actor &&) = delete;
        actor& operator=(actor const&) = delete;
        actor& operator=(actor &&) = delete;
    };

    const char* getActorStateName(actorState state);
}
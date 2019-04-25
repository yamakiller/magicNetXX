//WorkSteal

#ifndef CIS_ENGINE_VPID_H
#define CIS_ENGINE_VPID_H

#include "actor.h"
#include <condition_variable>
#include <mutex>
#include <atomic>

namespace engine {

    class scheduler;

    class vpid {
        friend class scheduler;
        private:
            int m_id;
             * m_lpsch;

            actor* m_running{nullptr};
    };
}


#endif

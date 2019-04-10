#include "usignal.h"

using namespace cis;

static unsigned char signalEvent[__SIGRTMIN] = {0, 0, 0, 0, 0, 0, 0, 0,
                                                0, 0, 0, 0, 0, 0, 0, 0,
                                                0, 0, 0, 0, 0, 0, 0, 0,
                                                0, 0, 0, 0, 0, 0, 0, 0};

static void
signalCallback(int signal)
{
    signalEvent[signal] = 1;
}

usignal::usignal(int flags, int sig, signalFun cb)
{
    m_sa.sa_handler = &signalCallback;
    m_sa.sa_flags = flags;
    sigfillset(&m_sa.sa_mask);
    sigaction(sig, &m_sa, NULL);
    m_iSig = sig;
    m_callback = cb;
}

usignal::~usignal()
{
}

void usignal::wait()
{
    if (signalEvent[m_iSig])
    {
        m_callback();
        signalEvent[m_iSig] = 0;
    }
}

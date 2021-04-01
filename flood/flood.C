#include <ck.h>
#include "flood.decl.h"

CProxy_test testers;
CkpvDeclare(int, handler_idx_);

void handler(envelope* env) {
    auto msg = EnvToUsr(env);
    msg = CMessage_CkMarshallMsg::unpack(msg);
    CkCallback cb(CkIndex_test::recv(0), testers);
    cb.send(msg);
}

void registration(void) {
    if (!CkpvInitialized(handler_idx_)) {
        CkpvInitialize(int, handler_idx_);
    }

    CkpvAccess(handler_idx_) = CmiRegisterHandler((CmiHandler)handler);
}

struct main: public CBase_main {
    int numReps, numIters;

    main(CkArgMsg* msg)
    : numReps(11), numIters(msg->argc > 1 ? atoi(msg->argv[1]) : 128) {
        CkCallback cb(CkIndex_main::run(), thisProxy);
        testers = CProxy_test::ckNew(cb);
    }

    void run(void) {
        for (int i = 0; i < numReps; i += 1) {
            for (int j = 0; j < numIters; j += 1) {
                testers.run(i * numIters + j);

                CkWaitQD();
            }
        }

        CkExit();
    }
};

struct test: public CBase_test {
    test_SDAG_CODE;

    test(const CkCallback& cb) {
        contribute(cb);
    }

    void send(void) {
        for (int i = 0; i < CkNumPes(); i += 1) {
            auto msg = CkAllocateMarshallMsg(sizeof(int));
            auto& val = *(reinterpret_cast<int*>(msg->msgBuf));
            val = i;
            msg = (CkMarshallMsg*)CMessage_CkMarshallMsg::pack(msg);
            auto env = UsrToEnv(msg);
            CmiSetHandler(env, CkpvAccess(handler_idx_));
            CmiSyncSendAndFree(i, env->getTotalsize(),
                               reinterpret_cast<char*>(env));
        }
    }

private:
    int i;
};


#include "flood.def.h"

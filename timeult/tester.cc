#include "tester.hh"

#include <thread>

static_assert(!CMK_SMP, "this benchmark only runs on non-SMP builds");

void handleMsg(void *msg) {
  if (++CpvAccess(it) >= CpvAccess(nIters)) {
    CmiSetHandler(msg, CpvAccess(doneHandlerIdx));
  } else {
    CmiSetHandler(msg, CpvAccess(msgHandlerIdx));
  }

  CmiSyncSendAndFree(CmiMyPe(), sizeof(EmptyMsg), (char *)msg);
}

inline void action(void *msg) {
  switch (CpvAccess(phase)) {
  case 0: {
    CpvAccess(startTime) = CmiWallTimer();
    CmiSetHandler(msg, CpvAccess(msgHandlerIdx));
    CmiSyncSendAndFree(CmiMyPe(), sizeof(EmptyMsg), (char *)msg);
    break;
  }
  case 1: {
    auto th = CthCreate((CthVoidFn)runCthThread, msg, 0);
    CthAwaken(th);
    break;
  }
  case 2: {
    std::thread th(runCppThread, msg);
    th.detach();
    break;
  }
  case 3: {
    pthread_t th;
    int err = pthread_create(&th, nullptr, &runPthread, msg);
    CmiEnforceMsg(!err, "could not create pthread!");
    pthread_detach(th);
    break;
  }
#if HAS_BOOST_FIBER
  case 4: {
    boost::fibers::fiber f(runBoostFiber, msg);
    f.join();
    break;
  }
#endif
  default: {
    CmiFree(msg);
    CsdExitScheduler();
    break;
  }
  }
}

inline void handleCleanup(void) {
  CpvAccess(phase)++;
  CpvAccess(rep) = 0;
  CpvAccess(totalTime) = 0;
}

void handleDone(void *msg) {
  auto time = CmiWallTimer() - CpvAccess(startTime);
  auto &totalTime = CpvAccess(totalTime);
  totalTime += time;

  CpvAccess(it) = 0;

  if (++CpvAccess(rep) >= CpvAccess(nReps)) {
    const char *phase = phaseToString(CpvAccess(phase));
    CmiPrintf("%d> average time for %d %s switches was %g us.\n", CmiMyPe(),
              CpvAccess(nIters), phase, (1e6 * totalTime) / CpvAccess(nReps));
    CmiPrintf("%d> average time per %s switch was: %g ns\n\n", CmiMyPe(), phase,
              (1e9 * totalTime) / (CpvAccess(nIters) * CpvAccess(nReps)));
    handleCleanup();
  }

  action(msg);
}

using yield_fn_t = void (*)(void);
template <yield_fn_t Fn>
void runThread(void *msg) {
  CpvAccess(startTime) = CmiWallTimer();

  auto &it = CpvAccess(it);
  for (CmiAssert(it == 0); it < CpvAccess(nIters); it++) {
    Fn();
  }

  CmiSetHandler(msg, CpvAccess(doneHandlerIdx));
  CmiSyncSendAndFree(CmiMyPe(), sizeof(EmptyMsg), (char *)msg);
}

void runCthThread(void *msg) { runThread<CthYield>(msg); }

void runCppThread(void *msg) { runThread<std::this_thread::yield>(msg); }

void runBoostFiber(void *msg) { runThread<boost::this_fiber::yield>(msg); }

void yieldPthread(void) {
  CmiEnforceMsg(sched_yield() == 0, "could not yield pthread!");
}

void *runPthread(void *msg) {
  runThread<yieldPthread>(msg);
  return nullptr;
}

void handleInit(int argc, char **argv) {
  // Call CpvInitialize on all pseudo-globals
  initializeGlobals();
  // Register handlers
  CpvAccess(msgHandlerIdx) = CmiRegisterHandler((CmiHandler)handleMsg);
  CpvAccess(doneHandlerIdx) = CmiRegisterHandler((CmiHandler)handleDone);
  // Setup arguments
  CpvAccess(it) = 0;
  CpvAccess(rep) = 0;
  CpvAccess(phase) = 0;
  CpvAccess(nIters) = (argc > 1) ? atoi(argv[1]) : 128;
  CpvAccess(nReps) = (argc > 2) ? atoi(argv[2]) : 11;
  CmiPrintf("%d> will run %d switches total\n", CmiMyPe(),
            CpvAccess(nIters) * CpvAccess(nReps));
  // Setup timing
  CpvAccess(totalTime) = 0;
  CpvAccess(startTime) = CmiWallTimer();
  // GO--!
  if (CmiMyPe() == 0) {
    CmiPrintf("%d> cthread built with: %s\n\n", CmiMyPe(), cthThreadBuild());
    EmptyMsg msg;
    CmiSetHandler(&msg, CpvAccess(msgHandlerIdx));
    CmiSyncSend(CmiMyPe(), sizeof(EmptyMsg), reinterpret_cast<char *>(&msg));
  } else {
    CsdExitScheduler();
  }
}

int main(int argc, char **argv) { ConverseInit(argc, argv, handleInit, 0, 0); }

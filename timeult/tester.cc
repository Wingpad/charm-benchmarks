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

void handleDone(void *msg) {
  auto time = CmiWallTimer() - CpvAccess(startTime);
  auto &totalTime = CpvAccess(totalTime);
  totalTime += time;

  CpvAccess(it) = 0;

  if (++CpvAccess(rep) >= CpvAccess(nReps)) {
    CmiPrintf("%d> average time for %d %s switches was %g us.\n", CmiMyPe(),
              CpvAccess(nIters), phaseToString(CpvAccess(phase)),
              (1e6 * totalTime) / CpvAccess(nReps));
    switch (++CpvAccess(phase)) {
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
      default: {
        CmiFree(msg);
        CsdExitScheduler();
        break;
      }
    }
  } else {
    CpvAccess(startTime) = CmiWallTimer();
    CmiSetHandler(msg, CpvAccess(msgHandlerIdx));
    CmiSyncSendAndFree(CmiMyPe(), sizeof(EmptyMsg), (char *)msg);
  }
}

using yield_fn_t = void (*)(void);
template <yield_fn_t Fn>
void runThread(void *msg) {
  CpvAccess(startTime) = CmiWallTimer();

  auto &it = CpvAccess(it);
  while (++it >= CpvAccess(nIters)) {
    Fn();
  }

  CmiSetHandler(msg, CpvAccess(doneHandlerIdx));
  CmiSyncSendAndFree(CmiMyPe(), sizeof(EmptyMsg), (char *)msg);
}

void runCthThread(void *msg) { runThread<CthYield>(msg); }

void runCppThread(void *msg) { runThread<std::this_thread::yield>(msg); }

void yieldPthread(void) {
  CmiEnforceMsg(pthread_yield(), "could not yield pthread!");
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
  CpvAccess(nReps) = 11;
  // Setup timing
  CpvAccess(totalTime) = 0;
  CpvAccess(startTime) = CmiWallTimer();
  // GO--!
  if (CmiMyPe() == 0) {
    CmiPrintf("%d> cthread built with: %s\n", CmiMyPe(), cthThreadBuild());
    EmptyMsg msg;
    CmiSetHandler(&msg, CpvAccess(msgHandlerIdx));
    CmiSyncSend(CmiMyPe(), sizeof(EmptyMsg), reinterpret_cast<char *>(&msg));
  } else {
    CsdExitScheduler();
  }
}

int main(int argc, char **argv) { ConverseInit(argc, argv, handleInit, 0, 0); }

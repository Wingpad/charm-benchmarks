#include "tester.hh"

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
    const char *phase = CpvAccess(phase) ? "thread" : "handler";
    CmiPrintf("%d> average time for %d %s switches was %g us.\n", CmiMyPe(),
              CpvAccess(nIters), phase, (1e6 * totalTime) / CpvAccess(nReps));
    if (++CpvAccess(phase) == 1) {
      auto th = CthCreate((CthVoidFn)runThread, msg, 0);
      CthAwaken(th);
    } else {
      CmiFree(msg);
      CsdExitScheduler();
    }
  } else {
    CpvAccess(startTime) = CmiWallTimer();
    CmiSetHandler(msg, CpvAccess(msgHandlerIdx));
    CmiSyncSendAndFree(CmiMyPe(), sizeof(EmptyMsg), (char *)msg);
  }
}

void runThread(void* msg) {
  CpvAccess(startTime) = CmiWallTimer();

  while (++CpvAccess(it) >= CpvAccess(nIters)) {
    CthYield();
  }

  CmiSetHandler(msg, CpvAccess(doneHandlerIdx));
  CmiSyncSendAndFree(CmiMyPe(), sizeof(EmptyMsg), (char *)msg);
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
    EmptyMsg msg;
    CmiSetHandler(&msg, CpvAccess(msgHandlerIdx));
    CmiSyncSend(CmiMyPe(), sizeof(EmptyMsg), reinterpret_cast<char *>(&msg));
  } else {
    CsdExitScheduler();
  }
}

int main(int argc, char **argv) { ConverseInit(argc, argv, handleInit, 0, 0); }

#include "tester.hh"

#include <thread>

void handleMsg(void *msg) {
  if (++CpvAccess(it) >= CpvAccess(nIters)) {
    CmiSetHandler(msg, CpvAccess(doneHandlerIdx));
  } else {
    CmiSetHandler(msg, CpvAccess(msgHandlerIdx));
  }

  CmiSyncNodeSendAndFree(CmiMyNode(), sizeof(EmptyMsg), (char *)msg);
}

inline void handleCleanup(void) {
  CpvAccess(phase)++;
  CpvAccess(rep) = 0;
  CpvAccess(totalTime) = 0;
}

inline void action(void *msg) {
  switch (CpvAccess(phase)) {
  case 0: {
    CpvAccess(startTime) = CmiWallTimer();
    CmiSetHandler(msg, CpvAccess(msgHandlerIdx));
    CmiSyncNodeSendAndFree(CmiMyNode(), sizeof(EmptyMsg), (char *)msg);
    break;
  }
  case 1: {
    CmiSetHandler(msg, CpvAccess(exitHandlerIdx));
    CmiSyncBroadcastAllAndFree(sizeof(EmptyMsg), (char *)msg);
    break;
  }
  }
}

void handleDone(void *msg) {
  auto time = CmiWallTimer() - CpvAccess(startTime);
  auto &totalTime = CpvAccess(totalTime);
  totalTime += time;

  CpvAccess(it) = 0;

  if (++CpvAccess(rep) >= CpvAccess(nReps)) {
    CmiPrintf("%d> average time for %d exchanges was %g us.\n", CmiMyPe(),
              CpvAccess(nIters), (1e6 * totalTime) / CpvAccess(nReps));
    CmiPrintf("%d> average time for per exchange was %g ns.\n", CmiMyPe(),
              ((1e9 * totalTime) / (CpvAccess(nReps) * CpvAccess(nIters))));

    handleCleanup();
  }

  action(msg);
}

void handleExit(void* msg) {
  CmiFree(msg);
  CsdExitScheduler();
}

void handleInit(int argc, char **argv) {
  if (CmiInCommThread()) return;
  // Call CpvInitialize on all pseudo-globals
  initializeGlobals();
  // Register handlers
  CpvAccess(msgHandlerIdx) = CmiRegisterHandler((CmiHandler)handleMsg);
  CpvAccess(doneHandlerIdx) = CmiRegisterHandler((CmiHandler)handleDone);
  CpvAccess(exitHandlerIdx) = CmiRegisterHandler((CmiHandler)handleExit);
  // Setup arguments
  CpvAccess(it) = 0;
  CpvAccess(rep) = 0;
  CpvAccess(phase) = 0;
  CpvAccess(nReps) = (argc > 2) ? atoi(argv[2]) : 11;
  CpvAccess(nIters) = (argc > 1) ? atoi(argv[1]) : 128;
  // Setup timing
  CpvAccess(totalTime) = 0;
  CpvAccess(startTime) = CmiWallTimer();
  // GO--!
  if (CmiMyPe() == 0) {
    EmptyMsg msg;
    CmiSetHandler(&msg, CpvAccess(msgHandlerIdx));
    CmiSyncSend(CmiMyPe(), sizeof(EmptyMsg), reinterpret_cast<char *>(&msg));
  }
}

int main(int argc, char **argv) { ConverseInit(argc, argv, handleInit, 0, 0); }

#include "tester.hh"

#include <thread>

void handleMsg(void *msg) {
  if (++CsvAccess(it) >= CsvAccess(nIters)) {
    CmiSetHandler(msg, CpvAccess(doneHandlerIdx));
  } else {
    CmiSetHandler(msg, CpvAccess(msgHandlerIdx));
  }

  CmiSyncNodeSendAndFree(CmiMyNode(), sizeof(EmptyMsg), (char *)msg);
}

inline void handleCleanup(void) {
  CsvAccess(phase)++;
  CsvAccess(rep) = 0;
  CsvAccess(totalTime) = 0;
}

inline void action(void *msg) {
  switch (CsvAccess(phase)) {
  case 0: {
    CsvAccess(startTime) = CmiWallTimer();
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
  auto time = CmiWallTimer() - CsvAccess(startTime);
  auto &totalTime = CsvAccess(totalTime);
  totalTime += time;

  CsvAccess(it) = 0;

  if (++CsvAccess(rep) >= CsvAccess(nReps)) {
    CmiPrintf("%d> average time for %d exchanges was %g us.\n", CmiMyPe(),
              CsvAccess(nIters), (1e6 * totalTime) / CsvAccess(nReps));
    CmiPrintf("%d> average time for per exchange was %g ns.\n", CmiMyPe(),
              ((1e9 * totalTime) / (CsvAccess(nReps) * CsvAccess(nIters))));

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
  // Call CsvInitialize on all pseudo-globals
  initializeGlobals();
  // Register handlers
  CpvAccess(msgHandlerIdx) = CmiRegisterHandler((CmiHandler)handleMsg);
  CpvAccess(doneHandlerIdx) = CmiRegisterHandler((CmiHandler)handleDone);
  CpvAccess(exitHandlerIdx) = CmiRegisterHandler((CmiHandler)handleExit);
  // Setup arguments
  CsvAccess(it) = 0;
  CsvAccess(rep) = 0;
  CsvAccess(phase) = 0;
  CsvAccess(nReps) = (argc > 2) ? atoi(argv[2]) : 11;
  CsvAccess(nIters) = (argc > 1) ? atoi(argv[1]) : 128;
  // Setup timing
  CsvAccess(totalTime) = 0;
  CsvAccess(startTime) = CmiWallTimer();
  // GO--!
  if (CmiMyPe() == 0) {
    EmptyMsg msg;
    CmiSetHandler(&msg, CpvAccess(msgHandlerIdx));
    CmiSyncSend(CmiMyPe(), sizeof(EmptyMsg), reinterpret_cast<char *>(&msg));
  }
}

int main(int argc, char **argv) { ConverseInit(argc, argv, handleInit, 0, 0); }

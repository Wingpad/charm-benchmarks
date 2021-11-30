#ifndef __TESTER_HH__
#define __TESTER_HH__

#include <algorithm>
#include <ctime>

#include "converse.h"

CpvDeclare(int, it);
CpvDeclare(int, nIters);

CpvDeclare(int, rep);
CpvDeclare(int, nReps);

CpvDeclare(int, phase);

CpvDeclare(double, startTime);
CpvDeclare(double, totalTime);

CpvDeclare(int, msgHandlerIdx);
CpvDeclare(int, doneHandlerIdx);

struct EmptyMsg {
  char core[CmiMsgHeaderSizeBytes];

  EmptyMsg(void) { CmiInitMsgHeader(this, sizeof(EmptyMsg)); }
};

void initializeGlobals(void) {
  CpvInitialize(int, it);
  CpvInitialize(int, nIters);
  CpvInitialize(int, rep);
  CpvInitialize(int, nReps);
  CpvInitialize(int, phase);
  CpvInitialize(double, startTime);
  CpvInitialize(double, totalTime);
  CpvInitialize(int, msgHandlerIdx);
  CpvInitialize(int, doneHandlerIdx);
}

void *runPthread(void* msg);
void runCppThread(void* msg);
void runCthThread(void* msg);

const char* cthThreadBuild(void) {
#if CMK_THREADS_BUILD_DEFAULT
    return "default";
#elif CMK_THREADS_USE_JCONTEXT
    return "jcontext";
#elif CMK_THREADS_USE_FCONTEXT
    return "fcontext";
#elif CMK_THREADS_USE_CONTEXT
    return "context";
#elif CMK_THREADS_ARE_WIN32_FIBERS
    return "win32-fibers";
#elif CMK_THREADS_USE_PTHREADS
    return "pthreads";
#else
    return "???";
#endif
}

const char* phaseToString(int phase) {
  switch (phase) {
    case 0:
      return "converse handler";
    case 1:
      return "cthread (non-migratable)";
    case 2:
      return "std::thread";
    case 3:
      return "pthread";
    default:
      return "???";
  }
}

#endif

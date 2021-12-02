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
CpvDeclare(int, exitHandlerIdx);

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
  CpvInitialize(int, exitHandlerIdx);
}

#endif

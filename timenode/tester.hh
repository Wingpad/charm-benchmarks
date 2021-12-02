#ifndef __TESTER_HH__
#define __TESTER_HH__

#include <algorithm>
#include <ctime>

#include "converse.h"

CsvDeclare(int, it);
CsvDeclare(int, nIters);
CsvDeclare(int, rep);
CsvDeclare(int, nReps);
CsvDeclare(int, phase);

CsvDeclare(double, startTime);
CsvDeclare(double, totalTime);

CpvDeclare(int, msgHandlerIdx);
CpvDeclare(int, doneHandlerIdx);
CpvDeclare(int, exitHandlerIdx);

struct EmptyMsg {
  char core[CmiMsgHeaderSizeBytes];

  EmptyMsg(void) { CmiInitMsgHeader(this, sizeof(EmptyMsg)); }
};

void initializeGlobals(void) {
  CsvInitialize(int, it);
  CsvInitialize(int, nIters);
  CsvInitialize(int, rep);
  CsvInitialize(int, nReps);
  CsvInitialize(int, phase);
  CsvInitialize(double, startTime);
  CsvInitialize(double, totalTime);
  CpvInitialize(int, msgHandlerIdx);
  CpvInitialize(int, doneHandlerIdx);
  CpvInitialize(int, exitHandlerIdx);
}

#endif

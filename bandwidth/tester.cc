#include "tester.decl.h"

/* readonly */ CProxy_Main mainProxy;

struct Main : public CBase_Main {
  Main_SDAG_CODE;

 public:
  int nIters, nReps, nSkip;
  int rep;

  double *data;
  double start, totalTime;

  std::deque<int> sizes;

  CProxy_Receiver receiver;

  Main(CkArgMsg *m) {
    CkEnforceMsg(m->argc >= 2, "expected nIters and nElts");

    this->nIters = atoi(m->argv[1]);
    this->nReps = (m->argc >= 3) ? atoi(m->argv[2]) : (nIters / 10) + 1;
    this->nSkip = nReps / 2;

    auto whichNode = CkNumNodes() - 1;
    auto whichPe = (whichNode == CkMyNode()) ? ((CkMyPe() + 1) % CkNumPes())
                                             : CkNodeFirst(whichNode);

    auto maxSize = 512 * 1024;
    for (auto size = 1; size <= maxSize; size *= 2) {
      this->sizes.push_back(size);
    }

    CkPrintf("Size\t\tMB/s\n");

    mainProxy = thisProxy;
    receiver = CProxy_Receiver::ckNew(whichPe);
  }

  void next(void) {
    if (sizes.empty()) {
      CkExit();
    } else {
      auto size = sizes.front();
      thisProxy.run(size);
      sizes.pop_front();
    }
  }
};

class Receiver : public CBase_Receiver {
  int it;
  Receiver_SDAG_CODE;

 public:
  Receiver(void) { mainProxy.next(); }
};

#include "tester.def.h"

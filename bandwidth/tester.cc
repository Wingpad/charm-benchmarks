#include "tester.decl.h"

/* readonly */ CProxy_Main mainProxy;

struct Main : public CBase_Main {
  Main_SDAG_CODE;

 public:
  int nIters, nReps, nSkip;
  int rep;

  double *data;
  double start, totalTime;

  CProxy_Receiver receiver;

  Main(CkArgMsg *m) {
    CkEnforceMsg(m->argc >= 3, "expected nIters and nElts");

    this->nIters = atoi(m->argv[1]);
    this->nReps = (nIters / 10) + 1;
    this->nSkip = nReps / 2;

    auto nElts = atoi(m->argv[2]);

    auto whichNode = CkNumNodes() - 1;
    auto whichPe = (whichNode == CkMyNode()) ? ((CkMyPe() + 1) % CkNumPes())
                                             : CkNodeFirst(whichNode);
    receiver = CProxy_Receiver::ckNew(whichPe);

    mainProxy = thisProxy;
    thisProxy.run(nElts * 1024);
  }
};

class Receiver : public CBase_Receiver {
  int it;
  Receiver_SDAG_CODE;

 public:
  Receiver(void) {}
};

#include "tester.def.h"

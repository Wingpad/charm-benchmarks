#include "tester.decl.h"

/* readonly */ CProxy_Main mainProxy;

struct Main : public CBase_Main {
  Main_SDAG_CODE;

 public:
  int nIters, nReps, nSkip;
  int rep;

  double* data;
  double start, totalTime;

  std::deque<int> sizes;

  CProxy_Receiver receiver;
  CthThread th;

  Main(CkArgMsg* m) {
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

  inline void initialize(const int& size) {
    this->data = new double[size];
    this->totalTime = 0;
    this->th = CthSelf();
  }

  inline void finalize(const int& size) {
    auto avgTime = this->totalTime / this->nReps;
    auto totalSize = size * sizeof(double);
    auto bw = (totalSize * this->nIters) / (avgTime * 1024.0 * 1024.0);
    CkPrintf("%lu\t\t%g\n", totalSize, bw);
    delete[] this->data;
    this->th = nullptr;
    thisProxy.next();
  }

#if !USE_SDAG
  void run(int size) {
    this->initialize(size);

    for (rep = 0; rep < (nReps + nSkip); rep += 1) {
      this->start = CkWallTimer();
      receiver.run(nIters);
      for (auto it = 0; it < nIters; it += 1) {
        receiver.arrival(size, this->data);
      }
      CthSuspend();
      auto end = CkWallTimer();
      auto time = end - start;
      if (rep >= nSkip) {
        this->totalTime += time;
      }
    }

    this->finalize(size);
  }

  void completion(void) { CthAwaken(this->th); }
#endif
};

class Receiver : public CBase_Receiver {
  int it;
  int nIters;
  Receiver_SDAG_CODE;

 public:
  Receiver(void) : it(0), nIters(0) { mainProxy.next(); }

#if !USE_SDAG
  inline void run(const int& nMsgs) {
    this->nIters += nMsgs;
    this->arrival(0, nullptr, false);
  }

  inline void arrival(const int& size, double* data,
                      const bool& increment = true) {
    if (increment) {
      this->it += 1;
    }

    if ((this->it - this->nIters) == 0) {
      this->it = 0;
      this->nIters = 0;
      mainProxy.completion();
    }
  }
#endif
};

#include "tester.def.h"

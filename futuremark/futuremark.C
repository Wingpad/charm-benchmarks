#include <ck.h>

#include "futuremark.decl.h"

/* Compile-time options:
 *
 * -DOVERLAP_ITERATIONS
 *   issues all iterations of a repetition
 *   without waiting for one to finish
 *   before issuing the next. multiple
 *   cthreads will co-exist vs only one
 *   at a time. usually much slower.
 */

class Main : public CBase_Main {
  int numIters, numReps;
  CProxy_Exchanger exchangers;
 public:
  Main(CkArgMsg *m)
  : numIters(atoi(m->argv[1])), numReps(11) {
    exchangers = CProxy_Exchanger::ckNew();

    thisProxy.run();
  }

  void run(void) {
    double avgTime = 0.0;

    for (int i = 0; i < numReps; i += 1) {
      auto start = CkWallTimer();
      for (int j = 0; j < numIters; j += 1) {
        exchangers.exchange();

#ifndef OVERLAP_ITERATIONS
        CkWaitQD();
#endif
      }
#ifdef OVERLAP_ITERATIONS
      CkWaitQD();
#endif
      auto end = CkWallTimer();
      avgTime += (end - start);
    }

    CkPrintf("on avg, %d iterations of exchange took: %f s\n", numIters, avgTime / numReps);

    CkExit();
  }
};

struct Exchanger: public CBase_Exchanger {
  Exchanger(void) = default;

  void recvFuture(ck::future<int> f) {
    f.set(thisIndex);
  }

  void exchange(void) {
    std::vector<ck::future<int>> futs(CkNumPes());

    for (auto i = 0; i < CkNumPes(); i += 1) {
      thisProxy[i].recvFuture(futs[i]);
    }

    for (auto i = 0; i < CkNumPes(); i += 1) {
      int recvd = futs[i].get();
      futs[i].release();

      if (i != recvd) {
        CkAbort("did not receive expected value (%d vs. %d)", i, recvd);
      }
    }
  }
};

#include "futuremark.def.h"

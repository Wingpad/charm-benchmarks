#include <ck.h>

#include "avalanche.decl.h"


#ifdef USE_ARRAY

#ifndef DECOMP_FACTOR
#define DECOMP_FACTOR 4
#endif

constexpr auto kDecompFactor = DECOMP_FACTOR;

#endif

constexpr auto kMaxReps = 65;

/* readonly */ int numElements;

struct Main : public CBase_Main {
  int numIters, numReps;
  CProxy_Receiver receivers;
  CProxy_Sender senders;

  Main(CkArgMsg *m)
  : numIters(atoi(m->argv[1])), numReps(numIters / 2 + 1) {
    if (numReps > kMaxReps) numReps = kMaxReps;

#ifdef USE_ARRAY
    CkPrintf("main> kDecompFactor=%d, kNumPes=%d\n", kDecompFactor, CkNumPes());
    numElements = kDecompFactor * CkNumPes();
    receivers = CProxy_Receiver::ckNew(numIters, numElements);
    senders = CProxy_Sender::ckNew(numIters, receivers, numElements);
#else
    numElements = CkNumPes();
    receivers = CProxy_Receiver::ckNew(numIters);
    senders = CProxy_Sender::ckNew(numIters, receivers);
#endif

    thisProxy.run();
  }

  void run(void) {
    double avgTime = 0.0;

    for (int i = 0; i < numReps; i += 1) {
      auto start = CkWallTimer();

      senders.send();
      receivers.run();

      CkWaitQD();

      auto end = CkWallTimer();

      avgTime += end - start;
    }

    CkPrintf("on avg, exchange took: %f s\n", avgTime / numReps);

    CkExit();
  }
};

class Receiver: public CBase_Receiver {
  Receiver_SDAG_CODE;
public:
  int i, j;
  int numIters;

  Receiver(const int& _1): numIters(_1) {} 
};

class Sender: public CBase_Sender {
  Sender_SDAG_CODE;
public:
  int numIters;
  CProxy_Receiver receivers;

  Sender(const int& _1, const CProxy_Receiver& _2)
  : numIters(_1), receivers(_2) {}

  void send(void) {
    for (int i = 0; i < numIters; i++) {
      for (int j = 0; j < numElements; j++) {
        receivers[j].receive(i);
      }
    }
  }
};

#include "avalanche.def.h"

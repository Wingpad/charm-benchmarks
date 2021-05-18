#include <ck.h>

#include "secdest.decl.h"

struct Main : public CBase_Main {
  int numIters, numReps;
  CProxy_Receiver receivers;
  CProxy_Sender senders;

  Main(CkArgMsg *m)
  : numIters(atoi(m->argv[1])), numReps(numIters / 2 + 1) {
    if (numReps > 129) numReps = 129;

    receivers = CProxy_Receiver::ckNew(numIters);
    senders = CProxy_Sender::ckNew(numIters, receivers);

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
      for (int j = 0; j < CkNumPes(); j++) {
        receivers[j].receive(i);
      }
    }
  }
};

#include "secdest.def.h"

#include "tester.decl.h"

struct arguments {
  int nChares = 4 * CmiNumPes();
  int nReps = 11;
  int nIters = 129;
};

arguments parse_arguments(int argc, char** argv);

struct Main : public CBase_Main {
  arguments args;

  Main(CkArgMsg* m) : args(parse_arguments(m->argc, m->argv)) {
    CkCallback cb(CkIndex_Main::run(nullptr), thisProxy);
    CProxy_Communicator::ckNew(args.nChares, args.nIters, args.nChares, cb);
  }

  void run(CkArrayCreatedMsg* msg) {
    CProxy_Communicator arr(msg->aid);

    int nSkip = args.nReps / 2;
    auto totalReps = args.nReps + nSkip;
    double totalTime = 0;

    for (auto rep = 0; rep < totalReps; rep++) {
      CkPrintf("main> rep %d of %d\n", rep + 1, totalReps);

      auto startTime = CkWallTimer();

      arr.run(CkCallbackResumeThread());

      auto endTime = CkWallTimer();

      if (rep >= nSkip) {
        totalTime += (endTime - startTime);
      }
    }

    CkPrintf(
        "info> interleaved %d broadcasts and reductions across %d chares\n",
        args.nIters, args.nChares);
    CkPrintf("info> average time per repetition: %g ms\n",
             1e3 * (totalTime / args.nReps));
    CkPrintf("info> average time per broadcast+reduction: %g ns\n",
             1e6 * (totalTime / (args.nIters * args.nReps)));

    CkExit();
  }
};

class Communicator : public CBase_Communicator {
  int nChares, nIters;
  CkCallback onDone;
  int it;

 public:
  Communicator(int _1, int _2) : nChares(_1), nIters(_2) {}

  void run(const CkCallback& cb) {
    this->it = 0;
    this->onDone = cb;

    auto* msg = CkAllocateMarshallMsg(0);
    this->run(msg);
  }

  void run(CkMessage* msg) {
    if ((++it) == nIters) {
      this->contribute(onDone);

      CkFreeMsg(msg);
    } else if (it % nChares == thisIndex) {
      thisProxy.recv_broadcast(msg);
    }
  }

  void recv_broadcast(CkMessage* msg) {
    CkCallback cb(CkIndex_Communicator::run((CkMessage*)nullptr), thisProxy);
    this->contribute(cb);
    CkFreeMsg(msg);
  }
};

arguments parse_arguments(int argc, char** argv) {
  arguments args;

  int opt;
  opterr = 0;

  while ((opt = getopt(argc, argv, "i:r:k:")) != -1) {
    switch (opt) {
      case 'i':
        args.nIters = atoi(optarg);
        break;
      case 'r':
        args.nReps = atoi(optarg);
        break;
      case 'k':
        args.nChares = atoi(optarg);
        break;
      case '?':
        CmiError("error> unknown option, '%c'!\n", opt);
        break;
    }
  }

  return args;
}

#include "tester.def.h"

#include "tester.hh"

struct test_main : public CBase_test_main {
  int nReps, nIters;

  test_main(CkArgMsg *m) : nReps(11), nIters(128) {
    // read command-line arguments
    if (m->argc >= 2) {
      nIters = atoi(m->argv[1]);
    }

    if (m->argc >= 3) {
      nReps = atoi(m->argv[2]);
    }
    // warm up
    run_all(false);
    // then report
    CkPrintf("index\t\t\t\t\ttime (ns)\n");
    run_all(true);
    // exit
    CkExit();
  }

  void run_all(bool report) {
    run_single<CmiUInt8>(report);
#if CMK___int128_t_DEFINED
    run_single<CmiUInt16>(report);
#endif
    run_single<CkArrayIndex1D>(report);
    run_single<CkArrayIndex2D>(report);
    run_single<CkArrayIndex3D>(report);
    run_single<CkArrayIndex4D>(report);
    run_single<CkArrayIndex5D>(report);
    run_single<CkArrayIndex6D>(report);
  }

  template <typename Index> void run_single(bool report) {
    auto totalTime = time_index_hash<Index>();
    if (report) {
      auto avgTime = totalTime / (nIters * nReps);
      CkPrintf("%s\t\t\t%g\n", type_name<Index>::value, 1e9 * avgTime);
    }
  }

  template <typename Index, typename Hasher = hash_of_t<Index>>
  double time_index_hash(void) {
    Hasher hasher;
    double totalTime = 0;
    std::size_t allHash = 0;
    for (auto rep = 0; rep < nReps; rep++) {
      auto start = CkWallTimer();
      auto idx = make_index<Index>(rep);
      for (auto it = 0; it < nIters; it++) {
        allHash += hasher(idx);
      }
      totalTime += CkWallTimer() - start;
    }
    dummy_ += allHash;
    return totalTime;
  }
};

#include "tester.def.h"

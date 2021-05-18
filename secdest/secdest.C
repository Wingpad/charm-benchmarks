#include <ck.h>

#ifndef DECOMP_FACTOR
#define DECOMP_FACTOR 4
#endif

#ifndef RECREATE_SECTION
#define RECREATE_SECTION 0
#endif

#ifndef REFRESH_SECTION
#define REFRESH_SECTION 0
#endif

constexpr auto kDecompFactor = DECOMP_FACTOR;
constexpr auto kRecreateSection = static_cast<bool>(RECREATE_SECTION);
constexpr auto kRefreshSection = static_cast<bool>(REFRESH_SECTION);

#include "secdest.decl.h"

CProxy_Main mainProxy;

struct cookie_msg : public CkMcastBaseMsg, public CMessage_cookie_msg {
  int numIters;

  cookie_msg(const int& _1) : numIters(_1) {}

  void pup(PUP::er& p) {
    CMessage_cookie_msg::pup(p);
    p | numIters;
  }
};

struct Main : public CBase_Main {
  int numIters, numReps;

  Main(CkArgMsg* m) : numIters(atoi(m->argv[1])), numReps(numIters / 2 + 1) {
    if (numReps > 129) numReps = 129;
    mainProxy = thisProxy;

    auto numChares = kDecompFactor * CkNumPes();
    CkCallback cb(CkIndex_Main::run(NULL), thisProxy);

    CkPrintf("main> kRecreateSection=%d, kRefreshSection=%d\n",
             kRecreateSection, kRefreshSection);

    CkPrintf("main> kDecompFactor=%d, kNumPes=%d\n", kDecompFactor, CkNumPes());

    CProxy_Contributor::ckNew(numChares, numChares, cb);
  }

  void run(CkArrayCreatedMsg* msg) {
    CProxy_Contributor contributors(msg->aid);
    double avgTime = 0.0;

    for (int i = 0; i < numReps; i += 1) {
      if ((i % 2) == 0) {
        CkPrintf("main> repetition %d of %d\n", i + 1, numReps);
      }

      auto start = CkWallTimer();

      contributors[0].run(numIters);

      CkWaitQD();

      auto end = CkWallTimer();

      avgTime += end - start;
    }

    CkPrintf("main> on average, each batch of %d iterations took: %f s\n", numIters, avgTime / numReps);

    CkExit();
  }
};

class Contributor : public CBase_Contributor {
  Contributor_SDAG_CODE;

 public:
  CProxySection_Contributor secProxy;
  CkSectionInfo cookie;

  int factor;
  int numChares, numIters, it;

  Contributor(const int& _1) : numChares(_1) {
    if (thisIndex == 0 && !kRecreateSection) {
      this->create_section();
    }
  }

  void create_section(void) {
    this->factor = 0;
    std::vector<CkArrayIndex> elems;  // add array indices
    elems.reserve(numChares / 2);
    for (auto i = 0; i < numChares; i += 2) {
      elems.emplace_back(i);
      factor += i;
    }
    this->secProxy =
        CProxySection_Contributor::ckNew(this->ckGetArrayID(), elems);
  }

  void run(cookie_msg* msg) {
    CkCallback cb(CkReductionTarget(Contributor, redn_done), thisProxy[0]);
    this->numIters = msg->numIters;
    CkGetSectionInfo(this->cookie, msg);

    for (auto it = 0; it < this->numIters; it += 1) {
      auto data = it * this->thisIndex;
      CProxySection_Contributor::contribute(
          sizeof(int), &data, CkReduction::sum_int, this->cookie, cb);
    }
  }
};

#include "secdest.def.h"

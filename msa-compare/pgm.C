#include "msa/msa.h"

using data_type = double;
using msa_type =
    MSA::MSA1D<data_type, DefaultEntry<data_type>, MSA_DEFAULT_ENTRIES_PER_PAGE>;

#include "pgm.decl.h"

std::uint32_t nElements;
std::uint32_t nReps;
std::uint32_t nWorkers;

class Main : public CBase_Main
{
public:
  Main(CkArgMsg* m)
  {
    if (m->argc >= 2)
    {
      nElements = atoi(m->argv[1]);
    }
    else
    {
      nElements = 1024;
    }

    if (m->argc >= 3)
    {
      nWorkers = atoi(m->argv[2]);
    }
    else
    {
      nWorkers = CmiNumPes();
    }

    nReps = 11;

    CkPrintf("main> running with %d elements across %d workers.\n", nElements, nWorkers);

    auto maxBytes =
        std::max<std::uint32_t>(MSA_DEFAULT_MAX_BYTES, nElements * sizeof(double));
    msa_type arr(nElements, nWorkers, maxBytes);
    CkCallback cb(CkIndex_Main::run(nullptr), thisProxy);
    CProxy_TestArray::ckNew(arr, nWorkers, cb);
  }

  template <typename Fn>
  void benchmark(const char* cfg, const Fn& fn)
  {
    auto nSkip = nReps / 2;
    auto totalReps = nSkip + nReps;

    double totalTime = 0;

    for (auto rep = 0; rep < totalReps; rep++)
    {
      auto startTime = CmiWallTimer();
      fn();
      auto endTime = CmiWallTimer();

      if (rep >= nSkip)
      {
        totalTime += endTime - startTime;
      }
    }

    auto avgTime = totalTime / nReps;
    CkPrintf("main> average time per %s repetition was %lf s.\n", cfg, avgTime);
  }

  void run(CkArrayCreatedMsg* msg)
  {
    CProxy_TestArray workers(msg->aid);

    benchmark("better", [&](void) { workers.run_better(CkCallbackResumeThread()); });

    if (nElements % nWorkers == 0)
    {
      benchmark("worse", [&](void) { workers.run_worse(CkCallbackResumeThread()); });
    }
    else
    {
      CmiPrintf("main> skipping worse configuration since %u %% %u =/= 0.\n", nWorkers,
                nElements);
    }

    CkExit();
  }
};

class TestArray : public CBase_TestArray
{
protected:
  msa_type arr;

  using read_type = MSA::MSARead<msa_type>;

  read_type* r;

public:
  TestArray(const msa_type& arr_) : arr(arr_), r(nullptr) { this->arr.enroll(nWorkers); }

  ~TestArray()
  {
    if (r)
      delete r;
  }

  std::tuple<int, int> range(void) const
  {
    auto rangeSize = nElements / nWorkers;
    auto start = thisIndex * rangeSize;
    auto end = std::min(nElements, (thisIndex + 1) * rangeSize);
    return {start, end};
  }

  void run_better(const CkCallback& cb)
  {
    auto rng = this->range();
    auto& begin = std::get<0>(rng);
    auto& end = std::get<1>(rng);

    auto w = r ? r->syncToWrite() : this->arr.getInitialWrite();
    for (auto i = begin; i < end; i++)
    {
      w.set(i) = 2 * (i + 1);
    }

    if (r == nullptr)
    {
      r = (read_type*)::operator new(sizeof(read_type));
    }

    new (r) read_type(w.syncToRead());

    data_type sum = 0;

    for (auto i = begin; i < end; i++)
    {
      sum += r->get(i);
    }

    (void)sum;

    this->contribute(cb);
  }

  void run_worse(const CkCallback& cb)
  {
    auto rng = this->range();
    auto& begin = std::get<0>(rng);
    auto& end = std::get<1>(rng);

    data_type sum = 0;
    for (auto i = begin; i < end; i++)
    {
      auto w = r ? r->syncToWrite() : this->arr.getInitialWrite();
      w.set(i) = 2 * (i + 1);

      if (r == nullptr)
      {
        r = (read_type*)::operator new(sizeof(read_type));
      }

      new (r) read_type(w.syncToRead());
      sum += r->get(i);
    }

    (void)sum;

    this->contribute(cb);
  }
};

#define CK_TEMPLATES_ONLY
#include "pgm.def.h"
#undef CK_TEMPLATES_ONLY

#include "pgm.def.h"

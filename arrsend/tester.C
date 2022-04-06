#include "tester.hh"

Main::Main(CkArgMsg* msg) {
  int pe = (CkMyPe() + 1) % CkNumPes();
  receiver = CProxy_Receiver::ckNew(pe);

  nElements = (msg->argc >= 2) ? atoi(msg->argv[1]) : 128;
  nIterations = (msg->argc >= 3) ? atoi(msg->argv[2]) : 1024;
  nRepetitions = (msg->argc >= 4) ? atoi(msg->argv[3]) : 11;
  nSkip = nRepetitions / 2 + 1;

  thisProxy.run();
}

template <typename Fn>
double Main::time(bool hold, const Fn& fn) {
  auto totalTime = 0.0;

  for (auto rep = 0; rep <= nRepetitions; rep += 1) {
    auto start = CkWallTimer();

    for (auto it = 0; it < nIterations; it += 1) {
      fn();
    }

    receiver.run(hold, nIterations, CkCallbackResumeThread());

    auto end = CkWallTimer();

    if (rep >= nSkip) {
      totalTime += (end - start);
    }
  }

  return totalTime / (nIterations * nRepetitions);
}

void initialize(int size, dtype* arr) {
  for (auto i = 0; i < size; i += 1) {
    arr[i] = dtype(2 * i + 1);
  }
}

bool check(int size, dtype* arr) {
  auto status = true;
  for (auto i = 0; i < size; i += 1) {
    status = (arr[i] == dtype(2 * i + 1));
  }
  return status;
}

void Main::run(void) {
  auto* arr = (dtype*)malloc(nElements * sizeof(dtype));
  holder<dtype> holder(nElements, arr);

  initialize(nElements, arr);

  auto arrayTime = time(false, [&](void) { receiver.receive(nElements, arr); });
  auto objectTime = time(true, [&](void) { receiver.receive(holder); });

  CkPrintf("running %d iterations with arrays of %d elements\n", nIterations,
           nElements);
  CkPrintf("on average, arrays took: %lg s\n", arrayTime);
  CkPrintf("on average, objects took: %lg s\n", objectTime);
  CkPrintf("arrays were %lgX slower\n", objectTime / arrayTime);

  CkExit();
}

template <typename T>
void holder<T>::pup(PUP::er& p) {
  p | this->size;

  if (p.isUnpacking()) {
    this->arr = (T*)malloc(this->size * sizeof(T));
  }

  PUParray(p, this->arr, this->size);
}

#include "tester.def.h"

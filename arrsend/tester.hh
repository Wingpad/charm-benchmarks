using dtype = double;

template <typename T>
struct holder;

#include "tester.decl.h"

class Main : public CBase_Main {
  CProxy_Receiver receiver;

  int nElements;
  int nIterations;
  int nRepetitions;
  int nSkip;

 public:
  Main(CkArgMsg*);
  void run(void);

  template <typename Fn>
  double time(bool holder, const Fn& fn);
};

class Receiver : public CBase_Receiver {
  int it;

 public:
  Receiver_SDAG_CODE;

  Receiver(void) = default;
};

template <typename T>
struct holder {
  T* arr;
  int size;

  holder(int size_, T* arr_) : arr(arr_), size(size_) {}

  holder(void) : arr(nullptr) {}

  ~holder() {
    if (arr) free(arr);
  }

  void pup(PUP::er&);
};

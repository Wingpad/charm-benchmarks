#ifndef __TESTER_HH__
#define __TESTER_HH__

#include "ck128bitHash.h"
#include "tester.decl.h"

volatile std::size_t dummy_ = 0;

template <typename T> struct type_name { static const char *value; };

template <> const char *type_name<CmiUInt8>::value = "CmiUInt8\t";

template <> const char *type_name<CmiUInt16>::value = "CmiUInt16\t";

template <typename T> const char *type_name<T>::value = typeid(T).name();

template <typename T, typename Enable = void> struct hash_of {
  using type = std::hash<T>;
};

template <typename T>
struct hash_of<
    T, typename std::enable_if<std::is_base_of<CkArrayIndex, T>::value>::type> {
  using type = IndexHasher;
};

template <typename T> using hash_of_t = typename hash_of<T>::type;

template <typename Index> Index make_index(int);

template <> CkArrayIndex1D make_index<CkArrayIndex1D>(int i) {
  return CkArrayIndex1D(i);
}

template <> CkArrayIndex2D make_index<CkArrayIndex2D>(int i) {
  return CkArrayIndex2D(i, i);
}

template <> CkArrayIndex3D make_index<CkArrayIndex3D>(int i) {
  return CkArrayIndex3D(i, i, i);
}

template <> CkArrayIndex4D make_index<CkArrayIndex4D>(int i) {
  return CkArrayIndex4D(i, i, i, i);
}

template <> CkArrayIndex5D make_index<CkArrayIndex5D>(int i) {
  return CkArrayIndex5D(i, i, i, i, i);
}

template <> CkArrayIndex6D make_index<CkArrayIndex6D>(int i) {
  return CkArrayIndex6D(i, i, i, i, i, i);
}

template <> CmiUInt8 make_index<CmiUInt8>(int i) { return (CmiUInt8)i; }

#if CMK___int128_t_DEFINED
template <> CmiUInt16 make_index<CmiUInt16>(int i) { return (CmiUInt16)i; }
#endif

#endif

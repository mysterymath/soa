#ifndef _SOA_H
#define _SOA_H

#include <cstdint>
#include <initializer_list>
#include <type_traits>

namespace soa {

template <typename T> struct BaseConstRef {
protected:
  uint8_t *BytePtrs[sizeof(T)];

public:
  template <uint8_t N>
  [[clang::always_inline]] constexpr BaseConstRef(uint8_t ByteArrays[][N],
                                                  uint8_t Idx) {
#pragma unroll
    for (uint8_t ByteIdx = 0; ByteIdx < sizeof(T); ++ByteIdx)
      BytePtrs[ByteIdx] = &ByteArrays[ByteIdx][Idx];
  }

  [[clang::always_inline]] T get() const { return static_cast<T>(*this); }

  [[clang::always_inline]] constexpr operator T() const {
    uint8_t Bytes[sizeof(T)];
#pragma unroll
    for (uint8_t Idx = 0; Idx < sizeof(T); ++Idx)
      Bytes[Idx] = *BytePtrs[Idx];
    return *reinterpret_cast<const T *>(Bytes);
  }
};

template <typename T> struct BaseRef : public BaseConstRef<T> {
  using BaseConstRef<T>::BytePtrs;

public:
  template <uint8_t N>
  [[clang::always_inline]] constexpr BaseRef(const uint8_t ByteArrays[][N],
                                             uint8_t Idx)
      : BaseConstRef<T>(const_cast<uint8_t [][N]>(ByteArrays), Idx) {}

  [[clang::always_inline]] BaseRef &operator=(T Val) const {
    uint8_t *Bytes = reinterpret_cast<uint8_t *>(&Val);
#pragma unroll
    for (uint8_t Idx = 0; Idx < sizeof(T); ++Idx)
      *BytePtrs[Idx] = Bytes[Idx];
    return *this;
  }
};

template <typename T> struct Ref : public BaseRef<T> {};
template <typename T> struct ConstRef : public BaseConstRef<T> {};

template <typename T, uint8_t N> class Array {
  uint8_t ByteArrays[sizeof(T)][N];

public:
  [[clang::always_inline]] constexpr Array(
      std::initializer_list<T> Entries = {}) {
    uint8_t Idx = 0;
    for (const T Entry : Entries)
      (*this)[Idx++] = Entry;
  }

  [[clang::always_inline]] constexpr Ref<T> operator[](uint8_t Idx) {
    return {ByteArrays, Idx};
  }
  [[clang::always_inline]] constexpr ConstRef<T> operator[](uint8_t Idx) const {
    return {ByteArrays, Idx};
  }
};

template <typename T, uint8_t N> struct Array;

} // namespace soa

#endif // _SOA_H

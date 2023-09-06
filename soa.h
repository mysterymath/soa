#ifndef _SOA_H
#define _SOA_H

#include <cstdint>
#include <initializer_list>
#include <type_traits>

namespace soa {

template <typename T> struct Number {
  uint8_t *Bytes[sizeof(T)];

public:
  template <uint8_t N>
  [[clang::always_inline]] constexpr Number(uint8_t ByteArrays[][N],
                                            uint8_t Idx) {
#pragma unroll
    for (uint8_t ByteIdx = 0; ByteIdx < sizeof(T); ++ByteIdx)
      Bytes[ByteIdx] = &ByteArrays[ByteIdx][Idx];
  }

  [[clang::always_inline]] T operator*() const { return static_cast<T>(*this); }

  [[clang::always_inline]] Number &operator=(T Number) {
    uint8_t *NumberBytes = reinterpret_cast<uint8_t *>(&Number);
#pragma unroll
    for (uint8_t Idx = 0; Idx < sizeof(T); ++Idx)
      *Bytes[Idx] = NumberBytes[Idx];
    return *this;
  }

  [[clang::always_inline]] Number &operator+=(T Number) {
    *this = static_cast<T>(*this) + Number;
    return *this;
  }

  [[clang::always_inline]] constexpr operator T() const {
    uint8_t ByteVals[sizeof(T)];
#pragma unroll
    for (uint8_t Idx = 0; Idx < sizeof(T); ++Idx)
      ByteVals[Idx] = *Bytes[Idx];
    return *reinterpret_cast<const T *>(ByteVals);
  }
};

template <typename T, uint8_t N> class NumberArray {
  uint8_t ByteArrays[sizeof(T)][N];

public:
  [[clang::always_inline]] constexpr NumberArray(
      std::initializer_list<T> Numbers = {}) {
    uint8_t Idx = 0;
    for (const T Number : Numbers)
      (*this)[Idx++] = Number;
  }

  [[clang::always_inline]] constexpr Number<T> operator[](uint8_t Idx) {
    return {ByteArrays, Idx};
  }
};

template <typename T, typename Enable = void> struct Types {
  template <size_t N> using Array = T[N];
  using Reference = T &;
};

template <typename T>
struct Types<
    T, std::enable_if_t<std::is_integral<T>::value || std::is_enum<T>::value>> {
  template <size_t N> using Array = NumberArray<T, N>;
  using Reference = Number<T>;
};

template <typename T> struct Reference;

template <typename T, size_t N> struct Array;

} // namespace soa
  
#endif // _SOA_H

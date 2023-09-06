#include <cstdint>
#include <cstdio>
#include <initializer_list>
#include <string.h>
#include <type_traits>
// #include <iostream>
// #include <tuple>
// #include <utility>

// #include <boost/pfr.hpp>

/*
template <typename T, size_t N> struct ArrayTuple;

template <size_t N, typename T> struct ArrayTuple<std::tuple<T>, N> {
  using Type = std::tuple<T[N]>;
};
template <size_t N, typename T, typename... Rest>
struct ArrayTuple<std::tuple<T, Rest...>, N> {
  using Type = decltype(std::tuple_cat(
      std::declval<std::tuple<T[N]>>(),
      std::declval<typename ArrayTuple<std::tuple<Rest...>, N>::Type>()));
};

template <typename T, size_t N> class StructOfArrays {
  using TupleType = decltype(boost::pfr::structure_to_tuple(std::declval<T>()));
  using ArrayTupleType = typename ArrayTuple<TupleType, N>::Type;

public:
  ArrayTupleType Tuple;
};
*/

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

struct Ball {
  int x;
  int y;
  int dx;
  int dy;
};

#define DEFINE_SOA_REFERENCE                                                   \
  template <> struct soa::Reference<Ball> {                                    \
    soa::Types<int>::Reference x;                                              \
    soa::Types<int>::Reference y;                                              \
    soa::Types<int>::Reference dx;                                             \
    soa::Types<int>::Reference dy;                                             \
                                                                               \
    [[clang::always_inline]] Reference<Ball> &operator=(const Ball &Other) {   \
      x = Other.x;                                                             \
      y = Other.y;                                                             \
      dx = Other.dx;                                                           \
      dy = Other.dy;                                                           \
      return *this;                                                            \
    }                                                                          \
  };

#define DEFINE_SOA_ARRAY                                                       \
  template <uint8_t N> class soa::Array<Ball, N> {                             \
    Types<int>::Array<N> x;                                                    \
    Types<int>::Array<N> y;                                                    \
    Types<int>::Array<N> dx;                                                   \
    Types<int>::Array<N> dy;                                                   \
                                                                               \
  public:                                                                      \
    [[clang::always_inline]] constexpr Array(                                  \
        std::initializer_list<Ball> Balls = {}) {                              \
      uint8_t Idx = 0;                                                         \
      for (const auto &Ball : Balls)                                           \
        (*this)[Idx++] = Ball;                                                 \
    }                                                                          \
                                                                               \
    [[clang::always_inline]] constexpr Reference<Ball>                         \
    operator[](uint8_t Idx) {                                                  \
      return {x[Idx], y[Idx], dx[Idx], dy[Idx]};                               \
    }                                                                          \
                                                                               \
    [[clang::always_inline]] constexpr uint8_t size() const { return N; }      \
  };

#define DEFINE_STRUCT_OF_ARRAYS                                                \
  DEFINE_SOA_REFERENCE                                                         \
  DEFINE_SOA_ARRAY

DEFINE_STRUCT_OF_ARRAYS

soa::Array<Ball, 10> balls = {Ball{.x = 10, .dx = 20}};

void updateBall(uint8_t Idx) {
  auto ball = balls[Idx];
  ball.x += ball.dx;
  ball.y += ball.dy;
}

void updateBalls() {
  for (int i = 0; i < balls.size(); i++)
    updateBall(i);
}

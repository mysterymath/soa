#include <cstdint>
#include <cstdio>
#include <string.h>
#include <initializer_list>
// #include <iostream>
// #include <tuple>
// #include <utility>

// #include <boost/pfr.hpp>

struct Ball {
  int x;
  int y;
  int dx;
  int dy;
};

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

template <typename T> struct SoAProxyNumber {
  uint8_t *Bytes[sizeof(T)];

public:
  template <uint8_t N>
  [[clang::always_inline]] constexpr SoAProxyNumber(uint8_t ByteArrays[][N],
                                                    uint8_t Idx) {
#pragma unroll
    for (uint8_t ByteIdx = 0; ByteIdx < sizeof(T); ++ByteIdx)
      Bytes[ByteIdx] = &ByteArrays[ByteIdx][Idx];
  }

  [[clang::always_inline]] T operator*() const {
    return static_cast<T>(*this);
  }

  [[clang::always_inline]] SoAProxyNumber &operator=(T Number) {
    uint8_t *NumberBytes = reinterpret_cast<uint8_t*>(&Number);
#pragma unroll
    for (uint8_t Idx = 0; Idx < sizeof(T); ++Idx)
      *Bytes[Idx] = NumberBytes[Idx];
    return *this;
  }

  [[clang::always_inline]] SoAProxyNumber &operator+=(T Number) {
    *this = static_cast<T>(*this) + Number;
    return *this;
  }

  [[clang::always_inline]] constexpr operator T() const {
    uint8_t ByteVals[sizeof(T)];
#pragma unroll
    for (uint8_t Idx = 0; Idx < sizeof(T); ++Idx)
      ByteVals[Idx] = *Bytes[Idx];
    return *reinterpret_cast<const T*>(ByteVals);
  }
};

template <typename T, uint8_t N> class StructOfNumberByteArrays {
  uint8_t ByteArrays[sizeof(T)][N];

public:
  [[clang::always_inline]] constexpr StructOfNumberByteArrays(
      std::initializer_list<T> Numbers = {}) {
    uint8_t Idx = 0;
    for (const T Number : Numbers)
      (*this)[Idx++] = Number;
  }

  [[clang::always_inline]] constexpr SoAProxyNumber<T> operator[](uint8_t Idx) {
    return {ByteArrays, Idx};
  }
};

struct SoAProxyBall {
  SoAProxyNumber<int> x;
  SoAProxyNumber<int> y;
  SoAProxyNumber<int> dx;
  SoAProxyNumber<int> dy;

  [[clang::always_inline]] SoAProxyBall &operator=(const Ball &Other) {
    x = Other.x;
    y = Other.y;
    dx = Other.dx;
    dy = Other.dy;
    return *this;
  }
};

template <uint8_t N> class StructOfArraysBall {
  StructOfNumberByteArrays<int, N> x;
  StructOfNumberByteArrays<int, N> y;
  StructOfNumberByteArrays<int, N> dx;
  StructOfNumberByteArrays<int, N> dy;

public:
  [[clang::always_inline]] constexpr StructOfArraysBall(
      std::initializer_list<Ball> Balls = {}) {
    uint8_t Idx = 0;
    for (const auto &Ball : Balls)
      (*this)[Idx++] = Ball;
  }

  [[clang::always_inline]] constexpr SoAProxyBall operator[](uint8_t Idx) {
    return {x[Idx], y[Idx], dx[Idx], dy[Idx]};
  }

  [[clang::always_inline]] constexpr uint8_t size() const { return N; }
};

StructOfArraysBall<10> balls = {Ball{.x = 10, .dx = 20}};

void updateBall(uint8_t Idx) {
  auto ball = balls[Idx];
  ball.x += ball.dx;
  ball.y += ball.dy;
}

void updateBalls() {
  for (int i = 0; i < balls.size(); i++)
    updateBall(i);
}

volatile int c = 10;

int main(void) {
  StructOfNumberByteArrays<int, 2> x;
  x[0] = 0x1234;
  printf("%x\n", *x[0]);
  x[1] = 0xabcd;
  printf("%x\n", *x[1]);
  printf("%x\n", *x[0]);

  balls[5].dy = c;
  updateBalls();
  printf("%d\n", *balls[5].y);

  return 0;
}

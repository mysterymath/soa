#include <cstdint>
#include <initializer_list>
#include <cstdio>
//#include <iostream>
//#include <tuple>
//#include <utility>

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

struct SoAProxyBall {
  int &x;
  int &y;
  int &dx;
  int &dy;

  SoAProxyBall &operator=(const Ball &Other) {
    x = Other.x;
    y = Other.y;
    dx = Other.dx;
    dy = Other.dy;
    return *this;
  }
};

template <uint8_t N> class StructOfArraysBall {
  int x[N];
  int y[N];
  int dx[N];
  int dy[N];

public:
  constexpr StructOfArraysBall(std::initializer_list<Ball> Balls) {
    uint8_t Idx = 0;
    for (const auto &Ball : Balls)
      (*this)[Idx] = Ball;
  }

  constexpr SoAProxyBall operator[](uint8_t Idx) {
    return {x[Idx], y[Idx], dx[Idx], dy[Idx]};
  }

  constexpr uint8_t size() const { return N; }
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

int main(void) {
  balls[5].dy = 8;
  updateBalls();
  printf("%d\n", balls[5].x);

  return 0;
}

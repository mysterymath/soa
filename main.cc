#include <iostream>
#include <tuple>
#include <utility>

#include <boost/pfr.hpp>

struct Ball {
  int x;
  int y;
  int dx;
  int dy;
};

void updateBall(struct Ball *ball) {
  ball->x += ball->dx;
  ball->y += ball->dy;
}

void updateBalls(struct Ball *balls, int ballCnt) {
  for (int i = 0; i < ballCnt; i++) {
    updateBall(&balls[i]);
  }
}

template <typename T, size_t N> struct ArrayTuple;

template <size_t N, typename T, typename... Rest>
struct ArrayTuple<std::tuple<T, Rest...>, N> {
  using Type = decltype(std::declval<std::tuple<T[N]>>().tuple_cat(
      std::declval<ArrayTuple<std::tuple<Rest...>, N>::Type>()));
};

template <typename T, size_t N> class StructOfArrays {
  using TupleType = decltype(boost::pfr::structure_to_tuple(std::declval<T>()));
};

int main(void) {
  struct Ball balls[10] = {0};
  balls[5].x = 10;
  balls[5].dx = 20;
  updateBalls(balls, sizeof(balls) / sizeof(Ball));
  std::cout << balls[5].x << '\n';
  return 0;
}

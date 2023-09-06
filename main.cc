#include <cstdio>
#include <string.h>
#include <soa.h>

struct Ball {
  int x;
  int y;
  int dx;
  int dy;
};

#define SOA_TYPE Ball
#define SOA_MEMBERS MEMBER(x) MEMBER(y) MEMBER(dx) MEMBER(dy)
#include <soa-impl.inc>

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

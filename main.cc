#include <cstdio>
#include <soa.h>
#include <string.h>

#if 0
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
#endif

struct InnerStruct {
  const char *char_ptr;
};

struct Struct {
  int i;
} s;

struct Functor {
  int x;
  int operator()() const { return x; }
};

struct Test {
  int i;
  InnerStruct inner;
  Struct *struct_ptr;
  Functor functor;
};

// TODO: Default initialization? = 42 in struct?
// TODO: It'll need to be standard layout dawg; add a check for that
  // This allows inheritance, but only one class can have data members. We need to find that class..
  // In that case, the user can just tell us. There's exactly one, and that's where *all* of the members come from, so there's no additional members to specify.
// TODO: Struct inheritance
// TODO: Alignment
// TODO: Volatile
// TODO: See if we can change the ref representation to a base pointer and
//       stride; that improves the worst case of storing one.
// TODO: Std::forward implementation in SDK

#define SOA_STRUCT InnerStruct
#define SOA_MEMBERS MEMBER(char_ptr)
#include <soa-struct.inc>

#define SOA_STRUCT Test
#define SOA_MEMBERS MEMBER(i) MEMBER(inner) MEMBER(struct_ptr) MEMBER(functor)
#include <soa-struct.inc>

extern soa::Array<Test, 100> TestArray;

int test() {
  TestArray[10].i = 42;
  TestArray[10].i += 42;
  TestArray[10].inner.char_ptr = "Hello";
  TestArray[10].struct_ptr = &s;

  (*TestArray[10].struct_ptr).i = 3;
  TestArray[10].struct_ptr->i = 4;

  return TestArray[10].functor();
}

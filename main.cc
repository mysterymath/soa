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

struct DerivedFunctor : public Struct {
  int operator()() const { return i; }
};

struct Test {
  int i;
  InnerStruct inner;
  Struct *struct_ptr;
  Functor functor;
  DerivedFunctor derived_functor;
};

// TODO: See if we can change the ref representation to a base pointer and
//       stride; that improves the worst case of storing one.
// TODO: Std::forward implementation in SDK

#define SOA_STRUCT InnerStruct
#define SOA_MEMBERS MEMBER(char_ptr)
#include <soa-struct.inc>

#define SOA_STRUCT Functor
#define SOA_MEMBERS MEMBER(x)
#include <soa-struct.inc>

#define SOA_STRUCT DerivedFunctor
#define SOA_MEMBERS MEMBER(i)
#include <soa-struct.inc>

#define SOA_STRUCT Test
#define SOA_MEMBERS MEMBER(i) MEMBER(inner) MEMBER(struct_ptr) MEMBER(functor) MEMBER(derived_functor)
#include <soa-struct.inc>

extern soa::Array<Test, 100> TestArray;

int test() {
  TestArray[10].i = 42;
  TestArray[10].i += 42;
  TestArray[10].inner.char_ptr = "Hello";
  TestArray[10].struct_ptr = &s;

  (*TestArray[10].struct_ptr).i = 3;
  TestArray[10].struct_ptr->i = 4;

  TestArray[10].functor = Functor{42};
  TestArray[10].derived_functor.i = 8;

  return TestArray[10].functor() + TestArray[10].derived_functor();
}

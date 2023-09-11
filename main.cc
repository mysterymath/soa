#include <cstdio>
#include <soa.h>
#include <string.h>

struct Struct {
  char c;
} s;

/*
struct Functor {
  int x;
  int operator()() const { return x; }
  int dbl() const { return x + x; }
  int dbl_mut() { return x += x; }
};

struct DerivedFunctor : public Struct {
  int operator()() const { return i; }
};

struct OpaqueStruct {
  int i;
};

*/

struct Element {
  char c;
  int i;
  const char *ptr;
  Struct s;
  /*
  int array[3];
  Struct *struct_ptr;
  Functor functor;
  DerivedFunctor derived_functor;
  OpaqueStruct opaque;
  */
};

#define SOA_STRUCT Struct
#define SOA_MEMBERS MEMBER(c)
#include <soa-struct.inc>

/*
#define SOA_STRUCT Functor
#define SOA_MEMBERS MEMBER(x)
#include <soa-struct.inc>

#define SOA_STRUCT DerivedFunctor
#define SOA_MEMBERS MEMBER(i)
#include <soa-struct.inc>
*/

#define SOA_STRUCT Element
#define SOA_MEMBERS                                                            \
  MEMBER(c)                                                                    \
  MEMBER(i)                                                                    \
  MEMBER(ptr)                                                                  \
  MEMBER(s)                                                                    \
  //MEMBER(struct_ptr)                                                           \
  //MEMBER(functor)                                                              \
  M//EMBER(derived_functor)                                                      \
  //MEMBER(opaque)                                                               \
  //MEMBER(array)

#include <soa-struct.inc>

extern soa::Array<Element, 100> A;

int main() {
  // TODO: Name these examples.
  // TODO: Get rid of all but one of the gets.
  A[0].c = 0;
  printf("%d\n", A[0].c.get());

  A[1].c = 1;
  printf("%d\n", A[1].c.get());

  A[0].i = 1234;
  printf("%d\n", A[0].i.get());

  A[1].i = 4321;
  printf("%d\n", A[1].i.get());

  A[0].ptr = "Hello";
  printf("%s\n", A[0].ptr.get());
  printf("%c\n", *A[0].ptr);

  A[0].s = Struct{2};
  printf("%d\n", A[0].s.c.get());
  A[0].s.c = 3;
  printf("%d\n", A[0].s.c);

  /*
    TestArray[10].i += 42;
    TestArray[10].inner.char_ptr = "Hello";
    TestArray[10].struct_ptr = &s;

    (*TestArray[10].struct_ptr).i = 3;
    TestArray[10].struct_ptr->i = 4;

    TestArray[10].functor = Functor{42};
    TestArray[10].functor->dbl();
    TestArray[10].derived_functor.i = 8;
    TestArray[10].array[1] = 42;
    TestArray[10].array[2] = 43;
    TestArray[11].array[1] = 44;

    for (auto Entry : TestArray)
      ++Entry.i;

    int sum = 0;
    const auto &ArrayRef = TestArray;
    for (const auto &Entry : ArrayRef)
      sum += Entry.i;
  */
  return 0;
}

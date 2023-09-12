#include <cstdio>
#include <soa.h>
#include <string.h>

#if 0
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
#endif

int main() {
  static soa::Array<char, 10> CharArray;

  // SoA array elements can be assigned to.
  CharArray[0] = 1;

  // An element is a wrapper type, soa::Ptr<T>, that can implicitly be coerced
  // to the wrapped type, T.
  char v = CharArray[0];
  printf("%d\n", v);
  printf("%d\n", CharArray[0] + 42);

  // When applicable, mutation operations are defined on the wrapper type in
  // terms of binary operations. For example, the following:
  CharArray[0] += 42;
  // Lowers to CharArray[0] = CharArray[0] + 42
  printf("%d\n", CharArray[0].get());

  // Using them in contexts where implicit coercion doesn't occur (e.g. printf)
  // requires .get()
  printf("%d\n", CharArray[0].get());

  // Multi-byte types are stored strided by byte, i.e., the below type has all
  // 10 of the low bytes followed by all 10 of the high bytes. This is true for
  // any element type; the bytes are treated totally agnostically of what they
  // contain.
  static soa::Array<int, 10> IntArray;
  // The low byte will be at IntArray+1, and the high byte will be at
  // IntArray+11.
  IntArray[1] = 0x1234;
  printf("%x\n", IntArray[1].get());

  // This is true for array types as well. Indices into contained arrays must
  // be compile-time constants; otherwise, the compiler will instantiate a
  // temporary array of pointers, one per array element, and index through that.
  static soa::Array<char[3], 10> CharArrayArray = {{42}};
  printf("%d\n", CharArrayArray[0][0].get());
  // At CharArrayArray + 2 * 10 + 1
  CharArrayArray[1][2] = 43;
  printf("%d\n", CharArrayArray[1][2].get());

  static soa::Array<char[4][4], 10> CharMDArray;
  // At CharMDArray + ((2 * 4) + 3) * 10 + 1 (whew!)
  CharMDArray[1][2][3] = 44;
  // Note that the first index can still be variable, and this would still use
  // absolute indexed addressing. E.g., CharMDArray[x][2][3], but not
  // CharMDArray[2][x][3].
  printf("%d\n", CharMDArray[1][1][1].get());

  // Both indirection and the arrow operator are supported for pointer elements;
  // using these on the wrapper type forwards to the wrapped type.
  struct Struct {
    char c;
  } s = {45};
  static soa::Array<Struct *, 10> PtrArray;
  PtrArray[0] = &s;
  printf("%d\n", (*PtrArray[0]).c);
  printf("%d\n", PtrArray[0]->c);

  // For non-pointer types, the arrow operator produces a copy of the wrapped
  // element; this allows calling const methods on the type. Note that this
  // produces a copy; the this pointer may be different on each invocation.
  struct MemberFunctions {
    const MemberFunctions *whoami() const { return this; }
    char c() const { return 46; }
  };
  static soa::Array<MemberFunctions, 10> MemberFunctionsArray;
  printf("%p\n", MemberFunctionsArray[0]->whoami());
  printf("%p\n", MemberFunctionsArray[0]->whoami());
  printf("%d\n", MemberFunctionsArray[0]->c());

  // Call operators are forwarded to functors.
  struct Functor {
    char operator()(char x) const { return x; }
    char operator()(int x) const { return 2 * x; }
  };
  static soa::Array<Functor, 10> FunctorArray;
  // 5
  printf("%d\n", FunctorArray[0](static_cast<char>(5)));
  // 20
  printf("%d\n", FunctorArray[0](10));

#if 0
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
#endif
  return 0;
}

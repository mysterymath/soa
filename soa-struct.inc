// Including this header defines a specialization of soa::Ref for a structure
// type that makes the structure members accessible as their own individual
// soa::Ref objects. That is, given struct Foo { int a, b; }, soa::Ref<Foo>
// would have two public members, a, and b, both of type soa::Ref<int>.
//
// To use this, #define SOA_STRUCT to the struct type, and #define SOA_MEMBERS
// to a space-separated list of the struct member names, each wrapped with
// MEMBER(). Then, include the header. These preprocessors macros will be
// automatically undefined by this header.
//
// This header must be used before the first use of soa::Ref for the struct
// type.
//
// Example:
//   struct Foo { int a, b; }
//   #define SOA_STRUCT Foo
//   #define SOA_MEMBERS \
//     MEMBER(a)         \
//     MEMBER(b)
//   #include <soa-struct.inc>
//
//   soa::Array<Foo, 10> a;
//   a[5].b = 42;

template <>
struct soa::Ref<const SOA_STRUCT> : public soa::BaseConstRef<SOA_STRUCT> {
#define MEMBER(NAME) soa::Ref<const decltype(SOA_STRUCT::NAME)> NAME;
  SOA_MEMBERS
#undef MEMBER

  template <uint8_t N>
  [[clang::always_inline]] constexpr Ref(const uint8_t ByteArrays[][N],
                                         uint8_t Idx)
      : BaseConstRef<SOA_STRUCT>(ByteArrays, Idx)
#define MEMBER(NAME) , NAME(ByteArrays + offsetof(SOA_STRUCT, NAME), Idx)
            SOA_MEMBERS
#undef MEMBER
  {
  }
};

template <> struct soa::Ref<SOA_STRUCT> : public soa::BaseRef<SOA_STRUCT> {
#define MEMBER(NAME) soa::Ref<decltype(SOA_STRUCT::NAME)> NAME;
  SOA_MEMBERS
#undef MEMBER

  template <uint8_t N>
  [[clang::always_inline]] constexpr Ref(uint8_t ByteArrays[][N], uint8_t Idx)
      : BaseRef(ByteArrays, Idx)
#define MEMBER(NAME) , NAME(ByteArrays + offsetof(SOA_STRUCT, NAME), Idx)
            SOA_MEMBERS
#undef MEMBER
  {
  }

  [[clang::always_inline]] Ref &operator=(const SOA_STRUCT &Val) {
    BaseRef::operator=(Val);
    return *this;
  }
};

#undef SOA_STRUCT
#undef SOA_MEMBERS

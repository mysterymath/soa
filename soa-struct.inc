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

template <>
struct soa::Ref<std::add_const<SOA_STRUCT>::type>
    : public soa::BaseConstRef<SOA_STRUCT> {
#define MEMBER(NAME)                                                           \
  soa::Ref<std::add_const<decltype(SOA_STRUCT::NAME)>::type> NAME;
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

#undef SOA_STRUCT
#undef SOA_MEMBERS

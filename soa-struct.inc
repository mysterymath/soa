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

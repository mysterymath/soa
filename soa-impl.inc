template <> struct soa::Ref<SOA_TYPE> : public soa::BaseRef<SOA_TYPE> {
#define MEMBER(NAME) soa::Ref<decltype(SOA_TYPE::NAME)> NAME;
  SOA_MEMBERS
#undef MEMBER

  template <uint8_t N>
  [[clang::always_inline]] constexpr Ref(uint8_t ByteArrays[][N],
                                         uint8_t Idx)
      : BaseRef<SOA_TYPE>(ByteArrays, Idx)
#define MEMBER(NAME) , NAME(ByteArrays + offsetof(SOA_TYPE, NAME), Idx)
        SOA_MEMBERS
#undef MEMBER
  {}
};

template <> struct soa::ConstRef<SOA_TYPE> : public soa::BaseConstRef<SOA_TYPE> {
#define MEMBER(NAME) soa::ConstRef<decltype(SOA_TYPE::NAME)> NAME;
  SOA_MEMBERS
#undef MEMBER

  template <uint8_t N>
  [[clang::always_inline]] constexpr ConstRef(const uint8_t ByteArrays[][N],
                                              uint8_t Idx)
      : BaseConstRef<SOA_TYPE>(ByteArrays, Idx)
#define MEMBER(NAME) , NAME(ByteArrays + offsetof(SOA_TYPE, NAME), Idx)
        SOA_MEMBERS
#undef MEMBER
        {}
};

#undef SOA_TYPE
#undef SOA_MEMBERS
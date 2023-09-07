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

template <> struct soa::Ref<std::add_const<SOA_TYPE>::type> : public soa::BaseConstRef<SOA_TYPE> {
#define MEMBER(NAME) soa::Ref<std::add_const<decltype(SOA_TYPE::NAME)>::type> NAME;
  SOA_MEMBERS
#undef MEMBER

  template <uint8_t N>
  [[clang::always_inline]] constexpr Ref(const uint8_t ByteArrays[][N],
                                         uint8_t Idx)
      : BaseConstRef<SOA_TYPE>(ByteArrays, Idx)
#define MEMBER(NAME) , NAME(ByteArrays + offsetof(SOA_TYPE, NAME), Idx)
        SOA_MEMBERS
#undef MEMBER
        {}
};

#undef SOA_TYPE
#undef SOA_MEMBERS

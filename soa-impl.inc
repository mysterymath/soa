template <> struct soa::Ref<SOA_TYPE> {
#define MEMBER(NAME) soa::Types<decltype(SOA_TYPE::NAME)>::Ref NAME;
  SOA_MEMBERS
#undef MEMBER

  [[clang::always_inline]] Ref<SOA_TYPE> &operator=(const SOA_TYPE &Other) {
#define MEMBER(NAME) NAME = Other.NAME;
    SOA_MEMBERS
#undef MEMBER
    return *this;
  }
};

template <> struct soa::ConstRef<SOA_TYPE> {
#define MEMBER(NAME) soa::Types<decltype(SOA_TYPE::NAME)>::ConstRef NAME;
  SOA_MEMBERS
#undef MEMBER
};

template <uint8_t N> class soa::Array<SOA_TYPE, N> {
#define MEMBER(NAME) Types<decltype(SOA_TYPE::NAME)>::Array<N> NAME;
  SOA_MEMBERS
#undef MEMBER

public:
  [[clang::always_inline]] constexpr Array(
      std::initializer_list<SOA_TYPE> Entries = {}) {
    uint8_t Idx = 0;
    for (const auto &Entry : Entries)
      (*this)[Idx++] = Entry;
  }

  [[clang::always_inline]] constexpr Ref<SOA_TYPE> operator[](uint8_t Idx) {
    return {
#define MEMBER(NAME) NAME[Idx],
        SOA_MEMBERS
#undef MEMBER
    };
  }

  [[clang::always_inline]] constexpr ConstRef<SOA_TYPE>
  operator[](uint8_t Idx) const {
    return {
#define MEMBER(NAME) NAME[Idx],
        SOA_MEMBERS
#undef MEMBER
    };
  }

  [[clang::always_inline]] constexpr uint8_t size() const { return N; }
};

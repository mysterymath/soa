#ifndef _SOA_H
#define _SOA_H

#include <cstdint>
#include <initializer_list>
#include <type_traits>
#include <utility>

namespace std {
template <typename T>              // For lvalues (T is T&),
T &&forward(T &&param)             // take/return lvalue refs.
{                                  // For rvalues (T is T),
  return static_cast<T &&>(param); // take/return rvalue refs.
}
} // namespace std

namespace soa {

template <typename T> struct BaseConstRef {
  static_assert(!std::is_volatile_v<T>, "volatile types are not supported");
  static_assert(std::is_trivial_v<T>, "non-trivial types are unsupported");
  static_assert(std::is_standard_layout_v<T>,
                "non-standard layout types are unsupported");
  static_assert(std::alignment_of_v<T> == 1, "aligned types are not supported");

protected:
  const uint8_t *BytePtrs[sizeof(T)];

public:
  template <uint8_t N>
  [[clang::always_inline]] constexpr BaseConstRef(const uint8_t ByteArrays[][N],
                                                  uint8_t Idx) {
#pragma unroll
    for (uint8_t ByteIdx = 0; ByteIdx < sizeof(T); ++ByteIdx)
      BytePtrs[ByteIdx] = &ByteArrays[ByteIdx][Idx];
  }

  [[clang::always_inline]] T get() const { return static_cast<T>(*this); }

  [[clang::always_inline]] constexpr operator T() const {
    uint8_t Bytes[sizeof(T)];
#pragma unroll
    for (uint8_t Idx = 0; Idx < sizeof(T); ++Idx)
      Bytes[Idx] = *BytePtrs[Idx];
    return *reinterpret_cast<const T *>(Bytes);
  }

  template <typename... ArgsT>
  [[clang::always_inline]] auto operator()(ArgsT &&...Args) const -> auto {
    return get()(std::forward(Args)...);
  }
};

template <typename T> struct BaseRef : public BaseConstRef<T> {
  [[clang::always_inline]] uint8_t **byte_ptrs() const {
    return const_cast<uint8_t **>(BaseConstRef<T>::BytePtrs);
  }

public:
  template <uint8_t N>
  [[clang::always_inline]] constexpr BaseRef(uint8_t ByteArrays[][N],
                                             uint8_t Idx)
      : BaseConstRef<T>(ByteArrays, Idx) {}

  [[clang::always_inline]] BaseRef &operator=(const T &Val) {
    auto *Bytes = reinterpret_cast<const uint8_t *>(&Val);
#pragma unroll
    for (uint8_t Idx = 0; Idx < sizeof(T); ++Idx)
      *byte_ptrs()[Idx] = Bytes[Idx];
    return *this;
  }

  template <typename... ArgsT>
  [[clang::always_inline]] auto operator()(ArgsT &&...Args) -> auto {
    return BaseConstRef<T>::get()(std::forward(Args)...);
  }
};

template <typename T, typename Enable = void> struct Ref;

template <typename T>
struct Ref<T,
           std::enable_if_t<!std::is_const_v<T> && !std::is_arithmetic_v<T> &&
                            !std::is_pointer_v<T>>> : public BaseRef<T> {
  template <uint8_t N>
  [[clang::always_inline]] constexpr Ref(uint8_t ByteArrays[][N], uint8_t Idx)
      : BaseRef<T>(ByteArrays, Idx) {}

  [[clang::always_inline]] Ref &operator=(const T &Val) {
    BaseRef<T>::operator=(Val);
    return *this;
  }
};

template <typename T>
struct Ref<T,
           std::enable_if_t<!std::is_const_v<T> &&
                            (std::is_arithmetic_v<T> || std::is_pointer_v<T>)>>
    : public BaseRef<T> {
  template <uint8_t N>
  [[clang::always_inline]] constexpr Ref(uint8_t ByteArrays[][N], uint8_t Idx)
      : BaseRef<T>(ByteArrays, Idx) {}

  [[clang::always_inline]] Ref &operator=(const T &Val) {
    BaseRef<T>::operator=(Val);
    return *this;
  }

  // SFINAE means that these following operator overloads only exist if
  // the corresponding operation is defined on T.

  template <typename U>
  [[clang::always_inline]] Ref &operator+=(const U &Right) {
    *this = *this + Right;
    return *this;
  }

  template <typename U>
  [[clang::always_inline]] Ref &operator-=(const U &Right) {
    *this = *this - Right;
    return *this;
  }

  template <typename U>
  [[clang::always_inline]] Ref &operator*=(const U &Right) {
    *this = *this * Right;
    return *this;
  }

  template <typename U>
  [[clang::always_inline]] Ref &operator/=(const U &Right) {
    *this = *this / Right;
    return *this;
  }

  template <typename U>
  [[clang::always_inline]] Ref &operator%=(const U &Right) {
    *this = *this % Right;
    return *this;
  }

  template <typename U>
  [[clang::always_inline]] Ref &operator^=(const U &Right) {
    *this = *this ^ Right;
    return *this;
  }

  template <typename U>
  [[clang::always_inline]] Ref &operator&=(const U &Right) {
    *this = *this & Right;
    return *this;
  }

  template <typename U>
  [[clang::always_inline]] Ref &operator|=(const U &Right) {
    *this = *this | Right;
    return *this;
  }

  template <typename U>
  [[clang::always_inline]] Ref &operator<<=(const U &Right) {
    *this = *this << Right;
    return *this;
  }

  template <typename U>
  [[clang::always_inline]] Ref &operator>>=(const U &Right) {
    *this = *this >> Right;
    return *this;
  }

  [[clang::always_inline]] T operator->() const { return *this; }

  template <typename = void> [[clang::always_inline]] Ref &operator++() {
    *this += 1;
    return *this;
  }
  template <typename = void> [[clang::always_inline]] Ref &operator--() {
    *this -= 1;
    return *this;
  }

  template <typename = void> [[clang::always_inline]] T operator++(int) {
    T old = *this;
    ++*this;
    return old;
  }

  template <typename = void> [[clang::always_inline]] T operator--(int) {
    T old = *this;
    --*this;
    return old;
  }
};

template <typename T>
struct Ref<T, std::enable_if_t<std::is_const_v<T>>> : public BaseConstRef<T> {
  template <uint8_t N>
  [[clang::always_inline]] constexpr Ref(const uint8_t ByteArrays[][N],
                                         uint8_t Idx)
      : BaseConstRef<T>(ByteArrays, Idx) {}
};

template <typename T, uint8_t N> class Array;

template <typename T, uint8_t N> class ArrayConstIterator {
  friend class Array<T, N>;

protected:
  const Array<T, N> &A;
  uint8_t Idx;

  ArrayConstIterator(const Array<T, N> &A, uint8_t Idx) : A(A), Idx(Idx) {}

public:
  [[clang::always_inline]] Ref<const T> operator*() const { return A[Idx]; }

  [[clang::always_inline]] ArrayConstIterator &operator++() {
    ++Idx;
    return *this;
  }

  bool operator==(const ArrayConstIterator &Other) const {
    return &A == &Other.A && Idx == Other.Idx;
  }
  bool operator!=(const ArrayConstIterator &Other) const {
    return !(*this == Other);
  }
};

template <typename T, uint8_t N>
class ArrayIterator : public ArrayConstIterator<T, N> {
  friend class Array<T, N>;

  using ArrayConstIterator<T, N>::A;
  using ArrayConstIterator<T, N>::Idx;

  ArrayIterator(Array<T, N> &A, uint8_t Idx)
      : ArrayConstIterator<T, N>(A, Idx) {}

public:
  [[clang::always_inline]] Ref<T> operator*() const {
    return const_cast<soa::Array<T, N> &>(A)[Idx];
  }

  [[clang::always_inline]] ArrayIterator &operator++() {
    ArrayConstIterator<T, N>::operator++();
    return *this;
  }
};

template <typename T, uint8_t N> class Array {
  static_assert(!std::is_volatile_v<T>, "volatile types are not supported");
  static_assert(std::is_trivial_v<T>, "non-trivial types are unsupported");
  static_assert(std::is_standard_layout_v<T>,
                "only standard layout types are supported");
  static_assert(std::alignment_of_v<T> == 1, "aligned types are not supported");

  uint8_t ByteArrays[sizeof(T)][N];

public:
  [[clang::always_inline]] constexpr Array(
      std::initializer_list<T> Entries = {}) {
    uint8_t Idx = 0;
    for (const T Entry : Entries)
      (*this)[Idx++] = Entry;
  }

  template <uint8_t M>
  [[clang::always_inline]] constexpr Array(const Array<T, M> &Other) {
    memcpy(ByteArrays, Other.ByteArrays, sizeof(Other.ByteArrays));
  }

  [[clang::always_inline]] constexpr Ref<T> operator[](uint8_t Idx) {
    return {ByteArrays, Idx};
  }
  [[clang::always_inline]] constexpr Ref<const T>
  operator[](uint8_t Idx) const {
    return {ByteArrays, Idx};
  }

  [[clang::always_inline]] constexpr ArrayConstIterator<T, N> begin() const {
    return {*this, 0};
  }
  [[clang::always_inline]] constexpr ArrayConstIterator<T, N> end() const {
    return {*this, size()};
  }

  [[clang::always_inline]] constexpr ArrayIterator<T, N> begin() {
    return {*this, 0};
  }
  [[clang::always_inline]] constexpr ArrayIterator<T, N> end() {
    return {*this, size()};
  }

  [[clang::always_inline]] constexpr uint8_t size() const { return N; }
};

template <typename T, uint8_t N> struct Array;

} // namespace soa

#endif // _SOA_H

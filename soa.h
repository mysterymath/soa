#ifndef _SOA_H
#define _SOA_H

#include <cstdint>
#include <initializer_list>
#include <new>
#include <type_traits>
#include <utility>

namespace soa {

/// An array implemented as a struct of arrays.
///
/// This data structure is logically similar to a C-style array, but each byte
/// of the array's element type is represented as a seperate array of bytes. In
/// other words, if a C array were represented as a multidimimensional array of
/// bytes, uint8_t Bytes[ArrayIdx][ByteIdx], this data structure is its
/// transpose, uint8_t Bytes[ByteIdx][ArrayIdx]. This can provide more efficient
/// addressing on the 6502, since its 8-bit indexed addressing modes preclude
/// automatic scaling needed to efficiently access traditional C arrays.
///
/// Because the representation of the elements is broken apart, only trivial
/// types with standard layouts are supported (think C-style types).
///
/// This class loses its performance benefit, and indeed, may hurt performance,
/// if accessed through a pointer. All uses of this object should be by
/// explicitly naming the definition. This tends to imply that the definition
/// is global, but it may also be used as a local variable in functions that
/// the compiler can prove do not recurse.
template <typename T, uint8_t N> class Array;

/// Pointer to an array element.
///
/// This proxy provides access to the contents of a specific array element. If
/// the type is arithmetic or a pointer, it can be used in numeric expressions
/// with the full range of operators, including the various assignment
/// operators. If it is a struct, by default the struct type is opaque. It's
/// members are not directly accessible, and the struct can only be read or
/// written as a whole. A specialization can be generated to allow for member
/// access using the soa-struct.inc header; see that header for details.
///
/// Pointers should not be stored more than temporarily, and they should not
/// be used as arguments to functions. This may cause them to acually take on
/// their logical representation at runtime (an array of pointers, one per
/// byte), which is typically worse than using a regular C-style array.
///
/// A number of helpers are added to make the type more ergonomic, that is, more
/// like a reference. First, the type is implicitly convertable to and from the
/// wrapped type, the wrapped type is also directly assignable to the pointer.
/// Arithmetic assignment operators are implemented in terms of binary
/// arithmetic on the wrapped type wherever possible. If the wrapped type is a
/// pointer, the arrow operator functions on the wrapped type. Otherwise, the
/// arrow operator provides access to the wrapped type itself. This operates by
/// making a copy of the value and writing it back if modified, so take care
/// when using this.
template <typename T> struct Ptr;

/// Base class for pointer to array elements.
template <typename T> class BasePtr {
  static_assert(!std::is_volatile_v<T>, "volatile types are not supported");
  static_assert(std::is_trivial_v<T>, "non-trivial types are unsupported");
  static_assert(std::is_standard_layout_v<T>,
                "non-standard layout types are unsupported");
  static_assert(std::alignment_of_v<T> == 1, "aligned types are not supported");

protected:
  // Pointers to array elements are represented as an array of pointers to
  // each byte. This keeps the representation agnostic of the size of the
  // original array.
  const uint8_t *BytePtrs[sizeof(T)];

public:
  template <uint8_t N>
  [[clang::always_inline]] constexpr BasePtr(const uint8_t ByteArrays[][N],
                                             uint8_t Idx) {
#pragma unroll
    for (uint8_t ByteIdx = 0; ByteIdx < sizeof(T); ++ByteIdx)
      BytePtrs[ByteIdx] = &ByteArrays[ByteIdx][Idx];
  }

  template <uint8_t N>
  [[clang::always_inline]] constexpr BasePtr(uint8_t ByteArrays[][N],
                                             uint8_t Idx) {
#pragma unroll
    for (uint8_t ByteIdx = 0; ByteIdx < sizeof(T); ++ByteIdx)
      BytePtrs[ByteIdx] = &ByteArrays[ByteIdx][Idx];
  }

  /// Return the value of the pointer.
  ///
  /// This provides a more readable syntax than casting for contexts where the
  /// implicit conversion to T doesn't trigger, e.g., in printf.
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
    return static_cast<const T>(get())(std::forward<ArgsT>(Args)...);
  }

  template <typename Q = T>
  [[clang::always_inline]] std::enable_if_t<!std::is_const_v<Q>, const Ptr<T> &>
  operator=(const T &Val) {
    auto *Bytes = reinterpret_cast<const uint8_t *>(&Val);
#pragma unroll
    for (uint8_t Idx = 0; Idx < sizeof(T); ++Idx)
      *const_cast<uint8_t *>(BytePtrs[Idx]) = Bytes[Idx];
    return *static_cast<Ptr<T> *>(this);
  }

  template <typename Q = T>
  [[clang::always_inline]] std::enable_if_t<std::is_pointer_v<Q>, const T>
  operator->() const {
    return *this;
  }
  template <typename Q = T>
  [[clang::always_inline]] std::enable_if_t<std::is_pointer_v<Q>, T>
  operator->() {
    return *this;
  }

private:
  class ConstWrapper {
    const T V;

  public:
    [[clang::always_inline]] ConstWrapper(const Ptr<T> &P) : V(P) {}
    [[clang::always_inline]] const T *operator->() const { return &V; }
  };

public:
  template <typename Q = T>
  [[clang::always_inline]] std::enable_if_t<!std::is_pointer_v<Q>, ConstWrapper>
  operator->() const {
    return *static_cast<const Ptr<T> *>(this);
  }

  template <typename U>
  [[clang::always_inline]] Ptr<T> &operator+=(const U &Right) {
    *this = *this + Right;
    return *static_cast<Ptr<T> *>(this);
  }

  template <typename U>
  [[clang::always_inline]] Ptr<T> &operator-=(const U &Right) {
    *this = *this - Right;
    return *static_cast<Ptr<T> *>(this);
  }

  template <typename U>
  [[clang::always_inline]] Ptr<T> &operator*=(const U &Right) {
    *this = *this * Right;
    return *static_cast<Ptr<T> *>(this);
  }

  template <typename U>
  [[clang::always_inline]] Ptr<T> &operator/=(const U &Right) {
    *this = *this / Right;
    return *static_cast<Ptr<T> *>(this);
  }

  template <typename U>
  [[clang::always_inline]] Ptr<T> &operator%=(const U &Right) {
    *this = *this % Right;
    return *static_cast<Ptr<T> *>(this);
  }

  template <typename U>
  [[clang::always_inline]] Ptr<T> &operator^=(const U &Right) {
    *this = *this ^ Right;
    return *static_cast<Ptr<T> *>(this);
  }

  template <typename U>
  [[clang::always_inline]] Ptr<T> &operator&=(const U &Right) {
    *this = *this & Right;
    return *static_cast<Ptr<T> *>(this);
  }

  template <typename U>
  [[clang::always_inline]] Ptr<T> &operator|=(const U &Right) {
    *this = *this | Right;
    return *static_cast<Ptr<T> *>(this);
  }

  template <typename U>
  [[clang::always_inline]] Ptr<T> &operator<<=(const U &Right) {
    *this = *this << Right;
    return *static_cast<Ptr<T> *>(this);
  }

  template <typename U>
  [[clang::always_inline]] Ptr<T> &operator>>=(const U &Right) {
    *this = *this >> Right;
    return *static_cast<Ptr<T> *>(this);
  }

  [[clang::always_inline]] Ptr<T> &operator++() {
    *this += 1;
    return *static_cast<Ptr<T> *>(this);
  }
  [[clang::always_inline]] Ptr<T> &operator--() {
    *this -= 1;
    return *static_cast<Ptr<T> *>(this);
  }

  [[clang::always_inline]] T operator++(int) {
    T old = *this;
    ++*this;
    return *static_cast<Ptr<T> *>(this);
  }

  [[clang::always_inline]] T operator--(int) {
    T old = *this;
    --*this;
    return *static_cast<Ptr<T> *>(this);
  }
};

template <typename T> class Ptr : public BasePtr<T> {
public:
  using BasePtr<T>::BasePtr;
  using BasePtr<T>::operator=;
};

template <typename T, uint8_t N> class Ptr<T[N]> {
  static_assert(!std::is_volatile_v<T>, "volatile types are not supported");
  static_assert(std::is_trivial_v<T>, "non-trivial types are unsupported");
  static_assert(std::is_standard_layout_v<T>,
                "non-standard layout types are unsupported");
  static_assert(std::alignment_of_v<T> == 1, "aligned types are not supported");

  uint8_t PtrStorage[sizeof(Ptr<T>[N])];
  [[clang::always_inline]] Ptr<T> *ptrs() {
    return reinterpret_cast<Ptr<T> *>(PtrStorage);
  }

public:
  template <uint8_t M>
  [[clang::always_inline]] constexpr Ptr(const uint8_t ByteArrays[][M],
                                         uint8_t Idx) {
#pragma unroll
    for (uint8_t ArrayIdx = 0; ArrayIdx < N; ++ArrayIdx)
      new (&ptrs()[ArrayIdx]) Ptr<T>(ByteArrays + ArrayIdx * sizeof(T), Idx);
  }

  template <uint8_t M>
  [[clang::always_inline]] constexpr Ptr(uint8_t ByteArrays[][M], uint8_t Idx) {
#pragma unroll
    for (uint8_t ArrayIdx = 0; ArrayIdx < N; ++ArrayIdx)
      new (&ptrs()[ArrayIdx]) Ptr<T>(ByteArrays + ArrayIdx * sizeof(T), Idx);
  }

  Ptr<const T> operator[](uint8_t Idx) const { return ptrs()[Idx]; }
  Ptr<T> operator[](uint8_t Idx) { return ptrs()[Idx]; }
};

template <typename T, uint8_t N> class ArrayConstIterator {
  friend class Array<T, N>;

protected:
  const Array<T, N> &A;
  uint8_t Idx;

  [[clang::always_inline]] ArrayConstIterator(const Array<T, N> &A, uint8_t Idx)
      : A(A), Idx(Idx) {}

public:
  [[clang::always_inline]] Ptr<const T> operator*() const { return A[Idx]; }

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

  [[clang::always_inline]] ArrayIterator(Array<T, N> &A, uint8_t Idx)
      : ArrayConstIterator<T, N>(A, Idx) {}

public:
  [[clang::always_inline]] Ptr<T> operator*() const {
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
  // Note: This partially duplicates the logic in Ptr::operator=, but works for
  // types like multidimensional arrays where assignment isn't defined.
  [[clang::always_inline]] constexpr Array(std::initializer_list<T> Entries) {
    uint8_t Idx = 0;
    for (const T &Entry : Entries) {
      const auto *Bytes = reinterpret_cast<const uint8_t *>(&Entry);
#pragma unroll
      for (uint8_t ByteIdx = 0; ByteIdx < sizeof(T); ++ByteIdx)
        ByteArrays[ByteIdx][Idx] = Bytes[ByteIdx];
      ++Idx;
    }
  }

  // Arrays cannot be assigned with operator= above, so provide a specific out
  // to initialize them with nested initializer lists.
  [[clang::always_inline]] constexpr Array() = default;

  template <uint8_t M>
  [[clang::always_inline]] constexpr Array(const Array<T, M> &Other) {
    memcpy(ByteArrays, Other.ByteArrays, sizeof(Other.ByteArrays));
  }

  [[clang::always_inline]] constexpr Ptr<T> operator[](uint8_t Idx) {
    return {ByteArrays, Idx};
  }
  [[clang::always_inline]] constexpr Ptr<const T>
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

} // namespace soa

#endif // _SOA_H

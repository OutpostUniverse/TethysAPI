
#pragma once

#include "Tethys/Common/Types.h"

#include <initializer_list>
#include <cstdlib>
#include <cstdio>

#if defined(_WIN32)
# include <intrin.h>
#endif

namespace Tethys::TethysUtil {

// =====================================================================================================================
/// Returns the size in elements of a C-style array.
template <typename T, size_t N>  constexpr size_t ArrayLen(const T (&array)[N]) { return N; }

/// Tests if any bit flag is set in mask.
template <typename M, typename F> bool BitFlagTest(M mask, F flag) { return (mask & flag) != 0; }

/// Tests if all bit flags are set in mask.
template <typename M, typename F> bool BitFlagsTest(M mask, F flags) { return (mask & flags) == flags; }

/// Sets or unsets a bit flag.
template <typename M, typename F> void SetBitFlag(M& out, F flag, bool on) { out ^= (out ^ (on ? flag : 0)) & flag; }

// =====================================================================================================================
/// Iterates forward on a bitmask.  Found bit must be masked out after each call in a loop.
inline bool GetNextBit(
  uint32*  pIndex,
  uint32   mask)
{
  bool result = false;

#if defined(_WIN32) && defined(_M_IX86)
  *pIndex = _tzcnt_u32(mask);
#elif defined(_WIN32)
  _BitScanForward(reinterpret_cast<unsigned long*>(pIndex), mask);
#elif defined(__GNUC__)
  *pIndex = __builtin_ctz(mask);
#else
  if (mask != 0) {
    uint32 index = 0;
    for (; (mask & 1) == 0; mask >>= 1, ++index);
    *pIndex = index;
  }
#endif

  if (mask != 0) {
    result = (mask != 0);
  }

  return result;
}

/// Type erasure reference accessor class for immutable, possibly temporary, array-like types.
template <typename T>
class Span {
public:
  constexpr Span(std::nullptr_t = nullptr)               : Span(nullptr,             0)                   { }
  constexpr Span(const T* pSrc, size_t length)           : pData_(pSrc),             length_(length)      { }
  constexpr Span(std::initializer_list<T> list)          : Span(list.begin(),        list.size())         { }
  template <size_t N>  constexpr Span(const T (&arr)[N]) : Span(&arr[0],             N)                   { }
  template
    <typename U, typename = std::enable_if_t<std::is_same<decltype(std::declval<const U>().data()), const T*>::value>>
  constexpr Span(const U& stlContainer)                  : Span(stlContainer.data(), stlContainer.size()) { }

  constexpr const T* begin()  const { return pData_;           }
  constexpr const T* cbegin() const { return pData_;           }
  constexpr const T* end()    const { return pData_ + length_; }
  constexpr const T* cend()   const { return pData_ + length_; }

  constexpr const T* data()     const { return pData_; }
  constexpr operator const T*() const { return pData_; }
  template <typename I>  constexpr const T& operator[](I index) const { return *(pData_ + static_cast<size_t>(index)); }

  constexpr size_t size()  const { return  length_;       }
  constexpr bool   empty() const { return (length_ == 0); }

private:
  const T*  pData_;
  size_t    length_;
};

} // Tethys::TethysUtil


#pragma once

#include "Tethys/Common/Memory.h"
#include <type_traits>

namespace Tethys::TethysUtil {

// =====================================================================================================================
/// Helper function to get the base load address of the module containing pAddress.
/// Note that heap memory does not belong to a module, in which case this function returns NULL.
inline HMODULE GetModuleFromAddress(
  const void*  pAddress,
  bool         addReference = false)
{
# ifndef GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
#   define GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS        0x4
# endif
# ifndef GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT
#   define GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT  0x2
# endif

  const DWORD Flags = (GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                       (addReference ? 0 : GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT));
  HMODULE hModule = NULL;
  return (GetModuleHandleExA(Flags, static_cast<LPCSTR>(pAddress), &hModule) == TRUE) ? hModule : NULL;
}

// =====================================================================================================================
/// RAII class wrapping a DLL.
class Library {
  template <typename T>  using FnToPfn = TethysImpl::FnToPfn<T>;
public:
   Library(std::nullptr_t) : hModule_(GetModuleHandleA(nullptr)), ownHandle_(false) { }
   Library(const char*    pModuleName, bool load = true)
     : hModule_((load && pModuleName) ? LoadLibraryA(pModuleName) : GetModuleHandleA(pModuleName)), ownHandle_(load) { }
   Library(const wchar_t* pModuleName, bool load = true)
     : hModule_((load && pModuleName) ? LoadLibraryW(pModuleName) : GetModuleHandleW(pModuleName)), ownHandle_(load) { }
   Library(HMODULE        hModule,     bool strongRef = true)
     : hModule_(GetModuleFromAddress(hModule, strongRef)), ownHandle_(strongRef) { }
   Library(const Library& src) : Library(hModule_, ownHandle_) { }
  ~Library() {
    if (ownHandle_ && (hModule_ != NULL)) {
      FreeLibrary(hModule_);
      hModule_ = NULL;
    }
  }

  bool IsLoaded() const { return (hModule_ != NULL); }  ///< Is module loaded?

  /// Gets an export by name.
  template <typename T = void*>
  auto Get(const char* pName) const { return IsLoaded() ? FnToPfn<T>(GetProcAddress(hModule_, pName)) : nullptr; }

  /// Gets an export by ordinal.
  template <typename T = void*>  auto Get(uint16 ordinal) const { return Get((char*)(static_cast<uintptr>(ordinal))); }

  /// Gets an export via an output parameter and returns true if it exists.
  template <typename T = void*, typename NameOrOrdinal = const char*>
  bool Get(NameOrOrdinal pName, FnToPfn<T>* pPfnOut) const { return (pPfnOut != nullptr) && (*pPfnOut = Get(pName)); }

  /// Gets an export by name, with built-in init-once.  @note Each template instance is expected to be unique.
  template <auto Pfn, typename PfnType = decltype(Pfn), typename NameOrOrdinal = const char*>
  auto Get(NameOrOrdinal pName) const { static auto pfn = Get<PfnType>(pName);  return pfn; }

  HMODULE GetHandle() const { return hModule_; }  ///< Gets the OS module handle.
  operator HMODULE()  const { return hModule_; }  ///< Implicitly converts to the OS module handle.

private:
  HMODULE hModule_;
  bool    ownHandle_;
};

} // Tethys::TethysUtil

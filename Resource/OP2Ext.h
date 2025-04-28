/**
 ***********************************************************************************************************************
 * @file  Op2ext.h
 * @brief OP2Ext is the OPU patch's mod loader & interface.
 ***********************************************************************************************************************
 */

#pragma once

#include "Tethys/Common/Memory.h"
#include "Tethys/Common/Library.h"

#include <vector>
#include <string>
#include <string_view>
#include <filesystem>
#include <memory>

namespace Tethys::OP2Ext {

// =====================================================================================================================
/// Get the OP2Ext Library object.
inline const TethysUtil::Library& GetOP2Ext() { static TethysUtil::Library extLib("op2ext.dll");  return extLib; }

///@{ op2ext log-to-file functions.
inline void      Log(std::string_view msg) { return GetOP2Ext().Get<&Log,      void(const char*)>(__func__)(&msg[0]); }
inline void LogError(std::string_view msg) { return GetOP2Ext().Get<&LogError, void(const char*)>(__func__)(&msg[0]); }
inline void LogDebug(std::string_view msg) { return GetOP2Ext().Get<&LogDebug, void(const char*)>(__func__)(&msg[0]); }
///@}

// =====================================================================================================================
// --------------------------------------------------- Legacy C APIs ---------------------------------------------------
inline size_t GetGameDir_s(char* buffer, size_t bufferSize)
  { return GetOP2Ext().Get<&GetGameDir_s>(__func__)(buffer, bufferSize); }
inline size_t GetConsoleModDir_s(char* buffer, size_t bufferSize)
  { return GetOP2Ext().Get<&GetConsoleModDir_s>(__func__)(buffer, bufferSize); }

inline size_t GetLoadedModuleCount() { return GetOP2Ext().Get<&GetLoadedModuleCount>(__func__)(); }
inline size_t GetLoadedModuleName(size_t moduleIndex, char* buffer, size_t bufferSize)
  { return GetOP2Ext().Get<&GetLoadedModuleName>(__func__)(moduleIndex, buffer, bufferSize); }

inline size_t GetModuleDirectoryCount() { return GetOP2Ext().Get<&GetModuleDirectoryCount>(__func__)(); }
inline size_t GetModuleDirectory(size_t moduleIndex, char* buffer, size_t bufferSize)
  { return GetOP2Ext().Get<&GetModuleDirectory>(__func__)(moduleIndex, buffer, bufferSize); }

// ------------------------------------------------------ Detail -------------------------------------------------------
namespace Impl {
template <auto* FnGet, size_t BufferSize = 1024, typename... Args>
std::string GetStringHelper(Args... args) {
  char buf[BufferSize] = "";
  if (size_t allocSize = FnGet(args..., &buf[0], sizeof(buf));  allocSize > sizeof(buf)) {
    std::unique_ptr<char[]> pBuf(new char[allocSize]);
    return (FnGet(args..., pBuf.get(), allocSize) == 0) ? pBuf.get() : "";
  }
  else return std::string(buf);
}

template <typename T, auto* FnCount, auto* FnGet>
auto GetModListHelper(size_t count = FnCount()) {
  static std::vector<T> out;
  if (out.empty()) {
    out.reserve(count);
    for (size_t i = 0; i < count; out.emplace_back(GetStringHelper<FnGet>(i++)));
  }
  return out;
}
}  // Impl

// =====================================================================================================================
// ----------------------------------------------------- Game/Mods -----------------------------------------------------
inline bool IsModuleLoaded(std::string_view pModuleName)
  { return GetOP2Ext().Get<&IsModuleLoaded, bool(const char*)>(__func__)(pModuleName.data()); }

inline std::filesystem::path GetGameDir()
  { static const auto out = Impl::GetStringHelper<&GetGameDir_s>();        return out; }
inline std::filesystem::path GetConsoleModDir()
  { static const auto out = Impl::GetStringHelper<&GetConsoleModDir_s>();  return out; }

inline std::vector<std::string> GetLoadedModuleNames()
  { return Impl::GetModListHelper<std::string, &GetLoadedModuleCount, &GetLoadedModuleName>(); }

inline std::vector<std::filesystem::path> GetModuleDirectories()
  { return Impl::GetModListHelper<std::filesystem::path, &GetModuleDirectoryCount, &GetModuleDirectory>(); }

/// Registers a new .vol archive with ResManager.
inline void AddVol(std::filesystem::path file)
  { return GetOP2Ext().Get<void(const char*)>("AddVolToList")(file.string().data()); }

} // Tethys::OP2Ext

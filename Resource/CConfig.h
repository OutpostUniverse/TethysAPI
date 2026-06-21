
#pragma once

#include "Tethys/Common/Memory.h"
#include <string>
#include <string_view>
#include <algorithm>
#include <cstdlib>

namespace Tethys {

/// Tri-state enum describing a setting's default / forced-off / forced-on state.
enum class SettingToggle : int {
  Default = -1,
  Off     =  0,
  On      =  1
};

/// Outpost2.ini settings singleton class.  Wraps GetPrivateProfileStringA(), etc. Win32 APIs.
class CConfig : public OP2Class<CConfig> {
public:
  using Toggle = SettingToggle;

   CConfig(std::string_view filename) { InternalCtor<0x410750, const char*>(filename.data()); }
  ~CConfig()                          { Thunk<0x410820>();                                    }

  static CConfig* GetInstance() { return OP2Mem<0x547090, CConfig*>(); }

  /// Returns true if setting string is one of {"1", "true", "yes", "on"}, false otherwise.
  bool GetBool(std::string_view section, std::string_view key, bool defaultVal = false) {
    std::string str = GetString(section, key, defaultVal ? "1" : "0");
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    const bool falseCases = (str == "0") || (str == "false") || (str == "no") || (str == "none") || (str == "off");
    return (!falseCases) || GetInt(section, key, defaultVal);
  }

  /// Returns Default if setting string is one of {"", "default", "def", "-1"}, On if GetBool() == true, Off otherwise.
  Toggle GetToggle(std::string_view section, std::string_view key, Toggle defaultVal = Toggle::Default) {
    std::string str = GetString(section, key, (defaultVal != Toggle::Default) ? "0" : "");
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    return (str == "") ? Toggle::Default : GetBool(section, key, (defaultVal == Toggle::On)) ? Toggle::On : Toggle::Off;
  }

  std::string GetString(std::string_view section, std::string_view key, std::string_view defaultVal = "")
    { char buf[1024] = "";  return std::string(buf, GetString(section, key, &buf[0], sizeof(buf), defaultVal)); }

  int GetInt(std::string_view section, std::string_view key, int defaultVal)
    { return Thunk<0x410870, int(const char*, const char*, int)>(section.data(), key.data(), defaultVal); }

  ibool SetInt(std::string_view section, std::string_view key, auto value)
    { return Thunk<0x410830, ibool(const char*, const char*, int)>(section.data(), key.data(), int(value)); }

  ibool SetString(std::string_view section, std::string_view key, std::string_view value)
    { return Thunk<0x410890, ibool(const char*, const char*, const char*)>(section.data(), key.data(), value.data()); }

  // ------------------------------------------------ Internal methods -------------------------------------------------
public:
  int   CopyIniFile(std::string_view existingFile) { return Thunk<0x410C40, &$::CopyIniFile>(existingFile.data()); }
  ibool FileExists()                               { return Thunk<0x410C10, &$::FileExists>();                     }
  void  Flush()                                    { return Thunk<0x410C00>();                                     }

  int GetInt(std::string_view key, int defaultVal)
    { return Thunk<0x4109B0, int(const char*, int)>(key.data(), defaultVal); }

  // ** TODO Update pOut/bufferSize to use Span instead?
  int GetString(
    std::string_view section, std::string_view key, char* pOut, uint32 bufferSize, std::string_view defaultVal = "")
  {
    return Thunk<0x4108B0, int(const char*, const char*, char*, uint32, const char*)>(
      section.data(), key.data(), pOut, bufferSize, defaultVal.data());
  }
  int GetString(std::string_view key, char* pOut, uint32 bufferSize, std::string_view defaultVal = "") {
    return Thunk<0x410B30, int(const char*, char*, uint32, const char*)>(
      key.data(), pOut, bufferSize, defaultVal.data());
  }

  ibool SetInt(std::string_view key, auto value)
    { return Thunk<0x4108E0, int(const char*, int)>(key.data(), int(value)); }

  ibool SetString(std::string_view key, std::string_view value)
    { return Thunk<0x410A70, ibool(const char*, const char*)>(key.data(), value.data()); }

public:
  char iniPath_[MAX_PATH];  // ** TODO Can this somehow be made long path-aware or unicode-aware?
};
static_assert(sizeof(CConfig) == 260, "Incorrect CConfig size.");

inline CConfig& g_configFile = *CConfig::GetInstance();

namespace TethysAPI {
  using CConfig       = Tethys::CConfig;
  using SettingToggle = Tethys::SettingToggle;
}

} // Tethys

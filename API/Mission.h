/**
 ***********************************************************************************************************************
 * @file  Mission.h
 * @brief Contains the definitions for mission-related enums, API callbacks, and helper macros to define mission info.
 ***********************************************************************************************************************
 */

/**
 ***********************************************************************************************************************
 * Mission DLLs should always include this header.
 * 
 * Mission DLLs are required to export mission description data, which can be done with either of the following macros:
 *   EXPORT_OP2_MULTIPLAYER_SCRIPT("Mission Name", missionType, numHumans, "map_file.map", "tech_file.txt"(, numAIs))
 *     ** OR **
 *   EXPORT_OP2_MISSION_SCRIPT("Mission Name", missionType, numPlayers, "map_file.map", "tech_file.txt", maxTechLevel,
 *                             isUnitMission(, numMultiplayerAIs))
 *
 * Mission DLLs may (optionally) define the follow functions to interface with the game:
 *   MISSION_API ibool InitProc() { return 1; }          // Set up bases, triggers, etc. here.  Return false = error.
 *   MISSION_API void  AIProc()   {           }          // Gets called every 4 ticks during gameplay.
 *   MISSION_API void  GetSaveRegions(SaveRegion* pSave) // Single-player maps set pSave to point at a buffer to save.
 *     { pSave->pData = nullptr;  pSave->size = 0; }
 *   MISSION_API void  MyLegacyTriggerFunction() { }     // Passed by name to Create*Trigger().  Called when the trigger
 *                                                       // fires (only once if oneShot=true).
 *
 * The following extended APIs are introduced in OPU mod 1.4.0:
 *   MISSION_API ibool OnLoadMission(OnLoadMissionArgs*     pArgs) { return 1; } // On DLL load.   Return 0 = error.
 *   MISSION_API ibool OnUnloadMission(OnUnloadMissionArgs* pArgs) { return 1; } // On DLL unload. Return 0 = error.
 *   MISSION_API void  OnEndMission(OnEndArgsMission*       pArgs) {           } // On mission win/lose/abort.
 *   MISSION_API void  OnChat(OnChatArgs*                   pArgs) { } // Called when any player sends a chat message.
 *   MISSION_API void  OnCreateUnit(OnCreateUnitArgs*       pArgs) { } // Called when a unit/entity is created.
 *   MISSION_API void  OnDestroyUnit(OnDestroyUnitArgs*     pArgs) { } // Called when a unit/entity is destroyed.
 *   MISSION_API void  MyTriggerFunction(OnTriggerArgs*     pArgs) { } // Trigger function with access to extended info,
 *                                                                     // such as the source Trigger.
 * The following extended APIs are introduced in OPU mod 1.4.2:
 *   MISSION_API ibool OnSaveGame(OnSaveGameArgs*           pArgs) { return 1; } // Saving the game. Return 0 = error.
 *   MISSION_API ibool OnLoadSavedGame(OnLoadSavedGameArgs* pArgs) { return 1; } // Loading a save.  Return 0 = error.
 *   MISSION_API void  OnDamageUnit(OnDamageUnitArgs*       pArgs) { } // Called when a unit is damaged.
 *   MISSION_API void  OnTransferUnit(OnTransferUnitArgs*   pArgs) { } // Called when a unit is transferred.
 *   MISSION_API void  OnGameCommand(OnGameCommandArgs*     pArgs) { } // Called when processing a game command packet.
 ***********************************************************************************************************************
 */

#pragma once

#include "Tethys/Common/Types.h"

namespace Tethys {

struct MissionResults;
struct CommandPacket;
class  StreamIO;

namespace TethysAPI {

#ifndef MISSION_API
# define MISSION_API DLLAPI
#endif  // MISSION_API

/// Macro that defines and exports all global data required for Outpost 2 to interface with a mission DLL.
/// numMultiplayerAIs may be provided as an optional argument.
/// Most multiplayer missions can use the simplified @ref EXPORT_OP2_MULTIPLAYER_SCRIPT macro instead.
#define EXPORT_OP2_MISSION_SCRIPT(                                                                   \
    nameDesc, missionType, numPlayers, mapFilename, techFilename, maxTechLevel, isUnitMission, ...)  \
  MISSION_API char      LevelDesc[]    = nameDesc;                                                   \
  MISSION_API char      MapName[]      = mapFilename;                                                \
  MISSION_API char      TechtreeName[] = techFilename;                                               \
  MISSION_API ModDesc   DescBlock      = { missionType, numPlayers, maxTechLevel, isUnitMission };   \
  MISSION_API ModDescEx DescBlockEx    = { __VA_ARGS__ };

/// Macro that defines a typical Outpost 2 multiplayer mission DLL, with max tech level = 12 and isUnitMission = false.
/// numMultiplayerAIs may be provided as an optional argument.
#define EXPORT_OP2_MULTIPLAYER_SCRIPT(nameDesc, missionType, numHumans, mapFilename, techFilename, ...)  \
  EXPORT_OP2_MISSION_SCRIPT(nameDesc, missionType, numHumans, mapFilename, techFilename, 12, false, __VA_ARGS__)

/// Enum defining mission types.
enum class MissionType : int {
  // Single-player mission types
  Campaign1       =  1,
  Campaign2       =  2,
  Campaign3       =  3,
  Campaign4       =  4,
  Campaign5       =  5,
  Campaign6       =  6,
  Campaign7       =  7,
  Campaign8       =  8,
  Campaign9       =  9,
  Campaign10      = 10,
  Campaign11      = 11,
  Campaign12      = 12,

  Colony          = -1,
  AutoDemo        = -2,
  Tutorial        = -3,

  // Multiplayer mission types
  LandRush        = -4,
  SpaceRace       = -5,
  ResourceRace    = -6,
  Midas           = -7,
  LastOneStanding = -8
};

/// Defines mission information and dependencies.
struct ModDesc {
  constexpr ModDesc() : missionType(), numPlayers(), maxTechLevel(), unitMission() { }
  constexpr ModDesc(MissionType missionType, int numPlayers, int maxTechLevel = 12, ibool unitMission = false)
    : missionType(missionType), numPlayers(numPlayers), maxTechLevel(maxTechLevel), unitMission(unitMission) { }
  constexpr ModDesc(int missionType, int numPlayers, int maxTechLevel = 12, ibool unitMission = false)
    : missionType{missionType}, numPlayers(numPlayers), maxTechLevel(maxTechLevel), unitMission(unitMission) { }

  MissionType missionType;      ///< Campaign mission (positive values) or other mission type (negative values).
  int         numPlayers;       ///< Number of players (1-6), including AIs on single-player maps, but excluding
                                ///  them on multiplayer maps;  @see AIModDescEx::numMultiplayerAISlots.
  int         maxTechLevel;     ///< Maximum tech level.  Set to 12 to enable all techs for standard tech trees.
  ibool       unitMission;      ///< Set to 1 to disable most reports.  Suitable for unit-only missions.
};

/// Used for multiplayer missions only.
struct ModDescEx {
  /// The number of extra player slots to initialize for multiplayer AI (besides gaia).
  /// Not setting this correctly can cause weird bugs with things like alliances, player color/colony, etc.
  int numMultiplayerAIs;

  int field_04;
  int field_08;
  int field_0C;
  int field_10;
  int field_14;
  int field_18;
  int field_1C;
};

/// Used by GetSaveRegions() to determine the regions that need to be read/written.
struct SaveRegion {
  void*  pData;
  size_t size;
};


// Args used by extended mission callbacks.  Requires OPU mod 1.4.0+.

/// Info passed to trigger callbacks.  @see Trigger.h.
struct OnTriggerArgs;

/// Info passed to OnLoadMission() user callback.
struct OnLoadMissionArgs {
  size_t structSize;  ///< Size of this structure.
};

/// Info passed to OnUnloadMission() user callback.
struct OnUnloadMissionArgs {
  size_t structSize;  ///< Size of this structure.
};

/// Info passed to OnEndMission() user callback.
struct OnEndMissionArgs {
  size_t          structSize;       ///< Size of this structure.
  MissionResults* pMissionResults;  ///< Mission results info.
};

/// Info passed to OnSaveGame() user callback.
struct OnSaveGameArgs {
  size_t    structSize;  ///< Size of this structure.
  StreamIO* pSavedGame;  ///< Saved game file write stream, positioned at the end of normal data.
};

/// Info passed to OnLoadSavedGame() user callback.
struct OnLoadSavedGameArgs {
  size_t    structSize;  ///< Size of this structure.
  StreamIO* pSavedGame;  ///< Saved game file read stream, positioned at the end of normal data.
};

/// Info passed to OnChat() user callback.
struct OnChatArgs {
  size_t structSize;  ///< Size of this structures.
  char*  pText;       ///< Chat message (writable).
  size_t bufferSize;  ///< Size of chat message buffer.
  int    playerNum;   ///< Source player number.
};

/// Info passed to OnGameCommand() user callback.
struct OnGameCommandArgs {
  size_t         structSize;  ///< Size of this structure.
  CommandPacket* pPacket;     ///< Pointer to command packet data (writable up to header size + dataLength).
  int            playerNum;   ///< Player number of sender.
};


struct OnCreateUnitArgs;    ///< Info passed to OnCreateUnit()   user callback.  @see Unit.h.
struct OnDestroyUnitArgs;   ///< Info passed to OnDestroyUnit()  user callback.  @see Unit.h.
struct OnDamageUnitArgs;    ///< Info passed to OnDamageUnit()   user callback.  @see Unit.h.
struct OnTransferUnitArgs;  ///< Info passed to OnTransferUnit() user callback.  @see Unit.h.


///@{ Type aliases showing the function signature for each mission API callback.
using PfnInitProc        = ibool(CDECL*)();
using PfnAIProc          =  void(CDECL*)();
using PfnGetSaveRegions  =  void(CDECL*)(SaveRegion*);
using PfnLegacyOnTrigger =  void(CDECL*)();

// The following require OPU mod 1.4.0.
using PfnOnTrigger       =  void(CDECL*)(OnTriggerArgs*);
using PfnOnLoadMission   = ibool(CDECL*)(OnLoadMissionArgs*);
using PfnOnUnloadMission = ibool(CDECL*)(OnUnloadMissionArgs*);
using PfnOnEndMission    =  void(CDECL*)(OnEndMissionArgs*);
using PfnOnChat          =  void(CDECL*)(OnChatArgs*);
using PfnOnCreateUnit    =  void(CDECL*)(OnCreateUnitArgs*);
using PfnOnDestroyUnit   =  void(CDECL*)(OnDestroyUnitArgs*);

// The following require OPU mod 1.4.2.
using PfnOnSaveGame      = ibool(CDECL*)(OnSaveGameArgs*);
using PfnOnLoadSavedGame = ibool(CDECL*)(OnLoadSavedGameArgs*);
using PfnOnGameCommand   =  void(CDECL*)(OnGameCommandArgs*);
using PfnOnDamageUnit    =  void(CDECL*)(OnDamageUnitArgs*);
using PfnOnTransferUnit  =  void(CDECL*)(OnTransferUnitArgs*);
///@}

} // TethysAPI
} // Tethys

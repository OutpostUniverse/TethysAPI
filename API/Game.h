/**
 ***********************************************************************************************************************
 * @file  Game.h
 * @brief Defines the Game class which controls the overall game environment.
 ***********************************************************************************************************************
 */

#pragma once

#include "Tethys/Common/Memory.h"
#include "Tethys/Game/GameImpl.h"
#include "Tethys/Game/Random.h"
#include "Tethys/Game/BlightLavaManager.h"
#include "Tethys/Resource/SoundManager.h"
#include "Tethys/UI/GameFrame.h"
#include "Tethys/API/Location.h"
#include "Tethys/API/Player.h"
#include "Tethys/API/Unit.h"
#include <string_view>

namespace Tethys {

class TFileDialog;
class GameNetLayer;

/// Defines initial unit rotations and path-finding directions.
enum class UnitDirection : int {
  East = 0,
  SouthEast,
  South,
  SouthWest,
  West,
  NorthWest,
  North,
  NorthEast,
};

namespace TethysAPI {

using UnitDirection = Tethys::UnitDirection;

/// Defines mine resource types for Game::CreateMine().
enum class MineType : int {
  RandomOre = int(OreType::Random),  ///< 70% chance of CommonOre or 30% chance of RareOre.
  CommonOre = int(OreType::Common),  ///< Common ore.
  RareOre   = int(OreType::Rare),    ///< Rare ore.  @note Requires Rare Ore Processing to be visible.
  MagmaVent = -2,                    ///< Magma vent.  Robo-Miners can build Magma Wells, roughly equal to a 2 bar rare.
  Fumarole  = -3,                    ///< Fumarole.  GeoCons can build Geothermal Plants.
};

/// Defines meteor sizes for Game::CreateMeteor().
enum class MeteorSize : int {
  Random = -1,
  Small,
  Medium,
  Large
};

/// Defines marker graphic types for Game::CreateMarker().
enum class MarkerType : int {
  Circle = 0,  ///< Circular marker
  DNA,         ///< DNA strand
  Beaker,      ///< Beaker
};

/// Tri-state enum defining force-enable/disable toggle state.
enum class ToggleState : int {
  Default = 0,
  On      = 1,
  Off     = 2
};


/// Public interface for accessing global game state, creating units, adding game messages, RNG, etc. (wraps GameImpl)
class Game : public OP2Class<Game> {
public:
  static int LocalPlayer() { return GetImpl()->localPlayer_;     }  ///< Returns the local player index.
  static int NumPlayers()  { return GetImpl()->numPlayers_;      }  ///< Returns number of human and AI players.
  static int NumHumans()   { return GetImpl()->numHumanPlayers_; }  ///< Returns number of human players.

  static StartupFlags GetStartFlags() { return GetImpl()->gameStartInfo_.startupFlags; }

  static bool UsesDayNight()     { return GetStartFlags().dayNightEnabled;    }  ///< Day/night enabled?
  static bool UsesMorale()       { return GetStartFlags().moraleEnabled;      }  ///< Morale steady disabled?
  static bool CanHaveDisasters() { return GetStartFlags().disastersEnabled;   }  ///< Disasters enabled?
  static bool CanAllowCheats()   { return GetStartFlags().cheatsEnabled;      }  ///< Cheats enabled?
  static int  InitialUnits()     { return GetStartFlags().numInitialVehicles; }  ///< # of units to start with (0-12).

  static void SetDaylightEverywhere(bool on) { SetGameOpt(GameOpt::DaylightEverywhere, on); } ///< Sets full daylight.
  static void SetDaylightMoves(bool      on) { SetGameOpt(GameOpt::DaylightMoves,      on); } ///< Sets day/night cycle.

  static int Tick() { return GetImpl()->tick_; }  ///< Current time.  Tick is the smallest slice of game time.
  static int Time() { return Tick() / 4;       }  ///< Current tick / 4 (most game processing is at this interval).
  static int Mark() { return Tick() / 100;     }  ///< Current mark (tick / 100, as displayed in in-game message log).

  /// Outputs a global game sound.
  static void AddGameSound(SoundID soundID, int toPlayerNum = AllPlayers)
    { SoundManager::GetInstance()->AddGameSound(soundID, (toPlayerNum == -1) ? -1 : (1u << toPlayerNum)); }

  /// Outputs a game sound at the specified map tile location.
  static void AddMapSound(SoundID  soundID, Location location)
    { SoundManager::GetInstance()->AddMapSound(location.GetPixelX(), location.GetPixelY(), soundID); }

  /// Outputs a game message at the specified map pixel coordinates.  @note (0, -1) = no associated coordinates.
  static void AddMessage(std::string_view msg, SoundID soundID, int toPlayerNum, int pixelX, int pixelY) {
    if ((toPlayerNum == AllPlayers) || (toPlayerNum == LocalPlayer()))
      MessageLog::GetInstance()->AddMessage(pixelX, pixelY, msg.data(), soundID);
  }

  /// Outputs a game message at the specified map tile location.  @note Location{ } = no associated coordinates.
  static void AddMessage(std::string_view msg, SoundID soundID, int toPlayerNum = AllPlayers, Location tile = { }) {
    const auto pixel = tile ? tile.GetPixel() : POINT{ 0, -1 };
    AddMessage(msg.data(), soundID, toPlayerNum, pixel.x, pixel.y);
  }

  /// Outputs a game message at the specified Unit's location.
  static void AddMessage(std::string_view msg, SoundID soundID, int toPlayerNum, Unit owner)
    { const auto px = owner.GetPixel();  AddMessage(msg.data(), soundID, toPlayerNum, px.x, px.y); }

  /// Creates a Unit on the map.
  static Unit CreateUnit(
    MapID type, Location where, int ownerNum, MapID weaponCargo = {}, UnitDirection rotation = {}, bool lightsOn = true)
  {
    Unit u;
    OP2Thunk<0x478780, ibool FASTCALL(Unit&, MapID, Location, int, MapID, UnitDirection)>(
      u, type, where, ownerNum, weaponCargo, rotation);
    if (u.IsVehicle() && lightsOn) u.DoSetLights(true);
    return u;
  }

  /// Creates a mining beacon, magma vent, or fumarole on the map.
  /// @note @ref yield and @ref variant are only meaningful when @ref type is RandomOre, CommonOre, or RareOre.
  static Unit CreateMine(
    Location location, MineType type = MineType::RandomOre,
    OreYield yield = OreYield::Random, OreVariant variant = OreVariant::Random);

  /// Creates wreckage that grants the given tech ID when turned in at a spaceport.  Tech ID must be within 8000-12095.
  static Unit CreateWreckage(Location location, int techID, bool isDiscovered = false) {
    return ((techID >= 8000) && (techID <= 12095) && OP2Thunk<0x4789F0, ibool FASTCALL(int, int, int, ibool)>(
      location.x, location.y, techID, isDiscovered)) ? *(_Player::GetInstance(6)->GetBeacons()) : Unit();
  }

  /// Places a marker decal on the map.
  static Unit CreateMarker(Location location, MarkerType type)
    { Unit u; OP2Thunk<0x478BB0, int FASTCALL(Unit&, int, int, int)>(u, location.x, location.y, int(type)); return u; }

  /// Creates a wall on the given tile.
  static void CreateWall(MapID type, Location location)
    { OP2Thunk<0x478AA0, ibool FASTCALL(int, int, int, MapID)>(location.x, location.y, 0, type); }

  /// Creates a block of walls over the given area.
  static void CreateWall(MapID type, const MapRect& area) {
    for (int y = area.y1; y <= area.y2; ++y)
      for (int x = area.x1; x <= area.x2; CreateWall(type, Location(x++, y)));
  }

  static void CreateTube(Location       location) { CreateWall(MapID::Tube, location); }  ///< Creates a tube.
  static void CreateTube(const MapRect& area)     { CreateWall(MapID::Tube, area);     }  ///< Creates a block of tubes.

  /// Let morale vary according to colony state & events for the specified player.  @note toPlayerNum: -1 = all players
  static void FreeMoraleLevel(int player = AllPlayers) { SetGameOpt(GameOpt::FreeMoraleLevel, player); }
  /// Forces morale level for the specified player.  @note toPlayerNum: -1 = all players
  static void ForceMoraleLevel(MoraleLevel moraleLevel, int player = AllPlayers)
    { SetGameOpt(GameOpt(uint16(GameOpt::ForceMoraleExcellent) + uint16(moraleLevel)), player); }

  static void SetSeed(uint32 randNumSeed) { Random::GetInstance()->SetSeed(randNumSeed); } ///< Set random number seed.
  static int  GetRand(int    range)       { return Random::GetInstance()->Rand(range);   } ///< Returns # in [0, range).

  /// Sets random number seed for local RNG instance (not synced over the network).
  static void SetLocalSeed(uint32 randNumSeed) { Random::GetLocalInstance()->SetSeed(randNumSeed); }
  /// Returns # in [0, range) using local RNG instance (not synced over the network).
  static int  GetLocalRand(int    range)       { return Random::GetLocalInstance()->Rand(range);   }

  /// Load saved game.  @note Saved game names default to: "SGAME?.OP2" file name format.
  static void LoadGame(std::string_view savedGameName)
    { return GameFrame::GetInstance()->PostDelayedLoadMessage(savedGameName.data()); }

  ///@{ Sets the game music play list.  @see SongID
  static void SetMusicPlayList(int numSongs, int repeatStartIndex, const SongID* pSongIDList)
    { return MusicManager::GetInstance()->SetMusicPlaylist(numSongs, repeatStartIndex, pSongIDList); }
  static void SetMusicPlayList(TethysUtil::Span<SongID> songIDList, int repeatStartIndex = 0)
    { return SetMusicPlayList(songIDList.size(), repeatStartIndex, songIDList.data()); }
  ///@}

  /// Search aligned 8x8 blocks, for the block with the greatest weight.
  /// @note The target location is at the block center (+3, +3)
  /// @note Targets first found block of highest (non-negative) weight, or the first block if all blocks have negative
  ///       weight
  /// @note Target player military units weigh 64, non-target player military units weigh -32, and non-target player
  ///       non-military units weigh 1.
  static Location FindEMPMissileTarget(const MapRect& searchArea, int targetPlayerNum) {
    return OP2Thunk<0x478480, Location FASTCALL(int, int, int, int, int)>(
      searchArea.x1, searchArea.x2, searchArea.y1, searchArea.y2, targetPlayerNum);
  }

  /// Launches an EMP missile (owned by the specified player) and returns a Unit reference to it.
  static Unit CreateEMPMissile(Location target, Location launchArea = { }, int ownerNum = 6) {
    return DisasterThunk<0x478420, Disaster* FASTCALL(int, int, int, int, int)>(
      false, launchArea.x, launchArea.y, ownerNum, target.x, target.y);
  }

  /// Creates a meteor and returns a Unit reference to it.
  /// @note Size 0 = large, 1 = medium, 2 = small, -1 = random
  static Unit CreateMeteor(Location where, MeteorSize size, bool immediate = false)
    { return DisasterThunk<0x4783B0, Disaster* FASTCALL(int, int, MeteorSize)>(immediate, where.x, where.y, size); }

  /// Creates an earthquake and returns a Unit reference to it.
  static Unit CreateEarthquake(Location where, int magnitude, bool immediate = false)
    { return DisasterThunk<0x478320, Disaster* FASTCALL(int, int, int)>(immediate, where.x, where.y, magnitude); }

  /// Creates an eruption and returns a Unit reference to it.
  static Unit CreateEruption(Location where, int spreadSpeed, bool immediate = false)
    { return DisasterThunk<0x4782E0, Disaster* FASTCALL(int, int, int)>(immediate, where.x, where.y, spreadSpeed); }

  /// Creates an electrical storm and returns a Unit reference to it.
  static Unit CreateLightning(Location start, Location end, int duration, bool immediate = false) {
    return DisasterThunk<0x4783E0, Disaster* FASTCALL(int, int, int, int, int)>(
      immediate, start.x, start.y, duration, end.x, end.y);
  }

  /// Creates a vortex and returns a Unit reference to it.
  static Unit CreateTornado(Location start, Location end, int duration, bool immediate = false) {
    return DisasterThunk<0x478350, Disaster* FASTCALL(int, int, int, int, int, int)>(
      false, start.x, start.y, duration, end.x, end.y, immediate);
  }

  /// Creates the Blight.  Always immediate.  @note Use spreadSpeed = -1 to retain the current spread speed.
  static void CreateBlight(Location where, int spreadSpeed = -1)
    { if (spreadSpeed >= 0) SetBlightSpeed(spreadSpeed); OP2Thunk<0x476EA0, void FASTCALL(Location, ibool)>(where, 1); }
  /// Removes Blight at the specified map tile coordinates.
  static void UnsetBlight(Location where) { return OP2Thunk<0x476EA0, void FASTCALL(Location, ibool)>(where, 0); }

  /// Sets lava spread speed.  @note This also gets set by eruptions when triggered.
  static void SetLavaSpeed(int   spreadSpeed) { LavaManager::GetInstance()->SetLavaSpeed(spreadSpeed);     }
  /// Sets blight spread speed.
  static void SetBlightSpeed(int spreadSpeed) { BlightManager::GetInstance()->SetSpreadSpeed(spreadSpeed); }

  /// Gets the global GameImpl object instance.
  static GameImpl* GetImpl() { return GameImpl::GetInstance(); }

  /// Toggles the unlimited resources cheat.
  static void SetUnlimitedResources(bool on) { GetImpl()->unlimitedResources_ = on; }
  /// Toggles the all unit types unlocked cheat.  @note Disables research at labs.
  static void SetProduceAll(bool         on) { GetImpl()->produceAll_         = on; }
  /// Toggles the quad damage cheat.
  static void SetQuadDamage(bool         on) { GetImpl()->quadDamage_         = on; }
  /// Toggles the fast units cheat.  @warning This may make the game unstable.
  static void SetFastUnits(bool          on) { GetImpl()->fastUnits_          = on; }
  /// Toggles the instant build cheat.
  static void SetFastProduction(bool     on) { GetImpl()->fastProduction_     = on; }
  /// Toggles the force all units visible cheat.
  static void SetAllUnitsVisible(bool    on) { GetImpl()->allUnitsVisible_    = on; }

  /// Toggles force RCC-enhanced pathfinding on/off.
  static void SetForceRCCPathFinding(ToggleState state) {
    GetImpl()->forceEnableRCC_  = (state == ToggleState::On);
    GetImpl()->forceDisableRCC_ = (state == ToggleState::Off);
    if (state != ToggleState::Default)
      for (int i = 0; i < MaxPlayers; GetImpl()->GetPlayer(i++)->rccOperational_ = (state == ToggleState::On));
  }

  /// Toggles debug morale logging.
  static void SetLogMoraleToFile(bool on) { GetImpl()->logMorale_     = on; }
  /// Toggles debug overlay displaying unit paths.
  static void SetShowUnitPaths(bool   on) { GetImpl()->showUnitPaths_ = on; }

  /// Locally issues a ctGameOpt packet (without a "cheated game!" alert).
  static void SetGameOpt(GameOpt variableID, uint16 param1 = 0, uint16 param2 = 0) {
    CommandPacket packet{ CommandType::GameOpt, sizeof(GameOptCommand) };
    packet.data.gameOpt = { 0, variableID, param1, param2 };
    _Player::GetInstance(LocalPlayer())->ProcessCommandPacket(packet);
    DismissCheatedGameAlert();
  }

  /// Locally clears the "cheated game!" message that appears when a GameOpt is set after tick 0.
  static void DismissCheatedGameAlert() { GetImpl()->tickOfLastSetGameOpt_ = 0; }

private:
  /// @internal  Helper function for creating (possibly immediate) disasters and returning a Unit reference to them.
  template <uintptr Address, typename Fn, typename... Args>
  static Unit DisasterThunk(bool immediate, Args... args) {
    auto*const pUnit = OP2Thunk<Address, Fn>(args...);
    if ((pUnit != nullptr) && immediate) pUnit->StartDisaster();
    return Unit(pUnit);
  }

public:
  uint8 field_00;
};

// =====================================================================================================================
inline Unit Game::CreateMine(
  Location    location,
  MineType    type,
  OreYield    yield,
  OreVariant  variant)
{
  const MapID mapID = (type == MineType::MagmaVent) ? MapID::MagmaVent :
                      (type == MineType::Fumarole)  ? MapID::Fumarole  :
                                                      MapID::MiningBeacon;

  if ((mapID == MapID::MiningBeacon) && (yield == OreYield::Random) && (variant != OreVariant::Random)) {
    // We must pre-generate the random yield in order to be able to look up the internal variant number.
    const int result = GetRand(10);  // 20% chance of Bar3, 60% chance of Bar2, 20% chance of Bar1
    yield = (result <= 1) ? OreYield::Bar3 : (result <= 7) ? OreYield::Bar2 : OreYield::Bar1;
  }
  const int varNum = MineManager::GetInstance()->GetVariantNum(yield, variant);

  return OP2Thunk<0x478940, ibool FASTCALL(MapID, int, int, MineType, OreYield, int)>(
    mapID, location.x, location.y, max(type, MineType::RandomOre), yield, varNum) ?
      *(_Player::GetInstance(6)->GetBeacons()) : Unit();
}

} // TethysAPI
} // Tethys

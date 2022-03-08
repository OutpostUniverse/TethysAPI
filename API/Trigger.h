/**
 ***********************************************************************************************************************
 * @file  Trigger.h
 * @brief Contains the definitions of the Trigger classes and factory functions to create them.
 * @note  Passing nullptr for trigger function requires OP2 v1.4.1+.
 * @note  The Trigger API will be changed substantially.  The current interface should be considered a placeholder.
 ***********************************************************************************************************************
 */

#pragma once

#include "Tethys/Game/MapObjectType.h"
#include "Tethys/API/ScGroup.h"
#include "Tethys/API/Unit.h"
#include <string_view>

namespace Tethys::TethysAPI {

/// Comparison modes used by various trigger creation functions.
enum class CompareMode : int {
  Equal = 0,
  LowerEqual,
  GreaterEqual,
  Lower,
  Greater,
};

/// Trigger resource types used by CreateResourceTrigger.
enum class TriggerResource : int {
  Food = 0,
  CommonOre,
  RareOre,
  Kids,
  Workers,
  Scientists,
  Colonists,
};

/// Damage types used by CreateDamagedTrigger.  Refers to % of FightGroup that has been destroyed.
enum class TriggerDamage : int {
  _100 = 1,  ///< 100% damaged
  _75,       ///<  75% damaged
  _50,       ///<  50% damaged
};


/// Exported class wrapping TriggerImpl.
class Trigger : public ScStub {
  using $ = Trigger;
public:
  using ScStub::ScStub;

        TriggerImpl* GetImpl()       { return IsValid() ? TriggerImpl::GetInstance(id_) : nullptr; }
  const TriggerImpl* GetImpl() const { return IsValid() ? TriggerImpl::GetInstance(id_) : nullptr; }

  ibool HasFired(int playerNum) { return Thunk<0x478CC0, &$::HasFired>(playerNum); }  ///< @note Do not use AllPlayers.
};


/// Info passed to trigger callbacks.  (1.4.0+)
struct OnTriggerArgs {
  size_t        structSize;       ///< Size of this structure.
  Trigger       trigger;          ///< Trigger that was fired.
  
  // The following fields require OPU mod 1.4.2.
  PlayerBitmask triggeredBy;      ///< Bitmask of players currently activating this trigger.
  PlayerBitmask prevTriggeredBy;  ///< Bitmask of players that were previously activating this trigger.
};


/// Creates a victory condition (wraps another Trigger).
inline Trigger CreateVictoryCondition(
  Trigger condition, std::string_view text, bool oneShot = false, bool enabled = true)
{
  return OP2Thunk<0x479930, Trigger FASTCALL(ibool, ibool, const Trigger&, const char*)>(
    enabled, oneShot, condition, text.data());
}

/// Creates a failure condition (wraps another Trigger).
inline Trigger CreateFailureCondition(Trigger condition, bool enabled = true)
  { return OP2Thunk<0x479980, Trigger FASTCALL(ibool, ibool, const Trigger&, const char*)>(enabled, 0, condition, ""); }


/// Set trigger.  Used to collect a number of other triggers into a single trigger output.  Can be used for e.g. any 3
/// in a set of 5 objectives.
template <typename... Ts>
Trigger CreateSetTrigger(
  std::string_view triggerFunction, int needed = sizeof...(Ts), bool oneShot = false, bool enabled = true,
  Ts... triggers)
{
  return OP2Thunk<0x4794E0, Trigger CDECL(ibool, ibool, int, int, const char*, ...)>(
    enabled, oneShot, sizeof...(Ts), needed, triggerFunction.data(), triggers...);
}


// --------------------------------------------- Typical victory triggers ----------------------------------------------

/// Creates a trigger used for victory condition in Last One Standing and later part of Land Rush.
inline Trigger CreateLastOneStandingTrigger(
  std::string_view triggerFunction = "", bool oneShot = true, bool enabled = true)
    { return OP2Thunk<0x478F30, Trigger FASTCALL(ibool, ibool, const char*)>(enabled, oneShot, triggerFunction.data());}

/// Creates a trigger used for victory condition in Space Race.
inline Trigger CreateSpaceRaceTrigger(
  std::string_view triggerFunction = "", int playerNum = AllPlayers, bool oneShot = true, bool enabled = true)
{
  return OP2Thunk<0x479260, Trigger FASTCALL(ibool, ibool, int, const char*)>(
    enabled, oneShot, playerNum, triggerFunction.data());
}

/// Creates a trigger used for victory condition in Midas.
inline Trigger CreateMidasTrigger(
  int time, std::string_view triggerFunction = "", bool oneShot = true, bool enabled = true)
{
  return OP2Thunk<0x479300, Trigger FASTCALL(ibool, ibool, int, const char*)>(
    enabled, oneShot, time, triggerFunction.data());
}

// -------------------- Research and resource count triggers  (typically used in campaign missions) --------------------

/// Resource Race and campaign objectives
inline Trigger CreateResourceTrigger(
  TriggerResource resourceType, CompareMode compare, int refAmount,
  std::string_view triggerFunction = "", int playerNum = AllPlayers, bool oneShot = false, bool enabled = true)
{
  return OP2Thunk<0x478DE0, Trigger FASTCALL(ibool, ibool, TriggerResource, int, int, CompareMode, const char*)>(
    enabled, oneShot, resourceType, refAmount, playerNum, compare, triggerFunction.data());
}

/// Creates a tech research trigger.
inline Trigger CreateResearchTrigger(
  int techID,
  std::string_view triggerFunction = "", int playerNum = AllPlayers, bool oneShot = false, bool enabled = true)
{
  return OP2Thunk<0x478E90, Trigger FASTCALL(ibool, ibool, int, int, const char*)>(
    enabled, oneShot, techID, playerNum, triggerFunction.data());
}

/// Creates a structure kit count trigger.
inline Trigger CreateKitTrigger(
  MapID structureKitType, int refCount,
  std::string_view triggerFunction = "", int playerNum = AllPlayers, bool oneShot = false, bool enabled = true)
{
  return OP2Thunk<0x4791C0, Trigger FASTCALL(ibool, ibool, int, MapID, int, const char*)>(
    enabled, oneShot, playerNum, structureKitType, refCount, triggerFunction.data());
}

// ------------------------------------------------ Unit count triggers ------------------------------------------------

/// Creates a unit/cargo type count trigger.
inline Trigger CreateCountTrigger(
  MapID unitType, MapID cargoOrWeapon, CompareMode compare, int refCount,
  std::string_view triggerFunction = "", int playerNum = AllPlayers, bool oneShot = false, bool enabled = true)
{
  return OP2Thunk<0x479110, Trigger FASTCALL(ibool, ibool, int, MapID, MapID, int, CompareMode, const char*)>(
    enabled, oneShot, playerNum, unitType, cargoOrWeapon, refCount, compare, triggerFunction.data());
}

/// Creates a count trigger for Cargo Trucks with the specified cargo type.  @param refCount = number of Cargo Trucks.
inline Trigger CreateCountTrigger(
  CargoType truckCargoType, CompareMode compare, int refCount,
  std::string_view triggerFunction = "", int playerNum = AllPlayers, bool oneShot = false, bool enabled = true)
{
  return OP2Thunk<0x479110, Trigger FASTCALL(ibool, ibool, int, MapID, CargoType, int, CompareMode, const char*)>(
    enabled, oneShot, playerNum, MapID::CargoTruck, truckCargoType, refCount, compare, triggerFunction.data());
}

/// Used for Last One Standing failure condition and converting Land Rush to Last One Standing (when CC becomes active).
/// @warning Do not use AllPlayers.
inline Trigger CreateOperationalTrigger(
  MapID structureType, CompareMode compare, int refCount, int playerNum,
  std::string_view triggerFunction = "", bool oneShot = true, bool enabled = true)
{
  return OP2Thunk<0x479880, Trigger FASTCALL(ibool, ibool, int, MapID, int, CompareMode, const char*)>(
    enabled, oneShot, playerNum, structureType, refCount, compare, triggerFunction.data());
}

/// Creates a trigger that fires based on the player(s)' total number of vehicles.
inline Trigger CreateVehicleCountTrigger(
  CompareMode compare, int refCount,
  std::string_view triggerFunction = "", int playerNum = AllPlayers, bool oneShot = false, bool enabled = true)
{
  return OP2Thunk<0x479440, Trigger FASTCALL(ibool, ibool, int, int, CompareMode, const char*)>(
    enabled, oneShot, playerNum, refCount, compare, triggerFunction.data());
}

/// Creates a trigger that fires based on the player(s)' total number of structures.
inline Trigger CreateBuildingCountTrigger(
  CompareMode compare, int refCount,
  std::string_view triggerFunction = "", int playerNum = AllPlayers, bool oneShot = false, bool enabled = true)
{
  return OP2Thunk<0x4793A0, Trigger FASTCALL(ibool, ibool, int, int, CompareMode, const char*)>(
    enabled, oneShot, playerNum, refCount, compare, triggerFunction.data());
}

// --------------------------------------------------- Time triggers ---------------------------------------------------

/// Creates a trigger that fires on the specified time interval in ticks.
/// If @ref oneShot = false, fires again on every interval.
inline Trigger CreateTimeTrigger(
  int time, std::string_view triggerFunction = "", bool oneShot = true, bool enabled = true)
{
  return OP2Thunk<0x478D00, Trigger FASTCALL(ibool, ibool, int, const char*)>(
    enabled, oneShot, time, triggerFunction.data());
}

/// Creates a trigger that fires randomly between the specified time interval in ticks.
/// If @ref oneShot = false, fires again on every interval, chosen randomly each time.
inline Trigger CreateTimeTrigger(
  int timeMin, int timeMax, std::string_view triggerFunction = "", bool oneShot = true, bool enabled = true)
{
  return OP2Thunk<0x478DA0, Trigger FASTCALL(ibool, ibool, int, int, const char*)>(
    enabled, oneShot, timeMin, timeMax, triggerFunction.data());
}

// -------------------------------------------------- Special target ---------------------------------------------------

/// Creates a special target that triggers when any sourceUnitType instance moves next to targetUnit for a few seconds.
inline Trigger CreateSpecialTarget(
  const Unit& targetUnit, MapID sourceUnitType,
  std::string_view triggerFunction = "", bool oneShot = true, bool enabled = true)
{
  return OP2Thunk<0x4797A0, Trigger FASTCALL(ibool, ibool, const Unit&, MapID, const char*)>(
    enabled, oneShot, targetUnit, sourceUnitType, triggerFunction.data());
}

/// Gets the unit that triggered the special target.
inline Unit GetSpecialTargetData(const Trigger& specialTargetTrigger)
  { Unit u;  OP2Thunk<0x479860, void FASTCALL(const Trigger&, Unit*)>(specialTargetTrigger, &u);  return u; }

// ---------------------------------------------- Attack/Damage triggers -----------------------------------------------

/// Creates a trigger that fires when the given ScGroup is under attack.
inline Trigger CreateAttackedTrigger(
  const ScGroup& group, std::string_view triggerFunction = "", bool oneShot = true, bool enabled = true)
{
  return OP2Thunk<0x4795A0, Trigger FASTCALL(ibool, ibool, const ScGroup&, const char*)>(
    enabled, oneShot, group, triggerFunction.data());
}

/// Creates a trigger that fires when a percentage of the given ScGroup has been destroyed.
inline Trigger CreateDamagedTrigger(
  const ScGroup& group, TriggerDamage damage,
  std::string_view triggerFunction = "", bool oneShot = true, bool enabled = true)
{
  return OP2Thunk<0x479640, Trigger FASTCALL(ibool, ibool, const ScGroup&, TriggerDamage, const char*)>(
    enabled, oneShot, group, damage, triggerFunction.data());
}

// ------------------------------------------------ Positional triggers ------------------------------------------------

/// Creates a trigger that fires when any unit is at the location.
inline Trigger CreatePointTrigger(
  Location where,
  std::string_view triggerFunction = "", int playerNum = AllPlayers, bool oneShot = false, bool enabled = true)
{
  return OP2Thunk<0x479070, Trigger FASTCALL(ibool, ibool, int, int, int, const char*)>(
    enabled, oneShot, playerNum, where.x, where.y, triggerFunction.data());
}

/// Creates a trigger that fires when any unit enters the area rect.
inline Trigger CreateRectTrigger(
  MapRect area,
  std::string_view triggerFunction = "", int playerNum = AllPlayers, bool oneShot = false, bool enabled = true)
{
  return OP2Thunk<0x478FC0, Trigger FASTCALL(ibool, ibool, int, int, int, int, int, const char*)>(
    enabled, oneShot, playerNum, area.x1, area.x2, area.Width(), area.Height(), triggerFunction.data());
}

/// Creates a rect trigger that filters based on the specified unit type (and cargo type/amount).
inline Trigger CreateEscapeTrigger(
  MapRect area, MapID unitType, int refCount, int cargoType, int cargoAmount,
  std::string_view triggerFunction = "", int playerNum = AllPlayers, bool oneShot = false, bool enabled = true)
{
  return OP2Thunk<0x4796E0, Trigger FASTCALL(ibool, ibool, int, int, int, int, int, int, MapID, int, int, const char*)>(
    enabled, oneShot, playerNum, area.x1, area.y1, area.Width(), area.Height(), refCount, unitType, cargoType,
    cargoAmount, triggerFunction.data());
}

} // Tethys::TethysAPI

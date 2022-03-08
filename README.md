# TethysAPI

TethysAPI is a C++17 library providing access to Outpost 2's public API as well as its internal functions and data, which can be used to create custom missions and mods for the game.

TethysAPI is purely header-only, and does not require linking against any libs to function.

Compared to Outpost2DLL, the public API exposed in TethysAPI has been changed to be more modern and tidier, and with better const-correctness in order to properly support MSVC in C++20 mode. TethysAPI essentially supercedes the Outpost2DLL, HFL/OP2Types, and OP2Internal libraries, although it should not be considered a drop-in replacement for them due to interface changes. TethysAPI can be used alongside these legacy libraries, which is helpful when transitioning an existing project to it.

# Requirements

* Outpost 2 v1.2.7 or newer
* C++17

# Usage

**ATTENTION: TethysAPI is a work in progress. You should expect breaking major interface changes between releases prior to 1.0.**

TethysAPI is intended to be added as a Git submodule to projects. It must reside in a subdirectory called `Tethys` (not "TethysAPI"!)

The public mission API headers are located under the `API` directory. For your convenience, you may use `#include "Tethys/API/API.h"` to include all public API headers, but you are also free to include individual headers. `Tethys/API/Mission.h` documents how to interface a mission with the game.

Headers containing internals, non-exported enums, or non-mission public APIs are located under the other directories:
* `Game` contains internal gameplay interfaces.
* `Network` contains internal networking interfaces.
* `Resource` contains resource management and graphics rendering interfaces.
* `UI` contains graphical user interface and related interfaces.

The public mission APIs are within the `TethysAPI` namespace, while everything else is within the `Tethys` namespace. You may wish to do `using namespace TethysAPI` and/or `using namespace Tethys`.

# Change log

## Version 0.8.2
* Change `Create*Trigger()` functions to take `triggerFunction` as a `std::string_view` instead of a `const char*`. This is a backwards-compatible interface change.
* Rename `OnProcessCommandArgs` to `OnGameCommandArgs`
* Add `Unit::GetMapObject()` template overloads that take a `MapID` as a template arg
* Add `TethysUtil::AutoDestroy<>` template class
* Rename `TethysUtil::Span::Data()`, `Length()`, and `IsEmpty()` to `data(`), `length()`, and `empty()` in preparation to switch to C++20 std::span eventually
* Fix `IDirectDraw` forward declaration in `TApp.h`
* Add some member function definitions to `ScriptDataBlock` and `FuncReference`
* Define vtbl info for `TriggerImpl` and `VictoryConditionImpl`
* Add post-increment operators for unit iterator types
* Remove decrement operator for `GroupIterator`
* Remove `Location` operator+/- overload for scalar integers


## Version 0.8.1
* Rework Enumerator interfaces to more closely match `std::iterator` semantics.
* Remove `GroupEnumerator`; replace with `ScGroup::begin()` and `end()`.
* Rename `OnLoadArgs`, `OnUnloadArgs`, `OnEndArgs` to `OnLoadMissionArgs`, `OnUnloadMissionArgs`, and `UnEndMissionArgs` (1.4.2).
* Add `OnDamageUnitArgs`, `OnSaveGameArgs`, `OnLoadSavedGameArgs` (1.4.2).
* Add `triggeredBy` and `prevTriggeredBy` fields to `OnTriggerUnitArgs` (1.4.2).
* Replace `TethysGame::UnsetBlight()` with `GameMap::UnsetBlight()`.
* Change `GameMap::GetWidth()` to return unpadded width; add `GameMap::GetPaddedWidth()` to return padded width (old `GetWidth()` behavior).
* Add `GameMap::SetBulldozed()`, `CreateScorchMark()`, `CreateCommonRubble()`, `CreateRareRubble()`.
* Add `GameMap::CreateLavaFlow*()`, `FreezeLavaFlow*()` (where * can be SW, S, SE).
* Add `TethysUtil::Span`, an accessor class for array-like data.
* Change various methods in `API/*.h` headers to take `std::string_view` instead of `const char*`.  Other methods are changed to take `TethysUtil::Span` instead of a pointer and array size.
* Change `_Player::GetBuildings()`, `GetVehicles()`, `GetBeacons()`, `GetEntities()` to return a `PlayerUnitIterator` instead of a `Unit`.
* Rename `TethysGame::ForceMorale()` to `ForceMoraleLevel()`.
* Replace references to `Player[i]` within TethysAPI itself with `_Player::GetInstance(i)`.
* Add `GameImpl::ProcessUnits()`.
* Rename `MapObject::ProcessForGameCycle()` and `ProcessTimers()` to `ProcessCommands()` and `ProcessActions()`.
* Add missing `OP2_MO_GET_INSTANCE_IMPL()` to `Projectile` class.
* Add `StreamIO::WriteString()`, `ReadString()`, `WriteValue()`, `ReadValue()`, and `StreamIO` vtbl data.
* Add more constructors to `TethysUtil::Library`.
* Add `OP2Strdup()`.
* Rename `OP2Class<>::DestroyVirtual()` dummy to `_DestroyVirtual()`.
* Add member functions to `TerrainManager`.
* Add `MapImpl::FindFirstUnitInRange()`, `FindNextUnitInRange()`, `DestroyWall()`.
* Add `VictoryConditionImpl`.
* Change `TriggerIml::playerVectorHasFired_` type from `int` to `PlayerBitmask`.
* Change `RecordedTubeWall::cellType` type from `int` to `CellType`.

## Version 0.8
* Initial release.

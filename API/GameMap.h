/**
 ***********************************************************************************************************************
 * @file  GameMap.h
 * @brief Defines the GameMap class which controls the terrain tiles on the map and their properties.
 ***********************************************************************************************************************
 */

#pragma once

#include "Tethys/Common/Memory.h"
#include "Tethys/Game/MapImpl.h"
#include "Tethys/API/Location.h"
#include "Tethys/API/Unit.h"
#include <string_view>

namespace Tethys::TethysAPI {

/// Public game map interface (wraps MapImpl).  Used to access map tile data, set daylight level, and create Blight.
class GameMap : public OP2Class<GameMap> {
public:
  /// Gets the internal map instance.
  static MapImpl* GetImpl() { return MapImpl::GetInstance(); }

  ///@{ Helpers to access and set tile data.
  static int      GetTile(Location           where) { return OP2Thunk<0x476D00, int FASTCALL(Location)>(where); }
  static CellType GetCellType(Location       where) { return CellType(GetImpl()->Tile(where).cellType);         }
  static Unit     GetUnitOnTile(Location     where) { return Unit(GetImpl()->Tile(where).unitIndex);            }
  static bool     GetLavaPresent(Location    where) { return GetImpl()->Tile(where).lava;                       }
  static bool     GetLavaPossible(Location   where) { return GetImpl()->Tile(where).lavaPossible;               }
  static bool     GetMicrobe(Location        where) { return GetImpl()->Tile(where).microbe;                    }
  static bool     GetWallOrBuilding(Location where) { return GetImpl()->Tile(where).wallOrBuilding;             }

  static void InitialSetTile(Location   where, int tileIndex) { return GetImpl()->InitialSetTile(where, tileIndex); }
  static void FASTCALL SetTile(Location where, int tileIndex)
    { return OP2Thunk<0x476D80, &$::SetTile>(where, tileIndex); }
  static void FASTCALL SetLavaPossible(Location where, ibool lavaPossible)
    { return OP2Thunk<0x476F20, &$::SetLavaPossible>(where, lavaPossible); }

  static void SetCellType(Location       where, CellType type) { GetImpl()->Tile(where).cellType  = uint32(type); }
  static void SetUnitOnTile(Location     where, Unit     unit) { GetImpl()->Tile(where).unitIndex = unit.id_;     }
  static void SetLavaPresent(Location    where, bool     lava) { GetImpl()->Tile(where).lava      = lava;         }
  static void SetWallOrBuilding(Location where, bool     wallOrBuilding)
    { GetImpl()->Tile(where).wallOrBuilding = wallOrBuilding; }
  ///@}

  /// Sets the daylight position on the map.
  static void FASTCALL SetInitialLightLevel(int lightPosition)
    { return OP2Thunk<0x476F90, &$::SetInitialLightLevel>(lightPosition); }

  /// Gets the daylight level at the specified map tile coordinates.
  static int GetLightLevel(Location where, bool forSprite = false){ return GetImpl()->GetLightLevel(where, forSprite); }

  /// Damages a wall.  Damage state change is based on RNG.
  static void DamageWall(Location where, int damage) { GetImpl()->DamageWall(where.x, where.y, damage); }

  ///@{ Returns the map dimensions.
  static int GetPaddingWidth() { return GetImpl()->paddingOffsetTileX_; }  ///< 0 for world maps, +32 otherwise.
  static int GetPaddedWidth()  { return GetImpl()->tileWidth_;          }

  static int GetWidth()  { return GetPaddedWidth() / ((GetPaddingWidth() != 0) ? 2 : 1); }
  static int GetHeight() { return GetImpl()->tileHeight_;                                }
  ///@}

  /// Gets the clip rect used by MapRect::Clip().  @note For world maps, x1 = -1, x2 = INT_MAX.
  static MapRect GetClipRect() { return GetImpl()->clipRect_; }

  /// Translates in-game map (x, y) coordinates as displayed on the status bar, to real map coordinates.
  /// Useful with e.g. GetTile(), TethysGame::CreateUnit(), and other functions that take a Location.
  static Location At(int x, int y)
    { return Location(((x - 1 + GetPaddingWidth()) & GetImpl()->tileXMask_), ((y - 1) & (GetHeight() - 1))); }

  /// Finds a clear area to place a unit of the given type nearest to the requested location.
  static Location FindUnitPlacementLocation(Location where, MapID unitType = MapID::CargoTruck)
    { Location loc;  GetImpl()->FindUnitPlacementLocation(unitType, where, &loc);  return loc; }

  /// Loads a new map from the given file.  @warning This may crash if there are already units on the map.
  static bool Load(std::string_view filename) { GetImpl()->Deinit();  return GetImpl()->LoadFromFile(filename.data()); }

  /// Bulldozes a tile.
  static void SetBulldozed(Location where)
    { SetTile(where, GetImpl()->pTerrainManager_->GetBulldozedTileIndex(GetTile(where))); }

  /// Adds a scorch mark to a tile.
  static void CreateScorchMark(Location where)
    { SetTile(where, GetImpl()->pTerrainManager_->GetScorchMarkTileIndex(GetTile(where))); }

  /// Adds common rubble to a tile.
  static void CreateCommonRubble(Location where)
    { SetTile(where, GetImpl()->pTerrainManager_->GetCommonRubbleTileIndex(GetTile(where))); }

  /// Adds rare rubble to a tile.
  static void CreateRareRubble(Location where)
    { SetTile(where, GetImpl()->pTerrainManager_->GetRareRubbleTileIndex(GetTile(where))); }

  ///@{ Helper functions to create lava flow animations on the side of a volcano (default tileset only).
  static void CreateLavaFlowSW(Location where) { SetLavaFlowHelper(where, 0x447, 0x45E, 0x453, 0x469);           }
  static void CreateLavaFlowS(Location  where) { SetTile(where, 0x474);  SetTile(where + Location(0, 1), 0x47E); }
  static void CreateLavaFlowSE(Location where) { SetLavaFlowHelper(where, 0x489, 0x4A0, 0x494, 0x4AB);           }
  static void FreezeLavaFlowSW(Location where) { SetLavaFlowHelper(where, 0x44F, 0x465, 0x45A, 0x470);           }
  static void FreezeLavaFlowS(Location  where) { SetTile(where, 0x47B);  SetTile(where + Location(0, 1), 0x486); }
  static void FreezeLavaFlowSE(Location where) { SetLavaFlowHelper(where, 0x490, 0x4A8, 0x49C, 0x4B2);           }
  ///@}

private:
  static void SetLavaFlowHelper(Location where, int topLeft, int topRight, int bottomLeft, int bottomRight) {
    SetTile(where,                  topLeft);
    SetTile(where + Location(0, 1), topRight);
    SetTile(where + Location(1, 0), bottomLeft);
    SetTile(where + Location(1, 1), bottomRight);
  }

public:
  uint8 field_00;
};

} // Tethys::TethysAPI

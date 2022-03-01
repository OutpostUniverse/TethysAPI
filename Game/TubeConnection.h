
#pragma once

#include "Tethys/Common/Memory.h"

namespace Tethys {

class MapObject;

/// Internal manager for tube connection sets.
class TubeConnectionManager : public OP2Class<TubeConnectionManager> {
public:
  void Init()   { return Thunk<0x42FD20, &$::Init>();   }
  void Deinit() { return Thunk<0x42FCB0, &$::Deinit>(); }

  void RecordBuilding(MapObject* pMapObj) { return Thunk<0x42F150, &$::RecordBuilding>(pMapObj); }
  void RemoveBuilding(MapObject* pMapObj) { return Thunk<0x42F5F0, &$::RemoveBuilding>(pMapObj); }

  int CalculateAverageDIRTDamageProtection(int playerNum)
    { return Thunk<0x42FF90, &$::CalculateAverageDIRTDamageProtection>(playerNum); }

  static ibool FASTCALL GetTubeStatus(int tileX, int tileY)
    { return OP2Thunk<0x42ED90, &$::GetTubeStatus>(tileX, tileY); }

  static TubeConnectionManager* GetInstance() { return OP2Mem<0x547390, TubeConnectionManager*>(); }

  // ** TODO more member functions
  // 0x4300B0

  // ** TODO helper objects at 0x547370 (table size = 0x20 bytes), 0x5473A8 (table size = 0x2400 bytes),
  //         0x5497B0 (table size = 0x1800 bytes), 0x54AFB8 (bit vector size = 0x4000 bytes)?

public:
  // ** TODO member variables
  int    field_00;
};

/// Represents one contiguous set of tube-connected buildings.
class TubeConnection : public OP2Class<TubeConnection> {
public:
  /// Returns % prevented damage * 10000 (e.g. 2500 = 25% damage prevention)
  int CalculateDIRTDamagePrevention() { return Thunk<0x430050, &$::CalculateDIRTDamagePrevention>(); }

  // ** TODO more member functions
  // 0x430030

public:
  // ** TODO member variables
  int field_00;
};

} // Tethys

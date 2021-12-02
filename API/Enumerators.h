/**
 ***********************************************************************************************************************
 * @file  Enumerators.h
 * @brief Defines the unit enumerator and iterator classes used to search for or traverse a list of Units.
 ***********************************************************************************************************************
 */

#pragma once

#include "Tethys/API/Unit.h"
#include "Tethys/API/Location.h"
#include "Tethys/Game/GameImpl.h"
#include <iterator>
#include <list>

namespace Tethys {

namespace TethysImpl {

/// @internal  Base class providing necessary STL std::iterator traits for most unit iterator types.
class UnitIteratorBase {
public:
  using iterator_category = std::forward_iterator_tag;
  using difference_type   = std::ptrdiff_t;
  using value_type        = TethysAPI::Unit;
  using pointer           = TethysAPI::Unit*;
  using reference         = TethysAPI::Unit&;
};

/// @internal  Template CRTP base class providing necessary STL std::iterator functionality for map area iterators.
template <typename Iterator, typename Result = TethysAPI::Unit>
class AreaIteratorBase : public OP2Class<Iterator> {
public:
  using iterator_category = std::forward_iterator_tag;
  using difference_type   = std::ptrdiff_t;
  using value_type        = Result;
  using pointer           = Result*;
  using reference         = Result&;

  Iterator& operator++() { auto& e = static_cast<Iterator&>(*this); result_ = { }; e.GetNext(result_); return e; }
  bool operator==(const AreaIteratorBase& other) const { return result_ == other.result_; }
  bool operator!=(const AreaIteratorBase& other) const { return !(*this == other);        }
  operator bool()      const { return (result_ != Result{}); }
  Result   operator*() const { return  result_;              }

private:
  MapObject* pCurrentUnit_;

  int field_04;
  int field_08;
  int field_0C;
  int field_10;
  int field_14;
  int field_18;
  int field_1C;
  int field_20;
  int field_24;
  int field_28;
  int field_2C;
  int field_30;

  Result result_ = { };
};

} // TethysImpl

namespace TethysAPI {

 /// Iterates over a player's unit list.
class PlayerUnitIterator : public TethysImpl::UnitIteratorBase {
public:
  explicit PlayerUnitIterator(std::list<Unit>* pUnitCache, MapObject* pMo = nullptr)
    : pMo_(pMo), pUnitCache_(pUnitCache) {
    if (pUnitCache == nullptr) {
      pUnitCache_ = new std::list<Unit>;
      freeCache_ = true;
    }
  }
  explicit PlayerUnitIterator(std::list<Unit>* pUnitCache, Unit u) : PlayerUnitIterator(pUnitCache, u.GetMapObject()) { }
  virtual ~PlayerUnitIterator() { if (freeCache_) delete pUnitCache_; }

  PlayerUnitIterator& operator++() { pMo_ = (pMo_ != nullptr) ? pMo_->pPlayerNext_ : nullptr;  return *this; }
  PlayerUnitIterator operator++(int) { auto v = *this; operator++(); return v; }
  bool operator==(const PlayerUnitIterator& other) const { return (pMo_ == other.pMo_); }
  bool operator!=(const PlayerUnitIterator& other) const { return !(*this == other);    }
  operator bool()  const { return (pMo_ != nullptr); }
  Unit& operator*() const { auto& u = pUnitCache_->emplace_back(pMo_); return u; }

protected:
  MapObject* pMo_;
  std::list<Unit>* pUnitCache_;
  bool freeCache_ = false;
};

/// Iterates over a player's units of the specified type.
class FilterPlayerUnitIterator : public PlayerUnitIterator {
public:
  explicit FilterPlayerUnitIterator(std::list<Unit>* pUnitCache, MapObject* pMo = nullptr, MapID type = MapID::Any)
    : FilterPlayerUnitIterator(PlayerUnitIterator(pUnitCache, pMo), type) { }
  explicit FilterPlayerUnitIterator(std::list<Unit>* pUnitCache, Unit u, MapID type = MapID::Any)
    : FilterPlayerUnitIterator(PlayerUnitIterator(pUnitCache, u), type) { }
  explicit FilterPlayerUnitIterator(PlayerUnitIterator src, MapID type = MapID::Any)
    : PlayerUnitIterator(src), type_(type)
      { if (pMo_ && (type_ != MapID::Any) && (pMo_->GetTypeID() != type_)) { ++(*this); } }

  FilterPlayerUnitIterator& operator++() {
    if (type_ == MapID::Any) PlayerUnitIterator::operator++();
    else do PlayerUnitIterator::operator++(); while (pMo_ && (pMo_->GetTypeID() != type_));
    return *this;
  }
  FilterPlayerUnitIterator operator++(int) { auto v = *this; operator++(); return v; }

private:
  MapID type_;
};

} // TethysAPI

namespace TethysImpl {
/// @internal  Base class for player unit enumerators (e.g. PlayerVehicleEnum, PlayerBuildingEnum, PlayerEntityEnum).
class PlayerUnitEnumBase {
public:
  using Iterator = TethysAPI::FilterPlayerUnitIterator;

  explicit PlayerUnitEnumBase(int playerNum, MapID type = MapID::Any) : playerNum_(playerNum), type_(type) { }

  Iterator begin() { return Iterator(&unitCache_, nullptr, type_); }
  Iterator end()   { return Iterator(&unitCache_, nullptr, type_); }

protected:
  int   playerNum_;
  MapID type_;
  /// Holds onto all units that have been returned via iterators produced by this enumerator.
  /// This is required since CLIF only supports reference-valued iterators, so we need to ensure the Unit object outlives the iterator.
  std::list<TethysAPI::Unit> unitCache_;
};
} // TethysImpl

namespace TethysAPI {

/// Enumerates all vehicles (of the specified type) belonging to the specified player.
class PlayerVehicleEnum : public TethysImpl::PlayerUnitEnumBase {
public:
  using PlayerUnitEnumBase::PlayerUnitEnumBase;
  Iterator begin() { return Iterator(&unitCache_, GameImpl::GetInstance()->GetPlayer(playerNum_)->pVehicleList_, type_); }
};

/// Enumerates all buildings (of the specified type) belonging to the specified player.
class PlayerBuildingEnum : public TethysImpl::PlayerUnitEnumBase {
public: 
  using PlayerUnitEnumBase::PlayerUnitEnumBase;
  Iterator begin() { return Iterator(&unitCache_, GameImpl::GetInstance()->GetPlayer(playerNum_)->pBuildingList_, type_); }
};

/// Enumerates all entities (of the specified type) belonging to the specified player.
class PlayerEntityEnum : public TethysImpl::PlayerUnitEnumBase {
public:
  using PlayerUnitEnumBase::PlayerUnitEnumBase;
  explicit PlayerEntityEnum() : PlayerUnitEnumBase(6) { }
  Iterator begin() { return Iterator(&unitCache_, GameImpl::GetInstance()->GetPlayer(playerNum_)->pEntityList_, type_); }
};

/// Enumerates all units within a given distance of a given location.
class InRangeEnumerator : public OP2Class<InRangeEnumerator> {
public:
  class Iterator : public TethysImpl::AreaIteratorBase<Iterator> {
  public:
    using AreaIteratorBase::AreaIteratorBase;
    Iterator(const Location& centerPoint, int maxTileDistance)
      { InternalCtor<0x47A740, const Location&, int>(centerPoint, maxTileDistance);  ++(*this); }
    ibool GetNext(Unit& currentUnit) { return Thunk<0x47A780, &$::GetNext>(currentUnit); };
  };

  InRangeEnumerator(const Location& centerPoint, int maxTileDistance)
    : centerPoint_(centerPoint), maxTileDistance_(maxTileDistance) { }

  Iterator begin() { return Iterator(centerPoint_, maxTileDistance_); }
  Iterator end()   { return Iterator();                               }

private:
  Location centerPoint_;
  int      maxTileDistance_;
};

/// Eenumerates all units within a given rectangle.
class InRectEnumerator : public OP2Class<InRectEnumerator> {
public:
  class Iterator : public TethysImpl::AreaIteratorBase<Iterator> {
  public:
    using AreaIteratorBase::AreaIteratorBase;
    explicit Iterator(const MapRect& rect) { InternalCtor<0x47A610, const MapRect&>(rect);  ++(*this); }
    ibool GetNext(Unit& currentUnit) { return Thunk<0x47A6A0, &$::GetNext>(currentUnit); }
  };

  explicit InRectEnumerator(const MapRect& rect) : rect_(rect) { }

  Iterator begin() { return Iterator(rect_); }
  Iterator end()   { return Iterator();      }

private:
  MapRect rect_;
};

/// Enumerates all units at a given location.
class LocationEnumerator : public OP2Class<LocationEnumerator> {
public:
  class Iterator : public TethysImpl::AreaIteratorBase<Iterator> {
  public:
    using AreaIteratorBase::AreaIteratorBase;
    explicit Iterator(const Location& location) { InternalCtor<0x47A6D0, const Location&>(location);  ++(*this); }
    ibool GetNext(Unit& currentUnit) { return Thunk<0x47A710, &$::GetNext>(currentUnit); }
  };

  explicit LocationEnumerator(const Location& location) : location_(location) { }

  Iterator begin() { return Iterator(location_); }
  Iterator end()   { return Iterator();          }

private:
  Location location_;
};

/// Enumerates all units ordered by their distance to a given location.
class ClosestEnumerator : public OP2Class<ClosestEnumerator> {
public:
  using Result = std::pair<Unit, uint32>;  ///< Mapping of unit : distance

  class Iterator : public TethysImpl::AreaIteratorBase<Iterator, Result> {
  public:
    using AreaIteratorBase::AreaIteratorBase;
    explicit Iterator(const Location& location) { InternalCtor<0x47A7B0, const Location&>(location);  ++(*this); }
    ibool GetNext(Result& result) { return Thunk<0x47A7F0, ibool(Unit&, uint32&)>(result.first, result.second); }
  };

  explicit ClosestEnumerator(const Location& location) : location_(location) { }

  Iterator begin() { return Iterator(location_); }
  Iterator end()   { return Iterator();          }

private:
  Location location_;
};

} // TethysAPI

} // Tethys

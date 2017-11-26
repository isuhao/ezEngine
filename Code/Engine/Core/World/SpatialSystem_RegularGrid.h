#pragma once

#include <Foundation/SimdMath/SimdVec4i.h>
#include <Core/World/SpatialSystem.h>

class EZ_CORE_DLL ezSpatialSystem_RegularGrid : public ezSpatialSystem
{
  EZ_ADD_DYNAMIC_REFLECTION(ezSpatialSystem_RegularGrid, ezSpatialSystem);

public:
  ezSpatialSystem_RegularGrid(ezUInt32 uiCellSize = 128);
  ~ezSpatialSystem_RegularGrid();

  /// \brief Returns the bounding box of the cell associated with the given spatial data. Useful for debug visualizations.
  ezResult GetCellBoxForSpatialData(const ezSpatialDataHandle& hData, ezBoundingBox& out_BoundingBox) const;

  /// \brief Returns bounding boxes of all existing cells.
  void GetAllCellBoxes(ezHybridArray<ezBoundingBox, 16>& out_BoundingBoxes) const;

private:
  // ezSpatialSystem implementation
  virtual void FindObjectsInSphereInternal(const ezBoundingSphere& sphere, QueryCallback callback, QueryStats* pStats = nullptr) const override;
  virtual void FindObjectsInBoxInternal(const ezBoundingBox& box, QueryCallback callback, QueryStats* pStats = nullptr) const override;

  virtual void FindVisibleObjectsInternal(const ezFrustum& frustum, ezDynamicArray<const ezGameObject*>& out_Objects, QueryStats* pStats = nullptr) const override;

  virtual void SpatialDataAdded(ezSpatialData* pData) override;
  virtual void SpatialDataRemoved(ezSpatialData* pData) override;
  virtual void SpatialDataChanged(ezSpatialData* pData, const ezSimdBBoxSphere& oldBounds) override;
  virtual void FixSpatialDataPointer(ezSpatialData* pOldPtr, ezSpatialData* pNewPtr) override;

  ezProxyAllocator m_AlignedAllocator;
  ezSimdVec4i m_iCellSize;
  ezSimdVec4f m_fOverlapSize;
  ezSimdFloat m_fInvCellSize;

  struct Cell
  {
    Cell(ezAllocatorBase* pAllocator, ezAllocatorBase* pAlignedAllocator);

    void AddData(ezSpatialData* pData);
    void RemoveData(ezSpatialData* pData);
    void UpdateData(ezSpatialData* pData);

    ezBoundingBox GetBoundingBox() const;

    ezSimdBBoxSphere m_Bounds;

    ezDynamicArray<ezSimdBSphere> m_BoundingSpheres;
    ezDynamicArray<ezSpatialData*> m_DataPointers;
    ezHashTable<ezSpatialData*, ezUInt32> m_PointerToIndexTable;
  };

  ezHashTable<ezUInt64, Cell*, ezHashHelper<ezUInt64>, ezLocalAllocatorWrapper> m_Cells;

  Cell* m_pOverflowCell;

  template <typename Functor>
  void ForEachCellInBox(const ezSimdBBox& box, Functor func) const;

  Cell* GetOrCreateCell(const ezSimdBBoxSphere& bounds);
};

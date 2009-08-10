#ifndef _MOVEMAPBOXCONTAINER_H_
#define _MOVEMAPBOXCONTAINER_H_

#include "AABSPTree.h"

#include "ShortVector.h"
#include "TreeNode.h"
#include "VMapTools.h"
#include "MoveMapBox.h"
#include "MoveZone.h"

#include <G3D/AABox.h>
#include <G3D/Vector3.h>



namespace VMAP
{
  class MoveZoneContainer;
  class MoveZoneContainerGenerator;

  //========================================================

  class MoveMapDestHandle
  {
  private:
    // Identification of entries, that belong to the same group
    unsigned short iGroupId;
    // index of the MoveMapBox
    unsigned short iIndex;
    unsigned short iDestX; // Destination position in the next map
    unsigned short iDestY; // Destination position in the next map
  public:

    MoveMapDestHandle () { }

    MoveMapDestHandle (unsigned short pGroupId, unsigned short pIndex, unsigned short pDestX, unsigned short pDestY)
    {
      iGroupId = pGroupId;
      iIndex = pIndex;
      iDestX = pDestX;
      iDestY = pDestY;
    }

    inline unsigned short
    getGroupId ()
    {
      return (iGroupId);
    }

    inline unsigned short
    getIndex ()
    {
      return (iIndex);
    }

    inline void
    setIndex (unsigned short pIndex)
    {
      iIndex = pIndex;
    }

    inline unsigned short
    getDestX ()
    {
      return iDestX;
    }

    inline unsigned short
    getDestY ()
    {
      return iDestY;
    }
  };

  //========================================================

  class MoveMapDestHandleArray
  {
  private:
    Array<MoveMapDestHandle> iDestHandleArray;
  public:
    MoveMapDestHandleArray (const MoveLayerConnectionPoints& pMoveLayerConnectionPoints, unsigned short pGroupId, const Table<MoveLayer*, MoveMapBox*>& pLayerMapBoxTable, const Table<MoveMapBox*, unsigned short>& pMapBoxPosTable);

    inline size_t
    size ()
    {
      return (iDestHandleArray.size ());
    }

    inline const MoveMapDestHandle&
    get (int pIndex)
    {
      return iDestHandleArray[pIndex];
    }
  };


  //========================================================

  class MoveMapConnctionHandle
  {
  private:
    // Position for one axis
    unsigned short iPosition;
    // index of the next MoveMapConnctionAxisArray for the next axis or 
    unsigned short iIndex;
  public:

    MoveMapConnctionHandle () { }

    MoveMapConnctionHandle (unsigned short pPosition, unsigned short pIndex)
    {
      iPosition = pPosition;
      iIndex = pIndex;
    }

    inline unsigned short
    getPosition ()
    {
      return (iPosition);
    }

    inline unsigned short
    getIndex ()
    {
      return (iIndex);
    }

    inline void
    setPosition (unsigned short pPos)
    {
      iPosition = pPos;
    }

    inline void
    setIndex (unsigned short pIndex)
    {
      iIndex = pIndex;
    }
  };

  //========================================================

  class MoveMapConnctionAxisArray
  {
  public:
    unsigned short iNElements;
    unsigned short iIndex; // The index where the MoveMapConnctionHandle is located in the global array

    MoveMapConnctionAxisArray () { };

    MoveMapConnctionAxisArray (unsigned short pNElements, unsigned short pIndex)
    {
      iNElements = pNElements;
      iIndex = pIndex;
    }
  };

  //========================================================

  class MoveMapConnectionManager
  {
  private:
    MoveMapDestHandle* iMoveMapDestHandle;
    MoveMapConnctionHandle* iMoveMapConnctionHandle;
    MoveMapConnctionAxisArray *iMoveMapConnctionAxisArray;
    float iGranularity;
  private:
    int
    _findValueInArray (int pIndex, int pSize, unsigned int pSearchVaue) const;
  public:

    MoveMapConnectionManager ()
    {
      iMoveMapDestHandle = NULL;
      iMoveMapConnctionHandle = NULL;
      iMoveMapConnctionAxisArray = NULL;
    }
    MoveMapConnectionManager (MoveMapConnctionHandle* pMoveMapConnctionHandle, MoveMapConnctionAxisArray* pMoveMapConnctionAxisArray, MoveMapDestHandle* pMoveMapDestHandle, float pGranularity);
    //MoveMapConnectionManager(const Array<MoveMapConnctionHandle>& pHandleArray, const Array<MoveMapConnctionAxisArray>& pAxisArray, const Array<MoveMapDestHandle>& pDestHandleArray);
    MoveMapConnectionManager (const Table<Vector3, MoveLayerConnectionPoints>& pPosConnTable, const Table<MoveLayer*, MoveMapBox*>& pLayerMapBoxTable, const Table<MoveMapBox*, unsigned short>& pBoxPositionsTable);
    ~MoveMapConnectionManager ();

    /**
     * return the index or -1 if no connection was found
     */
    int findMapIndex (const Vector3& pPos) const;
  };

  //========================================================

  /**
   * A container represents a tile or an instance
   */

  class MoveMapContainer
  {
  private:
    unsigned char* iMapBase;
    MoveMapBox* iMoveMapBoxArray;
    unsigned int iNMoveMapBoxes;
    TreeNode *iTreeNodes;
    unsigned int iNTreeNodes;
    MoveMapConnectionManager* iMoveMapConnectionManager;
    MoveZoneContainer* iMoveZoneContainer;
    float iGranularity;
    AABox iBounds;
    Vector3 iPosition;
  private:

    inline TreeNode*
    getTreeNodes () const
    {
      return iTreeNodes;
    }

    inline MoveMapBox*
    getMoveMapBoxArray () const
    {
      return iMoveMapBoxArray;
    }

    const unsigned char*
    getMapBaseAdr () const
    {
      return iMapBase;
    }

    void
    setTreeNode (const TreeNode& pTreeNode, int pos)
    {
      iTreeNodes[pos] = pTreeNode;
    }

    void
    setMoveMapBox (const MoveMapBox& pMoveMapBox, int pos)
    {
      iMoveMapBoxArray[pos] = pMoveMapBox;
    }
    void fillContainer (const AABSPTree<MoveMapBox *>::Node& pNode, int &pMoveMapBoxPos, int &pTreeNodePos, Vector3& pLo, Vector3& pHi, Vector3& pFinalLo, Vector3& pFinalHi, Table<MoveMapBox*, unsigned short>& pBoxPositionsTable);
    void
    countMoveMapBoxesAndNode (AABSPTree<MoveMapBox*>::Node& pNode, int& nBoxes, int& nNodes);

    /*void intersect(const G3D::Ray& pRay, float& pMaxDist, bool pStopAtFirstHit, G3D::Vector3& pOutLocation, G3D::Vector3& pOutNormal) const;
    bool intersect(const G3D::Ray& pRay, float& pMaxDist) const;
    template<typename RayCallback>
    void intersectRay(const G3D::Ray& ray, RayCallback& intersectCallback, float& distance, bool pStopAtFirstHit, bool intersectCallbackIsFast = false);*/
    //bool operator==(const MoveMapContainer& pSm2) const;
  public:
    MoveMapContainer ();
    MoveMapContainer (AABSPTree<MoveMapBox*>* pBoxTree, Table<MoveMapBox*, unsigned short>& pBoxPositionsTable);
    ~MoveMapContainer ();

    void fillMoveMapConnectionManagerArray (const MoveLayerConnectionPointsContainer& pMoveLayerConnectionPointsContainer, const Table<MoveLayer*, MoveMapBox*>& pLayerMapBoxTable, const Table<MoveMapBox*, unsigned short>& pBoxPositionsTable);

    void setMoveZonesContainer (AABox gridBounds);

    MoveZoneContainer*
    getMoveZoneContainer()
    {
      return iMoveZoneContainer;
    }

    const AABox&
    getBounds () const
    {
      return iBounds;
    }

    void
    setBounds (const AABox& pBox)
    {
      iBounds = pBox;
    }

    void
    setBounds (const Vector3& pLo, const Vector3& pHi)
    {
      iBounds.set (pLo, pHi);
    }

    const G3D::Vector3&
    getBasePosition () const
    {
      return iPosition;
    }

    bool
    contains (const Vector3& pPos) const
    {
      return iBounds.contains (pPos);
    }

    const MoveMapBox* findMoveMapBox (const Vector3& pPos) const;

    const MoveMapBox&
    getMoveMapBox (unsigned int pIndex)
    {
      return iMoveMapBoxArray[pIndex];
    }

    inline unsigned int
    getMapValue (MoveMapBox* pMoveBox, const Vector3& pPos)
    {
      return (pMoveBox->get (getMapBaseAdr (), pPos.x, pPos.y));
    }

    inline void
    setMapValue (MoveMapBox* pMoveBox, const Vector3& pPos, unsigned int pVal)
    {
      pMoveBox->set (getMapBaseAdr (), pVal, pPos.x, pPos.y);
    }

    inline unsigned int
    getNTreeNodes () const
    {
      return (iNTreeNodes);
    }

    inline unsigned int
    getNMoveMapBoxes () const
    {
      return (iNMoveMapBoxes);
    }

    /**
    Find the index in the MoveMapConnectionManager and return the matching MoveMapBox or 0 if not found
     */
    MoveMapBox* getConnectedMoveBox (const Vector3& pPos) const;
    void saveGridCnx (unsigned int MapId, unsigned int mapx, unsigned int mapy);
    void save (const char* pDir, const Vector3& pInnerLow, const Vector3& pInnerHigh, const Vector3& oriLow, const Vector3& oriHigh, unsigned int MapId, unsigned int x, unsigned int y, bool GenCoords);
    void load (const char* pDir, const char* tname);
  };

}
#endif /*_MOVEMAPBOXCONTAINER_H_*/

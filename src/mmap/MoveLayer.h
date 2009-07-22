#ifndef _MOVELAYER_H
#define _MOVELAYER_H

#include "PositionControlArray.h"
#include "VectorStack.h"
//#include "MoveMapBox.h"


#include <G3D/AABox.h>
#include <G3D/Set.h>
#include <G3D/Vector3.h>

using namespace G3D;

namespace VMAP
{

  class AccessMap
  {
  private:
    PositionControlArray<unsigned char, 2 > * iMovePoints;
    PositionControlArray<unsigned char, 7 > * iNoOfAccessPoints;
    PositionControlArray<unsigned short, 10 > * iMaxYPosInAccessPoints;
    PositionControlArray<unsigned short, 10 > * iMinYPosInAccessPoints;
    int iMinAccessPointsInXY;
    int iMaxAccessPointsInXY;
  public:
    AccessMap (const Vector3& pBasePos, unsigned int pSizeX, unsigned int pSizeY, unsigned int pMaxHightDiff);
    ~AccessMap ();

    void addAccess (const Vector3& pPos);


  };

  //================================================
  class MoveLayer;

  class MoveLayerConnectionPoint
  {
  private:
    MoveLayer* iDestLayer;
    Array<Vector3> iDestPos;
  public:

    MoveLayerConnectionPoint () { };

    MoveLayerConnectionPoint (MoveLayer* pLayer)
    {
      iDestLayer = pLayer;
    }

    void
    addPos (const Vector3& pPos)
    {
      iDestPos.push_back (pPos);
    }

    const Array<Vector3>&
    getDestArray () const
    {
      return (iDestPos);
    }
  };

  //================================================

  class MoveLayerConnectionPoints
  {
  public:
    Vector3 iSourcePos;
    Table<MoveLayer*, MoveLayerConnectionPoint> iDestPoints;

    MoveLayerConnectionPoint&
    createAndGetMoveLayerConnectionPoint (MoveLayer* pDestLayer)
    {
      if (!iDestPoints.containsKey (pDestLayer))
        {
          MoveLayerConnectionPoint cp (pDestLayer);
          iDestPoints.set (pDestLayer, cp);
        }
      return iDestPoints.get (pDestLayer);
    }

    MoveLayerConnectionPoint&
    getMoveLayerConnectionPoint (MoveLayer* pDestLayer) const
    {
      return iDestPoints.get (pDestLayer);
    }

    void
    getKeys (Array<MoveLayer*>& pArray) const
    {
      iDestPoints.getKeys (pArray);
    }

    size_t
    hashCode ()
    {
      return (iSourcePos.y * iSourcePos.x);
    }
  };

  //================================================

  class MoveLayerConnectionPointsContainer
  {
    // 1. key = sourceLayer
    // 2. Key = sourcePos
    int count;
    Table<MoveLayer*, Table<Vector3, MoveLayerConnectionPoints> > iSourceLayerDestLayerConnection;

    /*
    void createMoveLayerConnectionPoints(MoveLayer* pLayer, const Vector3& pPos) {
                MoveLayerConnectionPoints cp;
                cp.iSourcePos = pPos;
                Table<Vector3, MoveLayerConnectionPoints> table;
                table.set(pPos, cp);
                iSourceLayerDestLayerConnection.set(pLayer, table);
            }
            bool hasMoveLayerConnectionPoints(MoveLayer* pLayer, const Vector3& pPos) {
                return (iSourceLayerDestLayerConnection.containsKey(pLayer) && iSourceLayerDestLayerConnection.get(pLayer).containsKey(pPos));
            }
     */

  public:

    MoveLayerConnectionPointsContainer ()
    {
      count = 0;
    }

    MoveLayerConnectionPoints&
    getMoveLayerConnectionPoints (MoveLayer* pLayer, const Vector3& pPos)
    {
      if (!iSourceLayerDestLayerConnection.containsKey (pLayer))
        {
          count++;
          Table<Vector3, MoveLayerConnectionPoints> table;
          iSourceLayerDestLayerConnection.set (pLayer, table);
        }
      Table<Vector3, MoveLayerConnectionPoints>& table = iSourceLayerDestLayerConnection.get (pLayer);
      if (!table.containsKey (pPos))
        {
          MoveLayerConnectionPoints cp;
          cp.iSourcePos = pPos;
          table.set (pPos, cp);
        }
      return (table.get (pPos));
    }

    const Table<Vector3, MoveLayerConnectionPoints>&
    getVectorMoveLayerConnectionPointsTable (MoveLayer* pLayer) const
    {
      return (iSourceLayerDestLayerConnection.get (pLayer));
    }

    void
    getLayerKeys (Array<MoveLayer*>& pLayerArray) const
    {
      iSourceLayerDestLayerConnection.getKeys (pLayerArray);
    }

    void
    getVectorKeys (MoveLayer* pLayer, Array<Vector3>& pPosArray) const
    {
      iSourceLayerDestLayerConnection.get (pLayer).getKeys (pPosArray);
    }
  };



  //================================================
  // the box max hight must match the relative hight diff value within the MoveMapBox
  // if the hight can not be stored in the MoveMapBox we are in trouble
#define MAX_BOX_HIGHT 62
#define SHORT_HEIGHT_FACTOR 32.0f
#define MAP_VALUE_UNDEF 0xffff
#define MAP_VALUE_CANT_REACH 0xfffe


#define FLOAT2SHORTHEIGHT(val) ((short) (val * SHORT_HEIGHT_FACTOR) + 0.5f)
#define SHORTHEIGHT2FLOAT(val) ((float) ((float) val / SHORT_HEIGHT_FACTOR))

  class MoveLayer
  {
  private:
    PositionControlArray<unsigned short, 16 > * iMovePoints;
    // key = destPos, Value = sourcePos
    Table<Vector3, MoveLayer*> iAccessToDifferentLayer;
    VectorStack iTempStoredVectors;

    Vector3 iLow;
    Vector3 iHigh;
    unsigned int iSize;
    Table<Vector3, bool> iTestDoubleTable;
    Array<MoveLayer*> iConnectedLayer;
    //MoveMapBox* iRelatedBox;
  private:
    void processPoint (const Vector3& pPos);

  public:
    MoveLayer (PositionControlArray<unsigned char, 2 > * pSourceMovePoints);
    ~MoveLayer ();

    inline void
    getBounds (AABox& pBox) const
    {
      pBox.set (iLow, iHigh);
    }

    inline const VectorStack&
    getTempStoredVectors ()
    {
      return iTempStoredVectors;
    }

    inline void
    resetTempStoredVectors ()
    {
      /*iTempStoredVectors.clear();*/ iTempStoredVectors = VectorStack ();
    }

    inline unsigned int
    size ()
    {
      return iSize;
    }
    bool processOneStep (const Vector3& pPos, int pHeightDiff, int pVal);

    void processNeighborLayer (Array<Vector3>* pNextPoints);

    void
    getAccessToDifferentLayer (Array<Vector3>& pAccessPoints) const
    {
      iAccessToDifferentLayer.getKeys (pAccessPoints);
    }
    //void addConnectedMoveLayer(MoveLayer* pConnectedLayer) { iConnectedLayer.push_back(pConnectedLayer); }

    const Array<MoveLayer*>&
    getConnectedLayer ()
    {
      return (iConnectedLayer);
    }

    inline PositionControlArray<unsigned short, 16 > *
    getMovePointsArray () const
    {
      return iMovePoints;
    }

    inline unsigned int
    getMovePointValue (const Vector3& pPos)
    {
      return (getMovePointsArray ()->get (Vector3 (pPos.x, getMovePointsArray ()->getBasePos ().y, pPos.z)));
    }

    inline bool
    isConnectionPoint (const Vector3& pPos) const
    {
      return (iAccessToDifferentLayer.containsKey (pPos));
    }

    inline void
    setConnectionMoveLayer (const Vector3& pPos, MoveLayer* pMoveLayer)
    {
      iAccessToDifferentLayer.set (pPos, pMoveLayer);
    }

    inline MoveLayer*
    getConnectionLayer (const Vector3& pPos) const
    {
      return ( iAccessToDifferentLayer.get (pPos));
    }

    inline bool
    containsXZ (const Vector3& pPos) const
    {
      return (iLow.x <= pPos.x && iLow.z <= pPos.z && iHigh.x >= pPos.x && iHigh.z >= pPos.z);
    }

    inline void
    setPosReached (const Vector3& pPos)
    {
      Vector3 layerTestV (pPos.x, iMovePoints->getBasePos ().y, pPos.z);
      short shortHeight = FLOAT2SHORTHEIGHT (pPos.y);
      debugAssert (shortHeight != MAP_VALUE_UNDEF && shortHeight != MAP_VALUE_CANT_REACH);
      iMovePoints->set (shortHeight, layerTestV);
      ++iSize;
      iLow = iLow.min (pPos);
      iHigh = iHigh.max (pPos);
    }

    inline void
    RemoveVal (MoveLayer* pMoveLayer)
    {
      if (iConnectedLayer.size() > 0)
        {
        // is this function usefull ? :
        assert(false);
          for (Table<Vector3, MoveLayer*>::Iterator itr = iAccessToDifferentLayer.begin (); itr != iAccessToDifferentLayer.end ();)
            {
              if (itr->value == pMoveLayer)
                {
                  Table<Vector3, MoveLayer*>::Iterator itr2 = itr;
                  ++itr;
                  iAccessToDifferentLayer.remove (itr2->key);
                }
              else
                ++itr;
            }
        }
    }
    //inline void setMoveMapBox(MoveMapBox* pBox) { iRelatedBox = pBox; }
    //inline MoveMapBox* getMoveMapBox() { return(iRelatedBox); }
  };


  //================================================
}

#endif /* _MOVELAYER_H */

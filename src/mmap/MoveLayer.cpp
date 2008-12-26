#include "MoveLayer.h"

namespace VMAP
{

  //================================================

  AccessMap::AccessMap (const Vector3& pBasePos, unsigned int pSizeX, unsigned int pSizeY, unsigned int pMaxHightDiff)
  {
    iMovePoints = new PositionControlArray<unsigned char, 2 > (pBasePos, pSizeX, pSizeY, 1);
    iNoOfAccessPoints = new PositionControlArray<unsigned char, 7 > (pBasePos, pSizeX, pSizeY, 1);
    iMinYPosInAccessPoints = new PositionControlArray<unsigned short, 10 > (pBasePos, pSizeX, pSizeY, 1);
    iMaxYPosInAccessPoints = new PositionControlArray<unsigned short, 10 > (pBasePos, pSizeX, pSizeY, 1);
  }

  //================================================

  AccessMap::~AccessMap ()
  {
    delete iMovePoints;
    delete iNoOfAccessPoints;
    delete iMinYPosInAccessPoints;
    delete iMaxYPosInAccessPoints;
  }


  //================================================

  void
  AccessMap::addAccess (const Vector3& pPos)
  {
    iMovePoints->set (POS_REACHED, pPos);
    iNoOfAccessPoints->add (1, pPos);
    int minY = iMinYPosInAccessPoints->get (pPos);
    int maxY = iMaxYPosInAccessPoints->get (pPos);
    int vHeight = iMaxYPosInAccessPoints->getHeightValueFromVector (pPos);
    if (minY > vHeight)
      {
        iMinYPosInAccessPoints->set (vHeight, pPos);
      }
    if (maxY < vHeight)
      {
        iMaxYPosInAccessPoints->set (vHeight, pPos);
      }
  }
  //================================================
  //============= MoveLayerConnectionPoints ========
  //================================================


  //================================================
  //================ MoveLayer =====================
  //================================================

  MoveLayer::MoveLayer (PositionControlArray<unsigned char, 2 > * pSourceMovePoints)
  {
    iMovePoints = new PositionControlArray<unsigned short, 16 > (pSourceMovePoints->getBasePos (),
                                                                 pSourceMovePoints->getSizeX (),
                                                                 pSourceMovePoints->getSizeY (), 1);
    //printf("Creating a %u x %u PositionControlArray 16 bits for %f,%f,%f\n",pSourceMovePoints->getSizeX(),pSourceMovePoints->getSizeY(),pSourceMovePoints->getBasePos().x,pSourceMovePoints->getBasePos().y,pSourceMovePoints->getBasePos().z);
    iMovePoints->fill (MAP_VALUE_UNDEF);
    //iSourceMovePoints = pSourceMovePoints;
    float max = inf ();
    iLow = Vector3 (max, max, max);
    iHigh = Vector3 (-max, -max, -max);
    iSize = 0;
    iAccessToDifferentLayer;
  }

  //================================================

  MoveLayer::~MoveLayer ()
  {
    //printf("~MoveLayer() called\n");
    delete iMovePoints;
  }

  //================================================

  bool
  MoveLayer::processOneStep (const Vector3& pPos, int pHeightDiff, int pVal)
  {
    bool result = false;
    Vector3 layerTestV (pPos.x, iMovePoints->getBasePos ().y, pPos.z);
    if (pVal == POS_REACHED)
      {
        unsigned short layerVal = iMovePoints->get (layerTestV);
        Vector3 low = iLow.min (pPos);
        Vector3 high = iHigh.max (pPos);
        if (pHeightDiff == 0 &&
            ((layerVal == MAP_VALUE_UNDEF) || (layerVal == MAP_VALUE_CANT_REACH)) && //BUGFIX added MAP_VALUE_CANT_REACH
            ((high.y - low.y) < MAX_BOX_HIGHT))
          { // same height

            iLow = low;
            iHigh = high; // has to be set here If set in setPosReached only it would be to late
            result = true;
          }
        else
          {
            // this position is not on the same hight level and will be processed after all position for this hight are done
            if (!iTestDoubleTable.containsKey (pPos))
              {
                iTempStoredVectors.push_back (pPos);
                iTestDoubleTable.set (pPos, true);
              }
          }
      }
    else if (pVal == POS_NOT_REACHED && pHeightDiff == 0)
      {
        iMovePoints->set (MAP_VALUE_CANT_REACH, layerTestV);
      }
    return result;
  }


  //================================================

  /*
  Analyse the current points stored in iTempStoredVectors
  These are points that can be reached, but are on a different hight level than the once we analysed in the last pass
  Fill the given point array with all points that are not reached jet and are in the limits 
  of the layer
  All the other points are access points to different layers
   */
  void
  MoveLayer::processNeighborLayer (Array<Vector3>* pNextPoints)
  {
    const VectorStack& pNeighbor = iTempStoredVectors;
    for (unsigned int i = 0; i < pNeighbor.size (); ++i)
      {
        Vector3 pos (pNeighbor[i].x, iMovePoints->getBasePos ().y, pNeighbor[i].z);
        Vector3 low = iLow.min (pNeighbor[i]);
        Vector3 high = iHigh.max (pNeighbor[i]);

        if (iMovePoints->get (pos) == MAP_VALUE_UNDEF && ((high.y - low.y) < MAX_BOX_HIGHT))
          {
            pNextPoints->push_back (pNeighbor[i]);
          }
        else
          {
            iAccessToDifferentLayer.set (pNeighbor[i], NULL); //TODO delete existing layer if any exists !! examine
          }
      }
  }
  //================================================
#if 0

  void
  MoveLayer::createLayer (const Vector3& pPos)
  {
    Array<Array<Vector3> *> pointStackStack;

    iPointStack = new Array<Vector3 > ();
    iPointStack->push_back (pPos);
    pointStackStack.push_back (iPointStack);

    while (pointStackStack.size () > 0)
      {
        iPointStack = pointStackStack.last ();
        pointStackStack.pop_back ();
        iHigherStack = Set<Vector3 > ();
        iLowerStack = Set<Vector3 > ();
        while (iPointStack->size () > 0)
          {
            Vector3 pos = iPointStack->last ();
            iPointStack->pop_back ();
            processPoint (pos);
          }
        if (iHigherStack.size () > 0)
          {
            Array<Vector3>* nextPointStack = new Array<Vector3 > ();
            processNeighborLayer (iHigherStack, nextPointStack, iAccessToHigherLayer);
            if (nextPointStack->size () > 0)
              {
                pointStackStack.push_back (nextPointStack);
              }
            else
              {
                delete nextPointStack;
              }
          }
        if (iLowerStack.size () > 0)
          {
            Array<Vector3>* nextPointStack = new Array<Vector3 > ();
            processNeighborLayer (iLowerStack, nextPointStack, iAccessToLowerLayer);
            if (nextPointStack->size () > 0)
              {
                pointStackStack.push_back (nextPointStack);
              }
            else
              {
                delete nextPointStack;
              }
          }
        delete iPointStack;
      }
  }
#endif
  //================================================
}


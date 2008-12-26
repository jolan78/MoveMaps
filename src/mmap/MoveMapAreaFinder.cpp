#include "MoveMapAreaFinder.h"
#include "MoveLayer.h"

namespace VMAP
{

  //================================================
  // ====== HorizontalSplitter =====================
  //================================================

  /**
  Find the neighbours to the position or the range it fits
  If we found a fitting range return true
   */

  bool
  HorizontalSplitter::getRangeOrNeighbour (int pValue, int& pLowIndex, int& pHighIndex)
  {
    bool found = false;
    int testLow, testHigh;
    testLow = testHigh = 0;
    pLowIndex = pHighIndex = -1;
    for (int i = 0; i < iHeightBoxArray.size (); ++i)
      {
        if (iHeightBoxArray[i].contains (pValue))
          {
            found = true;
            pLowIndex = i;
            pHighIndex = 1;
            break;
          }
        else
          {
            if ((iHeightBoxArray[i].iLow > pValue) && (iHeightBoxArray[i].iLow <= iHeightBoxArray[testHigh].iLow))
              {
                pHighIndex = i;
                testHigh = i;
              }
            if ((iHeightBoxArray[i].iHigh < pValue) && (iHeightBoxArray[i].iHigh >= iHeightBoxArray[testLow].iHigh))
              {
                pLowIndex = i;
                testLow = i;
              }
          }
      }
    return (found);
  }

  //================================================

  void
  HorizontalSplitter::insert (int* pValues, unsigned int pNValues)
  {
    Array<int> blockedRange;
    for (int i = 0; i < iHeightBoxArray.size (); i++)
      {
        blockedRange.push_back (INT_MAX);
      }
    for (unsigned int i = 0; i < pNValues; ++i)
      {
        int low, high;
        int val = pValues[i];
        // debug
        if (pNValues >= 2)
          {
            int xxx = 0;
          }
        bool found = getRangeOrNeighbour (pValues[i], low, high);
        Range oldRange;

        if (!found)
          {
            if (low == -1 && high == -1)
              { //startup
                Range insertRange;
                insertRange.iLow = pValues[i];
                insertRange.iHigh = pValues[i];
                iHeightBoxArray.insert (0, insertRange);
                blockedRange.insert (0, INT_MAX);
                low = high = 0;
              }
            else if (low == high)
              {
                oldRange = iHeightBoxArray[low];
                if (iHeightBoxArray[high].iLow > pValues[i])
                  {
                    iHeightBoxArray[high].iLow = pValues[i];
                  }
                else
                  {
                    iHeightBoxArray[high].iHigh = pValues[i];
                  }
              }
            else
              {
                int rangeDiffLow = low != -1 ? iHeightBoxArray[low].getDist () : -1;
                int rangeDiffHigh = high != -1 ? iHeightBoxArray[high].getDist () : -1;
                if ((rangeDiffHigh == -1) || ((rangeDiffLow < rangeDiffHigh) && (rangeDiffLow != -1)))
                  {
                    oldRange = iHeightBoxArray[low];
                    iHeightBoxArray[low].iHigh = pValues[i];
                    high = low; // now we created a fitting range and low == high
                  }
                else
                  {
                    oldRange = iHeightBoxArray[high];
                    iHeightBoxArray[high].iLow = pValues[i];
                    low = high; // now we created a fitting range and low == high
                  }
              }
          }
        else
          {
            oldRange = iHeightBoxArray[low]; // low == high
          }
        if (blockedRange[low] != INT_MAX)
          {
            int lastInsert = blockedRange[low];
            debugAssert (iHeightBoxArray[low].getDist () >= 1);
            Range insertRange;
            int splitPos = iHeightBoxArray[low].iLow + iHeightBoxArray[low].getDist () / 2;
            if (splitPos < oldRange.iHigh && oldRange.iHigh < iHeightBoxArray[low].iHigh) splitPos = oldRange.iHigh;
            if (splitPos >= oldRange.iLow && oldRange.iLow > iHeightBoxArray[low].iLow) splitPos = oldRange.iLow - 1;
            if (splitPos <= lastInsert && splitPos <= pValues[i])
              {
                splitPos = min (pValues[i], lastInsert);
              }
            if (splitPos > lastInsert && splitPos > pValues[i])
              {
                splitPos = max (pValues[i], lastInsert) - 1;
              }
            insertRange.iLow = iHeightBoxArray[low].iLow;
            insertRange.iHigh = splitPos;
            iHeightBoxArray[low].iLow = splitPos + 1;
            iHeightBoxArray.insert (low, insertRange);
            blockedRange.insert (low, true);
          }
        else
          {
            blockedRange[low] = pValues[i]; // This range is blocked now
          }
      }
  }
  //================================================
  // ====== MoveMapAreaFinder ======================
  //================================================
  /**
  Compress the layer on the height axis
   */
#if 0
  extern Array<AABox>gBoxArray;

  void
  MoveMapAreaFinder::findAreas (PositionControlArray<unsigned char, 2 > * pMovePositionArray)
  {
    Vector3 basePos ((int) pMovePositionArray->getBounds ().low ().x, (int) pMovePositionArray->getBounds ().low ().y, (int) pMovePositionArray->getBounds ().low ().z);
    for (unsigned int x = 0; x < pMovePositionArray->getSizeX (); ++x)
      {
        for (unsigned int y = 0; y < pMovePositionArray->getSizeY (); ++y)
          {
            for (unsigned int z = 0; z < pMovePositionArray->getHeightArraySize (); ++z)
              {
                Vector3 v = basePos + Vector3 (x, z, y);
                if (pMovePositionArray->directGet (x, y, z) == POS_REACHED)
                  {
                    MoveLayer* moveLayer = new MoveLayer (pMovePositionArray);
                    moveLayer->createLayer (v);
                    AABox b;
                    if (moveLayer->size () > 0)
                      {
                        moveLayer->getBounds (b);
                        if (b.low () != b.high ())
                          {
                            gBoxArray.push_back (b);
                          }
                      }

                    const Array<Vector3>& higher = moveLayer->getAccessPointsHigh ();
                    const Array<Vector3>& lower = moveLayer->getAccessPointsLow ();
                    for (int i = 0; i < higher.size (); ++i)
                      {
                        moveLayer = new MoveLayer (pMovePositionArray);
                        moveLayer->createLayer (higher[i]);
                        if (moveLayer->size () > 0)
                          {
                            moveLayer->getBounds (b);
                            if (b.low () != b.high ())
                              {
                                gBoxArray.push_back (b);
                              }
                          }
                      }
                    for (int i = 0; i < lower.size (); ++i)
                      {
                        MoveLayer* moveLayer = new MoveLayer (pMovePositionArray);
                        moveLayer->createLayer (lower[i]);
                        if (moveLayer->size () > 0)
                          {
                            moveLayer->getBounds (b);
                            if (b.low () != b.high ())
                              {
                                gBoxArray.push_back (b);
                              }
                          }
                      }
                  }
              }
          }
      }


    /*
    unsigned int zArrayNo = pPositionControlArray->getHeightArraySize();
    int* positionZArray = new int[zArrayNo];
    HorizontalSplitter horizontalSplitter;

    for(unsigned int y=0; y<pPositionControlArray->getSizeY(); ++y) {
        for(unsigned int x=0; x<pPositionControlArray->getSizeX(); ++x) {
            int nElements;
            pPositionControlArray->fillPositionZArray(x, y, 1, positionZArray, nElements);
            horizontalSplitter.insert(positionZArray, nElements);
        }
    }
    debugPrintf(horizontalSplitter.toString().c_str());

    delete [] positionZArray;
     */
  }
#endif
  //================================================

}


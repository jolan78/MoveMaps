#include <time.h>

#include "VectorMoveMap.h"
#include "StepDirections.h"

namespace VMAP
{
  int globalLOSCals = 0;
  //extern Array<Vector3> gVectorArray;

  //================================================

  VectorMoveMap::VectorMoveMap (int pMapId, int x, int y, GridMapManager* pGridMapManager, IVMapManager* pVMapManager)
  {
    iGridMapManager = pGridMapManager;
    iVMapManager = pVMapManager;
    iMapId = pMapId;
    iGridX = x;
    iGridY = y;
  }

  //================================================

  VectorMoveMap::~VectorMoveMap () {
 }

  //================================================

  Vector3
  VectorMoveMap::convertPositionToInternalRep (float x, float y, float z) const
  {
    float pos[3];
    pos[0] = y;
    pos[1] = z;
    pos[2] = x;
    double full = 64.0 * 533.33333333;
    double mid = full / 2.0;
    pos[0] = full - (pos[0] + mid);
    pos[2] = full - (pos[2] + mid);

    return (Vector3 (pos));
  }

  //=========================================================

  Vector3
  VectorMoveMap::convertPositionToMangosRep (float x, float y, float z) const
  {
    float pos[3];
    pos[0] = z;
    pos[1] = x;
    pos[2] = y;
    double full = 64.0 * 533.33333333;
    double mid = full / 2.0;
    pos[0] = -((mid + pos[0]) - full);
    pos[1] = -((mid + pos[1]) - full);

    return (Vector3 (pos));
  }
  //================================================

  void
  blockPosWithDir (int pDir, const Vector3& pPos, CalcHelper& pCalcHelper)
  {
    if (pDir == D_W)
      { //W
        pCalcHelper.iPositionControlArray[D_SW]->set (1, pPos);
        pCalcHelper.iPositionControlArray[D_NW]->set (1, pPos);
      }
    if (pDir == D_E)
      { //E
        pCalcHelper.iPositionControlArray[D_NE]->set (1, pPos);
        pCalcHelper.iPositionControlArray[D_SE]->set (1, pPos);
      }
    if (pDir == D_N)
      { //N
        pCalcHelper.iPositionControlArray[D_NE]->set (1, pPos);
        pCalcHelper.iPositionControlArray[D_NW]->set (1, pPos);
      }
    if (pDir == D_S)
      { //
        pCalcHelper.iPositionControlArray[D_SE]->set (1, pPos);
        pCalcHelper.iPositionControlArray[D_SW]->set (1, pPos);
      }
  }

  //================================================

  float
  VectorMoveMap::getHeight (const Vector3& pPos, CalcHelper& pCalcHelper)
  {
    float height = -10000;

    int value = pCalcHelper.iHightStoreMap != NULL ? pCalcHelper.iHightStoreMap->get (pPos) : 0;
    if (pCalcHelper.iHightStoreMap == NULL || value == pCalcHelper.iHightStoreMap->getMaxValue ())
      {
        Vector3 mPos = convertPositionToMangosRep (pPos.x, pPos.y, pPos.z);
        height = iGridMapManager->getHeight (mPos.x, mPos.y);
        float vmapheight = iVMapManager->getHeight (iMapId, mPos.x, mPos.y, mPos.z); // look from a bit higher pos to find the floor

        //++globalLOSCals;
        // if the land map did not find the height or if we are already under the surface and vmap found a height
        // or if the distance of the vmap height is less the land height distance
        if ((vmapheight > -10000) && (mPos.z < height || height <= -1000 || vmapheight > height))
          {
            height = vmapheight;
          }
        if (pCalcHelper.iHightStoreMap != NULL)
          {
            m_floatValue = height;
            pCalcHelper.iHightStoreMap->set (m_uint32Value, pPos);
            ++pCalcHelper.calcHeight;
          }
      }
    else
      {
        ++pCalcHelper.getHeight;
        m_uint32Value = value;
        height = m_floatValue;
      }
    return height;
  }

  //================================================

  void
  VectorMoveMap::processMoves (const Vector3& pPos, const Vector3& pTestPos1, const Vector3& pTestPos2, CalcHelper& pCalcHelper)
  {

    float hight1 = getHeight (pTestPos1, pCalcHelper);
    float hight2 = getHeight (pTestPos2, pCalcHelper);
    Vector3 testPos1 = Vector3 (pTestPos1.x, hight1, pTestPos1.z);
    Vector3 testPos2 = Vector3 (pTestPos2.x, hight2, pTestPos2.z);

    bool reachPos1 = false;
    bool reachPos2 = false;
    if (abs (pPos.y - hight1) < getMaxZDiff ())
      {
        reachPos1 = true;
      }// else { gVectorArray.push_back(testPos1); gVectorArray.push_back(pPos); }
    if (abs (pPos.y - hight2) < getMaxZDiff ())
      {
        reachPos2 = true;
      }// else { gVectorArray.push_back(testPos2); gVectorArray.push_back(pPos); }
    if (reachPos1 && reachPos2)
      {
        unsigned int dir1 = pCalcHelper.dir1;
        unsigned int dir2 = pCalcHelper.dir2;

        //BUGFIXED added POS_CAN_REACH check
        if (((pCalcHelper.iSourcePointPositionControl->get (testPos1) == POS_UNDEF) || (pCalcHelper.iSourcePointPositionControl->get (testPos1) == POS_CAN_REACH)) &&
            ((pCalcHelper.iSourcePointPositionControl->get (testPos2) == POS_UNDEF) || (pCalcHelper.iSourcePointPositionControl->get (testPos2) == POS_CAN_REACH)) &&
            !pCalcHelper.iPositionControlArray[dir1]->get (testPos1) &&
            !pCalcHelper.iPositionControlArray[dir2]->get (testPos2))
          {

            Vector3 mPos = convertPositionToMangosRep (pPos.x, pPos.y, pPos.z);
            Vector3 mtPos1 = convertPositionToMangosRep (testPos1.x, testPos1.y, testPos1.z);
            Vector3 mtPos2 = convertPositionToMangosRep (testPos2.x, testPos2.y, testPos2.z);

            reachPos1 = iVMapManager->isInLineOfSight (iMapId, mPos.x, mPos.y, mPos.z + getZDiff () / 2, mtPos1.x, mtPos1.y, mtPos1.z + getZDiff ());
            ++globalLOSCals;

            reachPos2 = iVMapManager->isInLineOfSight (iMapId, mPos.x, mPos.y, mPos.z + getZDiff () / 2, mtPos2.x, mtPos2.y, mtPos2.z + getZDiff ());
            ++globalLOSCals;

            blockPosWithDir (dir1, testPos1, pCalcHelper);
            blockPosWithDir (dir2, testPos2, pCalcHelper);

            int posHeightVal = pCalcHelper.iSourcePointPositionControl->getHeightValueFromVector (pPos);
            int testPos2HeightVal = pCalcHelper.iSourcePointPositionControl->getHeightValueFromVector (testPos2);
            int testPos1HeightVal = pCalcHelper.iSourcePointPositionControl->getHeightValueFromVector (testPos1);
            int hdiff1 = testPos1HeightVal - posHeightVal;
            int hdiff2 = testPos2HeightVal - posHeightVal;

            if (reachPos1)
              {
                reachPos1 = iVMapManager->isInLineOfSight (iMapId, mtPos1.x, mtPos1.y, mtPos1.z + 0.5f, mtPos1.x, mtPos1.y, mtPos1.z + 2.5);
                if (reachPos1)
                  {
                    bool okToReach = pCalcHelper.iCurrentMoveLayer->processOneStep (testPos1, hdiff1, POS_REACHED);
                    if (okToReach)
                      {
                        pCalcHelper.iTestVectors->push_back (testPos1);
                      }
                    else
                      {
                        // +++ debug
                        if (pCalcHelper.iSourcePointPositionControl->get (testPos1) != POS_UNDEF)
                          {
                            int sss = 0;
                          }
                      }
                    pCalcHelper.iSourcePointPositionControl->set (POS_CAN_REACH, testPos1);

                  }
              }
            if (reachPos2)
              {
                reachPos2 = iVMapManager->isInLineOfSight (iMapId, mtPos2.x, mtPos2.y, mtPos2.z + 0.5f, mtPos2.x, mtPos2.y, mtPos2.z + 2.5);
                if (reachPos2)
                  {
                    bool okToReach = pCalcHelper.iCurrentMoveLayer->processOneStep (testPos2, hdiff2, POS_REACHED);
                    if (okToReach)
                      {
                        pCalcHelper.iTestVectors->push_back (testPos2);
                      }
                    else
                      {
                        // +++ debug
                        if (pCalcHelper.iSourcePointPositionControl->get (testPos2) != POS_UNDEF)
                          {
                            int sss = 0;
                          }
                      }
                    pCalcHelper.iSourcePointPositionControl->set (POS_CAN_REACH, testPos2);
                  }
              }

#if 0
            if (reachPos1 && reachPos2)
              {
                Vector3 resultPos1 = testPos1;
                resultPos1.x -= pCalcHelper.iLow.x;
                resultPos1.z -= pCalcHelper.iLow.z;
                Vector3 resultPos2 = testPos2;
                ;
                resultPos2.x -= pCalcHelper.iLow.x;
                resultPos2.z -= pCalcHelper.iLow.z;
                Vector3 startPos = Vector3 (pPos.x - pCalcHelper.iLow.x, pPos.y, pPos.z - pCalcHelper.iLow.z);
                Triangle t = Triangle (startPos, resultPos1, resultPos2);
                pCalcHelper.iTree->insert (t);
              }
#endif
            pCalcHelper.testPos1Ok = reachPos1;
            pCalcHelper.testPos2Ok = reachPos2;
            pCalcHelper.chainBroken = false;
            if (!reachPos1)
              {
                pCalcHelper.iSourcePointPositionControl->set (POS_UNDEF, testPos1); //BUGFIX was POS_NOT_REACHED
                pCalcHelper.iCurrentMoveLayer->processOneStep (testPos1, hdiff1, POS_NOT_REACHED);
              }
            if (!reachPos2)
              {
                pCalcHelper.iSourcePointPositionControl->set (POS_UNDEF, testPos2); //BUGFIX was POS_NOT_REACHED
                pCalcHelper.iCurrentMoveLayer->processOneStep (testPos2, hdiff2, POS_NOT_REACHED);
              }
          }
        else
          {
            pCalcHelper.chainBroken = true;
            pCalcHelper.testPos1Ok = false;
            pCalcHelper.testPos2Ok = false;
          }
      }
    else
      {
        pCalcHelper.chainBroken = true;
        pCalcHelper.testPos1Ok = false;
        pCalcHelper.testPos2Ok = false;
        //gVectorArray.push_back(testPos1);
        //gVectorArray.push_back(testPos2);
        //if(abs(pPos.y - hight1) < -getMaxZDiff() && (abs(hight1) < MAX_HIGHT)) { gVectorArray.push_back(pPos); }
        //if(abs(pPos.y - hight2) < -getMaxZDiff() && (abs(hight2) < MAX_HIGHT)) { gVectorArray.push_back(pPos); }
      }
  }

  //================================================

  void
  VectorMoveMap::processMoves (const Vector3& pPos, CalcHelper& pCalcHelper, const AABox& pBounds)
  {
    Vector3 testPos1;
    Vector3 testPos2;
    float zdiff = getZDiff ();
    float res = getResolution ();
    if (pCalcHelper.iSourcePointPositionControl->get (pPos) == POS_CAN_REACH || pCalcHelper.iSourcePointPositionControl->get (pPos) == POS_UNDEF)
      {
        pCalcHelper.iSourcePointPositionControl->set (POS_REACHED, pPos);
        pCalcHelper.iCurrentMoveLayer->setPosReached (pPos);

        pCalcHelper.chainBroken = true;
        pCalcHelper.testPos1Ok = false;
        pCalcHelper.testPos2Ok = false;

        testPos1 = pPos + Vector3 (res, zdiff, 0);
        testPos2 = pPos + Vector3 (res, zdiff, -res);
        pCalcHelper.dir1 = D_E;
        pCalcHelper.dir2 = D_SE;
        if (pBounds.contains (testPos1) && pBounds.contains (testPos2))
          processMoves (pPos, testPos1, testPos2, pCalcHelper);

        testPos1 = testPos2;
        testPos2 = pPos + Vector3 (0, zdiff, -res);
        pCalcHelper.dir1 = pCalcHelper.dir2;
        pCalcHelper.dir2 = D_S;
        if ((pCalcHelper.testPos2Ok || pCalcHelper.chainBroken) && pBounds.contains (testPos1) && pBounds.contains (testPos2))
          processMoves (pPos, testPos1, testPos2, pCalcHelper);

        testPos1 = testPos2;
        testPos2 = pPos + Vector3 (-res, zdiff, -res);
        pCalcHelper.dir1 = pCalcHelper.dir2;
        pCalcHelper.dir2 = D_SW;
        if ((pCalcHelper.testPos2Ok || pCalcHelper.chainBroken) && pBounds.contains (testPos1) && pBounds.contains (testPos2))
          processMoves (pPos, testPos1, testPos2, pCalcHelper);

        testPos1 = testPos2;
        testPos2 = pPos + Vector3 (-res, zdiff, 0);
        pCalcHelper.dir1 = pCalcHelper.dir2;
        pCalcHelper.dir2 = D_W;
        if ((pCalcHelper.testPos2Ok || pCalcHelper.chainBroken) && pBounds.contains (testPos1) && pBounds.contains (testPos2))
          processMoves (pPos, testPos1, testPos2, pCalcHelper);

        testPos1 = testPos2;
        testPos2 = pPos + Vector3 (-res, zdiff, res);
        pCalcHelper.dir1 = pCalcHelper.dir2;
        pCalcHelper.dir2 = D_NW;
        if ((pCalcHelper.testPos2Ok || pCalcHelper.chainBroken) && pBounds.contains (testPos1) && pBounds.contains (testPos2))
          processMoves (pPos, testPos1, testPos2, pCalcHelper);

        testPos1 = testPos2;
        testPos2 = pPos + Vector3 (0, zdiff, res);
        pCalcHelper.dir1 = pCalcHelper.dir2;
        pCalcHelper.dir2 = D_N;
        if ((pCalcHelper.testPos2Ok || pCalcHelper.chainBroken) && pBounds.contains (testPos1) && pBounds.contains (testPos2))
          processMoves (pPos, testPos1, testPos2, pCalcHelper);

        testPos1 = testPos2;
        testPos2 = pPos + Vector3 (res, zdiff, res);
        pCalcHelper.dir1 = pCalcHelper.dir2;
        pCalcHelper.dir2 = D_NE;
        if ((pCalcHelper.testPos2Ok || pCalcHelper.chainBroken) && pBounds.contains (testPos1) && pBounds.contains (testPos2))
          processMoves (pPos, testPos1, testPos2, pCalcHelper);

        testPos1 = testPos2;
        testPos2 = pPos + Vector3 (res, zdiff, 0);
        pCalcHelper.dir1 = pCalcHelper.dir2;
        pCalcHelper.dir2 = D_E;
        if ((pCalcHelper.testPos2Ok || pCalcHelper.chainBroken) && pBounds.contains (testPos1) && pBounds.contains (testPos2))
          processMoves (pPos, testPos1, testPos2, pCalcHelper);
      }
  }

  //================================================

  void
  VectorMoveMap::processMovesWithHeightOnly (const Vector3& pPos, const Vector3& pTestPos1, const Vector3& pTestPos2, CalcHelper& pCalcHelper)
  {
    Vector3 mtPos1 = convertPositionToMangosRep (pTestPos1.x, pTestPos1.y, pTestPos1.z);
    Vector3 mtPos2 = convertPositionToMangosRep (pTestPos2.x, pTestPos2.y, pTestPos2.z);

    float height1 = iGridMapManager->getHeight (mtPos1.x, mtPos1.y);
    float height2 = iGridMapManager->getHeight (mtPos2.x, mtPos2.y);
    /*        //debug
    if((pTestPos1.x == 15435.000 && pTestPos1.z == 15093.000) || (pTestPos2.x == 15435.000 && pTestPos2.z == 15093.000)) {
    int vxv = 0;
    }
     */
    if ((abs (height1) < MAX_HIGHT && abs (height2) < MAX_HIGHT) && (abs (pPos.y - height1) < getMaxZDiff ()) && (abs (pPos.y - height2) < getMaxZDiff ()))
      {
        Vector3 testPos1 = Vector3 (pTestPos1.x, height1, pTestPos1.z);
        Vector3 testPos2 = Vector3 (pTestPos2.x, height2, pTestPos2.z);
        mtPos1.z = height1;
        mtPos2.z = height2;

        unsigned int dir1 = pCalcHelper.dir1;
        unsigned int dir2 = pCalcHelper.dir2;

        if (!pCalcHelper.iSourcePointPositionControl->get (testPos1) &&
            !pCalcHelper.iSourcePointPositionControl->get (testPos2) &&
            !pCalcHelper.iPositionControlArray[dir1]->get (testPos1) &&
            !pCalcHelper.iPositionControlArray[dir2]->get (testPos2))
          {

            pCalcHelper.iTestVectors->push_back (testPos1);
            debugAssert (pCalcHelper.iTestVectors->size () < 5999999);

            blockPosWithDir (dir1, testPos1, pCalcHelper);
            blockPosWithDir (dir2, testPos2, pCalcHelper);
            Vector3 resultPos1 = testPos1;
            resultPos1.x -= pCalcHelper.iLow.x;
            resultPos1.z -= pCalcHelper.iLow.z;
            Vector3 resultPos2 = testPos2;
            ;
            resultPos2.x -= pCalcHelper.iLow.x;
            resultPos2.z -= pCalcHelper.iLow.z;
            Vector3 startPos = Vector3 (pPos.x - pCalcHelper.iLow.x, pPos.y, pPos.z - pCalcHelper.iLow.z);
            Triangle t = Triangle (startPos, resultPos1, resultPos2);
          }
      }
  }

  //================================================

  void
  VectorMoveMap::processMovesWithHeightOnly (const Vector3& pPos, CalcHelper& pCalcHelper, const AABox& pBounds)
  {
    Vector3 testPos1;
    Vector3 testPos2;
    float zdiff = 0;
    float res = getResolution ();

    if (!pCalcHelper.iSourcePointPositionControl->get (pPos))
      {
        pCalcHelper.iSourcePointPositionControl->set (1, pPos);
        testPos1 = pPos + Vector3 (res, zdiff, 0);
        testPos2 = pPos + Vector3 (res, zdiff, -res);
        pCalcHelper.dir1 = D_E;
        pCalcHelper.dir2 = D_SE;
        if (pBounds.contains (testPos1) && pBounds.contains (testPos2))
          processMovesWithHeightOnly (pPos, testPos1, testPos2, pCalcHelper);

        testPos1 = testPos2;
        testPos2 = pPos + Vector3 (0, zdiff, -res);
        pCalcHelper.dir1 = pCalcHelper.dir2;
        pCalcHelper.dir2 = D_S;
        if (pBounds.contains (testPos1) && pBounds.contains (testPos2))
          processMovesWithHeightOnly (pPos, testPos1, testPos2, pCalcHelper);

        testPos1 = testPos2;
        testPos2 = pPos + Vector3 (-res, zdiff, -res);
        pCalcHelper.dir1 = pCalcHelper.dir2;
        pCalcHelper.dir2 = D_SW;
        if (pBounds.contains (testPos1) && pBounds.contains (testPos2))
          processMovesWithHeightOnly (pPos, testPos1, testPos2, pCalcHelper);

        testPos1 = testPos2;
        testPos2 = pPos + Vector3 (-res, zdiff, 0);
        pCalcHelper.dir1 = pCalcHelper.dir2;
        pCalcHelper.dir2 = D_W;
        if (pBounds.contains (testPos1) && pBounds.contains (testPos2))
          processMovesWithHeightOnly (pPos, testPos1, testPos2, pCalcHelper);

        testPos1 = testPos2;
        testPos2 = pPos + Vector3 (-res, zdiff, res);
        pCalcHelper.dir1 = pCalcHelper.dir2;
        pCalcHelper.dir2 = D_NW;
        if (pBounds.contains (testPos1) && pBounds.contains (testPos2))
          processMovesWithHeightOnly (pPos, testPos1, testPos2, pCalcHelper);

        testPos1 = testPos2;
        testPos2 = pPos + Vector3 (0, zdiff, res);
        pCalcHelper.dir1 = pCalcHelper.dir2;
        pCalcHelper.dir2 = D_N;
        if (pBounds.contains (testPos1) && pBounds.contains (testPos2))
          processMovesWithHeightOnly (pPos, testPos1, testPos2, pCalcHelper);

        testPos1 = testPos2;
        testPos2 = pPos + Vector3 (res, zdiff, res);
        pCalcHelper.dir1 = pCalcHelper.dir2;
        pCalcHelper.dir2 = D_NE;
        if (pBounds.contains (testPos1) && pBounds.contains (testPos2))
          processMovesWithHeightOnly (pPos, testPos1, testPos2, pCalcHelper);

        testPos1 = testPos2;
        testPos2 = pPos + Vector3 (res, zdiff, 0);
        pCalcHelper.dir1 = pCalcHelper.dir2;
        pCalcHelper.dir2 = D_E;
        if (pBounds.contains (testPos1) && pBounds.contains (testPos2))
          processMovesWithHeightOnly (pPos, testPos1, testPos2, pCalcHelper);
      }
  }

  //================================================

  /**
  Find a place on the ground map, that is not covered by a model
   */

  void
  VectorMoveMap::findStartVector (const Vector3& pLow, const Vector3& pHigh, Array<Vector3>& pStartArray, unsigned int MapId)
  {
    Array<StartPosition> posArary;
    if (readStartPositions (getStartPosFileName (), posArary))
      {
        AABox box (pLow, pHigh);
        //Vector3 mLow, mHigh;
        //mLow = convertPositionToMangosRep(pLow.x,pLow.y,pLow.z);
        //mHigh = convertPositionToMangosRep(pHigh.x,pHigh.y,pHigh.z);
        //printf("Box: x:%f, y:%f, z:%f to x:%f, y:%f, z:%f\n",mHigh.x,mHigh.y,mLow.z,mLow.x,mLow.y,mHigh.z);
        for (int i = 0; i < posArary.size (); ++i)
          {
            StartPosition sp = posArary[i];
            //mLow = convertPositionToMangosRep(sp.iPos.x,sp.iPos.y,sp.iPos.z);
            //printf("Pos: x:%f, y:%f, z:%f\n",mLow.x,mLow.y,mLow.z);
            if (sp.iMapId == MapId && box.contains (sp.iPos))
              {
                pStartArray.push_back (sp.iPos);
                //printf("Added start vector x:%f, y:%f, z:%f\n",sp.iPos.x, sp.iPos.y, sp.iPos.z);
              }
          }
      }

  }

  //================================================

  /**
  remove all points that lye in the box
   */

  void
  VectorMoveMap::removeAccessablePointsFromGrid (const AABox& modelBounds, CalcHelper& pCalcHelper)
  {
    const Vector3& low = Vector3 ((int) modelBounds.low ().x, (int) modelBounds.low ().y, (int) modelBounds.low ().z);
    const Vector3& high = Vector3 ((int) modelBounds.high ().x, (int) modelBounds.high ().y, (int) modelBounds.high ().z);

    float res = getResolution ();
    for (float x = low.x; x < high.x; x += res)
      {
        for (float y = low.y; y < high.y; y += res)
          {
            for (float z = low.z; z < high.z; z += res)
              {
                Vector3 pos = Vector3 (x, y, z);
                for (int i = 0; i < D_SIZE; ++i)
                  {
                    pCalcHelper.iPositionControlArray[i]->set (0, pos);
                  }
                pCalcHelper.iSourcePointPositionControl->set (0, pos);
              }
          }
      }
  }

  //================================================

  void
  VectorMoveMap::fillTestVectorsWithSurroundingPoints (const AABox& surroundBox, CalcHelper& pCalcHelper)
  {
    float res = getResolution ();
    const Vector3& low = Vector3 ((int) surroundBox.low ().x, (int) surroundBox.low ().y, (int) surroundBox.low ().z);
    const Vector3& high = Vector3 ((int) surroundBox.high ().x, (int) surroundBox.high ().y, (int) surroundBox.high ().z);

    for (float x = low.x; x < high.x; x += res)
      {
        for (float z = low.z; z < high.z; z += res)
          {
            Vector3 pos1 = Vector3 (x, low.y, z);
            Vector3 pos2 = Vector3 (x, high.y, z);
            if (pCalcHelper.iSourcePointPositionControl->get (pos1))
              {
                float height = getHeight (Vector3 (0, 1, 0) + pos1, pCalcHelper);
                pos1.y = height;
                pCalcHelper.iTestVectors->push_back (pos1);
              }
            if (pCalcHelper.iSourcePointPositionControl->get (pos2))
              {
                float height = getHeight (Vector3 (0, 1, 0) + pos2, pCalcHelper);
                pos2.y = height;
                pCalcHelper.iTestVectors->push_back (pos2);
              }
          }
      }

    for (float y = low.y; y < high.y; y += res)
      {
        for (float z = low.z; z < high.z - 1; z += res)
          {
            Vector3 pos1 = Vector3 (low.x, y, z);
            Vector3 pos2 = Vector3 (high.x, y, z);
            if (pCalcHelper.iSourcePointPositionControl->get (pos1))
              {
                float height = getHeight (Vector3 (0, 1, 0) + pos1, pCalcHelper);
                pos1.y = height;
                pCalcHelper.iTestVectors->push_back (pos1);
              }
            if (pCalcHelper.iSourcePointPositionControl->get (pos2))
              {
                float height = getHeight (Vector3 (0, 1, 0) + pos2, pCalcHelper);
                pos2.y = height;
                pCalcHelper.iTestVectors->push_back (pos2);
              }
          }
      }

    for (float x = low.x; x < high.x; x += res)
      {
        for (float y = low.y; y < high.y; y += res)
          {
            Vector3 pos1 = Vector3 (x, y, low.z);
            Vector3 pos2 = Vector3 (x, y, high.z);
            if (pCalcHelper.iSourcePointPositionControl->get (pos1))
              {
                float height = getHeight (Vector3 (0, 1, 0) + pos1, pCalcHelper);
                pos1.y = height;
                pCalcHelper.iTestVectors->push_back (pos1);
              }
            if (pCalcHelper.iSourcePointPositionControl->get (pos2))
              {
                float height = getHeight (Vector3 (0, 1, 0) + pos2, pCalcHelper);
                pos2.y = height;
                pCalcHelper.iTestVectors->push_back (pos2);
              }
          }
      }
  }

  //================================================

  void
  VectorMoveMap::getSerroundingBoxWithinBox (const SubModel& pSubModel, const AABox& pOuterBox, AABox& pResultInnerBox)
  {
    Vector3 low = Vector3 (inf (), inf (), inf ());
    Vector3 high = Vector3 (-inf (), -inf (), -inf ());

    for (unsigned int i = 0; i < pSubModel.getNTriangles (); ++i)
      {
        TriangleBox t = pSubModel.getTriangle (i);
        for (int j = 0; j < 3; ++j)
          {
            Vector3 pos = t.vertex (j).getVector3 () + pSubModel.getBasePosition ();
            if (pOuterBox.contains (pos))
              {
                low = low.min (pos);
                high = high.max (pos);
              }
          }
      }
    if (low.x >= inf ()) low.x = pOuterBox.low ().x;
    if (low.y >= inf ()) low.y = pOuterBox.low ().y;
    if (low.z >= inf ()) low.z = pOuterBox.low ().z;
    if (high.x <= -inf ()) high.x = pOuterBox.high ().x;
    if (high.y <= -inf ()) high.y = pOuterBox.high ().y;
    if (high.z <= -inf ()) high.z = pOuterBox.high ().z;
    pResultInnerBox.set (low, high);
  }


  //================================================

  float
  VectorMoveMap::getMaxHeightForBox (const AABox& pBounds)
  {
    // low and high are swaped
    Vector3 mtHigh = convertPositionToMangosRep (pBounds.low ().x, pBounds.low ().y, pBounds.low ().z);
    Vector3 mtLow = convertPositionToMangosRep (pBounds.high ().x, pBounds.high ().y, pBounds.high ().z);

    float hight = -10000;
    for (float x = mtLow.x; x < mtHigh.x; x += 1)
      {
        for (float y = mtLow.y; y < mtHigh.y; y += 1)
          {
            float thight = iGridMapManager->getHeight (x, y);
            if (thight > hight)
              {
                hight = thight;
              }
          }
      }
    return hight;
  }

  //================================================

  void
  VectorMoveMap::deletePointAffectedByModels (const SubModel& pSubModel, const AABox& pBounds, CalcHelper& pCalcHelper)
  {
    AABox modelBounds = pSubModel.getAABoxBounds ();
    Vector3 mlow = modelBounds.low ().max (pBounds.low ());
    Vector3 mhigh = modelBounds.high ().min (pBounds.high ());
    if (mlow.x < mhigh.x && mlow.y < mhigh.y && mlow.z < mhigh.z)
      {
        modelBounds.set (mlow, mhigh);
        fillTestVectorsWithSurroundingPoints (modelBounds, pCalcHelper);
        removeAccessablePointsFromGrid (modelBounds, pCalcHelper);
      }
  }


  //================================================

  bool
  VectorMoveMap::readStartPositions (const std::string& pFilename, Array<StartPosition>& pPosArray)
  {
    bool result = false;
    FILE* f = fopen (pFilename.c_str (), "rb");
    if (f)
      {
        int bufferSize = 500;
        char buffer[500];
        while (fgets (buffer, bufferSize - 1, f))
          {
            //int tilex, tiley, map;
            float y, x, z;
            sscanf (buffer, "%f, %f, %f", &x, &y, &z); //"%d, %d, %d, %f, %f, %f", &map, &tilex, &tiley, &x, &y, &z);
            Vector3 pos = convertPositionToInternalRep (x, y, z);
            StartPosition sp (iMapId, pos);
            pPosArray.push_back (sp);
          }
        fclose (f);
        result = true;
      }
    return result;
  }

  //================================================

  void
  VectorMoveMap::processOneMoveLayer (const Vector3& pStartPos, CalcHelper& pCalcHelper, MoveLayer* pPrevMoveLayer)
  {
    Array<VectorStack *> pointStackStack;

    bool connected = false;
    VectorStack* testVectors = new VectorStack ();
    testVectors->push_back (pStartPos);
    pointStackStack.push_back (testVectors);

    MoveLayer* moveLayer = new MoveLayer (pCalcHelper.iSourcePointPositionControl);
    pCalcHelper.iCurrentMoveLayer = moveLayer;
    //printf("Process One move layer %f,%f,%f\n",pStartPos.x,pStartPos.y,pStartPos.z);
    while (pointStackStack.size () > 0)
      {
        pCalcHelper.iTestVectors = pointStackStack.last ();
        moveLayer->resetTempStoredVectors ();
        while (pCalcHelper.iTestVectors->size () > 0)
          {
            Vector3 pos = pCalcHelper.iTestVectors->last ();
            pCalcHelper.iTestVectors->pop_back ();
            if (pPrevMoveLayer && pPrevMoveLayer->isConnectionPoint (pos))
              {
                // store the current layer to be the destination for that point
                // The destination layer might be empty at the end, but we do not know that jet
                pPrevMoveLayer->setConnectionMoveLayer (pos, moveLayer);
                connected = true;
              }
            processMoves (pos, pCalcHelper, pCalcHelper.iBounds);
          }
        pointStackStack.pop_back ();
        // check, if there are other reachable points for that layer, located on a different hight level
        // It is important to process the points on the same hight first, therefore we need to temp
        // store the others and process them now
        if (moveLayer->getTempStoredVectors ().size () > 0)
          {
            VectorStack* nextPointStack = new VectorStack ();
            moveLayer->processNeighborLayer (nextPointStack);
            if (nextPointStack->size () > 0)
              {
                pointStackStack.push_back (nextPointStack);
              }
            else
              {
                delete nextPointStack;
              }
          }
        delete pCalcHelper.iTestVectors;
      }
    if (moveLayer->size () > 0)
      {
        AABox b;
        moveLayer->getBounds (b);
        if (b.low () != b.high ())
          {
            pCalcHelper.iMoveLayers.push_back (moveLayer);
            //printf("%u\n",pCalcHelper.iMoveLayers.size());
            // add layer to the debug output
            //                gBoxArray.push_back(b);

            Array<Vector3> differenLayerPos;
            moveLayer->getAccessToDifferentLayer (differenLayerPos);
            if (moveLayer)
              {
                for (int i = 0; i < differenLayerPos.size (); ++i)
                  {
                    if (pCalcHelper.iSourcePointPositionControl->get (differenLayerPos[i]) == POS_CAN_REACH || pCalcHelper.iSourcePointPositionControl->get (differenLayerPos[i]) == POS_UNDEF)
                      {
                        processOneMoveLayer (differenLayerPos[i], pCalcHelper, moveLayer);
                      }
                  }
                differenLayerPos.clear ();
              }
          }
        else
          {
            if (connected)
              pPrevMoveLayer->RemoveVal (moveLayer);
            
            delete moveLayer;
          }
      }
    else
      {
        delete moveLayer;
      }

  }


  //================================================

  /**
  Fill the tree with trianges of reachable floor areas
   */
  bool
  VectorMoveMap::processMapGrid (CalcHelper& pCalcHelper)
  { //, ModelContainer* pMc) { //, unsigned int MapId) {

    globalLOSCals = 0;
    pCalcHelper.heightNotFound = 0;
    Array<Vector3> startArray;

    time_t sec1;
    time (&sec1);
    findStartVector (pCalcHelper.iInnerLow, pCalcHelper.iInnerHigh, startArray, iMapId);
    printf ("Found %u starting point(s)\n", startArray.size ());
    if (startArray.size () == 0)
      return false;

    for (int si = 0; si < startArray.size (); ++si)
      {
        Vector3 startVector = startArray[si];
        Vector3 normStart ((int) startVector.x, (int) startVector.y, (int) startVector.z); // x<->z
        if (pCalcHelper.iSourcePointPositionControl->get (normStart) == POS_UNDEF)
          {
            processOneMoveLayer (normStart, pCalcHelper);
          }
      }
    //processMapGridProcessModels(pMc, pCalcHelper.iBounds, pCalcHelper.iTree, pCalcHelper);

    time_t sec2;
    time (&sec2);
    long diff = sec2 - sec1;
    printf ("Time = %d, calls=%d, gains in height=%d\n", diff, globalLOSCals, pCalcHelper.calcHeight);
    /*
    for(unsigned int j=0; j<pMc->getNSubModel(); ++j) {
    const SubModel& sm = pMc->getSubModel(j);
    const AABox& testbox =  sm.getAABoxBounds();
    debugPrintf("nvect =%d, %f, %f, %f, -  %f, %f, %f\n",sm.getNTriangles(), testbox.low().x, testbox.low().y,testbox.low().z, testbox.high().x,testbox.high().y,testbox.high().z);
    }
     */
    return true;
  }

  //================================================

  extern std::string startCoordsPath;

  std::string
  VectorMoveMap::getStartPosFileName ()
  {
    char coordname[15];
    sprintf (coordname, "%03u_%02u_%02u.txt", iMapId, iGridX, iGridY);
    return (std::string (startCoordsPath + "/" + (std::string)coordname));
  }
}

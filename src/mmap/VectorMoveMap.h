#ifndef _VECTORMOVEMAP_H
#define _VECTORMOVEMAP_H

#include "AABSPTree.h"

#include <G3D/Vector3.h>
#include <G3D/Triangle.h>
#include <G3D/Array.h>
#include <G3D/Table.h>

#include "VMapManager.h"
#include "GridMapManager.h"
#include "PositionControlArray.h"
#include "MoveLayer.h"
#include "VectorStack.h"


using namespace G3D;

namespace VMAP
{
  class CalcHelper;
  
  class StartPosition
  {
  public:
    int iMapId;
    Vector3 iPos;

    StartPosition () { };

    StartPosition (int pMapId, const Vector3& pPos)
    {
      iMapId = pMapId, iPos = pPos;
    }
  };

  //================================================
#define VMOVEMAP_RESOLUTION 534
#define VMOVEMAP_STEP_SIZE (SIZE_OF_GRIDS / VMOVEMAP_RESOLUTION)

  class VectorMoveMap
  {
  private:
    //		Vector3 iVectorMap[VMOVEMAP_RESOLUTION][VMOVEMAP_RESOLUTION];
    //		bool iCanReach[VMOVEMAP_RESOLUTION][VMOVEMAP_RESOLUTION];
    GridMapManager* iGridMapManager;
    IVMapManager* iVMapManager;
    int iMapId, iGridX, iGridY;
  private:
    std::string getLineDescString (const Vector3& pPos1, const Vector3& pPos2);
    void processMovesWithHeightOnly (const Vector3& pPos, CalcHelper& pCalcHelper, const AABox& pBounds);
    void fillTestVectorsWithSurroundingPoints (const AABox& surroundBox, CalcHelper& pCalcHelper);
    void removeAccessablePointsFromGrid (const AABox& modelBounds, CalcHelper& pCalcHelper);
    void getSerroundingBoxWithinBox (const SubModel& pSubModel, const AABox& pOuterBox, AABox& pResultInnerBox);
    void processMovesWithHeightOnly (const Vector3& pPos, const Vector3& pTestPos1, const Vector3& pTestPos2, CalcHelper& pCalcHelper);
    void processMoves (const Vector3& pPos, const Vector3& pTestPos1, const Vector3& pTestPos2, CalcHelper& pCalcHelper);
    void processMoves (const Vector3& pPos, CalcHelper& pCalcHelper, const AABox& pBounds);

    float
    getZDiff ()
    {
      return 1.8f;
    }

    float
    getMaxZDiff ()
    {
      return (1.8f * getResolution ());
    }
    void findStartVector (const Vector3& pLow, const Vector3& pHight, Array<Vector3>& pStartArray, unsigned int MapId);
    float getHeight (const Vector3& pPos, CalcHelper& pCalcHelper);
    void getWayDir (const Vector3& pCurrentPos, const Vector3& pDestPos, unsigned int& pDir, unsigned int& pRevDir);
    float getMaxHeightForBox (const AABox& pBounds);
    void deletePointAffectedByModels (const SubModel& pSubModel, const AABox& pBounds, CalcHelper& pCalcHelper);

    Vector3 convertPositionToInternalRep (float x, float y, float z) const;
    Vector3 convertPositionToMangosRep (float x, float y, float z) const;
    bool readStartPositions (const std::string& pFilename, Array<StartPosition>& pPosArray);
    std::string getStartPosFileName (); // { return(std::string("..\\..\\StartCoords\\" + )); }
    void processOneMoveLayer (const Vector3& pStartPos, CalcHelper& pCalcHelper, MoveLayer* pPrevMoveLayer = NULL);

    void
    SetFloatValue (float value)
    {
      m_floatValue = value;
    }

    float
    GetFloatValue ()
    {
      return m_floatValue;
    }

    void
    SetUInt32Value (unsigned int value)
    {
      m_uint32Value = value;
    }

    unsigned int
    GetUInt32Value ()
    {
      return m_uint32Value;
    }

  public:

    static float
    getResolution ()
    {
      return 1;
    }
    VectorMoveMap (int pMapId, int x, int y, GridMapManager* pGridMapManager, IVMapManager* pVMapManager);
    ~VectorMoveMap ();

    void processMoves (const Vector3& pPos, Array<Vector3>& pTestVectors);
    bool
    processMapGrid (CalcHelper& pCalcHelper); //, ModelContainer* pMc); //, unsigned int MapId);

  protected:

    union
    {
      unsigned int m_uint32Value;
      float m_floatValue;
    };
  };

  //================================================
  //================================================

  class CalcHelper
  {
  public:
    Vector3 iLow;
    Vector3 iHigh;
    Vector3 iInnerLow;
    Vector3 iInnerHigh;
    VectorStack* iTestVectors;
    MoveLayer* iCurrentMoveLayer;
    Array<MoveLayer* > iMoveLayers;
    PositionControlArray<unsigned char, 1 > ** iPositionControlArray;
    PositionControlArray<unsigned char, 2 > * iSourcePointPositionControl;
    PositionControlArray<unsigned int, 32 > * iHightStoreMap;

    /*
    PositionControlArray<unsigned char,7>* iNoOfAccessPointsArray;
    PositionControlArray<unsigned short,10>* iMaxYPosInAccessPoints;
    PositionControlArray<unsigned short,10>* iMinYPosInAccessPoints;
     */
    AABSPTree<Triangle>* iTree;
    AABox iBounds;

    bool testPos1Ok;
    bool testPos2Ok;
    bool chainBroken;
    int calcHeight;
    int getHeight;
    int heightNotFound;
    //Vector3 prevTestPos2;
    int dir1;
    int dir2;
  };
  //================================================
}

#endif

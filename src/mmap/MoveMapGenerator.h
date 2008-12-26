#ifndef _MOVEMAPGENERATOR_H
#define _MOVEMAPGENERATOR_H

#include "VMapManager.h"
#include "GridMapManager.h"
#include "PositionControlArray.h"
#include "VectorMoveMap.h"
#include "MoveMapBoxContainer.h"

using namespace G3D;

namespace VMAP
{
  class MoveMapGenerator
  {
  private:
    VMapManager* iVMapManager;
    std::string iDataDirPath;
    std::string iMMapDirPath;
  private:
    void claculateMoveLayerConnections (const Array<MoveLayer*>& pMoveLayerArray);

  public:
    MoveMapGenerator (const std::string& pDataDirPath, const std::string& pMMapDirPath);
    ~MoveMapGenerator ();

    void fillMCList (Array<ModelContainer*>& McList, int mapId, int x, int y, Vector3& low, Vector3& high, Vector3& oriLow, Vector3& oriHigh);

    void fillNeighborPositionsInLayer (Vector3 pSourcePos, MoveLayer* pSourceMoveLayer, MoveLayer* pDestMoveLayer, MoveLayerConnectionPointsContainer& pConnPointsCont);
    MoveMapContainer* calculateMoveLayerConnections (const Array<MoveLayer*>& pMoveLayerArray);
    bool calculate (VectorMoveMap* pVectorMoveMap, const Vector3& pInnerLow, const Vector3& pInnerHigh, const Vector3& pLow, const Vector3& pHigh, const Vector3& oriLow, const Vector3& oriHigh, AABSPTree<Triangle>* pResultTree, unsigned int MapId, unsigned int x, unsigned int y, bool GenCoords);
    bool generateMoveMaps (int mapId, int x, int y, bool GenCoords);

  };
}

#endif
